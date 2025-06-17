
#include "Entity.h"
#include "EntityManager.h"

Entity::Entity()
	:m_Name("INVALID")
	,m_Parent(nullptr)
	,m_Layer(EL_NODE)
	,m_ModelMatrix(Matrix44::IDENTITY)
	,m_WorldMatrix(Matrix44::IDENTITY)
	,m_WorldDirty(true)
	,m_Enabled(true)
	,m_Visible(true)
{

}

Entity::~Entity()
{
	for (auto* child : m_Childs)
		EntityManager::instance->_RemoveEntity(child->GetName());
}

void Entity::UpdateWorldMatrix()
{
	if (!m_WorldDirty)
		return;

	if (!m_Parent)
	{
		m_WorldMatrix = m_ModelMatrix;
		m_WorldDirty = false;
		return;
	}

	for (auto* child : m_Childs)
		child->m_WorldDirty = true;

	m_Parent->UpdateWorldMatrix();
	
	const auto& parent = m_Parent->GetWorldMatrix();
	m_WorldMatrix = m_ModelMatrix * parent;
	OnWorldUpdated();
	m_WorldDirty = false;
}

bool Entity::IsChildRegistered(const std::string& childName) const
{
	return EntityManager::instance->IsEntityRegistered(childName);
}

void Entity::AddChild(const std::string& name, Entity* child)
{
	child->m_Name.assign(name);
	child->m_Parent = this;
	child->m_WorldDirty = true;
	m_Childs.push_back(child);
	EntityManager::instance->RegisterEntity(child);
}

const std::string& Entity::GetName()const
{
	return m_Name;
}

Entity* Entity::GetParent()const
{
	return m_Parent;
}

Entity* Entity::GetChild(const std::string& childName, bool recursive)const
{
	for (auto* child : m_Childs)
	{
		if (child->GetName() == childName)
			return child;

		if (recursive)
		{
			auto* found = child->GetChild(childName, recursive);
			if (found)
				return found;
		}
	}
	return nullptr;
}

Entity* Entity::RemoveChild(const std::string& childName, bool recursive)
{
	for (auto it = m_Childs.begin(); it != m_Childs.end(); ++it)
	{
		auto* child = *it;

		if (child->GetName() == childName)
		{
			child->SetParent(nullptr, false);
			m_Childs.erase(it);
			return child;
		}

		if (recursive)
		{
			auto* found = child->GetChild(childName, recursive);
			if (found)
				return found;
		}
	}
	return nullptr;
}

void Entity::DestroyChild(const std::string& childName, bool recursive)
{
	for (auto it = m_Childs.begin(); it != m_Childs.end(); ++it)
	{
		auto* child = *it;

		if (child->GetName() == childName)
		{
			for (auto* cchild : child->m_Childs)
			{
				child->DestroyChild(cchild->m_Name, false);
			}
			m_Childs.erase(it);
			EntityManager::instance->_RemoveEntity(child->GetName());
			return;
		}

		if (recursive)
		{
			child->DestroyChild(childName, recursive);
		}
	}
}

const std::vector<Entity*>& Entity::GetChilds()const
{
	return m_Childs;
}

std::vector<Entity*>& Entity::GetChilds()
{
	return m_Childs;
}

Entity* Entity::AddChild(const std::string& childName)
{
	if (IsChildRegistered(childName))
		return nullptr;

	auto* child = (Entity*)malloc(sizeof(Entity));
	new((void*)child)Entity();
	AddChild(childName, child);
	return child;
}

void Entity::AddExistingEntityAsChild(Entity* entity)
{
	assert(entity != nullptr && "Trying to add a nullptr entity");

	bool removeParent = entity->GetParent() != nullptr;
	entity->SetParent(this, removeParent);
	m_Childs.push_back(entity);
}

