#pragma once

#ifndef PLAY_STAGE_H
#define PLAY_STAGE_H 1

#include "GameStageManager.h"
#include "EntityMesh.h"
#include "ZombieManager.h"
#include "EntityWeapon.h"
#include "TileMap.h"
#include "Audio.h"
#include "EntityWall.h"

class PlayStage : public GameStage
{
	float m_CameraHeight;
	bool m_Started;
	
	EntityWeapon* Weapon;
	int objetive_frame;
	float objetive_time;
	int m_Level;
	bool m_WasNight;
	bool m_WasDay;
	int m_SurvivedDays;

public:
	static HCHANNEL channel_music;
	Mesh* floor_mesh;
	Mesh* Sky_mesh;
	PlayStage()
		:GameStage("PlayStage")
		,m_CameraHeight(0.2f)
		,m_Started(false)
		,floor_mesh(nullptr)
		, Sky_mesh(nullptr)
		, Weapon(nullptr)
		,objetive_time(0.0f)
		,m_Level(0)
		,m_WasNight(false)
		,m_WasDay(false)
		,m_SurvivedDays(0)
	{

	}

	~PlayStage() = default;

	void AddWall()
	{
		Vector3 posmapa = Camera::current->eye;

		Entity* parent = nullptr;

		auto* map = Game::instance->GetMapAt(posmapa);
		if (map != nullptr)
		{
			parent = map->GetTile(map->World2TileIdx(posmapa));
		}
		if (parent == nullptr)// Outside maps
		{
			parent = GetRootEntity();
		}
		auto* entity = AddEntity<EntityWall>(parent, "ironFence");


		auto& player = Game::instance->player;

		auto pFront = player.model.frontVector();


		Vector3 pos = player.position + pFront.normalize() * 0.05f;

		if (entity != nullptr)
		{
			entity->SetPosition(pos - parent->GetWorldPosition());
			entity->setfront(pFront);
		}
	}

	void ClearLevel()
	{
		auto& maps = Game::instance->Maps;
		for (auto* map : maps)
			map->Clear();
	}

	void LoadLevel(int level)
	{
		const float prob = 0.25f;
		const float rngMax = 1.f / float(RAND_MAX);
		unsigned int horizontalTiles = 10;
		unsigned int verticalTiles = 10;
		auto& maps = Game::instance->Maps;
		unsigned int hvMaps = (unsigned int)floorf(sqrtf((float)maps.size()));
		const auto lvlStr = std::to_string(level);
		for (auto* map : maps)
		{
			std::string path = "data/" + map->GetMapEntity()->GetName() +
				"_L" + lvlStr + ".map";
			if (!map->LoadFromFile(path))
			{
				// Generate random stuff
				srand(SDL_GetTicks());
				for (unsigned int y = 0; y < verticalTiles; ++y)
				{
					for (unsigned int x = 0; x < horizontalTiles; ++x)
					{
						const auto rng = float(rand()) * rngMax;
						auto* tile = map->GetMapEntity()->GetChilds()[x + y * horizontalTiles];
						const auto rngH = (float(rand()) * rngMax) * 0.5f;
						const auto rngV = (float(rand()) * rngMax) * 0.5f;
						if (rng <= prob)
						{
							auto* tree = tile->AddChild<EntityMesh>(tile->GetName() + "_tree",
								Mesh::Get("data/FinalMeshes/tree.obj"),
								Shader::Get("data/shaders/basic.vs", "data/shaders/flat.fs"),
								"data/FinalMeshes/tree.png",
								false);

							tree->SetPosition(Vector3(rngH, 0.f, rngV));
						}
						else if (rng <= (prob + prob * 0.5f))
						{
							auto* wood = tile->AddChild<EntityMesh>(tile->GetName() + "_wood",
								Mesh::Get("data/FinalMeshes/Vallas_y_madera/logs_stackLarge.obj"),
								Shader::Get("data/shaders/basic.vs", "data/shaders/flat.fs"),
								"data/FinalMeshes/Vallas_y_madera/logs_stackLarge.mtl");
							wood->SetPosition(Vector3(rngH, 0.f, rngV));
							wood->SetLayer(wood->GetLayer() | EL_WOOD);
						}
					}
				}
			}
		}
	}

