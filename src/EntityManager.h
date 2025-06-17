#pragma once

#ifndef ENTITY_MANAGER_H
#define ENTITY_MANAGER_H 1

#include "includes.h"
#include "Entity.h"
#include "EntityMesh.h"

class EntityManager
{
	std::unordered_map<std::string, Entity*> m_Entities;
	Entity* m_Root;

	bool IsEntityRegistered(const std::string& name)const;
	void RegisterEntity(Entity* entity);

public:
	EntityManager();
	~EntityManager();

	Entity* GetRoot()const;

	void Update(double elapsed_seconds);

	void Render();
	
	void _RemoveEntity(const std::string& name);

	static EntityManager* instance;

	friend class Entity;
};

#endif /* ENTITY_MANAGER_H */