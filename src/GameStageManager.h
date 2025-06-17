
#pragma once

#ifndef GAME_STAGE_MANAGER_H
#define GAME_STAGE_MANAGER_H 1

#include "includes.h"
#include "EntityManager.h"

/*
	GameStage holds a pointer to an Entity that will be the Root of that stage, enabling fast
	GameStage change and only render and update what it is in the current stage.
*/
class GameStage
{
protected:
	std::string m_StateName;
	Entity* m_StageRootEntity;

public:
	GameStage(const std::string& stageName)
		:m_StateName(stageName)
		,m_StageRootEntity(nullptr)
	{
		m_StageRootEntity = EntityManager::instance->GetRoot()->AddChild(m_StateName + "_ROOT");
		m_StageRootEntity->SetVisible(false);
		m_StageRootEntity->SetEnabled(false);
	}

	virtual ~GameStage()
	{
		m_StageRootEntity->SetParent(nullptr, true);
		EntityManager::instance->_RemoveEntity(m_StageRootEntity->GetName());
	}

	// GameStage starting
	virtual void OnGSInit()
	{
#if GAME_DEBUG
		std::cout << "Starting GameState: " + m_StateName << std::endl;
#endif
		m_StageRootEntity->SetVisible(true);
		m_StageRootEntity->SetEnabled(true);
	}

	// GameStage stopping
	virtual void OnGSStop()
	{
#if GAME_DEBUG
		std::cout << "Stopping GameState: " + m_StateName << std::endl;
#endif
		m_StageRootEntity->SetVisible(false);
		m_StageRootEntity->SetEnabled(false);
	}

	virtual void OnUpdate(double seconds_elapsed)
	{
		/* StageRootEntity will auto update if Enabled */
	}

	virtual void OnRender()
	{
		/* StageRootEntity will auto render if visible */
	}

	virtual void OnRenderGUI()
	{
		/*if(!Game::instance->pause){

		

		}*/

	}

	const std::string& GetStateName()const { return m_StateName; }

	Entity* GetRootEntity()const { return m_StageRootEntity; }
};

class GameStageManager
{
	
	GameStage* m_CurrentStage = nullptr;
	GameStage* m_PreviousStage = nullptr;

public:
	static GameStageManager* instance;
	std::unordered_map<std::string, GameStage*> m_GameStages;

	GameStageManager() = default;
	~GameStageManager()
	{
		for (auto& stage : m_GameStages)
		{
			delete stage.second;
		}
	}

	void RegisterStage(GameStage* stage)
	{
		if (stage == nullptr)
		{
			std::cout << "Warning - trying to register a nullptr GameStage!" << std::endl;
			return;
		}
#if GAME_DEBUG
		const auto findIT = m_GameStages.find(stage->GetStateName());
		if (findIT != m_GameStages.end())
		{
			std::cout << "Warning - trying to register an already registered stage: " + stage->GetStateName() << std::endl;
		}
#endif
		m_GameStages.insert_or_assign(stage->GetStateName(), stage);
	}

	void UnregisterStage(const std::string& stageName)
	{
		const auto findIT = m_GameStages.find(stageName);
		if (findIT != m_GameStages.end())
		{
			delete findIT->second;
			m_GameStages.erase(findIT);
		}
	}

	void ChangeStage(const std::string& stageName)
	{
		const auto findIT = m_GameStages.find(stageName);
		if (findIT == m_GameStages.end())
		{
			std::cout << "Error - trying to change into an unregistered GameStage: " + stageName << std::endl;
			return;
		}

		auto* state = findIT->second;
		if (m_CurrentStage != nullptr)
		{
			m_CurrentStage->OnGSStop();
		}
		m_PreviousStage = m_CurrentStage;
		m_CurrentStage = state;
		m_CurrentStage->OnGSInit();
	}

	GameStage* GetCurrentState()const
	{
		return m_CurrentStage;
	}

	GameStage* GetPreviousState()const
	{
		return m_PreviousStage;
	}

	void Update(double seconds_elapsed)
	{
		assert(m_CurrentStage != nullptr);
		m_CurrentStage->OnUpdate(seconds_elapsed);
	}

	void Render()
	{
		assert(m_CurrentStage != nullptr);
		m_CurrentStage->OnRender();
	}

	void RenderGUI()
	{
		assert(m_CurrentStage != nullptr);
		m_CurrentStage->OnRenderGUI();
	}

	GameStage* GetStage(const std::string& stageName)const
	{
		const auto findIT = m_GameStages.find(stageName);
		if (findIT == m_GameStages.end())
			return nullptr;
		return findIT->second;
	}
};

#endif /* GAME_STAGE_MANAGER_H */