#include "EntityMesh.h"
#include "camera.h"
#include "texture.h"
#include "extra/textparser.h"
#include "game.h"
#include "StaticObjectRenderer.h"

bool EntityMesh::ShowAABB = false;
bool EntityMesh::ShowFrontVector = false;
Mesh EntityMesh::FrontVectorMesh = {};

static void InitFrontMesh()
{
	if (!EntityMesh::FrontVectorMesh.vertices.empty())
		return;

	EntityMesh::FrontVectorMesh.vertices.resize(2);
	Vector4 color(0.f, 1.f, 0.f, 1.f);
	EntityMesh::FrontVectorMesh.colors.reserve(2);
	EntityMesh::FrontVectorMesh.colors.push_back(color);
	EntityMesh::FrontVectorMesh.colors.push_back(color);
}

void EntityMesh::Render()
{
	Entity::Render();

	Camera* camera = Camera::current;

	if (camera->testBoxInFrustum(m_WorldAABB.center, m_WorldAABB.halfsize) == CLIP_OUTSIDE)
		return;

	StaticObjectRenderer::instance->AddVisibleObject(mesh, m_WorldMatrix);



	RenderAABB();
	RenderRays();
}

void EntityMesh::RenderRays()
{
	if (ShowFrontVector)
	{
		auto front = getfrontVector();
		front = front.normalize();
		auto pos = m_WorldMatrix.getTranslation() + Vector3(0.f, 0.5f, 0.f);
		FrontVectorMesh.vertices[0] = pos;
		FrontVectorMesh.vertices[1] = pos + front;
		RenderGeneralMesh(Matrix44(), &FrontVectorMesh, nullptr, { 0.f, 1.f, 0.f, 1.f }, nullptr, 0, GL_LINES);
	}
}

void EntityMesh::RenderAABB()
{
	if (ShowAABB)
		mesh->renderBounding(m_WorldMatrix, true);
}

void EntityMesh::OnWorldUpdated()
{
	m_WorldAABB = transformBoundingBox(m_WorldMatrix, mesh->box);
}

EntityMesh::EntityMesh(Mesh* mesh, Texture* texture, Shader* shader, Vector3 color)
	:m_WorldAABB(transformBoundingBox(GetWorldMatrix(), mesh->box))
	,mesh(mesh)
	,texture(texture)
	,shader(shader)
	,color(color)
	,is_mtl(false)
{
	InitFrontMesh();
	SetLayer(GetLayer() | EL_MESH);
}

EntityMesh::EntityMesh(Mesh* mesh,  Shader* shader, const std::string& material, bool is_mtl)
	:m_WorldAABB(transformBoundingBox(GetWorldMatrix(), mesh->box))
	,mesh(mesh)
	,texture(nullptr)
	,shader(shader)
	,material(material)
	,is_mtl(is_mtl)
{
	InitFrontMesh();
	SetLayer(GetLayer() | EL_MESH);
	Map_texture_color = Getmtl(material);


	Vector3 color;
	int size_material_anterior = 0;
	int size_material_actual = 0;

	//this->mesh->colors.clear();
	if (mesh->colors.empty())
	{
		for (int i = 1; i <= (int)this->mesh->getNumSubmeshes(); i++) {


			if (is_mtl) {
				color = Map_texture_color[this->mesh->material_name.at(i - 1)]; /////////
			}
			else {
				Image* image = Image::Get(material);
				Color colortexture;
				colortexture = image->getPixel(i - 1, 0);
				color = Vector3(float(colortexture.x) / 255, float(colortexture.y) / 255, float(colortexture.z) / 255);

			}
			size_material_actual = this->mesh->material_range.at(i - 1);

			for (int j = size_material_anterior; j <= size_material_actual - 1; j++) {

				this->mesh->colors.push_back(Vector4(color.x, color.y, color.z, 1));
				this->mesh->colors.push_back(Vector4(color.x, color.y, color.z, 1));
				this->mesh->colors.push_back(Vector4(color.x, color.y, color.z, 1));
			}

			size_material_anterior = size_material_actual;
		}

		this->mesh->uploadToVRAM();
	}
}

void EntityMesh:: Update(double elapsed_seconds)
{
	Entity::Update(elapsed_seconds);
}