	void OnGSInit()override
	{
		GameStage::OnGSInit();
		if (!m_Started)
		{
			unsigned int tileSize = 1;
			unsigned int horizontalTiles = 10;
			unsigned int verticalTiles = 10;
			unsigned int horizontalSize = tileSize * horizontalTiles;
			unsigned int verticalSize = tileSize * verticalTiles;
			auto& maps = Game::instance->Maps;
			unsigned int hvMaps = (unsigned int)floorf(sqrtf((float)maps.size()));
			for (unsigned int y = 0; y < hvMaps; ++y)
			{
				unsigned int yOffset = y * hvMaps;
				for (unsigned int x = 0; x < hvMaps; ++x)
				{
					auto*& map = maps[x + yOffset];
					map = new TileMap(GetRootEntity(), Vector3(float(x * horizontalSize), 0.f, float(y * verticalSize)), tileSize, horizontalTiles, verticalTiles);
				}
			}
			LoadLevel(m_Level);
			

			Input::centerMouse();
			Game::instance->mouse_locked = true;
			Game::instance->player.angle = 135.f;
			Game::instance->player.pitch = 0.5f;
			
			// Appear in the middle of the maps
			size_t mapHV = (size_t)sqrtf((float)Game::MapAmount);
			size_t mapIdx = (size_t)floorf((float)mapHV * 0.5f);
			mapIdx = mapIdx + mapIdx * mapHV;
			float halfMapH = float(horizontalSize) * 0.5f;
			float halfMapV = float(verticalSize) * 0.5f;
			const auto mapWPos = Game::instance->Maps[mapIdx]->GetMapEntity()->GetWorldPosition();
			Vector3 playerPos{ mapWPos.x + halfMapH, 0.f, mapWPos.z + halfMapV };

			Game::instance->player.position = playerPos;

		
			ZombieManager::instance->Init(GetRootEntity());

		

			Weapon = GetRootEntity()->AddChild<EntityWeapon>("Weapon");

		

			m_Started = true;
		}
		TileMap::gShowTiles = false;
		SDL_ShowCursor(false);
		
	
	}

	void OnGSStop()override
	{
		GameStage::OnGSStop();

	}

