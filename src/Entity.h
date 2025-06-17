#pragma once

#ifndef ENTITY_H
#define ENTITY_H 1

#include "includes.h"
#include "framework.h"
#include "mesh.h"
#include "texture.h"
#include "shader.h"

enum EEntityLayer : size_t
{
	EL_NODE = (1 << 0),
	EL_MESH = (1 << 1),
	EL_TILE = (1 << 2),
	EL_MAP	= (1 << 3),
	EL_WOOD	= (1 << 4),
	EL_VALLA = (1 << 5)
};

class Entity
{
protected:
	std::string m_Name;
	Entity* m_Parent;
	std::vector<Entity*> m_Childs;
	size_t m_Layer;

	Matrix44 m_ModelMatrix;
	Matrix44 m_WorldMatrix;
	bool m_WorldDirty;

	bool m_Enabled;
	bool m_Visible;

	void UpdateWorldMatrix();
	virtual void OnWorldUpdated() {}

private:
	// Same as EntityManager::IsEntityRegisterd, to avoid header conflict
	bool IsChildRegistered(const std::string& childName)const;
	// Adds the child into the child vector and sets the initial parameters
	void AddChild(const std::string& name, Entity* child);

public:
	Entity();

	virtual ~Entity();
	

	const std::string& GetName()const;

	Entity* GetParent()const;

	Entity* GetChild(const std::string& childName, bool recursive = false)const;

	// Child will be removed from child vector and it will be orphan
	Entity* RemoveChild(const std::string& childName, bool recursive = false);
	
	// Child will be removed and destroyed
	void DestroyChild(const std::string& childName, bool recursive = false);

	const std::vector<Entity*>& GetChilds()const;
	std::vector<Entity*>& GetChilds();

	// Creates a new base entity it just holds PRS info, must have a unique name, and sets the parent
	Entity* AddChild(const std::string& childName);

	// Creates a new entity, must have a unique name, and sets the parent
	template<class T, typename... Args>
	T* AddChild(const std::string& childName, Args&&... args)
	{
		static_assert(std::is_base_of<Entity, T>::value,
			"Class must derive from Entity.");

		if (IsChildRegistered(childName))
			return nullptr;

		T* child = (T*)malloc(sizeof(T));
		new((void*)child)T(std::forward<Args>(args)...);
		AddChild(childName, (Entity*)child);
		return child;
	}

	void AddExistingEntityAsChild(Entity* entity);

	void SetParent(Entity* parent, bool removeFromParent = true);

	void SetModelMatrix(const Matrix44& model);
	const Matrix44& GetModelMatrix()const;

	const Matrix44& GetWorldMatrix();
	// Without updating it
	const Matrix44& _GetWorldMatrix()const;

	// Won't render neither their childs
	void SetVisible(bool visible);
	bool IsVisible()const;

	// Won't update neither their childs
	void SetEnabled(bool enable);
	bool IsEnabled()const;

	/*
		This method does nothing more than tell the children to render,
		override it with your render code if you want to render stuff.
		In all subclasses do:
		void Render()override
		{
			Entity::Render(); <-- this will trigger child rendering
		}
	*/
	virtual void Render();

	/*
		This method does nothing more than tell the children to update,
		override it with your update code if you want to do it.
		In all subclasses do:
		void Update(double elapsed_seconds)override
		{
			Entity::Update(elapsed_seconds); <-- this will trigger child update and wolrd matrix update
		}
	*/
	virtual void Update(double elapsed_seconds);

	void SetLayer(size_t layer);
	size_t GetLayer()const;

	Vector3 GetWorldPosition();
	Vector3 GetLocalPosition()const;
	void SetPosition(const Vector3& newPosition);
	void Move(const Vector3& offset);
	void MoveRelative(const Vector3& offset);

	Vector3 GetWorldEulerRotation();
	Vector3 GetLocalEulerRotation()const;
	void Rotate(float angle, const Vector3& axis);
	void translate(float x, float y, float z);
	void Scale(const Vector3& scale);
	void setfront(const Vector3& front);
	Vector3 getfrontVector();
	void SetTranslation(float x, float y, float z);

	bool _GetWorldDirty()const;

	friend class EntityManager;
};

template<class T, typename... Args>
T* AddEntity(Entity* parent, const std::string& name, Args&&... args)
{
	size_t tries = 0;
	T* entity = nullptr;
	char buffer[128];
	while (entity == nullptr)
	{
		std::string tname;
		if (tries < 1)
		{
			tname = name;
		}
		else
		{
			snprintf(&buffer[0], ARRAYSIZE(buffer),
				"%s_%d", name.c_str(), tries - 1);
			tname = std::string(buffer);
		}
		buffer[0] = 0;

		entity = parent->AddChild<T>(tname, args...);
		++tries;
	}
	return entity;
}

#endif /* ENTITY_H */