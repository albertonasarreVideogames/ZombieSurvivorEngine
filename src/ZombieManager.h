
#pragma once

#ifndef ZOMBIE_MANAGER_H
#define ZOMBIE_MANAGER_H 1

#include "EntityZombie.h"

class ZombieManager
{
	static const Vector4 ShootedZombieColor;
	static const Vector4 DayZombieColor;
	static const Vector4 NightZombieColor;
	static constexpr float DefaultZombieSpeed = 0.8f;
	static constexpr float MaxZombieSpeed = 1.2f;
	static constexpr float ZombieSpawnProbability = 0.01f;

	struct CollidedZombie
	{
		EntityZombie* Zombie;
		Vector3 Collision;
		float Distance;
	};

	std::vector<EntityZombie*> m_Zombies;
	std::vector<CollidedZombie> m_CollidedZombies;
	Entity* m_Parent;
	bool m_WasNight;
	bool m_WasDay;
	bool m_SpawnZombies;
	size_t m_RayIdx;

	// Zombie spawning stuff
	//RectF m_MapRect;
	struct SpawningZone
	{
		Vector3 Min;
		Vector3 Max;
		Vector3 Diff;
	};
	//SpawningZone m_Spawns[4];
	std::vector<std::array<SpawningZone, 4>> m_Spawns;

	Vector4 m_CurrentZombieColor;
	float m_ZombieSpeed;
	std::function<void(EntityZombie*)> m_OnZombieKilled;


public:
	static ZombieManager* instance;
	
	ZombieManager()
		:m_Parent(nullptr)
		,m_WasNight(false)
		,m_WasDay(false)
		,m_SpawnZombies(true)
		,m_RayIdx((size_t)-1)
		,m_CurrentZombieColor(NightZombieColor)
		,m_ZombieSpeed(DefaultZombieSpeed)
	{

	}

	void Clear()
	{
		for (auto* zombie : m_Zombies)
			m_Parent->DestroyChild(zombie->GetName(), false);
		m_Zombies.clear();
	}

	void SpawnZombies(bool spawn)
	{
		m_SpawnZombies = spawn;
	}

	void Init(Entity* parent)
	{
		m_Parent = parent;
		assert(m_Parent != nullptr);

		m_Spawns.clear();
		
		auto& maps = Game::instance->Maps;
		for (auto* map : maps)
		{
			const auto tileSize = map->GetTileSize();
			for (auto* tile : map->GetTiles())
			{
				for (auto* child : tile->GetChilds())
				{
					const auto findIT = child->GetName().find("gravestoneCross");
					if (findIT == std::string::npos)
						continue;

					m_Spawns.push_back({});
					auto& spawnRegion = m_Spawns.back();
					auto tPos = tile->GetWorldPosition();

					auto* spawn = &spawnRegion[0];
					spawn->Min.set(tPos.x, 0.f, tPos.z);				// LeftTop
					spawn->Max.set(tPos.x, 0.f, tPos.z + tileSize);	// LeftBottom
					spawn->Diff = spawn->Max - spawn->Min;

					spawn = &spawnRegion[1];
					spawn->Min.set(tPos.x, 0.f, tPos.z + tileSize);	// LeftBottom
					spawn->Max.set(tPos.x + tileSize, 0.f, tPos.z + tileSize);	// RightBottom
					spawn->Diff = spawn->Max - spawn->Min;

					spawn = &spawnRegion[2];
					spawn->Min.set(tPos.x + tileSize, 0.f, tPos.z + tileSize);	// RightBottom
					spawn->Max.set(tPos.x + tileSize, 0.f, tPos.z);				// RightTop
					spawn->Diff = spawn->Max - spawn->Min;

					spawn = &spawnRegion[3];
					spawn->Min.set(tPos.x + tileSize, 0.f, tPos.z);					// RightTop
					spawn->Max.set(tPos.x, 0.f, tPos.z);							// LeftTop
					spawn->Diff = spawn->Max - spawn->Min;
				}
			}
		}
		

		m_Zombies.reserve(512);
		m_CollidedZombies.reserve(128);

		m_OnZombieKilled = std::bind(&ZombieManager::KillZombie, this, std::placeholders::_1);

		srand(SDL_GetTicks());
	}

