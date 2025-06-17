#pragma once
#include "GameStageManager.h"
#include "utils.h"

enum { PLAY, EXIT,CHOOSE };
int state = PLAY;

int Vector_states_int[3] = { PLAY, EXIT,CHOOSE };
int len_arrays_int = sizeof(Vector_states_int) / sizeof(Vector_states_int[0]);

class MainMenuStage : public GameStage
{
	PlayStage* m_PlayStage;
	bool m_Initialized;
public:
	MainMenuStage()
		:GameStage("MainMenuStage")
		, m_PlayStage(nullptr)
		, m_Initialized(false)
	{

	}

	~MainMenuStage() = default;

	void OnGSInit()override
	{
		GameStage::OnGSInit();
		Texture::Get("data/FinalMeshes/ground3.png");
		if (!m_Initialized)
		{
			m_PlayStage = (PlayStage*)GameStageManager::instance->GetStage("PlayStage");
			m_Initialized = true;
		}
		
		m_PlayStage->floor_mesh = new Mesh();
		m_PlayStage->floor_mesh->createPlane(1024);
		m_PlayStage->Sky_mesh = Mesh::Get("data/FinalMeshes/cielo/dome.ASE");

	}

	void OnGSStop()override
	{
		GameStage::OnGSStop();

	}

	void OnUpdate(double seconds_elapsed)override
	{
		GameStage::OnUpdate(seconds_elapsed);

		Audio* select = Audio::Get("data/Audio/select.wav", false);

		

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

		if (Input::wasKeyPressed(SDL_SCANCODE_RETURN)) {

			if (state == PLAY) {
				GameStageManager::instance->ChangeStage("TutorialStage");
				/*Game::instance->pause = false;

				Audio* test = Audio::Get("data/Audio/Lounge_Game1.wav", true);

				PlayStage::channel_music = test->play(0.2f);
			GameStageManager::instance->ChangeStage("PlayStage");*/
			}

			else if (state == EXIT) {

				Game::instance->must_exit = true;
			}

			else if (state == CHOOSE) {

				GameStageManager::instance->ChangeStage("SelectLevelStage");
			}

		}
	}

	void OnRender()override
	{
		GameStage::OnRender();
		drawText(300.f, 250.f, "Press Enter to select", Vector3(1.f, 1.f, 1.f), 2.f);
		drawText(300.f, 300.f, "Start Game", Vector3(1.f, 1.f, 1.f), 2.f);
		drawText(300.f, 320.f, "Exit", Vector3(1.f, 1.f, 1.f), 2.f);
		drawText(300.f, 340.f, "Choose level", Vector3(1.f, 1.f, 1.f), 2.f);

		if (state == PLAY) {
			drawText(250.f, 300.f, ">>>", Vector3(1.f, 1.f, 1.f), 2.f);
		}
		else if (state == EXIT) {
		
			drawText(250.f, 320.f, ">>>", Vector3(1.f, 1.f, 1.f), 2.f);
		}

		else if (state == CHOOSE) {

			drawText(250.f, 340.f, ">>>", Vector3(1.f, 1.f, 1.f), 2.f);
		}
	};

};