#pragma once

#ifndef ENTITY_ZOMBIE_H
#define ENTITY_ZOMBIE_H 1

#include "includes.h"
#include "EntityMesh.h"
#include "game.h"
#include "StaticObjectRenderer.h"
#include "EntityWall.h"

constexpr int ZombieMaxHealth = 100;
constexpr int WeaponDamage = 10;
constexpr float ZombieDetectRange = 5.f;
constexpr float ZombieSightRange = 10.f;

enum class EZombieState
{
	ROAMING,
	CHASING,
};

constexpr float ZombieChangeDirProb = 0.001f;

class EntityZombie : public EntityMesh
{
	float* m_Speed;
	Matrix44 rotation_matrix;
	//float target_angle;
	float prev_target_angle;
	float temp_x;	
	int m_Health;
	Vector4* m_ColorMult;
	bool m_WasShooted;
	EZombieState m_State;
	Vector3 m_TargetDirection;
	Vector3 m_CurrentDirection;
	size_t m_ShootedFrame;
	std::function<void(EntityZombie*)>* m_OnZombieKilled;
	friend class ZombieManager;
	
public:
	EntityZombie()
		:EntityMesh(Mesh::Get("data/FinalMeshes/zombie.obj"),
			Shader::Get("data/shaders/basic-zombie.vs", "data/shaders/flat-zombie.fs"),
			"data/FinalMeshes/zombie.mtl")
		,m_Speed(nullptr)
		//,target_angle(0.f)
		,temp_x(0.f)
		,m_Health(ZombieMaxHealth)
		,m_ColorMult(nullptr)
		,m_WasShooted(false)
		,m_State(EZombieState::ROAMING)
		,m_TargetDirection(0.f, 0.f, 0.f)
		,m_CurrentDirection(0.f, 0.f, 0.f)
		,m_ShootedFrame((size_t)-1)
		,m_OnZombieKilled(nullptr)
	{

	}

	void Shooted()
	{
		m_WasShooted = true;
		m_ShootedFrame = Game::instance->CurrentFrame + 10; //  target frame to remove the change of color
		m_Health -= WeaponDamage;
	}

	void Update(double elapsed_seconds)override
	{
		if (m_Health <= 0)
		{
			(*m_OnZombieKilled)(this);
			Game::instance->Score += 100;
			return;
		}
		if (Game::instance->pause)
			return;

		auto& player = Game::instance->player;
		const auto speed = float(elapsed_seconds) * *m_Speed;
		const auto position = m_WorldMatrix.getTranslation();
		auto pDir = player.position - position;
		pDir.y = 0.f;

		if (m_State == EZombieState::ROAMING)
		{
			const auto pDistance = position.distance(player.position);
			auto pnDir = pDir;
			pnDir.normalize();
			const auto angle = acos(m_CurrentDirection.dot(pnDir));

			if (pDistance < ZombieDetectRange || (pDistance < ZombieSightRange && angle < 0.6))
			{
				m_State = EZombieState::CHASING;
			}
			else // keep roaming
			{
				const float rngMax = 1.f / float(RAND_MAX);
				auto rng = float(rand()) * rngMax;
				if (rng < ZombieChangeDirProb || m_TargetDirection == Vector3(0.f, 0.f, 0.f))
				{
					m_TargetDirection.set(float(rand()) * rngMax * 2.f - 1.f, 0.f, float(rand()) * rngMax * 2.f - 1.f);
				}
				else
				{
					if (abs(m_TargetDirection.x) > 0.005f && abs(m_TargetDirection.z) > 0.005f)
					{
						m_TargetDirection.normalize();
						Vector3 target_to_test = (m_TargetDirection * speed) + position;
						CollisionData colData{ position, target_to_test, this, m_Parent, true, true, elapsed_seconds };
						EntityMesh* emesh = nullptr;
						const auto target = CheckCollision(colData, &emesh);
						if (emesh != nullptr)
						{
							if ((emesh->GetLayer() & EL_VALLA) != 0)
								((EntityWall*)emesh)->OnZombieTouch();
							else
								m_TargetDirection.set(-m_TargetDirection.x, m_TargetDirection.y, -m_TargetDirection.z);
						}
						SetPosition(target - position);
					}
					else
					{
						m_TargetDirection.x = -m_TargetDirection.x;
					}
				}
			}
		}
		else if (m_State == EZombieState::CHASING)
		{
			m_TargetDirection = pDir;
			if (abs(m_TargetDirection.x) > 0.005f && abs(m_TargetDirection.z) > 0.005f)
			{
				m_TargetDirection.normalize();
				Vector3 target_to_test = (m_TargetDirection * speed) + position;
				CollisionData colData{ position, target_to_test, this, m_Parent, true, true, elapsed_seconds };
				EntityMesh* emesh = nullptr;

				const auto target = CheckCollision(colData, &emesh);
				if (emesh != nullptr)
				{
					if ((emesh->GetLayer() & EL_VALLA) != 0)
						((EntityWall*)emesh)->OnZombieTouch();
				}
				SetPosition(target - position);
			}
			else
			{
				m_TargetDirection.x = -m_TargetDirection.x;
			}
		}


		
		EntityMesh::Update(elapsed_seconds);
	}

