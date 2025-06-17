#pragma once

#ifndef EDITOR_STAGE_H
#define EDITOR_STAGE_H 1

#include "includes.h"
#include "GameStageManager.h"
#include "camera.h"
#include "input.h"
#include "TileMap.h"

class EditorStage : public GameStage
{
	float m_CameraHeight;
	bool m_Started;
	std::array<TileMap*, 4> m_Maps;

public:
	EditorStage()
		:GameStage("EditorStage")
		,m_CameraHeight(5.f)
		,m_Started(false)
	{

	}

	~EditorStage() = default;

	void OnGSInit()override
	{
		auto* playStage = GameStageManager::instance->GetStage("PlayStage");
		if (playStage->GetRootEntity() != m_StageRootEntity)
		{
			m_StageRootEntity->GetParent()->DestroyChild(m_StageRootEntity->GetName(), false);
			m_StageRootEntity = playStage->GetRootEntity();
		}
		GameStage::OnGSInit(); // GSInit must be done after root entity change
		if (!m_Started)
		{
			m_Started = true;
		}
		TileMap::gShowTiles = true;
		SDL_ShowCursor(true);
		Game::instance->mouse_locked = true;


	}

	void OnGSStop()override
	{
		GameStage::OnGSStop();
	}

	void OnUpdate(double seconds_elapsed)override
	{
		GameStage::OnUpdate(seconds_elapsed);

		if (Input::wasKeyPressed(SDL_SCANCODE_TAB))
			GameStageManager::instance->ChangeStage("PlayStage");

		auto& player = Game::instance->player;

		float speed = float(seconds_elapsed * 20.0); //the speed is defined by the seconds_elapsed so it goes constant

		Matrix44 R;

		R.setRotation(float(double(player.angle) * DEG2RAD), Vector3(0.f, 1.f, 0.f));
		Vector3 front = R.rotateVector(Vector3(0.f, 0.f, -1.f));
		Vector3 right = R.rotateVector(Vector3(1.f, 0.f, 0.f));

		if (Input::isKeyPressed(SDL_SCANCODE_LSHIFT)) speed *= 10; //move faster with left shift
		if (Input::isKeyPressed(SDL_SCANCODE_W) || Input::isKeyPressed(SDL_SCANCODE_UP)) player.position = player.position + front * speed;
		if (Input::isKeyPressed(SDL_SCANCODE_S) || Input::isKeyPressed(SDL_SCANCODE_DOWN)) player.position = player.position - front * speed;
		if (Input::isKeyPressed(SDL_SCANCODE_A) || Input::isKeyPressed(SDL_SCANCODE_LEFT)) player.position = player.position - right * speed;
		if (Input::isKeyPressed(SDL_SCANCODE_D) || Input::isKeyPressed(SDL_SCANCODE_RIGHT)) player.position = player.position + right * speed;
		if (Input::wasKeyPressed(SDL_SCANCODE_Z)) setObjectinfront("data/FinalMeshes/tree.obj", "data/FinalMeshes/tree.png", false);
		if (Input::wasKeyPressed(SDL_SCANCODE_X)) setObjectinfront("data/FinalMeshes/vampire.obj", "data/FinalMeshes/vampire.mtl", true);
		if (Input::wasKeyPressed(SDL_SCANCODE_C)) setObjectinfront("data/FinalMeshes/Vallas_y_madera/logs_stackLarge.obj", "data/FinalMeshes/Vallas_y_madera/logs_stackLarge.mtl", true, EL_WOOD);
		if (Input::isMousePressed(SDL_BUTTON_RIGHT)) setObjectinfront("data/FinalMeshes/Vallas_y_madera/ironFence.obj", "data/FinalMeshes/Vallas_y_madera/ironFence.mtl", true);
		if (Input::wasKeyPressed(SDL_SCANCODE_V)) setObjectinfront("data/FinalMeshes/rockLarge.obj", "data/FinalMeshes/rockLarge.mtl", true);
		if (Input::wasKeyPressed(SDL_SCANCODE_B)) setObjectinfront("data/FinalMeshes/rockSmall.obj", "data/FinalMeshes/rockSmall.mtl", true);
		if (Input::wasKeyPressed(SDL_SCANCODE_N)) setObjectinfront("data/FinalMeshes/rockWide.obj", "data/FinalMeshes/rockWide.mtl", true);
		if (Input::wasKeyPressed(SDL_SCANCODE_M)) setObjectinfront("data/FinalMeshes/gravestoneCross.obj", "data/FinalMeshes/gravestoneCross.mtl", true); 
		if (Input::wasKeyPressed(SDL_SCANCODE_P))
		{
			auto& maps = Game::instance->Maps;
			const auto lvlStr = std::to_string(Game::instance->CurrentLevel);
			for (auto* map : maps)
			{
				std::string path = "data/" + map->GetMapEntity()->GetName() + 
					"_L" + lvlStr + ".map";
				map->SaveMapToFile(path);
			}
		}
		player.pitch -= Input::mouse_delta.y * 0.1f;
		player.angle -= Input::mouse_delta.x * 0.1f;

		player.model.setIdentity();
		player.model.setTranslation(player.position.x, player.position.y, player.position.z);
		player.model.rotate(float(double(player.angle) * DEG2RAD), Vector3(0.f, 1.f, 0.f));
	}

