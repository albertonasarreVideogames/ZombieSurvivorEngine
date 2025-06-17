
#pragma once

#ifndef STATIC_OBJECT_RENDERER_H
#define STATIC_OBJECT_RENDERER_H 1

#include "framework.h"
#include "mesh.h"
#include "shader.h"
#include "camera.h"

class StaticObjectRenderer
{
	struct StaticObjectList
	{
		Mesh* mesh;
		std::vector<Matrix44> matrices;
	};
	Shader* m_StaticShader;
	std::vector<StaticObjectList> m_Objects;

	struct ZombieInfo
	{
		Shader* shader;
		Mesh* mesh;
		std::vector<Matrix44> WorldMatrices;
		std::vector<Matrix44> RotationMatrices;
		std::vector<Vector4> Colors;
	};
	ZombieInfo m_Zombies;

public:
	static StaticObjectRenderer* instance;

	void SetObjectShader(Shader* shader)
	{
		m_StaticShader = shader;
	}

	void SetZombieInfo(Mesh* mesh, Shader* shader)
	{
		m_Zombies.mesh = mesh;
		m_Zombies.shader = shader;
	}

	void StartFrame()
	{
		
		for (auto& sol : m_Objects)
			sol.matrices.clear();
		m_Zombies.WorldMatrices.clear();
		m_Zombies.RotationMatrices.clear();
		m_Zombies.Colors.clear();
		
	}

	void AddZombie(const Matrix44& worldMatrix, const Matrix44& rot, const Vector4& color)
	{
		m_Zombies.WorldMatrices.push_back(worldMatrix);
		m_Zombies.RotationMatrices.push_back(rot);
		m_Zombies.Colors.push_back(color);
	}

	void AddVisibleObject(Mesh* mesh, const Matrix44& worldMatrix)
	{
		const auto findIT = std::find_if(m_Objects.begin(), m_Objects.end(), 
			[&mesh](const StaticObjectList& it)
			{
				return it.mesh == mesh;
			});
		if (findIT != m_Objects.end())
		{
			findIT->matrices.push_back(worldMatrix);
		}
		else
		{
			m_Objects.push_back(StaticObjectList());
			auto& sol = m_Objects.back();
			sol.mesh = mesh;
			sol.matrices.push_back(worldMatrix);
		}
	}

