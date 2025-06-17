
#include "TileMap.h"
#include "shader.h"
#include "camera.h"
#include "game.h"

Shader* TileMap::gTileShader = nullptr;
//Texture* TileMap::gTileTexture = nullptr;
bool TileMap::gShowTiles = false;

Mesh* TileMap::CreateTileMesh()
{
	Mesh* mesh = nullptr;
	
	char buffer[16];
	const auto sz = snprintf(&buffer[0], ARRAYSIZE(buffer), "tile%d", m_TileSize);
	std::string tileMeshName(buffer);
	const auto findIT = Mesh::sMeshesLoaded.find(tileMeshName);
	if (findIT != Mesh::sMeshesLoaded.end())
	{
		mesh = findIT->second;
	}
	else
	{
		mesh = new Mesh();
		mesh->createPlane(float(m_TileSize) * 0.5f);
		mesh->registerMesh(tileMeshName);
		mesh->uploadToVRAM();
	}
	return mesh;
}

Entity* TileMap::CreateMapEntity(Entity* parent, const Vector3& mapWorldPosition)
{
	char idx = 'A' - 1;
	Entity* entity = nullptr;
	char buffer[32];
	while (entity == nullptr)
	{
		int sz = -1;
		if (idx < 'A')
		{
			sz = snprintf(&buffer[0], ARRAYSIZE(buffer), "map_%d_%d_%d",
				(int)mapWorldPosition.x, (int)mapWorldPosition.y, (int)mapWorldPosition.z);
		}
		else
		{
			sz = snprintf(&buffer[0], ARRAYSIZE(buffer), "map_%d_%d_%d_%c",
				(int)mapWorldPosition.x, (int)mapWorldPosition.y, (int)mapWorldPosition.z, idx);
		}
		std::string mapName(buffer);

		entity = parent->AddChild(mapName); //Returns nullptr if already exists.
		++idx;
	}
	return entity;
}

BoundingBox TileMap::GetLocalAABB() const
{
	return BoundingBox{ {0.f, 0.f, 0.f}, {float(m_TileSize) * float(m_HorizontalTiles), 0.f, float(m_TileSize) * float(m_VerticalTiles)} };
}

void TileMap::GenerateTiles()
{
	char buffer[64];
	for (unsigned int y = 0; y < m_VerticalTiles; ++y)
	{
		for (unsigned int x = 0; x < m_HorizontalTiles; ++x)
		{
			const auto sz = snprintf(&buffer[0], ARRAYSIZE(buffer), "%s_tile_%d_%d",
				m_MapEntity->GetName().c_str(), x, y);
			std::string tileName(buffer);
			auto* tile = m_MapEntity->AddChild(tileName);
			assert(tile != nullptr); // Should never happend, as the map name is unique
			tile->SetPosition(Vector3(float(x * m_TileSize) + float(m_TileSize) * 0.5f, 0.f, float(y * m_TileSize) + float(m_TileSize) * 0.5f));
			tile->SetLayer(tile->GetLayer() | EL_TILE);
		}
	}
	UpdateTileWorldMatrix();
}

void TileMap::UpdateTileWorldMatrix()
{
	const auto& childs = m_MapEntity->GetChilds();
	for (size_t i = 0; i < m_TileWorldMatrix.size(); ++i)
	{
		memcpy(m_TileWorldMatrix[i].m, childs[i]->GetWorldMatrix().m, sizeof(Matrix44));
	}
}

TileMap::TileMap(Entity* parent, const Vector3& mapWorldPosition, unsigned int tileSize, unsigned int horizontalTiles,
	unsigned int verticalTiles)
	:m_HorizontalTiles(horizontalTiles)
	,m_VerticalTiles(verticalTiles)
	,m_TileSize(tileSize)
	,m_MapEntity(nullptr)
	,m_TileMesh(nullptr)
	,m_MapWidth(tileSize * horizontalTiles)
	,m_MapHeight(tileSize * verticalTiles)
	,m_InvMapWidth(1.f / m_MapWidth)
	,m_InvMapHeight(1.f / m_MapHeight)
{
	assert(parent != nullptr && "A tilemap must have a valid parent.");

	// Create and register one plane for this size
	m_TileMesh = CreateTileMesh();

	if(gTileShader == nullptr)
		gTileShader = Shader::Get("data/shaders/InstancedTile.vs", "data/shaders/InstancedTile.fs");

	//if (gTileTexture == nullptr)
	//	gTileTexture = Texture::Get("data/tileTexture.png");

	// Create a unique map entity
	m_MapEntity = CreateMapEntity(parent, mapWorldPosition);

	// Place the map on the scene
	const auto parentPos = parent->GetWorldPosition();
	m_MapEntity->SetPosition(mapWorldPosition - parentPos);
	m_InvMapWorld = m_MapEntity->GetWorldMatrix();
	m_InvMapWorld.inverse();
	m_MapAABB = transformBoundingBox(m_MapEntity->GetWorldMatrix(), GetLocalAABB());

	m_TileWorldMatrix.resize(m_HorizontalTiles * m_VerticalTiles);
	GenerateTiles();
}

