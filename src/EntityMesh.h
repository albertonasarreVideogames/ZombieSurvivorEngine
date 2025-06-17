#pragma once

#ifndef ENTITY_MESH_H
#define ENTITY_MESH_H 1

#include "includes.h"
#include "Entity.h"

class EntityMesh : public Entity
{
protected:
	BoundingBox m_WorldAABB;

	void RenderRays();
	void RenderAABB();

	void OnWorldUpdated()override;

public:
	EntityMesh(Mesh* mesh, Texture* texture,Shader* shader, Vector3 color);
	EntityMesh(Mesh* mesh,  Shader* shader, const std::string& material, bool is_mtl=true);
		
	//atributes
	std::map<std::string, Vector3> Map_texture_color;
	Mesh* mesh;
	Texture* texture;
	Shader* shader;
	Vector3 color;
	std::string material;
	bool is_mtl;

	static bool ShowAABB;
	static bool ShowFrontVector;
	static Mesh FrontVectorMesh;

	//metods overwritten
	void Render()override;
	void Update(double elapsed_seconds)override;
	static std::map<std::string, Vector3> Getmtl(const std::string& filename);
};

struct CollisionData
{
	Vector3 Position = Vector3(0.f, 0.f, 0.f);
	Vector3 Target = Vector3(0.f, 0.f, 0.f);
	Entity* SourceEntity = nullptr; // nullptr --> player collisions
	Entity* DynamicObjectsRootEntity = nullptr; // nullptr --> DynamicCollisions off
	bool StaticCollisions = false;
	bool SmoothCollide = false; // instead of stopping the movement, bounce back
	double ElapsedSeconds = 0.0;
};

Vector3 CheckCollision(const CollisionData& data, EntityMesh** collided = nullptr);

#endif /* ENTITY_MESH_H */