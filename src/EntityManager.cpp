
#include "EntityManager.h"

EntityManager* EntityManager::instance = nullptr;

EntityManager::EntityManager()
	:m_Root(nullptr)
{
	m_Root = new Entity();
	m_Root->m_Name.assign("ROOT");
	m_Entities.insert_or_assign(m_Root->GetName(), m_Root);

	m_Root->m_Parent = nullptr;
	m_Root->m_ModelMatrix.setIdentity();
	m_Root->m_WorldMatrix.setIdentity();

}

bool EntityManager::IsEntityRegistered(const std::string& name) const
{
	return m_Entities.find(name) != m_Entities.end();
}

void EntityManager::_RemoveEntity(const std::string& name)
{
	const auto findIT = m_Entities.find(name);
	assert(findIT != m_Entities.end() && "Trying to delete a not registered Entity"); // Should exist
	delete findIT->second;
	m_Entities.erase(findIT);
}

void EntityManager::RegisterEntity(Entity* entity)
{
	assert(entity != nullptr && "Trying to add a nullptr entity");
	m_Entities.insert_or_assign(entity->GetName(), entity);
}

EntityManager::~EntityManager()
{

}

Entity* EntityManager::GetRoot()const
{
	return m_Root;
}

void EntityManager::Update(double elapsed_seconds)
{
	m_Root->Update(elapsed_seconds);
}

void EntityManager::Render()
{
	m_Root->Render();
}