	void OnUpdate(double seconds_elapsed)override
	{
		GameStage::OnUpdate(seconds_elapsed);

		bool isDay = Game::instance->IsDay();
		bool isNight = Game::instance->IsNight();

		if (!m_WasDay && isDay)
		{
			if(m_SurvivedDays != 0)
				Game::instance->Score += 1000;
			++m_SurvivedDays;
		}

		if (m_SurvivedDays >= 3)
		{
			// Win Condition
			if (Game::instance->CurrentLevel == 1)
			{
				m_SurvivedDays = 0;
				ClearLevel();
				ClearLevel();
				LoadLevel(0);
				Game::instance->WoodCounter = 0;
				GameStageManager::instance->ChangeStage("WinStage");
				return;
			}
			else
			{
				Game::instance->CurrentLevel = 1;
				ClearLevel();
				ClearLevel();
				LoadLevel(1);

				m_SurvivedDays = 0;

				unsigned int tileSize = 1;
				unsigned int horizontalTiles = 10;
				unsigned int verticalTiles = 10;
				unsigned int horizontalSize = tileSize * horizontalTiles;
				unsigned int verticalSize = tileSize * verticalTiles;

				size_t mapHV = (size_t)sqrtf((float)Game::MapAmount);
				size_t mapIdx = (size_t)floorf((float)mapHV * 0.5f);
				mapIdx = mapIdx + mapIdx * mapHV;
				float halfMapH = float(horizontalSize) * 0.5f;
				float halfMapV = float(verticalSize) * 0.5f;
				const auto mapWPos = Game::instance->Maps[mapIdx]->GetMapEntity()->GetWorldPosition();
				Vector3 playerPos{ mapWPos.x + halfMapH, 0.f, mapWPos.z + halfMapV };

				Game::instance->player.position = playerPos;

				Game::instance->time_pause = 0;
				ZombieManager::instance->Clear();

				Audio::Stop(channel_music);

				Audio* test = Audio::Get("data/Audio/Lounge_Game1.wav", true);

				channel_music = test->play(0.2f);
				Game::instance->pause = false;
				Game::instance->pause_clean = false;
				ZombieManager::instance->Init(GetRootEntity());
				Game::instance->WoodCounter = 0;
				Game::instance->player.Health = Game::PlayerMaxHealth;
			}
		}
	
		if (Input::wasKeyPressed(SDL_SCANCODE_TAB))
			GameStageManager::instance->ChangeStage("EditorStage");

		if (Input::wasKeyPressed(SDL_SCANCODE_0))
			m_SurvivedDays = 3;

		if (Input::wasKeyPressed(SDL_SCANCODE_RETURN)) {

			Game::instance->pause_clean = true;
			objetive_frame = Game::instance->CurrentFrame +3;

		}

		if (objetive_frame == Game::instance->CurrentFrame) { GameStageManager::instance->ChangeStage("PauseStage"); }

		auto& player = Game::instance->player;
		auto speed = float(seconds_elapsed * 2.0);

		Matrix44 R;

		R.setRotation(float(double(player.angle) * DEG2RAD), Vector3(0.f, 1.f, 0.f));
		Vector3 front = R.rotateVector(Vector3(0.f, 0.f, -1.f));
		Vector3 right = R.rotateVector(Vector3(1.f, 0.f, 0.f));

		Vector3 target_pos = player.position;

		int steps_frecuency = 0;

		bool pMoved = false;
		if (Input::isKeyPressed(SDL_SCANCODE_LSHIFT)){ speed *= 2.5f; steps_frecuency = 10;}//move faster with left shift
		if (Input::isKeyPressed(SDL_SCANCODE_W) || Input::isKeyPressed(SDL_SCANCODE_UP)) { target_pos = target_pos + front * speed; pMoved = true; }
		if (Input::isKeyPressed(SDL_SCANCODE_S) || Input::isKeyPressed(SDL_SCANCODE_DOWN)) { target_pos = target_pos - front * speed; pMoved = true; }
		if (Input::isKeyPressed(SDL_SCANCODE_A) || Input::isKeyPressed(SDL_SCANCODE_LEFT)) { target_pos = target_pos - right * speed; pMoved = true; }
		if (Input::isKeyPressed(SDL_SCANCODE_D) || Input::isKeyPressed(SDL_SCANCODE_RIGHT)) { target_pos = target_pos + right * speed; pMoved = true; }
	

		Audio* step = Audio::Get("data/Audio/step2.wav", false);
		//wood->play(0.6f);

		if (Game::instance->frame % (30 - steps_frecuency) ==0)
		{
			if (pMoved)
			{
				step->play(0.4f);
			}
			else
			{
				step->stop();
			}
		}

		if (Input::wasMousePressed(SDL_BUTTON_LEFT)) { // Shoot
			//std::cout << checkShootZombie() << std::endl;

			Audio* shoot = Audio::Get("data/Audio/shoot2.wav", false);
			shoot->play(0.8f);

			Vector3 pos = Camera::current->center;
			pos.y = 0.f;
			Vector3 origin = Camera::current->eye;
			Vector3 dir = Camera::current->getRayDirection(int(Input::mouse_position.x), int(Input::mouse_position.y), float(Game::instance->window_width), float(Game::instance->window_height));
			bool shot = ZombieManager::instance->Shoot(origin, dir);

			if (shot)
			{
				Audio* test = Audio::Get("data/Audio/zombieshoot.wav",false);
				test->play(0.6f);
			}
		}

		if (Input::wasMousePressed(SDL_BUTTON_RIGHT))
		{ // Place fence
			if (Game::instance->WoodCounter == 0)
			{
				std::cout << "necesitas madera" << std::endl;
			}
			else
			{
				AddWall();
				--Game::instance->WoodCounter;
				Audio* valla = Audio::Get("data/Audio/valla.wav", false);
				valla->play(1.0f);
			}
		}
		player.pitch -= Input::mouse_delta.y * 0.1f;
		player.angle -= Input::mouse_delta.x * 0.1f;

		//player.model.setIdentity();
		
		player.model.setTranslation(player.position.x, player.position.y, player.position.z);
		player.model.rotate(float(double(player.angle) * DEG2RAD), Vector3(0.f, 1.f, 0.f));


		CollisionData colData{ player.position, target_pos, nullptr, GetRootEntity(), true, true, seconds_elapsed };
		EntityMesh* collidedMesh = nullptr;
		player.position = CheckCollision(colData, &collidedMesh);
		
		if (collidedMesh && collidedMesh->mesh->name =="data/FinalMeshes/zombie.obj" && (objetive_time < Game::instance->time_pause)) {

			
			Audio* hurt = Audio::Get("data/Audio/hurt.wav", false);
			hurt->play(0.6f);
			player.Health -= 25;
			auto& score = Game::instance->Score;
			if (score > 10)
				score -= 10;
			objetive_time = Game::instance->time_pause + 1;

		}

		else if (collidedMesh && (collidedMesh->GetLayer() & EL_WOOD) != 0)
		{
			++Game::instance->WoodCounter;
			collidedMesh->GetParent()->DestroyChild(collidedMesh->GetName(), false);
			Audio* wood = Audio::Get("data/Audio/wood.wav", false);
			wood->play(0.6f);
		}

		if (player.Health  <= 0) {

			player.position = Vector3(0, 0, 0);
			player.Health = 100;
			Game::instance->pause = true;
			Game::instance->time_pause = 0;
			objetive_time = 0.0;
			ZombieManager::instance->Clear();

			unsigned int tileSize = 1;
			unsigned int horizontalTiles = 10;
			unsigned int verticalTiles = 10;
			unsigned int horizontalSize = tileSize * horizontalTiles;
			unsigned int verticalSize = tileSize * verticalTiles;

			size_t mapHV = (size_t)sqrtf((float)Game::MapAmount);
			size_t mapIdx = (size_t)floorf((float)mapHV * 0.5f);
			mapIdx = mapIdx + mapIdx * mapHV;
			float halfMapH = float(horizontalSize) * 0.5f;
			float halfMapV = float(verticalSize) * 0.5f;
			const auto mapWPos = Game::instance->Maps[mapIdx]->GetMapEntity()->GetWorldPosition();
			Vector3 playerPos{ mapWPos.x + halfMapH, 0.f, mapWPos.z + halfMapV };

			Game::instance->player.position = playerPos;
			GameStageManager::instance->ChangeStage("DeadStage");
		}
		
		m_WasDay = isDay;
		m_WasNight = isNight;
	}

