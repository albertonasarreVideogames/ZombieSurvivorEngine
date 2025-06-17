#pragma once

#ifndef ENTITY_WEAPON_H
#define ENTITY_WEAPON_H 1

#include "EntityMesh.h"
#include "game.h"

class EntityWeapon : public EntityMesh
{
public:
	Vector3 position_before;
	EntityWeapon()
		:EntityMesh(Mesh::Get("data/FinalMeshes/itemWeapon.obj"),
			Shader::Get("data/shaders/basic_weapon.vs", "data/shaders/flat.fs"),
			"data/FinalMeshes/itemWeapon.mtl")
	{
		
		m_ModelMatrix.setTranslation(0.5f, -0.5f, -0.1f);
		m_ModelMatrix.rotate(float(DEG2RAD * -15.0), Vector3(0.f, 1.f, 0.f));
		m_ModelMatrix.rotate(float(DEG2RAD * -5.0), Vector3(1.f, 0.f, 0.f));
		m_ModelMatrix.scale(0.5f, 0.5f, 0.5f);
	}

	void Render()override
	{
		Entity::Render();

		Camera* camera = Camera::current;
		shader->enable();

		shader->setUniform("u_model", m_ModelMatrix);
		shader->setUniform("u_color", Vector4(1.f, 1.f, 1.f, 1.f));
		shader->setUniform("u_time", Game::instance->time_pause); // Para el nuevo shader
		shader->setUniform("u_camera_position", camera->eye); // Para el nuevo shader
		shader->setUniform("u_light_direction", Vector3(0.3f, 0.6f, 0.2f)); // Para el nuevo shader

		mesh->render(GL_TRIANGLES, 0);
		shader->disable();
	}
};

#endif /* ENTITY_WEAPON_H */