	void Update(double elapsed_seconds)
	{
		bool isDay = Game::instance->IsDay();
		bool isNight = Game::instance->IsNight();
		auto* game = Game::instance;

		// Has changed to day
		if (!m_WasDay && isDay)
		{
			// Zombies are petrified
			m_CurrentZombieColor = DayZombieColor;
			m_ZombieSpeed = 0.f;
			std::cout << "New day, zombies petrified" << std::endl;
		}
		else if (!m_WasNight && isNight) // has changed to night
		{
			m_CurrentZombieColor = NightZombieColor;
			m_ZombieSpeed = DefaultZombieSpeed;
			std::cout << "New night, zombies are free" << std::endl;
		}
		else if (isDay) // During day
		{
			if (game->CurrentDayTime > 0.75f) // Zombies start moving
			{
				float speed = (1.f - game->CurrentDayTime)*(float)elapsed_seconds;
				m_ZombieSpeed = lerp(m_ZombieSpeed, DefaultZombieSpeed, speed);
				m_CurrentZombieColor = lerp(m_CurrentZombieColor, NightZombieColor, speed);
			}
		}
		else if (isNight && !game->pause) // During night and not pause
		{
			const float rngMax = 1.f / float(RAND_MAX);
			auto rng = float(rand()) * rngMax;
			if (rng <= ZombieSpawnProbability && m_SpawnZombies)
			{
				// Spawn selection
				float spawnGraveRng = float(rand()) * rngMax * float(m_Spawns.size());
				const auto spawnGraveIdx = (size_t)floorf(spawnGraveRng);

				float spawnZoneRng = float(rand()) * rngMax * 4.f;
				const auto spawnZoneIdx = (size_t)floorf(spawnZoneRng);
				auto& spawn = m_Spawns[spawnGraveIdx][spawnZoneIdx];
				//auto& spawn = m_Spawns[spawnIdx];
				rng = float(rand()) * rngMax;
				// kind of lerp
				const auto position = spawn.Min + (spawn.Diff * rng);
				//const auto position = lerp(spawn.Min, spawn.Max, rng);
				std::cout << "Spawning zombie " << rng << "-" << position.x << ", " << position.z << std::endl;
				auto* zombie = AddEntity<EntityZombie>(m_Parent, "zombie");
				m_Zombies.push_back(zombie);
				zombie->SetPosition(position);
				zombie->m_ColorMult = &m_CurrentZombieColor;
				zombie->m_Speed = &m_ZombieSpeed;
				zombie->m_OnZombieKilled = &m_OnZombieKilled;
				static bool infoSet = false;
				if (!infoSet)
				{
					StaticObjectRenderer::instance->SetZombieInfo(zombie->mesh, 
						Shader::Get("data/shaders/basic-zombie_instanced.vs", "data/shaders/flat-zombie.fs"));
					infoSet = true;
				}
			}
		}

		const auto curFrame = Game::instance->CurrentFrame;
		for (auto* zombie : m_Zombies)
		{
			if (zombie->m_WasShooted && zombie->m_ShootedFrame < curFrame)
			{
				zombie->m_ColorMult = &m_CurrentZombieColor;
				zombie->m_ShootedFrame = (size_t)-1;
			}
		}

		m_WasDay = isDay;
		m_WasNight = isNight;
	}

	bool Shoot(const Vector3& origin, const Vector3& direction)
	{
		m_CollidedZombies.clear();
		for (auto* zombie : m_Zombies)
		{
			Vector3 collision;
			Vector3 normal;

			bool shotCol = zombie->mesh->testRayCollision(zombie->GetWorldMatrix(),
				origin, direction, collision, normal);

			if (shotCol)
			{
				CollidedZombie colzombie;
				colzombie.Zombie = zombie;
				colzombie.Collision = collision;
				colzombie.Distance = collision.distance(origin);
				m_CollidedZombies.push_back(colzombie);
			}
		}

		if (m_CollidedZombies.empty())
			return false;

		std::sort(m_CollidedZombies.begin(), m_CollidedZombies.end(),
			[](CollidedZombie& left, CollidedZombie& right)
		{
			return left.Distance < right.Distance;
		});
		const auto& colzombie = m_CollidedZombies.front();
		auto* zombie = colzombie.Zombie;

		zombie->Shooted();
		zombie->m_ColorMult = (Vector4*)&ShootedZombieColor;
		CollidedRay ray{ origin, colzombie.Collision };
		if (m_RayIdx >= Game::instance->Rays.size())
		{
			Game::instance->Rays.push_back(ray);
			m_RayIdx = Game::instance->Rays.size() - 1;
		}
		else
		{
			Game::instance->Rays[m_RayIdx] = ray;
		}
		return true;
	}

	void KillZombie(EntityZombie* zombie)
	{
		const auto findIT = std::find(m_Zombies.begin(), m_Zombies.end(), zombie);
		if (findIT != m_Zombies.end())
		{
			m_Zombies.erase(findIT);
			m_Parent->DestroyChild(zombie->m_Name, false);
		}
	}
};

const Vector4 ZombieManager::ShootedZombieColor	= Vector4(1.f, 0.f, 0.f, 1.f);
const Vector4 ZombieManager::DayZombieColor		= Vector4(0.2f, 0.2f, 0.2f, 1.f);
const Vector4 ZombieManager::NightZombieColor	= Vector4(1.f, 1.f, 1.f, 1.f);

#endif /* ZOMBIE_MANAGER_H */