	void OnRender()override
	{
		GameStage::OnRender();

		auto& player = Game::instance->player;

		//m_GameCamera->view_matrix;
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

		Matrix44 m1;

		//m1.translate(50, 0, 0);

		Shader* shader = Shader::Get("data/shaders/basic.vs", "data/shaders/flat-floor.fs");

		//renderMesh(m1, floor_mesh, NULL);

		Camera* camera = Camera::current;

		Texture* texture_floor = Texture::Get("data/FinalMeshes/ground3.png");

		shader->enable();

		//upload uniforms

		shader->setUniform("u_texture", texture_floor);
		shader->setUniform("u_viewprojection", camera->viewprojection_matrix);
		shader->setUniform("u_model", m1);
		shader->setUniform("u_time", Game::instance->time_pause); // Para el nuevo shader
		shader->setUniform("u_camera_position", camera->eye); // Para el nuevo shader
		shader->setUniform("u_light_direction", Vector3(0.3f, 0.6f, 0.2f)); // Para el nuevo shader

		floor_mesh->render(GL_TRIANGLES, 0);

		//disable shader
		shader->disable();


		//Y aqui el cielo
		//glDisable(GL_DEPTH_TEST);

		Shader* shader_sky = Shader::Get("data/shaders/basic-sky.vs", "data/shaders/flat-Sky.fs");

		//renderMesh(m1, floor_mesh, NULL);

		Matrix44 skybox;
		skybox.setTranslation(camera->eye.x, camera->eye.y, camera->eye.z);
		//skybox.scale(0.01, 0.01, 0.01);

		shader_sky->enable();

		//upload uniforms


		shader_sky->setUniform("u_viewprojection", camera->viewprojection_matrix);
		shader_sky->setUniform("u_model", skybox);
		shader_sky->setUniform("u_texture", Texture::Get("data/FinalMeshes/cielo/cielo2.tga"));
		shader_sky->setUniform("u_color", Vector4(1.f, 1.f, 1.f, 1.f));
		shader_sky->setUniform("u_time", Game::instance->time_pause); // Para el nuevo shader
		shader_sky->setUniform("u_camera_position", camera->eye); // Para el nuevo shader
		shader_sky->setUniform("u_light_direction", Vector3(0.3f, 0.6f, 0.2f)); // Para el nuevo shader

		Sky_mesh->render(GL_TRIANGLES, 0);

		//disable shader
		shader_sky->disable();
	}

