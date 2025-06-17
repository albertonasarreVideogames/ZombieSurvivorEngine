#pragma once

#ifndef ENTITY_WALL_H
#define ENTITY_WALL_H 1

#include "includes.h"
#include "EntityMesh.h"
#include "game.h"
#include "StaticObjectRenderer.h"

constexpr int WallInitialHealth = 200;
constexpr size_t InvFrames = 10;
constexpr int ZombieWallDamage = 10;

class EntityWall : public EntityMesh
{
	int m_Health;
	size_t m_TouchedFrame;
	Vector4 m_Color;

public:
	EntityWall()
		:EntityMesh(Mesh::Get("data/FinalMeshes/Vallas_y_madera/ironFence.obj"),
			Shader::Get("data/shaders/basic.vs", "data/shaders/flat.fs"),
			"data/FinalMeshes/Vallas_y_madera/ironFence.mtl")
		,m_Health(WallInitialHealth)
		, m_TouchedFrame((size_t)-1)
		,m_Color(1.f, 1.f, 1.f, 1.f)
	{
		SetLayer(GetLayer() | EL_VALLA);
	}

	void OnZombieTouch()
	{
		auto* game = Game::instance;
		if (m_TouchedFrame == (size_t)-1 || game->CurrentFrame > (m_TouchedFrame + InvFrames))
		{
			m_TouchedFrame = game->CurrentFrame;
			m_Health -= ZombieWallDamage;
			m_Color.set(1.f, 0.f, 0.f, 1.f);
		}
	}

	void Update(double elapsed_seconds)override
	{
		if (m_Health <= 0)
		{
			m_Parent->DestroyChild(m_Name, false);
			return;
		}
		auto* game = Game::instance;
		if (m_TouchedFrame != (size_t)-1 && game->CurrentFrame > (m_TouchedFrame + (InvFrames / 2)))
			m_Color.set(1.f, 1.f, 1.f, 1.f);

		EntityMesh::Update(elapsed_seconds);
	}

	void Render()override
	{
		Entity::Render();
	
		shader->enable();

		auto* camera = Camera::current;
		auto time = Game::instance->time_pause;

		shader->setUniform("u_viewprojection", camera->viewprojection_matrix);
		shader->setUniform("u_model", m_WorldMatrix);
		shader->setUniform("u_color", m_Color);
		shader->setUniform("u_time", time);
		shader->setUniform("u_camera_position", camera->eye);
		shader->setUniform("u_light_direction", Vector3(0.3f, 0.6f, 0.2f));

		mesh->render(GL_TRIANGLES, 0);
		shader->disable();
	}
};

#endif /* ENTITY_WALL_H */