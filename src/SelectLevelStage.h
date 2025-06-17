#pragma once

#ifndef SELECT_LEVEL_STAGE_H
#define SELECT_LEVEL_STAGE_H 1

#include "GameStageManager.h"
#include "utils.h"

class SelectLevelStage : public GameStage
{

	PlayStage* m_PlayStage;
	bool m_Initialized;


	int state;

	int Vector_states_int[3];
	int len_arrays_int;
	enum { LEVEL1, LEVEL2 ,BACKTOMENU};

public:
	SelectLevelStage()
		:GameStage("SelectLevelStage")
		, m_PlayStage(nullptr)
		, m_Initialized(false)

	{
		state = LEVEL1;

		Vector_states_int[0] = { LEVEL1 };
		Vector_states_int[1] = { LEVEL2 };
		Vector_states_int[2] = { BACKTOMENU };
		len_arrays_int = sizeof(Vector_states_int) / sizeof(Vector_states_int[0]);


	}

	~SelectLevelStage() = default;

	void OnGSInit()override
	{
		GameStage::OnGSInit();

		if (!m_Initialized)
		{
			m_PlayStage = (PlayStage*)GameStageManager::instance->GetStage("PlayStage");
			m_Initialized = true;
		}

	}

	void OnGSStop()override
	{
		GameStage::OnGSStop();

	}

	void OnUpdate(double seconds_elapsed)override
	{
		GameStage::OnUpdate(seconds_elapsed);

		Audio* select = Audio::Get("data/Audio/select.wav", false);

		HCHANNEL musica_chenl = PlayStage::channel_music;

		Audio::Stop(musica_chenl);

		if (Input::wasKeyPressed(SDL_SCANCODE_W) || Input::wasKeyPressed(SDL_SCANCODE_UP)) {

			select->play(0.4f);

			if (state == Vector_states_int[0]) { state = Vector_states_int[len_arrays_int - 1]; }
			else { state = Vector_states_int[state - 1]; }
		}

		if (Input::wasKeyPressed(SDL_SCANCODE_S) || Input::wasKeyPressed(SDL_SCANCODE_DOWN)) {

			select->play(0.4f);

			if (state == len_arrays_int - 1) { state = Vector_states_int[0]; }
			else { state = Vector_states_int[state + 1]; }
		}

		bool wasLevelReestarted = false;
		if (Input::wasKeyPressed(SDL_SCANCODE_RETURN)) {

			if (state == LEVEL1) {


				m_PlayStage->OnGSInit();
				
				Game::instance->CurrentLevel = 0;
				m_PlayStage->ClearLevel();
				m_PlayStage->ClearLevel();
				m_PlayStage->LoadLevel(0);

				//// FUNCION reinicioar

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
				Audio::Stop(musica_chenl);

				Audio* test = Audio::Get("data/Audio/Lounge_Game1.wav", true);

				PlayStage::channel_music = test->play(0.2f);
				Game::instance->pause = false;
				Game::instance->pause_clean = false;

				/////
				GameStageManager::instance->ChangeStage("PlayStage");
				wasLevelReestarted = true;
			}

			else if (state == LEVEL2) {

				m_PlayStage->OnGSInit();

				Game::instance->CurrentLevel = 1;
				m_PlayStage->ClearLevel();
				m_PlayStage->ClearLevel();
				m_PlayStage->LoadLevel(1);

				//// FUNCION reinicioar

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
				Audio::Stop(musica_chenl);

				Audio* test = Audio::Get("data/Audio/Lounge_Game1.wav", true);

				PlayStage::channel_music = test->play(0.2f);
				Game::instance->pause = false;
				Game::instance->pause_clean = false;

				/////

				GameStageManager::instance->ChangeStage("PlayStage");
				wasLevelReestarted = true;
			}

			else if (state == BACKTOMENU) {

				Game::instance->must_exit = true;

			}

		}

		if (wasLevelReestarted)
		{
			ZombieManager::instance->Init(m_PlayStage->GetRootEntity());
			Game::instance->WoodCounter = 0;
			Game::instance->player.Health = Game::PlayerMaxHealth;
		}
	}

	void OnRenderGUI()override
	{
		glDisable(GL_DEPTH_TEST);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		drawText(300.f, 250.f, "Choose Level", Vector3(1.f, 1.f, 1.f), 2.f);
		drawText(300.f, 300.f, "Level 1", Vector3(1.f, 1.f, 1.f), 2.f);
		drawText(300.f, 320.f, "Level 2", Vector3(1.f, 1.f, 1.f), 2.f);
		drawText(300.f, 340.f, "Exit", Vector3(1.f, 1.f, 1.f), 2.f);

		if (state == LEVEL1) {
			drawText(250.f, 300.f, ">>>", Vector3(1.f, 1.f, 1.f), 2.f);
		}
		if (state == LEVEL2) {

			drawText(250.f, 320.f, ">>>", Vector3(1.f, 1.f, 1.f), 2.f);
		}
		if (state == BACKTOMENU) {

			drawText(250.f, 340.f, ">>>", Vector3(1.f, 1.f, 1.f), 2.f);
		}
		glDisable(GL_BLEND);
		glEnable(GL_DEPTH_TEST);
	}


	void OnRender()override
	{
		GameStage::OnRender();

		m_PlayStage->OnRender();
	};

};

#endif /* SELECT_LEVEL_STAGE_H */