	void OnRenderGUI()override
	{
		auto& player = Game::instance->player;

		Shader* shaderUI = Shader::Get("data/shaders/shaderUI.vs", "data/shaders/texture.fs");
		Texture* tex = Texture::Get("data/FinalMeshes/ForestIcons_10_b.PNG");
		Texture* tex2 = Texture::Get("data/FinalMeshes/heart.tga");

		Mesh HeartQuad;
		Mesh HeartQuad2;
		Mesh HeartQuad3;
		Mesh HeartQuad4;

		HeartQuad.createQuad(0.85f, 0.85f, 0.10f * ((float)Game::instance->window_width / (float)Game::instance->window_height), 0.15f, false);
		HeartQuad2.createQuad(0.70f, 0.85f, 0.10f * ((float)Game::instance->window_width / (float)Game::instance->window_height), 0.15f, false);
		HeartQuad3.createQuad(0.55f, 0.85f, 0.10f * ((float)Game::instance->window_width / (float)Game::instance->window_height), 0.15f, false);
		HeartQuad4.createQuad(0.40f, 0.85f, 0.10f * ((float)Game::instance->window_width / (float)Game::instance->window_height), 0.15f, false);

		glDisable(GL_DEPTH_TEST);
		shaderUI->enable();

		shaderUI->setUniform("u_color", Vector4(1.f, 1.f, 1.f, 1.f));
		shaderUI->setUniform("u_texture", tex);
		Game::instance->woodQuad.render(GL_TRIANGLES);
		shaderUI->disable();

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		shaderUI->enable();

		shaderUI->setUniform("u_color", Vector4(1.f, 1.f, 1.f, 1.f));
		shaderUI->setUniform("u_texture", tex2);
		if (player.Health > 0)
			HeartQuad.render(GL_TRIANGLES);
		if (player.Health > 25)
			HeartQuad2.render(GL_TRIANGLES);
		if (player.Health > 50)
			HeartQuad3.render(GL_TRIANGLES);
		if (player.Health > 75) {
			HeartQuad4.render(GL_TRIANGLES);
		}
		shaderUI->disable();
		glDisable(GL_BLEND);
		glEnable(GL_DEPTH_TEST);
	
		drawText(20.f, 520.f, std::to_string(Game::instance->WoodCounter), Vector3(1.f, 1.f, 1.f), 2.f);

		char buffer[128];
		snprintf(buffer, ARRAYSIZE(buffer), "Score:%d", Game::instance->Score);
		drawText(2.f, 2.f, std::string(buffer), Vector3(1.f, 1.f, 1.f), 2.f);

		// Draw the crosshair
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		gluOrtho2D(0.0, (double)Game::instance->window_width, 0.0, (double)Game::instance->window_height);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		glColor3f(1.f, 1.f, 1.f);
		glPointSize(5.f);
		glBegin(GL_POINTS);
		glColor3f(1.f, 1.f, 1.f);
		glVertex2i(GLint(float(Game::instance->window_width) * 0.5f), GLint(float(Game::instance->window_height) * 0.5f));
		glEnd();
	}
};

#endif /* PLAY_STAGE_H */