std::map<std::string, Vector3> EntityMesh::Getmtl(const std::string& filename){
	struct color {
		float red, green, blue;
	};


	typedef struct color Color;

	struct material {
		Color ambient, diffuse, specular;
		char name[128];
	};

	typedef struct material Material;

	struct material_group {
		Material* m;
		int slot;
	};

	typedef struct material_group MaterialG;

	MaterialG mg;

	FILE* fp = nullptr;
	char scanline[1024];

	const auto& material_file = filename;

	fp = fopen(material_file.c_str(), "r");

	/*if (fp == NULL) {
		return ;
	}*/


	mg.m = (Material*)malloc(sizeof(Material));
	mg.slot = 0;

	while (fgets(scanline, 1022, fp) != NULL) {
		if (scanline[0] == 'n' && scanline[1] == 'e' && scanline[2] == 'w' && scanline[3] == 'm' && scanline[4] == 't' && scanline[5] == 'l') {
			sscanf(scanline, "newmtl %s", mg.m[mg.slot].name);

			continue;
		}
		if (scanline[0] == 'K' && scanline[1] == 'a') {
			sscanf(scanline, "Ka %f %f %f", &mg.m[mg.slot].ambient.red, &mg.m[mg.slot].ambient.green, &mg.m[mg.slot].ambient.blue);

			continue;
		}
		else if (scanline[0] == 'K' && scanline[1] == 'd') {
			sscanf(scanline, "Kd %f %f %f", &mg.m[mg.slot].diffuse.red, &mg.m[mg.slot].diffuse.green, &mg.m[mg.slot].diffuse.blue);

			continue;
		}
		else if (scanline[0] == 'K' && scanline[1] == 's') {
			sscanf(scanline, "Ks %f %f %f", &mg.m[mg.slot].specular.red, &mg.m[mg.slot].specular.green, &mg.m[mg.slot].specular.blue);

			continue;
		}
		mg.slot++;
		mg.m = (Material*)realloc(mg.m, (mg.slot + 1) * sizeof(Material));
	}

	fclose(fp);


	std::map<std::string, Vector3> Map_texture_color;

	int numbermtl = mg.slot;

	for (int count = 0; count < numbermtl; count++) {

		Map_texture_color[mg.m[count].name] = Vector3(mg.m[count].diffuse.red, mg.m[count].diffuse.green, mg.m[count].diffuse.blue);

	}

	free(mg.m);
	return Map_texture_color;
}



Vector3 CheckCollision(const CollisionData& data, EntityMesh** collided)
{
	constexpr float colisionHeight = 0.21f;
	bool has_colide = false;
	Vector3 target_pos = data.Target.y < colisionHeight ? data.Target + Vector3(0.f, 0.21f, 0.f) : data.Target;
	Vector3 result;

	if (data.StaticCollisions)
	{
		auto* map = Game::instance->GetMapAt(data.Position);
		if (map != nullptr)
		{
			const auto tileSize = map->GetTileSize();
			std::array<size_t, 9> tileIdx =
			{
				map->World2TileIdx(Vector3(data.Position.x, 0.f,			data.Position.z)),
				map->World2TileIdx(Vector3(data.Position.x + tileSize, 0.f, data.Position.z)),
				map->World2TileIdx(Vector3(data.Position.x, 0.f,			data.Position.z + tileSize)),
				map->World2TileIdx(Vector3(data.Position.x + tileSize, 0.f, data.Position.z + tileSize)),
				map->World2TileIdx(Vector3(data.Position.x - tileSize, 0.f, data.Position.z)),
				map->World2TileIdx(Vector3(data.Position.x, 0.f,			data.Position.z - tileSize)),
				map->World2TileIdx(Vector3(data.Position.x - tileSize, 0.f, data.Position.z - tileSize)),
				map->World2TileIdx(Vector3(data.Position.x + tileSize, 0.f, data.Position.z - tileSize)),
				map->World2TileIdx(Vector3(data.Position.x - tileSize, 0.f, data.Position.z + tileSize)),
			};
			for (const auto& idx : tileIdx)
			{
				if (idx > map->GetTiles().size())
					continue;
				auto* tile = map->GetTile(idx);
				if (tile == nullptr)
					continue;
				for (auto* child : tile->GetChilds())
				{
					if ((child->GetLayer() & EL_MESH) == 0)
						continue; // If its not a mesh continue

					auto* emesh = (EntityMesh*)child;
					Vector3 coll;
					Vector3 collnorm;
					if (emesh->mesh->testSphereCollision(emesh->GetWorldMatrix(), target_pos, 0.2f, coll, collnorm) == false)
						continue;

					has_colide = true;
					if (data.SmoothCollide)
					{
						Vector3 push_away = normalize(coll - target_pos) * (float)data.ElapsedSeconds;
						push_away.y = 0.f;
						result = data.Position - push_away;
					}
					if (collided != nullptr)
						*collided = emesh;
					break;
				}
			}
		}
	}

	if (data.DynamicObjectsRootEntity != nullptr && !has_colide)
	{
		for (auto* it : data.DynamicObjectsRootEntity->GetChilds())
		{
			if ((it->GetLayer() & EL_MESH) == 0)
				continue; // If its not a mesh continue
			if (it == data.SourceEntity)
				continue; // Avoid self collision

			EntityMesh* test = (EntityMesh*)it;
			Vector3 coll;
			Vector3 collnorm;
			if (test->mesh->testSphereCollision(test->GetWorldMatrix(), target_pos, 0.2f, coll, collnorm) == false)
				continue;

			has_colide = true;
			if (data.SmoothCollide)
			{
				Vector3 push_away = normalize(coll - target_pos) * (float)data.ElapsedSeconds;
				push_away.y = 0.f;
				result = data.Position - push_away;
			}
			if(collided != nullptr)
				*collided = test;
			break;
		}
	}

	if (!has_colide)
		result = data.Target;
	else if (!data.SmoothCollide)
		result = data.Position;

	return result;
}