	void Render(float time)
	{
		m_StaticShader->enable();

		auto* camera = Camera::current;

		m_StaticShader->setUniform("u_viewprojection", camera->viewprojection_matrix);
		m_StaticShader->setUniform("u_time", time);
		m_StaticShader->setUniform("u_camera_position", camera->eye);
		m_StaticShader->setUniform("u_light_direction", Vector3(0.3f, 0.6f, 0.2f));
		m_StaticShader->setUniform("u_color", Vector4(1.f, 1.f, 1.f, 1.f));

		for (auto& sol : m_Objects)
		{
			if (sol.matrices.empty())
				continue;

			sol.mesh->renderInstanced(GL_TRIANGLES, sol.matrices.data(), sol.matrices.size());
		}

		m_StaticShader->disable();

		RenderZombies(time);
	}

private:
	void RenderZombies(float time)
	{
		if (m_Zombies.WorldMatrices.empty())
			return;

		static GLuint instancedBufferID_WorldMatrix = 0;
		static GLuint instancedBufferID_Rotation = 0;
		static GLuint instancedBufferID_Color = 0;

		auto* camera = Camera::current;
		auto& player = Game::instance->player;

		Matrix44 rotation_matrix_torse;
		Matrix44 rotation_matrix_leg_right;
		Matrix44 rotation_matrix_leg_left;
		Matrix44 rotation_matrix_head;

		float sinus = sin(Game::instance->time_pause * float(DEG2RAD) * 100.f);

		float animtime = Game::instance->IsDay() ? 0.f : Game::instance->time_pause;

		rotation_matrix_torse.rotate(0.5f * sin(animtime * 100.f * float(DEG2RAD)), Vector3(0.f, 1.f, 0.f));
		rotation_matrix_head.rotate(0.3f * sin(animtime * -100.f * float(DEG2RAD)), Vector3(0.f, 1.f, 0.f));
		rotation_matrix_leg_right.rotate(0.075f * sin(animtime * 90.f * float(DEG2RAD)), Vector3(0.f, 1.f, 0.f));
		rotation_matrix_leg_left.rotate(0.075f * sin(animtime * -90.f * float(DEG2RAD)), Vector3(0.f, 1.f, 0.f));

		m_Zombies.shader->enable();

		if (instancedBufferID_WorldMatrix == 0)
		{
			glGenBuffersARB(1, &instancedBufferID_WorldMatrix);
			glGenBuffersARB(1, &instancedBufferID_Rotation);
			glGenBuffersARB(1, &instancedBufferID_Color);
		}

		//for (size_t i = 0; i < m_Zombies.WorldMatrices.size(); ++i)
		//{
		glBindBufferARB(GL_ARRAY_BUFFER_ARB, instancedBufferID_WorldMatrix);
		glBufferDataARB(GL_ARRAY_BUFFER_ARB, m_Zombies.WorldMatrices.size() * sizeof(Matrix44),
			m_Zombies.WorldMatrices.data(), GL_STREAM_DRAW_ARB);

		glBindBufferARB(GL_ARRAY_BUFFER_ARB, instancedBufferID_Rotation);
		glBufferDataARB(GL_ARRAY_BUFFER_ARB, m_Zombies.RotationMatrices.size() * sizeof(Matrix44),
			m_Zombies.RotationMatrices.data(), GL_STREAM_DRAW_ARB);

		glBindBufferARB(GL_ARRAY_BUFFER_ARB, instancedBufferID_Color);
		glBufferDataARB(GL_ARRAY_BUFFER_ARB, m_Zombies.Colors.size() * sizeof(Vector4),
			m_Zombies.Colors.data(), GL_STREAM_DRAW_ARB);

		int attribLocation_WorldMatrix = m_Zombies.shader->getAttribLocation("u_model");
		assert(attribLocation_WorldMatrix != -1 && "shader must have attribute mat4 u_model (not a uniform)");
		if (attribLocation_WorldMatrix == -1)
			return; //this shader doesnt support instanced model

		int attribLocation_Rotation = m_Zombies.shader->getAttribLocation("u_model_rotate");
		assert(attribLocation_Rotation != -1 && "shader must have attribute mat4 u_model_rotate (not a uniform)");
		if (attribLocation_Rotation == -1)
			return; //this shader doesnt support instanced model

		int attribLocation_Color = m_Zombies.shader->getAttribLocation("u_color");
		assert(attribLocation_Color != -1 && "shader must have attribute mat4 u_color (not a uniform)");
		if (attribLocation_Color == -1)
			return; //this shader doesnt support instanced model

		for (int k = 0; k < 4; ++k)
		{
			glEnableVertexAttribArray(attribLocation_WorldMatrix + k);
			int offset = sizeof(float) * 4 * k;
			const Uint8* addr = (Uint8*)offset;
			glVertexAttribPointer(attribLocation_WorldMatrix + k, 4, GL_FLOAT, false, sizeof(Matrix44), addr);
			glVertexAttribDivisor(attribLocation_WorldMatrix + k, 1); // This makes it instanced!
		}

		for (int k = 0; k < 4; ++k)
		{
			glEnableVertexAttribArray(attribLocation_Rotation + k);
			int offset = sizeof(float) * 4 * k;
			const Uint8* addr = (Uint8*)offset;
			glVertexAttribPointer(attribLocation_Rotation + k, 4, GL_FLOAT, false, sizeof(Matrix44), addr);
			glVertexAttribDivisor(attribLocation_Rotation + k, 1); // This makes it instanced!
		}

		{
			glEnableVertexAttribArray(attribLocation_Color);
			glVertexAttribPointer(attribLocation_Color, 4, GL_FLOAT, false, sizeof(Vector4), (const void*)0);
			glVertexAttribDivisor(attribLocation_Color, 1); // This makes it instanced!
		}

		m_Zombies.mesh->render(GL_TRIANGLES, 0, m_Zombies.WorldMatrices.size());

		for (int k = 0; k < 4; ++k)
		{
			glDisableVertexAttribArray(attribLocation_WorldMatrix + k);
			glVertexAttribDivisor(attribLocation_WorldMatrix + k, 0);
		}

		for (int k = 0; k < 4; ++k)
		{
			glDisableVertexAttribArray(attribLocation_Rotation + k);
			glVertexAttribDivisor(attribLocation_Rotation + k, 0);
		}

		{
			glDisableVertexAttribArray(attribLocation_Color);
			glVertexAttribDivisor(attribLocation_Color, 0);
		}
		//}

		m_Zombies.shader->disable();
	}
};



#endif /* STATIC_OBJECT_RENDERER_H */