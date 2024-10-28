#pragma once

#include "Engine/Math/IntVec2.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Core/Rgba8.hpp"

#include <string>

struct TileDefinition
{
	std::string m_type;
	bool m_isSolid = false;
	bool m_isWater = false;
	int m_spriteIndex;
	Rgba8 m_color;

	static TileDefinition s_tileDefinitions[8];

	static void IntializeTileDef();
};

class Tile
{
public:
	IntVec2 m_coordinates;
	TileDefinition* m_tileDef;
public:
	Tile();
	Tile(IntVec2 coordinates);
	~Tile();

	AABB2 GetBounds() const;
	Rgba8 GetColor() const;
};