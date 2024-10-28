#include "Game/Tile.hpp"

#include "Engine/Core/XmlUtils.hpp"
#include "Engine/Core/EngineCommon.hpp"

TileDefinition TileDefinition::s_tileDefinitions[8];

Tile::Tile()
{
}

Tile::Tile(IntVec2 coordinates)
	: m_coordinates(coordinates)
{

}

Tile::~Tile()
{
}

AABB2 Tile::GetBounds() const
{
	Vec2 bottomLeft = Vec2(static_cast<float>(m_coordinates.x), static_cast<float>(m_coordinates.y));
	Vec2 topRight = Vec2(static_cast<float>(m_coordinates.x + 1), static_cast<float>(m_coordinates.y + 1));

	AABB2 bounds = AABB2(bottomLeft, topRight);

	return bounds;
}

Rgba8 Tile::GetColor() const
{
	return Rgba8();
}

void TileDefinition::IntializeTileDef()
{
	XmlDocument tileDoc;
	
	XmlError result = tileDoc.LoadFile("Data/TileDefinitions.xml");
	
	if (result != tinyxml2::XML_SUCCESS)
		GUARANTEE_OR_DIE(false, "COULD NOT LOAD XML");


	XmlElement* element = tileDoc.RootElement()->FirstChildElement();

	for (int index = 0; index < g_gameConfigBlackboard.GetValue("totalTileTypes", 0.0f); index++)
	{
		s_tileDefinitions[index].m_type				= ParseXmlAttribute(*element, "name", s_tileDefinitions[index].m_type);
		s_tileDefinitions[index].m_spriteIndex		= ParseXmlAttribute(*element, "spriteIndex", s_tileDefinitions[index].m_spriteIndex);
		s_tileDefinitions[index].m_isSolid			= ParseXmlAttribute(*element, "isSolid", s_tileDefinitions[index].m_isSolid);
		s_tileDefinitions[index].m_isWater			= ParseXmlAttribute(*element, "isWater", s_tileDefinitions[index].m_isWater);
		s_tileDefinitions[index].m_color			= ParseXmlAttribute(*element, "tint", s_tileDefinitions[index].m_color);

		element = element->NextSiblingElement();
	}
}