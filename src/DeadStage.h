#pragma once

#ifndef DEAD_STAGE_H
#define DEAD_STAGE_H 1

#include "GameStageManager.h"
#include "utils.h"



class DeadStage : public GameStage
{
	PlayStage* m_PlayStage;
	bool m_Initialized;
	
	int state;
	enum { PLAY, EXIT };

	int Vector_states_int[2];
	int Score;
	std::string ScoreText;

public:
	DeadStage()
		:GameStage("DeadStage")
		,Vector_states_int{ PLAY,EXIT }
		,state(PLAY)
		, Score(0)
	{
	}

	~DeadStage() = default;

	void OnGSInit()override
	{
		GameStage::OnGSInit();
		if (!m_Initialized)
		{
			m_PlayStage = (PlayStage*)GameStageManager::instance->GetStage("PlayStage");
			m_Initialized = true;
		}
		Score = Game::instance->Score;
		Game::instance->Score = 0;
		char buffer[128];
		snprintf(buffer, ARRAYSIZE(buffer), "Your Score: %d", Score);
		ScoreText.assign(buffer);
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

			if (state == Vector_states_int[0]) { state = Vector_states_int[ARRAYSIZE(Vector_states_int) - 1]; }
			else { state = Vector_states_int[state - 1]; }
		}

		if (Input::wasKeyPressed(SDL_SCANCODE_S) || Input::wasKeyPressed(SDL_SCANCODE_DOWN)) {

			select->play(0.4f);

			if (state == ARRAYSIZE(Vector_states_int) - 1) { state = Vector_states_int[0]; }
			else { state = Vector_states_int[state + 1]; }
		}

		if (Input::wasKeyPressed(SDL_SCANCODE_RETURN)) {

			if (state == PLAY) {

				ZombieManager::instance->Clear();

				m_PlayStage->ClearLevel();
				m_PlayStage->ClearLevel();
				Game::instance->WoodCounter = 0;
				m_PlayStage->LoadLevel(Game::instance->CurrentLevel);
				
				GameStageManager::instance->ChangeStage("MainMenuStage");
			}

			else if (state == EXIT) {

				Game::instance->must_exit = true;
			}

		}
	}

	void OnRender()override
	{
		GameStage::OnRender();
		drawText(300.f, 250.f, "YOU ARE DEAD", Vector3(1.f, 1.f, 1.f), 2.f);
		drawText(300.f, 270.f, ScoreText, Vector3(1.f, 1.f, 1.f), 2.f);
		drawText(300.f, 300.f, "back to main menu", Vector3(1.f, 1.f, 1.f), 2.f);
		drawText(300.f, 320.f, "Exit", Vector3(1.f, 1.f, 1.f), 2.f);

		if (state == PLAY) {
			drawText(250.f, 300.f, ">>>", Vector3(1.f, 1.f, 1.f), 2.f);
		}
		if (state == EXIT) {

			drawText(250.f, 320.f, ">>>", Vector3(1.f, 1.f, 1.f), 2.f);
		}
	};

};

#endif /* DEAD_STAGE_H */