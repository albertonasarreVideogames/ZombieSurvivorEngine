#pragma once
#include "GameStageManager.h"
#include "utils.h"


enum { PLAY_PAUSE, EXIT_PAUSE,CHANGE_PAUSE,RESTART
};
int state_PAUSE = PLAY_PAUSE;

int Vector_states_int_PAUSE[4] = { PLAY_PAUSE, EXIT_PAUSE ,CHANGE_PAUSE,RESTART };
int len_arrays_int_PAUSE = sizeof(Vector_states_int_PAUSE) / sizeof(Vector_states_int_PAUSE[0]);

HCHANNEL PlayStage::channel_music;

class PauseStage : public GameStage
{
	PlayStage* m_PlayStage;
	bool m_Initialized;
public:
	PauseStage()
		:GameStage("PauseStage")
		,m_PlayStage(nullptr)
		,m_Initialized(false)
	{

	}

	~PauseStage() = default;

	void OnGSInit()override
	{
		GameStage::OnGSInit();

		if (!m_Initialized)
		{
			m_PlayStage = (PlayStage*)GameStageManager::instance->GetStage("PlayStage");
			m_Initialized = true;
		}
		m_PlayStage->GetRootEntity()->SetVisible(true);
	}

	void OnGSStop()override
	{
		GameStage::OnGSStop();

	}

	void OnUpdate(double seconds_elapsed)override
	{
		GameStage::OnUpdate(seconds_elapsed);

		Game::instance->pause = true;

		HCHANNEL musica_chenl = PlayStage::channel_music;

		Audio::ChangeVolume(musica_chenl, 0.05f);

		Audio* select = Audio::Get("data/Audio/select.wav", false);

		if (Input::wasKeyPressed(SDL_SCANCODE_W) || Input::wasKeyPressed(SDL_SCANCODE_UP)) {

			select->play(0.4f);

			if (state_PAUSE == Vector_states_int_PAUSE[0]) { state_PAUSE = Vector_states_int_PAUSE[len_arrays_int_PAUSE - 1]; }
			else { state_PAUSE = Vector_states_int_PAUSE[state_PAUSE - 1]; }
		}

		if (Input::wasKeyPressed(SDL_SCANCODE_S) || Input::wasKeyPressed(SDL_SCANCODE_DOWN)) {

			select->play(0.4f);

			if (state_PAUSE == len_arrays_int_PAUSE - 1) { state_PAUSE = Vector_states_int_PAUSE[0]; }
			else { state_PAUSE = Vector_states_int_PAUSE[state_PAUSE + 1]; }
		}

		if (Input::wasKeyPressed(SDL_SCANCODE_RETURN)) {

			if (state_PAUSE == PLAY_PAUSE) {
				Audio::ChangeVolume(musica_chenl, 0.2f);
				Game::instance->pause = false;
				Game::instance->pause_clean = false;
				GameStageManager::instance->ChangeStage("PlayStage");
			}

			else if (state_PAUSE == EXIT_PAUSE) {

				Game::instance->must_exit = true;
			}

			else if (state_PAUSE == CHANGE_PAUSE) {

				HCHANNEL musica_chenl = PlayStage::channel_music;

				Audio::Stop(musica_chenl);

				Game::instance->pause_clean = false;

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
				
				GameStageManager::instance->ChangeStage("SelectLevelStage");
			}

			else if (state_PAUSE == RESTART) {

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
				Game::instance->WoodCounter = 0;

				
				m_PlayStage->ClearLevel();
				

				m_PlayStage->ClearLevel();
				m_PlayStage->LoadLevel(Game::instance->CurrentLevel);


				GameStageManager::instance->ChangeStage("PlayStage");
			}

		}
	}

	void OnRender()override
	{
		GameStage::OnRender();

		m_PlayStage->OnRender();
	};

	void OnRenderGUI()override
	{
		glDisable(GL_DEPTH_TEST);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		drawText(300.f, 250.f, "Pause", Vector3(1.f, 1.f, 1.f), 2.f);
		drawText(300.f, 300.f, "Continue Game", Vector3(1.f, 1.f, 1.f), 2.f);
		drawText(300.f, 320.f, "Exit", Vector3(1.f, 1.f, 1.f), 2.f);
		drawText(300.f, 340.f, "Change Level", Vector3(1.f, 1.f, 1.f), 2.f);
		drawText(300.f, 360.f, "Restart Level", Vector3(1.f, 1.f, 1.f), 2.f);

		if (state_PAUSE == PLAY_PAUSE) {
			drawText(250.f, 300.f, ">>>", Vector3(1.f, 1.f, 1.f), 2.f);
		}
		else if (state_PAUSE == EXIT_PAUSE) {

			drawText(250.f, 320.f, ">>>", Vector3(1.f, 1.f, 1.f), 2.f);
		}
		else if (state_PAUSE == CHANGE_PAUSE) {

			drawText(250.f, 340.f, ">>>", Vector3(1.f, 1.f, 1.f), 2.f);
		}
		else if (state_PAUSE == RESTART) {

			drawText(250.f, 360.f, ">>>", Vector3(1.f, 1.f, 1.f), 2.f);
		}
		glDisable(GL_BLEND);
		glEnable(GL_DEPTH_TEST);
	}
};