struct MapFileDef
{
	char MagicNumber[3] = { 'M', 'A', 'P' };
	Vector2u MapPosition = { 0, 0 };
	unsigned int HorizontalTiles = 0;
	unsigned int VerticalTiles = 0;
	unsigned short TileSize = 0;

	struct TileDef
	{
		unsigned int ChildCount = 0;
		struct ChildDef
		{
			char ChildName[128] = { 0 };
			char MeshName[MAX_PATH] = { 0 };
			char TextureName[MAX_PATH] = { 0 };
			char VertexShaderName[MAX_PATH] = { 0 };
			char FragmentShaderName[MAX_PATH] = { 0 };
			char Material[MAX_PATH] = { 0 };
			bool IsMaterial = false;
			Vector3 Color = {};
			size_t Layer = 0;
			Matrix44 LocalMatrix = {};
		};
	};
};

TileMap* TileMap::LoadFromFile(Entity* parent, const std::string& filePath)
{
	assert(parent != nullptr);
	assert(!filePath.empty() && "must provide a file name");

	MapFileDef mapdef;
	memset(&mapdef, 0, sizeof(mapdef));
	FILE* file = fopen(filePath.c_str(), "rb");
	assert(file != nullptr && "file does not exist?");
	fread(&mapdef, sizeof(mapdef), 1, file);
	char MagicNumber[3] = { 'M', 'A', 'P' };
	if(memcmp(&mapdef.MagicNumber[0], &MagicNumber[0], sizeof(MagicNumber)) != 0)
		assert(false && "the given file was not a map");
	auto* map = new TileMap(parent, Vector3((float)mapdef.MapPosition.x, 0.f, (float)mapdef.MapPosition.y),
		mapdef.TileSize, mapdef.HorizontalTiles, mapdef.VerticalTiles);

	const auto& childs = map->GetMapEntity()->GetChilds();
	for (unsigned int i = 0; i < childs.size(); ++i)
	{
		auto* tile = childs[i];
		unsigned int childCount = 0;
		fread(&childCount, sizeof(childCount), 1, file);
		for (unsigned int j = 0; j < childCount; ++j)
		{
			MapFileDef::TileDef::ChildDef childef;
			memset(&childef, 0, sizeof(childef));
			fread(&childef, sizeof(childef), 1, file);
			EntityMesh* child = nullptr;
			if (childef.TextureName[0] == 0)
			{
				child = tile->AddChild<EntityMesh>(childef.ChildName, Mesh::Get(childef.MeshName),
					Shader::Get(childef.VertexShaderName, childef.FragmentShaderName),
					childef.Material, childef.IsMaterial);
			}
			else
			{
				child = tile->AddChild<EntityMesh>(childef.ChildName, Mesh::Get(childef.MeshName),
					Texture::Get(childef.TextureName),
					Shader::Get(childef.VertexShaderName, childef.FragmentShaderName),
					childef.Color);
			}
			assert(child != nullptr && "couldnt create the loaded entity");
			child->SetLayer(childef.Layer);
			child->SetModelMatrix(childef.LocalMatrix);
		}
	}

	return map;
}