	void OnRender()override
	{
		GameStage::OnRender();

		auto& player = Game::instance->player;

		Vector3 eye = player.model * Vector3(0.f, m_CameraHeight, 0.f);
		Vector3 front(0, 0, -1);

		Matrix44 PITCH;
		PITCH.setRotation(float(double(player.pitch) * DEG2RAD), Vector3(1.f, 0.f, 0.f));
		front = PITCH.rotateVector(front);

		Matrix44 R;
		R.setRotation(float(double(player.angle) * DEG2RAD), Vector3(0.f, 1.f, 0.f));
		front = R.rotateVector(front);
		Vector3 center = eye + front;
		Vector3 up = Vector3(0.f, 1.f, 0.f);
		Camera::current->lookAt(eye, center, up);
	}

	void setObjectinfront(const char* mesh_name, const char* mtl_name, bool is_mtl, size_t layer = 0)
	{
		Vector3 pos = Camera::current->center;
		pos.y = 0;
		Vector3 origin = Camera::current->eye;
		Vector3 dir = Camera::current->getRayDirection(int(Input::mouse_position.x), int(Input::mouse_position.y), float(Game::instance->window_width), float(Game::instance->window_height));
		pos = RayPlaneCollision(Vector3(), Vector3(0.f, 1.f, 0.f), origin, dir);
		
		auto* shader = Shader::Get("data/shaders/basic.vs", "data/shaders/flat.fs");
		Vector3 color = Vector3(0.f, 0.f, 0.f);
		Mesh* mesh = Mesh::Get(mesh_name);
		Entity* parent = nullptr;

		auto* map = Game::instance->GetMapAt(pos);
		if (map != nullptr)
		{
			parent = map->GetTile(map->World2TileIdx(pos));
		}
		if (parent == nullptr)// Outside maps
		{
			parent = GetRootEntity();
		}

		std::string cleanMeshName(mesh_name);
		cleanMeshName = cleanMeshName.substr(0, cleanMeshName.find_last_of('.'));
		const auto lastSlash = cleanMeshName.find_last_of('/');
		if (lastSlash != std::string::npos)
		{
			cleanMeshName = cleanMeshName.substr(lastSlash + 1);
		}
		auto* entity = AddEntity<EntityMesh>(parent, cleanMeshName, mesh, shader, mtl_name, is_mtl);

		entity->SetPosition(pos - parent->GetWorldPosition());
		entity->SetLayer(entity->GetLayer() | layer);
	}
};


#endif /* EDITOR_STAGE_H */