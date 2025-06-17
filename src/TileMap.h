#pragma once

#ifndef TILE_MAP_H
#define TILE_MAP_H 1

#include "includes.h"
#include "framework.h"
#include "EntityManager.h"

class TileMap
{
	unsigned int m_HorizontalTiles;
	unsigned int m_VerticalTiles;
	unsigned int m_TileSize;
	Entity* m_MapEntity;
	BoundingBox m_MapAABB;
	Mesh* m_TileMesh;
	unsigned int m_MapWidth;
	unsigned int m_MapHeight;

	// Optimizations
	Matrix44 m_InvMapWorld;
	float m_InvMapWidth;
	float m_InvMapHeight;

	std::vector<Matrix44> m_TileWorldMatrix;

	static Shader* gTileShader;
	//static Texture* gTileTexture;

	Mesh* CreateTileMesh();
	Entity* CreateMapEntity(Entity* parent, const Vector3& mapWorldPosition);
	BoundingBox GetLocalAABB()const;

	void GenerateTiles();

	void UpdateTileWorldMatrix();

public:
	static bool gShowTiles;
	/* 
		Maps are also considered Entities, and all the objects inside of it are affected by the things that happend
		to the map, that enables us a nice improvement, like chunks in minecraft, we can only update maps that are
		close to the player, or render more efficiently.

		Parent is normally the stage root entity,
		mapWorldPosition, where you want to start the map,
		tileSize, how big tiles are, bigger tiles better performance but less object diversity
		horizontalTiles, number of tiles that the map has horizontally
		verticalTiles, number of tiles that the map has vertically
	*/
	TileMap(Entity* parent, const Vector3& mapWorldPosition, unsigned int tileSize, unsigned int horizontalTiles, unsigned int verticalTiles);

	static TileMap* LoadFromFile(Entity* parent, const std::string& filePath);

	bool LoadFromFile(const std::string& filePath);

	void SaveMapToFile(const std::string& filePath)const;

	void Clear()const;

	unsigned int GetHorizontalTiles()const;
	unsigned int GetVerticalTiles()const;
	unsigned int GetTileSize()const;
	unsigned int GetMapWidth()const;
	unsigned int GetMapHeight()const;

	size_t World2TileIdx(const Vector3& worldPosition)const;
	
	Entity* GetTile(size_t tileIdx);
	const Entity* GetTile(size_t tileIdx)const;

	Entity* GetMapEntity()const;

	const std::vector<Entity*>& GetTiles()const;
	const BoundingBox& GetMapBox()const;

	void Update(double elapsed_seconds);
	void Render();
};

#endif /* TILE_MAP_H */