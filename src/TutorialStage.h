#pragma once

#ifndef TUTORIAL_STAGE_H
#define TUTORIAL_STAGE_H 1

#include "GameStageManager.h"
#include "utils.h"

class TutorialStage : public GameStage
{
public:
	TutorialStage()
		:GameStage("TutorialStage")
	{

	}

	~TutorialStage() = default;

	void OnGSInit()override
	{
		GameStage::OnGSInit();
	}

	void OnGSStop()override
	{
		GameStage::OnGSStop();

	}

	void OnUpdate(double seconds_elapsed)override
	{
		GameStage::OnUpdate(seconds_elapsed);

		if (Input::wasKeyPressed(SDL_SCANCODE_RETURN))
		{
			Game::instance->pause = false;

			Audio* test = Audio::Get("data/Audio/Lounge_Game1.wav", true);

			PlayStage::channel_music = test->play(0.2f);
			GameStageManager::instance->ChangeStage("PlayStage");
		}
	}

	void OnRender()override
	{
		GameStage::OnRender();

		drawText(300.f, 250.f, "[WASD]: Movement", Vector3(1.f, 1.f, 1.f), 2.f);
		drawText(300.f, 270.f, "[LEFT_MOUSE]: Shoot", Vector3(1.f, 1.f, 1.f), 2.f);
		drawText(300.f, 290.f, "[RIGHT_MOUSE]: Place Fence", Vector3(1.f, 1.f, 1.f), 2.f);
		drawText(300.f, 320.f, "[ENTER]: StartPlaying", Vector3(1.f, 1.f, 1.f), 2.f);
	};

};

#endif /* TUTORIAL_STAGE_H */