void Entity::SetParent(Entity* parent, bool removeFromParent)
{
	if (m_Parent != nullptr && removeFromParent)
	{
		const auto findIT = std::find(m_Parent->m_Childs.begin(), m_Parent->m_Childs.end(), this);
		if (findIT != m_Parent->m_Childs.end())
			m_Parent->m_Childs.erase(findIT);
	}
	m_Parent = parent;
	m_WorldDirty = true;
}

void Entity::SetModelMatrix(const Matrix44& model)
{
	m_ModelMatrix = model;
	m_WorldDirty = true;
}

const Matrix44& Entity::GetModelMatrix()const
{
	return m_ModelMatrix;
}

const Matrix44& Entity::GetWorldMatrix()
{
	UpdateWorldMatrix();
	return m_WorldMatrix;
}

const Matrix44 & Entity::_GetWorldMatrix() const
{
	return m_WorldMatrix;
}

void Entity::SetVisible(bool visible)
{
	m_Visible = visible;
}

bool Entity::IsVisible()const
{
	return m_Visible;
}

void Entity::SetEnabled(bool enable)
{
	m_Enabled = enable;
}

bool Entity::IsEnabled()const
{
	return m_Enabled;
}

void Entity::Render()
{
	if (!m_Visible)
		return;

	for (auto* child : m_Childs)
		child->Render();
}

void Entity::Update(double elapsed_seconds)
{
	if (!m_Enabled)
		return;

	UpdateWorldMatrix();

	for (auto* child : m_Childs)
		child->Update(elapsed_seconds);
}

void Entity::SetLayer(size_t layer)
{
	m_Layer = layer;
}

size_t Entity::GetLayer() const
{
	return m_Layer;
}

Vector3 Entity::GetWorldPosition()
{
	UpdateWorldMatrix();
	return m_WorldMatrix.getTranslation();
}

Vector3 Entity::GetLocalPosition() const
{
	return m_ModelMatrix.getTranslation();
}

void Entity::SetPosition(const Vector3& newPosition)
{
	m_ModelMatrix.translate(newPosition.x, newPosition.y, newPosition.z);
	m_WorldDirty = true;
}

void Entity::Move(const Vector3& offset)
{
	if (offset == Vector3::VectorZero)
		return;

	SetPosition(GetLocalPosition() + offset);
	m_WorldDirty = true;
}

void Entity::MoveRelative(const Vector3& offset)
{
	if (offset == Vector3::VectorZero)
		return;

	auto tOffset = m_ModelMatrix.rotateVector(offset);
	SetPosition(GetLocalPosition() + offset);
	m_WorldDirty = true;
}

Vector3 Entity::GetWorldEulerRotation()
{
	UpdateWorldMatrix();
	Vector3 euler;
	m_WorldMatrix.getXYZ(&euler.v[0]);
	return euler;
}

Vector3 Entity::GetLocalEulerRotation() const
{
	Vector3 euler;
	m_ModelMatrix.getXYZ(&euler.v[0]);
	return euler;
}

void Entity::Rotate(float angle, const Vector3& axis)
{
	m_ModelMatrix.rotate(angle, axis);
	m_WorldDirty = true;
}

void Entity::Scale(const Vector3& scale)
{
	m_ModelMatrix.scale(scale.x, scale.y, scale.z);
	m_WorldDirty = true;
}

bool Entity::_GetWorldDirty() const
{
	return m_WorldDirty;
}

void Entity::translate(float x, float y,float z)
{
	m_ModelMatrix.translate(x, y,z);
	m_WorldDirty = true;
}

void Entity::setfront(const Vector3& front)
{
	m_ModelMatrix.setFrontAndOrthonormalize(front);
	m_WorldDirty = true;
}
Vector3 Entity::getfrontVector() {


	return m_ModelMatrix.frontVector();

}

void Entity::SetTranslation(float x, float y, float z)
{
	m_ModelMatrix.setTranslation(x, y, z);
	m_WorldDirty = true;
}