bool TileMap::LoadFromFile(const std::string& filePath)
{
	if (filePath.empty())
		return false; // must provide a file name
	MapFileDef mapdef;
	memset(&mapdef, 0, sizeof(mapdef));
	FILE* file = fopen(filePath.c_str(), "rb");
	if (file == nullptr)
		return false; // file does not exist?
	fread(&mapdef, sizeof(mapdef), 1, file);
	char MagicNumber[3] = { 'M', 'A', 'P' };
	if (memcmp(&mapdef.MagicNumber[0], &MagicNumber[0], sizeof(MagicNumber)) != 0)
		return false; // the given file was not a map

	if (m_HorizontalTiles != mapdef.HorizontalTiles
		|| m_VerticalTiles != mapdef.VerticalTiles
		|| m_TileSize != mapdef.TileSize)
		return false; // incompatible loaded map!

	const auto& childs = m_MapEntity->GetChilds();
	for (unsigned int i = 0; i < childs.size(); ++i)
	{
		auto* tile = childs[i];
		unsigned int childCount = 0;
		fread(&childCount, sizeof(childCount), 1, file);
		for (unsigned int j = 0; j < childCount; ++j)
		{
			MapFileDef::TileDef::ChildDef childef;
			memset(&childef, 0, sizeof(childef));
			fread(&childef, sizeof(childef), 1, file);
			EntityMesh* child = nullptr;
			std::string childName(childef.ChildName);
			char buffer[64];
			size_t tries = 0;
			while (child == nullptr)
			{
				if (childef.TextureName[0] == 0)
				{
					child = tile->AddChild<EntityMesh>(childef.ChildName, Mesh::Get(childef.MeshName),
						Shader::Get(childef.VertexShaderName, childef.FragmentShaderName),
						childef.Material, childef.IsMaterial);
				}
				else
				{
					child = tile->AddChild<EntityMesh>(childef.ChildName, Mesh::Get(childef.MeshName),
						Texture::Get(childef.TextureName),
						Shader::Get(childef.VertexShaderName, childef.FragmentShaderName),
						childef.Color);
				}
				if (child == nullptr)
				{
					snprintf(&buffer[0], ARRAYSIZE(buffer), "%s_%d", childef.ChildName, tries);
					childName.assign(buffer);
					buffer[0] = 0;
				}
				++tries;
			}
			assert(child != nullptr && "couldnt create the loaded entity");
			child->SetLayer(childef.Layer);
			child->SetModelMatrix(childef.LocalMatrix);
		}
	}
	return true;
}

void TileMap::SaveMapToFile(const std::string& filePath) const
{
	MapFileDef mapdef;
	const auto mapPos = m_MapEntity->GetWorldPosition();
	mapdef.MapPosition.set((unsigned int)mapPos.x, (unsigned int)mapPos.z);
	mapdef.HorizontalTiles = m_HorizontalTiles;
	mapdef.VerticalTiles = m_VerticalTiles;
	mapdef.TileSize = m_TileSize;

	FILE* file = fopen(filePath.c_str(), "wb+");
	assert(file != nullptr && "file does not exist?");

	fwrite(&mapdef, sizeof(mapdef), 1, file);
	for (unsigned int tileIdx = 0; tileIdx < m_HorizontalTiles * m_VerticalTiles; ++tileIdx)
	{
		auto* tile = m_MapEntity->GetChilds()[tileIdx];
		const auto& childs = tile->GetChilds();
		unsigned int childCount = 0;
		for (auto* child : childs) if ((child->GetLayer() & EL_MESH) != 0) ++childCount;
		fwrite(&childCount, sizeof(childCount), 1, file);

		for (auto* child : childs)
		{
			if ((child->GetLayer() & EL_MESH) == 0)
				continue;
			auto* emesh = (EntityMesh*)child;
			MapFileDef::TileDef::ChildDef childDef;
			
			strncpy(&childDef.ChildName[0], emesh->GetName().c_str(), emesh->GetName().size());
			strncpy(&childDef.MeshName[0], emesh->mesh->name.c_str(), emesh->mesh->name.size());
			
			if (emesh->texture != nullptr)
				strncpy(&childDef.TextureName[0], emesh->texture->filename.c_str(), emesh->texture->filename.size());
			else
				memset(&childDef.TextureName[0], 0, MAX_PATH * sizeof(char));

			if (emesh->shader != nullptr)
			{
				strncpy(&childDef.VertexShaderName[0], emesh->shader->GetVertexShaderName().c_str(), emesh->shader->GetVertexShaderName().size());
				strncpy(&childDef.FragmentShaderName[0], emesh->shader->GetFragmentShaderName().c_str(), emesh->shader->GetFragmentShaderName().size());
			}
			else
			{
				memset(&childDef.VertexShaderName[0], 0, MAX_PATH * sizeof(char));
				memset(&childDef.FragmentShaderName[0], 0, MAX_PATH * sizeof(char));
			}
			childDef.Color = emesh->color;

			strncpy(&childDef.Material[0], emesh->material.c_str(), emesh->material.size() * sizeof(std::string::value_type));
			childDef.IsMaterial = emesh->is_mtl;
			childDef.Layer = emesh->GetLayer();
			memcpy(&childDef.LocalMatrix, &emesh->GetModelMatrix(), sizeof(Matrix44));
			fwrite(&childDef, sizeof(childDef), 1, file);
		}
	}
	fclose(file);
}