	void Render()override
	{

		Entity::Render();
		Camera* camera = Camera::current;
		if (camera->testBoxInFrustum(m_WorldAABB.center, m_WorldAABB.halfsize) == CLIP_OUTSIDE)
			return;

		auto& player = Game::instance->player;

		float angle_in_rad = atan2(m_TargetDirection.x, m_TargetDirection.z);

		angle_in_rad = (float)fmod((double)angle_in_rad + PI * 2.0, PI * 2.0);


		auto target_angle = -angle_in_rad;

		Matrix44 rotation_matrix_torse;
		Matrix44 rotation_matrix_leg_right;
		Matrix44 rotation_matrix_leg_left;
		Matrix44 rotation_matrix_head;

		float aux = 0.f;

		if (Game::instance->time_pause < 3.f) { aux = 1.f; }
		else { aux = 0.01f; }

		Matrix44 rotation;

		temp_x += (target_angle - temp_x) * aux;

		//parche para el bug

		if (target_angle > -0.1 && prev_target_angle < -6) {

		
			temp_x = -0.01f;
		}
		else if (target_angle < -0.1 && prev_target_angle < -6) {

			temp_x = -6.2f;

		}

		//std::cout << x << std::endl;

		prev_target_angle = target_angle;

		//parche para el bug

		rotation.rotate(temp_x, Vector3(0.f, 1.f, 0.f));
		m_CurrentDirection = rotation.frontVector();

		//Vector4 color = Vector4(0.f, 1.f, 1.f,0.f);
		Vector4 color = m_ColorMult ? *m_ColorMult : Vector4(1.f, 1.f, 1.f, 1.f);


#if !INSTANCED_ZOMBIES || !defined(INSTANCED_ZOMBIES)
		shader->enable();

		float sinus = sin(Game::instance->time_pause * float(DEG2RAD) * 100.f);
		float time = Game::instance->IsDay() ? 0.f : Game::instance->time_pause;
		rotation_matrix_torse.rotate(0.5f * sin(time * 100.f * float(DEG2RAD)), Vector3(0.f, 1.f, 0.f));
		rotation_matrix_head.rotate(0.3f * sin(time * -100.f * float(DEG2RAD)), Vector3(0.f, 1.f, 0.f));
		rotation_matrix_leg_right.rotate(0.075f * sin(time * 90.f * float(DEG2RAD)), Vector3(0.f, 1.f, 0.f));
		rotation_matrix_leg_left.rotate(0.075f * sin(time * -90.f * float(DEG2RAD)), Vector3(0.f, 1.f, 0.f));

		shader->setUniform("u_color", color);
		shader->setUniform("u_viewprojection", camera->viewprojection_matrix);
		shader->setUniform("u_model", m_WorldMatrix);
		shader->setUniform("u_model_rotate", rotation);
		shader->setUniform("u_model_rotate_head", rotation_matrix_head);
		shader->setUniform("u_model_rotate_torse", rotation_matrix_torse);
		shader->setUniform("u_model_rotate_leg_r", rotation_matrix_leg_right);
		shader->setUniform("u_model_rotate_leg_l", rotation_matrix_leg_left);
		shader->setUniform("u_time", Game::instance->time_pause); // Para el nuevo shader
		shader->setUniform("u_camera_position", camera->eye); // Para el nuevo shader
		shader->setUniform("u_light_direction", Vector3(0.3f, 0.6f, 0.2f)); // Para el nuevo shader

		mesh->render(GL_TRIANGLES, 0);

		//disable shader
		shader->disable();
#else
		StaticObjectRenderer::instance->AddZombie(m_WorldMatrix, rotation, color);
#endif
		RenderAABB();
		if (ShowFrontVector)
		{
			auto front = m_CurrentDirection;
			front = front.normalize();
			auto pos = m_WorldMatrix.getTranslation() + Vector3(0.f, 0.5f, 0.f);
			FrontVectorMesh.vertices[0] = pos;
			FrontVectorMesh.vertices[1] = pos + front;
			RenderGeneralMesh(Matrix44(), &FrontVectorMesh, nullptr, { 0.f, 1.f, 0.f, 1.f }, nullptr, 0, GL_LINES);
			if (m_TargetDirection != Vector3(0.f, 0.f, 0.f))
			{
				front = m_TargetDirection;
				front.normalize();
				FrontVectorMesh.vertices[1] = pos + front;
				RenderGeneralMesh(Matrix44(), &FrontVectorMesh, nullptr, { 0.f, 0.f, 1.f, 1.f }, nullptr, 0, GL_LINES);
			}
		}
		//RenderRays();
	}
};

#endif /* ENTITY_ZOMBIE_H */