void TileMap::Clear() const
{
	for (auto* tile : m_MapEntity->GetChilds())
	{
		for (auto* child : tile->GetChilds())
		{
			tile->DestroyChild(child->GetName(), false);
		}
	}
}

unsigned int TileMap::GetHorizontalTiles() const
{
	return m_HorizontalTiles;
}

unsigned int TileMap::GetVerticalTiles() const
{
	return m_VerticalTiles;
}

unsigned int TileMap::GetTileSize() const
{
	return m_TileSize;
}

unsigned int TileMap::GetMapWidth() const
{
	return m_MapWidth;
}

unsigned int TileMap::GetMapHeight() const
{
	return m_MapHeight;
}

size_t TileMap::World2TileIdx(const Vector3& worldPosition) const
{
	const auto wpos = m_MapEntity->GetWorldPosition();
	if (worldPosition.x < wpos.x || worldPosition.z < wpos.z
		|| worldPosition.x > wpos.x + m_MapWidth
		|| worldPosition.z > wpos.z + m_MapHeight)
		return (size_t)-1;
	const auto localPosition = m_InvMapWorld * worldPosition;
	const auto horzIdx = (size_t)floorf((localPosition.x * m_InvMapWidth) * m_HorizontalTiles);
	const auto vertIdx = (size_t)floorf((localPosition.z * m_InvMapHeight) * m_VerticalTiles);
	return m_HorizontalTiles * vertIdx + horzIdx;
}

Entity* TileMap::GetTile(size_t tileIdx)
{
	if (tileIdx > m_MapEntity->GetChilds().size())
		return nullptr;
	return m_MapEntity->GetChilds()[tileIdx];
}

const Entity* TileMap::GetTile(size_t tileIdx) const
{
	if (tileIdx > m_MapEntity->GetChilds().size())
		return nullptr;
	return m_MapEntity->GetChilds()[tileIdx];
}

Entity* TileMap::GetMapEntity() const
{
	return m_MapEntity;
}

const std::vector<Entity*>& TileMap::GetTiles() const
{
	return m_MapEntity->GetChilds();
}

const BoundingBox& TileMap::GetMapBox() const
{
	return m_MapAABB;
}

void TileMap::Update(double elapsed_seconds)
{
	bool wasDirty = m_MapEntity->_GetWorldDirty();
	m_MapEntity->Update(elapsed_seconds);
	if (wasDirty)
	{
		m_InvMapWorld = m_MapEntity->GetWorldMatrix();
		m_InvMapWorld.inverse();
		m_MapAABB = transformBoundingBox(m_MapEntity->GetWorldMatrix(), GetLocalAABB());
		UpdateTileWorldMatrix();
	}
}

void TileMap::Render()
{
	if (gShowTiles)
	{
		if (Camera::current->testBoxInFrustum(m_MapAABB.center, m_MapAABB.halfsize) == CLIP_OUTSIDE)
			return;
		glEnable(GL_BLEND);
		gTileShader->enable();
		//gTileShader->setUniform("u_texture", gTileTexture);
		gTileShader->setUniform("u_maxpos", Game::instance->Maps.back()->m_MapEntity->GetChilds().back()->GetWorldPosition());
		gTileShader->setUniform("u_viewprojection", Camera::current->viewprojection_matrix);
		m_TileMesh->renderInstanced(GL_TRIANGLES, m_TileWorldMatrix.data(), m_TileWorldMatrix.size());
		gTileShader->disable();
		glDisable(GL_BLEND);
	}
	
	//m_MapEntity->Render();
}