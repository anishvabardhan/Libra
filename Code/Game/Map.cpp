#include "Game/Map.hpp"

#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/XmlUtils.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/TileHeatMap.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"
#include "Engine/Renderer/VertexBuffer.hpp"
#include "Engine/Renderer/SpriteDefinition.hpp"

#include "Game/GameCommon.hpp"
#include "Game/Game.hpp"
#include "Game/Player.hpp"
#include "Game/Explosion.hpp"
#include "Game/Scorpio.hpp"
#include "Game/Capricorn.hpp"
#include "Game/Aries.hpp"
#include "Game/Leo.hpp"
#include "Game/Bullet.hpp"
#include "Game/HealthBar.hpp"

MapDefinition MapDefinition::s_definitions[3];

void MapDefinition::InitializeDef()
{
	XmlDocument tileDoc;

	XmlError result = tileDoc.LoadFile("Data/MapDefinitions.xml");

	if (result != tinyxml2::XML_SUCCESS)
		GUARANTEE_OR_DIE(false, "COULD NOT LOAD XML");

	XmlElement* element = tileDoc.RootElement()->FirstChildElement();

	for (int index = 0; index < g_gameConfigBlackboard.GetValue("totalMaps", 0.0f); index++)
	{
		s_definitions[index].m_name				= ParseXmlAttribute(*element, "name",					s_definitions[index].m_name);
		s_definitions[index].m_dimensions		= ParseXmlAttribute(*element, "dimensions",				s_definitions[index].m_dimensions);
		s_definitions[index].m_fillType			= ParseXmlAttribute(*element, "fillTileType",			s_definitions[index].m_fillType);
		s_definitions[index].m_borderType		= ParseXmlAttribute(*element, "edgeTileType",			s_definitions[index].m_borderType);
		s_definitions[index].m_bunkerFloorType	= ParseXmlAttribute(*element, "BunkerFloorTileType",	s_definitions[index].m_bunkerFloorType);
		s_definitions[index].m_bunkerWallType	= ParseXmlAttribute(*element, "BunkerWallTileType",		s_definitions[index].m_bunkerWallType);
		s_definitions[index].m_mapStartType		= ParseXmlAttribute(*element, "MapStartTileType",		s_definitions[index].m_mapStartType);
		s_definitions[index].m_mapEndType		= ParseXmlAttribute(*element, "MapEndTileType",			s_definitions[index].m_mapEndType);
		s_definitions[index].m_wormType1		= ParseXmlAttribute(*element, "worm1TileType",			s_definitions[index].m_wormType1);
		s_definitions[index].m_wormType2		= ParseXmlAttribute(*element, "worm2TileType",			s_definitions[index].m_wormType2);
		s_definitions[index].m_numOfWorms1		= ParseXmlAttribute(*element, "worm1Count",				s_definitions[index].m_numOfWorms1);
		s_definitions[index].m_worm1length		= ParseXmlAttribute(*element, "worm1MaxLength",			s_definitions[index].m_worm1length);
		s_definitions[index].m_numOfWorms2		= ParseXmlAttribute(*element, "worm2Count",				s_definitions[index].m_numOfWorms2);
		s_definitions[index].m_worm2length		= ParseXmlAttribute(*element, "worm2MaxLength",			s_definitions[index].m_worm2length);
		s_definitions[index].m_numOfLeo			= ParseXmlAttribute(*element, "leoCount",				s_definitions[index].m_numOfLeo);
		s_definitions[index].m_numOfAries		= ParseXmlAttribute(*element, "ariesCount",				s_definitions[index].m_numOfAries);
		s_definitions[index].m_numOfScorpio		= ParseXmlAttribute(*element, "scorpioCount",			s_definitions[index].m_numOfScorpio);
		s_definitions[index].m_numOfCapricorn	= ParseXmlAttribute(*element, "capricornCount",			s_definitions[index].m_numOfCapricorn);

		element = element->NextSiblingElement();
	}
}

Map::Map()
{
}

Map::Map(IntVec2 dimensions)
	:m_dimensions(dimensions)
{
}

Map::Map(MapDefinition definition)
	: m_definition(definition)
{
	m_terrainSpriteSheet = new SpriteSheet(*g_theGame->m_allTextures[GAME_TERRAIN_TEXTURE], IntVec2(8, 8));

	TileDefinition::IntializeTileDef();

	m_dimensions = m_definition.m_dimensions;

	InitializeMap();

	m_tileHeatMap = new TileHeatMap(m_dimensions);

	AABB2 exitTile = AABB2(static_cast<float>(m_dimensions.x - 2), static_cast<float>(m_dimensions.y - 2), static_cast<float>(m_dimensions.x - 1), static_cast<float>(m_dimensions.y - 1));

	m_exitPosition = exitTile.GetCenter();

	PopulateDistanceField(*m_tileHeatMap, IntVec2(1, 1), 999999.0f);

	float value = m_tileHeatMap->GetTileHeatValue(IntVec2(static_cast<int>(exitTile.m_mins.x), static_cast<int>(exitTile.m_mins.y)));

	m_mapGPUMesh = g_theRenderer->CreateVertexBuffer(sizeof(Vertex_PCU), std::wstring(L"MapMesh"));

	while (m_mapGenerationCounter < 1000 && value == 999999.0f)
	{
		m_mapGenerationCounter++;

		InitializeMap();
		PopulateDistanceField(*m_tileHeatMap, IntVec2(1, 1), 999999.0f);
	
		value = m_tileHeatMap->GetTileHeatValue(IntVec2(static_cast<int>(exitTile.m_mins.x), static_cast<int>(exitTile.m_mins.y)));
	}

	for (int tileIndex = 0; tileIndex < m_dimensions.x * m_dimensions.y; tileIndex++)
	{
		AddVertsForTile(m_mapCPUMesh, static_cast<int>(tileIndex), *m_terrainSpriteSheet);
	}

	g_theRenderer->CopyCPUToGPU(m_mapCPUMesh.data(), (int)m_mapCPUMesh.size() * sizeof(Vertex_PCU), m_mapGPUMesh);

	m_mapGenerationCounter = 0;

	Entity* player = SpawnNewEntity(ENTITY_TYPE_GOOD_PLAYER, Vec2(1.5f, 1.5f), 45.0f);
	AddEntityToMap(*player);

	InitializeLeo();
	InitializeAries();
	InitializeScorpio();
}

Map::~Map()
{
	for (int i = 0; i < (int)m_allEntites.size(); i++)
	{
		if (m_allEntites[i])
		{
			DELETE_PTR(m_allEntites[i]);
		}
	}

	for (int i = 0; i < NUM_ENTITY_TYPES; i++)
	{
		for (int j = 0; j < (int)m_entitiesByType[i].size();j++)
		{
			if (m_entitiesByType[i][j])
			{
				m_entitiesByType[i][j] = nullptr;
			}
		}

		m_entitiesByType[i].clear();
	}

	m_allEntites.clear();

	DELETE_PTR(m_mapGPUMesh);
	DELETE_PTR(m_terrainSpriteSheet);
}

void Map::PopulateDistanceField(TileHeatMap& out_distanceField, IntVec2 startCoords, float maxCost, bool treatWaterAsSolid)
{
	UNUSED(treatWaterAsSolid);

	out_distanceField.SetAllValues(maxCost);

	float currentTileCost = 0.0f;
	bool isDirty = true;

	out_distanceField.SetTileHeatValue(startCoords, currentTileCost);

	while (isDirty)
	{
		float nextTileCost = currentTileCost + 1.0f;
		isDirty = false;

		for (int tileY = 1; tileY < m_dimensions.y - 1; tileY++)
		{
			for (int tileX = 1; tileX < m_dimensions.x - 1; tileX++)
			{
				IntVec2 tileCoord = IntVec2(tileX, tileY);

				if (out_distanceField.GetTileHeatValue(tileCoord) != currentTileCost)
				{
					continue;
				}

				if (!IsTileSolid(tileCoord + IntVec2::NORTH))
				{
					if (nextTileCost < out_distanceField.GetTileHeatValue(tileCoord + IntVec2::NORTH))
					{
						isDirty = true;
						out_distanceField.SetTileHeatValue(tileCoord + IntVec2::NORTH, nextTileCost);
					}
				}

				if (!IsTileSolid(tileCoord + IntVec2::SOUTH))
				{
					if (nextTileCost < out_distanceField.GetTileHeatValue(tileCoord + IntVec2::SOUTH))
					{
						isDirty = true;
						out_distanceField.SetTileHeatValue(tileCoord + IntVec2::SOUTH, nextTileCost);
					}
				}

				if (!IsTileSolid(tileCoord + IntVec2::EAST))
				{
					if (nextTileCost < out_distanceField.GetTileHeatValue(tileCoord + IntVec2::EAST))
					{
						isDirty = true;
						out_distanceField.SetTileHeatValue(tileCoord + IntVec2::EAST, nextTileCost);
					}
				}

				if (!IsTileSolid(tileCoord + IntVec2::WEST))
				{
					if (nextTileCost < out_distanceField.GetTileHeatValue(tileCoord + IntVec2::WEST))
					{
						isDirty = true;
						out_distanceField.SetTileHeatValue(tileCoord + IntVec2::WEST, nextTileCost);
					}
				}
			}
		}

		currentTileCost++;
	}

	for (int tileY = 1; tileY < m_dimensions.y - 1; tileY++)
	{
		for (int tileX = 1; tileX < m_dimensions.x - 1; tileX++)
		{
			IntVec2 tileCoord = IntVec2(tileX, tileY);

			if (out_distanceField.GetTileHeatValue(tileCoord) == maxCost)
			{
				int tileIndex = tileCoord.x + (tileCoord.y * m_dimensions.x);
				int neighbourNorthTileIndex = tileCoord.x + ((tileCoord.y + 1) * m_dimensions.x);
				int neighbourSouthTileIndex = tileCoord.x + ((tileCoord.y - 1) * m_dimensions.x);
				int neighbourEastTileIndex = tileCoord.x + 1 + (tileCoord.y * m_dimensions.x);
				int neighbourWestTileIndex = tileCoord.x - 1 + (tileCoord.y * m_dimensions.x);



				if (m_tiles[tileIndex].m_tileDef->m_type == m_tiles[neighbourNorthTileIndex].m_tileDef->m_type)
				{
					m_tiles[tileIndex].m_tileDef = m_tiles[neighbourNorthTileIndex].m_tileDef;
				}
				else if (m_tiles[tileIndex].m_tileDef->m_type == m_tiles[neighbourSouthTileIndex].m_tileDef->m_type)
				{
					m_tiles[tileIndex].m_tileDef = m_tiles[neighbourSouthTileIndex].m_tileDef;
				}
				else if (m_tiles[tileIndex].m_tileDef->m_type == m_tiles[neighbourEastTileIndex].m_tileDef->m_type)
				{
					m_tiles[tileIndex].m_tileDef = m_tiles[neighbourEastTileIndex].m_tileDef;
				}
				else if (m_tiles[tileIndex].m_tileDef->m_type == m_tiles[neighbourWestTileIndex].m_tileDef->m_type)
				{
					m_tiles[tileIndex].m_tileDef = m_tiles[neighbourWestTileIndex].m_tileDef;
				}

				out_distanceField.SetTileHeatValue(tileCoord, 999999.0f);
			}
		}
	}

	for (int tileY = 1; tileY < m_dimensions.y - 1; tileY++)
	{
		for (int tileX = 1; tileX < m_dimensions.x - 1; tileX++)
		{
			IntVec2 tileCoord = IntVec2(tileX, tileY);

			if (out_distanceField.GetTileHeatValue(tileCoord) == 999999.0f)
			{
				IntVec2 NorthTileCoord = tileCoord + IntVec2::NORTH;
				IntVec2 SouthTileCoord = tileCoord + IntVec2::SOUTH;
				IntVec2 EastTileCoord =  tileCoord + IntVec2::EAST;
				IntVec2 WestTileCoord =  tileCoord + IntVec2::WEST;

				int tileIndex = tileCoord.x + (tileCoord.y * m_dimensions.x);
				int neighbourNorthTileIndex = NorthTileCoord.x + (NorthTileCoord.y * m_dimensions.x);
				int neighbourSouthTileIndex = SouthTileCoord.x + (SouthTileCoord.y * m_dimensions.x);
				int neighbourEastTileIndex = EastTileCoord.x + (EastTileCoord.y * m_dimensions.x);
				int neighbourWestTileIndex = WestTileCoord.x + (WestTileCoord.y * m_dimensions.x);


				if (out_distanceField.GetTileHeatValue(NorthTileCoord) == 999999.0f)
				{
					m_tiles[tileIndex].m_tileDef = m_tiles[neighbourNorthTileIndex].m_tileDef;
				}
				else if (out_distanceField.GetTileHeatValue(SouthTileCoord) == 999999.0f)
				{
					m_tiles[tileIndex].m_tileDef = m_tiles[neighbourSouthTileIndex].m_tileDef;
				}
				else if (out_distanceField.GetTileHeatValue(EastTileCoord) == 999999.0f)
				{
					m_tiles[tileIndex].m_tileDef = m_tiles[neighbourEastTileIndex].m_tileDef;
				}
				else if (out_distanceField.GetTileHeatValue(WestTileCoord) == 999999.0f)
				{
					m_tiles[tileIndex].m_tileDef = m_tiles[neighbourWestTileIndex].m_tileDef;
				}
			}
		}
	}
}

void Map::RespawnPlayer()
{
	m_entitiesByType[ENTITY_TYPE_GOOD_PLAYER][0]->m_isDead = false;
	m_entitiesByType[ENTITY_TYPE_GOOD_PLAYER][0]->m_health = 30;
}

void Map::DeleteGarbageEntites()
{
	for (int entityType = 0; entityType < NUM_ENTITY_TYPES; entityType++)
	{
		for (size_t index = 0; index < m_entitiesByType[entityType].size(); index++)
		{
			if (m_entitiesByType[entityType][index] && !m_entitiesByType[entityType][index]->IsAlive())
			{
				delete m_entitiesByType[entityType][index];
				m_entitiesByType[entityType][index] = nullptr;
			}
		}
	}
}

void Map::InitializeLeo()
{
	RandomNumberGenerator random = RandomNumberGenerator();

	for (int i = 0; i < m_definition.m_numOfLeo; i++)
	{
		Vec2 position = Vec2(random.RollRandomFloatInRange(1.0f, m_dimensions.x - 1.0f), random.RollRandomFloatInRange(1.0f, m_dimensions.y - 1.0f));
		IntVec2 tileCoord = GetTileCoordinates(position);

		while (IsTileSolid(tileCoord) || IsTileWater(tileCoord))
		{
			position = Vec2(random.RollRandomFloatInRange(1.0f, m_dimensions.x - 1.0f), random.RollRandomFloatInRange(1.0f, m_dimensions.y - 1.0f));
			tileCoord = GetTileCoordinates(position);
		}

		Entity* leo = SpawnNewEntity(ENTITY_TYPE_EVIL_LEO, position, 0.0f);
		AddEntityToMap(*leo);
	}
}

void Map::InitializeAries()
{
	RandomNumberGenerator random = RandomNumberGenerator();

	for (int i = 0; i < m_definition.m_numOfAries; i++)
	{
		Vec2 position = Vec2(random.RollRandomFloatInRange(1.0f, m_dimensions.x - 1.0f), random.RollRandomFloatInRange(1.0f, m_dimensions.y - 1.0f));
		IntVec2 tileCoord = GetTileCoordinates(position);

		while (IsTileSolid(tileCoord) || IsTileWater(tileCoord))
		{
			position = Vec2(random.RollRandomFloatInRange(1.0f, m_dimensions.x - 1.0f), random.RollRandomFloatInRange(1.0f, m_dimensions.y - 1.0f));
			tileCoord = GetTileCoordinates(position);
		}

		Entity* aries = SpawnNewEntity(ENTITY_TYPE_EVIL_ARIES, position, 0.0f);
		AddEntityToMap(*aries);
	}
}

void Map::InitializeScorpio()
{
	RandomNumberGenerator random = RandomNumberGenerator();

	for (int i = 0; i < m_definition.m_numOfScorpio; i++)
	{
		Vec2 position = Vec2(random.RollRandomFloatInRange(1.0f, m_dimensions.x - 1.0f), random.RollRandomFloatInRange(1.0f, m_dimensions.y - 1.0f));
		IntVec2 tileCoord = GetTileCoordinates(position);

		while (IsTileSolid(tileCoord) || IsTileWater(tileCoord))
		{
			position = Vec2(random.RollRandomFloatInRange(1.0f, m_dimensions.x - 1.0f), random.RollRandomFloatInRange(1.0f, m_dimensions.y - 1.0f));
			tileCoord = GetTileCoordinates(position);
		}

		Entity* scorpio = SpawnNewEntity(ENTITY_TYPE_EVIL_SCORPIO, position, random.RollRandomFloatInRange(g_gameConfigBlackboard.GetValue("scorpioOrientation", 0.0f), g_gameConfigBlackboard.GetValue("scorpioOrientation", 0.0f) + 125.0f));
		AddEntityToMap(*scorpio);
	}
}

void Map::InitializeCapicorn()
{
	RandomNumberGenerator random = RandomNumberGenerator();

	for (int i = 0; i < m_definition.m_numOfCapricorn; i++)
	{
		Vec2 position = Vec2(random.RollRandomFloatInRange(1.0f, m_dimensions.x - 1.0f), random.RollRandomFloatInRange(1.0f, m_dimensions.y - 1.0f));
		IntVec2 tileCoord = GetTileCoordinates(position);

		while (IsTileSolid(tileCoord) || IsTileWater(tileCoord))
		{
			position = Vec2(random.RollRandomFloatInRange(1.0f, m_dimensions.x - 1.0f), random.RollRandomFloatInRange(1.0f, m_dimensions.y - 1.0f));
			tileCoord = GetTileCoordinates(position);
		}

		Entity* capricorn = SpawnNewEntity(ENTITY_TYPE_EVIL_CAPRICORN, position, 0.0f);
		AddEntityToMap(*capricorn);
	}
}

void Map::AddVertsForTile(std::vector<Vertex_PCU>& verts, int tileIndex, SpriteSheet const& spriteSheet)
{
	AABB2 tileBounds = m_tiles[tileIndex].GetBounds();

	SpriteDefinition const& testSpriteDef = spriteSheet.GetSpriteDef(m_tiles[tileIndex].m_tileDef->m_spriteIndex);

	Vec2 uvMins, uvMaxs;
	testSpriteDef.GetUVs(uvMins, uvMaxs);

	AddVertsForAABB2D(verts, tileBounds, m_tiles[tileIndex].m_tileDef->m_color, uvMins, uvMaxs);
}

Entity* Map::SpawnNewEntity(EntityType type, Vec2 const& position, float orientationDegrees)
{
	Entity* entity = nullptr;

	switch (type)
	{
	case ENTITY_TYPE_GOOD_PLAYER:
	{
		entity = new Player(this, position, orientationDegrees, FACTION_GOOD, type);
		break;
	}
	case ENTITY_TYPE_EVIL_ARIES:
	{
		entity = new Aries(this, position, orientationDegrees, FACTION_EVIL, type);
		break;
	}
	case ENTITY_TYPE_EVIL_LEO:
	{
		entity = new Leo(this, position, orientationDegrees, FACTION_EVIL, type);
		break;
	}
	case ENTITY_TYPE_EVIL_SCORPIO:
	{
		entity = new Scorpio(this, position, orientationDegrees, FACTION_EVIL, type);
		break;
	}
	case ENTITY_TYPE_EVIL_CAPRICORN:
	{
		entity = new Capricorn(this, position, orientationDegrees, FACTION_EVIL, type);
		break;
	}
	}

	if (entity)
	{
		Entity* entityHealth = new HealthBar(this, entity, position, entity->GetHealth());
		AddEntityToMap(*entityHealth);
	}

	return entity;
}

void Map::InitializeMap()
{
	int totalTiles = m_dimensions.x * m_dimensions.y;
	m_tiles.reserve(totalTiles);

	// FILL TYPE TILES

	for (int tileIndex = 0; tileIndex < totalTiles; tileIndex++)
	{
		int tileX = static_cast<int>(tileIndex) % m_dimensions.x;
		int tileY = static_cast<int>(tileIndex) / m_dimensions.x;

		m_tiles.push_back(Tile(IntVec2(tileX, tileY)));
		m_tiles[tileIndex].m_tileDef = &TileDefinition::s_tileDefinitions[0];
	}

	// BORDER TYPE TILES

	for (int tileIndex = 0; tileIndex < totalTiles; tileIndex++)
	{
		int tileX = static_cast<int>(tileIndex) % m_dimensions.x;
		int tileY = static_cast<int>(tileIndex) / m_dimensions.x;

		if (tileX == 0 || tileX == m_dimensions.x - 1)
		{
			m_tiles[tileIndex] = Tile(IntVec2(tileX, tileY));
			m_tiles[tileIndex].m_tileDef = &TileDefinition::s_tileDefinitions[2];
		}
		else if (tileY == 0 || tileY == m_dimensions.y - 1)
		{
			m_tiles[tileIndex] = Tile(IntVec2(tileX, tileY));
			m_tiles[tileIndex].m_tileDef = &TileDefinition::s_tileDefinitions[2];
		}
	}

	// WORM TYPE 1 TILES

	for (int index = 0; index < m_definition.m_numOfWorms1; index++)
	{
		RandomNumberGenerator random1 = RandomNumberGenerator();
		int randomX = random1.RollRandomIntInRange(1, m_dimensions.x - 2);
	
		RandomNumberGenerator random2 = RandomNumberGenerator();
		int randomY = random2.RollRandomIntInRange(1, m_dimensions.y - 2);
	
		int tile = randomX + (randomY * m_dimensions.x);
	
		m_tiles[tile] = Tile(IntVec2(randomX, randomY));
		m_tiles[tile].m_tileDef = &TileDefinition::s_tileDefinitions[1];
	
		for (int length = 0; length < m_definition.m_worm1length; length++)
		{
			RandomNumberGenerator random3 = RandomNumberGenerator();
			int direction = random3.RollRandomIntInRange(0, 3);
	
			if (direction == 0)
			{
				int nextTile = (randomX) + ((randomY + 1) * m_dimensions.x);

				if (!IsTileSolid(IntVec2((randomX), (randomY + 1))) && !IsTileWater(IntVec2((randomX), (randomY + 1))))
				{
					m_tiles[nextTile] = Tile(IntVec2(randomX, randomY + 1));
					m_tiles[nextTile].m_tileDef = &TileDefinition::s_tileDefinitions[1];

					randomY += 1;
				}
			}
			else if (direction == 1)
			{
				int nextTile = (randomX)+((randomY - 1) * m_dimensions.x);

				if (!IsTileSolid(IntVec2((randomX), (randomY - 1))) && !IsTileWater(IntVec2((randomX), (randomY - 1))))
				{
					m_tiles[nextTile] = Tile(IntVec2(randomX, randomY - 1));
					m_tiles[nextTile].m_tileDef = &TileDefinition::s_tileDefinitions[1];

					randomY -= 1;
				}
			}
			else if (direction == 2)
			{
				int nextTile = (randomX + 1)+((randomY) * m_dimensions.x);

				if (!IsTileSolid(IntVec2((randomX + 1), (randomY))) && !IsTileWater(IntVec2((randomX + 1), (randomY))))
				{
					m_tiles[nextTile] = Tile(IntVec2(randomX + 1, randomY));
					m_tiles[nextTile].m_tileDef = &TileDefinition::s_tileDefinitions[1];

					randomX += 1;
				}
			}
			else if (direction == 3)
			{
				int nextTile = (randomX - 1)+((randomY) * m_dimensions.x);

				if (!IsTileSolid(IntVec2((randomX - 1), (randomY))) && !IsTileWater(IntVec2((randomX - 1), (randomY))))
				{
					m_tiles[nextTile] = Tile(IntVec2(randomX - 1, randomY));
					m_tiles[nextTile].m_tileDef = &TileDefinition::s_tileDefinitions[1];

					randomX -= 1;
				}
			}
		}
	}

	// WORM TYPE 2 TILES

	for (int index = 0; index < m_definition.m_numOfWorms2; index++)
	{
		RandomNumberGenerator random1 = RandomNumberGenerator();
		int randomX = random1.RollRandomIntInRange(1, m_dimensions.x - 2);

		RandomNumberGenerator random2 = RandomNumberGenerator();
		int randomY = random2.RollRandomIntInRange(1, m_dimensions.y - 2);

		int tile = randomX + (randomY * m_dimensions.x);

		m_tiles[tile] = Tile(IntVec2(randomX, randomY));
		m_tiles[tile].m_tileDef = &TileDefinition::s_tileDefinitions[7];

		for (int length = 0; length < m_definition.m_worm2length; length++)
		{
			RandomNumberGenerator random3 = RandomNumberGenerator();
			int direction = random3.RollRandomIntInRange(0, 3);

			if (direction == 0)
			{
				int nextTile = (randomX)+((randomY + 1) * m_dimensions.x);

				if (!IsTileSolid(IntVec2((randomX), (randomY + 1))) && !IsTileWater(IntVec2((randomX), (randomY + 1))))
				{
					m_tiles[nextTile] = Tile(IntVec2(randomX, randomY + 1));
					m_tiles[nextTile].m_tileDef = &TileDefinition::s_tileDefinitions[7];

					randomY += 1;
				}
			}
			else if (direction == 1)
			{
				int nextTile = (randomX)+((randomY - 1) * m_dimensions.x);

				if (!IsTileSolid(IntVec2((randomX), (randomY - 1))) && !IsTileWater(IntVec2((randomX), (randomY - 1))))
				{
					m_tiles[nextTile] = Tile(IntVec2(randomX, randomY - 1));
					m_tiles[nextTile].m_tileDef = &TileDefinition::s_tileDefinitions[7];

					randomY -= 1;
				}
			}
			else if (direction == 2)
			{
				int nextTile = (randomX + 1) + ((randomY)*m_dimensions.x);

				if (!IsTileSolid(IntVec2((randomX + 1), (randomY))) && !IsTileWater(IntVec2((randomX + 1), (randomY))))
				{
					m_tiles[nextTile] = Tile(IntVec2(randomX + 1, randomY));
					m_tiles[nextTile].m_tileDef = &TileDefinition::s_tileDefinitions[7];

					randomX += 1;
				}
			}
			else if (direction == 3)
			{
				int nextTile = (randomX - 1) + ((randomY)*m_dimensions.x);

				if (!IsTileSolid(IntVec2((randomX - 1), (randomY))) && !IsTileWater(IntVec2((randomX - 1), (randomY))))
				{
					m_tiles[nextTile] = Tile(IntVec2(randomX - 1, randomY));
					m_tiles[nextTile].m_tileDef = &TileDefinition::s_tileDefinitions[7];

					randomX -= 1;
				}
			}
		}
	}
	
	// BUNKER START TILES

	for (int tileIndex = 0; tileIndex < totalTiles; tileIndex++)
	{
		int tileX = static_cast<int>(tileIndex) % m_dimensions.x;
		int tileY = static_cast<int>(tileIndex) / m_dimensions.x;
	
		if (tileX >= 1 && tileX <= 5)
		{
			if (tileY >= 1 && tileY <= 5)
			{
				m_tiles[tileIndex] = Tile(IntVec2(tileX, tileY));
				m_tiles[tileIndex].m_tileDef = &TileDefinition::s_tileDefinitions[4];
			}
		}
	
		if (tileX == 4)
		{
			if (tileY == 2 || tileY == 3 || tileY == 4)
			{
				m_tiles[tileIndex] = Tile(IntVec2(tileX, tileY));
				m_tiles[tileIndex].m_tileDef = &TileDefinition::s_tileDefinitions[3];
			}
		}
	
		if (tileY == 4)
		{
			if (tileX == 2 || tileX == 3 || tileX == 4)
			{
				m_tiles[tileIndex] = Tile(IntVec2(tileX, tileY));
				m_tiles[tileIndex].m_tileDef = &TileDefinition::s_tileDefinitions[3];
			}
		}
	}
	
	int startIndex = 1 + (1 * m_dimensions.x);
	
	m_tiles[startIndex] = Tile(IntVec2(1, 1));
	m_tiles[startIndex].m_tileDef = &TileDefinition::s_tileDefinitions[5];
	
	
	// BUNKER END TILES
	
	for (int tileIndex = 0; tileIndex < totalTiles; tileIndex++)
	{
		int tileX = static_cast<int>(tileIndex) % m_dimensions.x;
		int tileY = static_cast<int>(tileIndex) / m_dimensions.x;
	
		if (tileX <= m_dimensions.x - 1 - 1 && tileX >= m_dimensions.x - 1 - 5)
		{
			if (tileY <= m_dimensions.y - 1 - 1 && tileY >= m_dimensions.y - 1 - 5)
			{
				m_tiles[tileIndex] = Tile(IntVec2(tileX, tileY));
				m_tiles[tileIndex].m_tileDef = &TileDefinition::s_tileDefinitions[4];
			}
		}
	
		if (tileX == m_dimensions.x - 1 - 4)
		{
			if (tileY == m_dimensions.y - 1 - 2 || tileY == m_dimensions.y - 1 - 3 || tileY == m_dimensions.y - 1 - 4)
			{
				m_tiles[tileIndex] = Tile(IntVec2(tileX, tileY));
				m_tiles[tileIndex].m_tileDef = &TileDefinition::s_tileDefinitions[3];
			}
		}
	
		if (tileY == m_dimensions.y - 1 - 4)
		{
			if (tileX == m_dimensions.x - 1 - 2 || tileX == m_dimensions.x - 1 - 3 || tileX == m_dimensions.x - 1 - 4)
			{
				m_tiles[tileIndex] = Tile(IntVec2(tileX, tileY));
				m_tiles[tileIndex].m_tileDef = &TileDefinition::s_tileDefinitions[3];
			}
		}
	}
	
	int endIndex = (m_dimensions.x - 2) + ((m_dimensions.y - 2) * m_dimensions.x);
	
	m_tiles[endIndex] = Tile(IntVec2((m_dimensions.x - 2), (m_dimensions.y - 2)));
	m_tiles[endIndex].m_tileDef = &TileDefinition::s_tileDefinitions[6];
}

void Map::HandleInput()
{
	if (g_theInputSystem->WasKeyJustPressed(KEYCODE_F3))
	{
		m_isClipping = !m_isClipping;
	}

	if (g_theInputSystem->WasKeyJustPressed(KEYCODE_F6))
	{
		m_isHeatMapOn = !m_isHeatMapOn;
	}
}

void Map::AddEntityToMap(Entity& e)
{
	m_allEntites.push_back(&e);
	m_entitiesByType[e.m_type].push_back(&e);
}

void Map::RemoveEntityFromMap(Entity& e)
{
	for (size_t index = 0; m_allEntites.size(); index++)
	{
		if (m_allEntites[index] == &e)
		{
			DELETE_PTR(m_allEntites[index]);
		}
	}
}

void Map::Update(float deltaseconds)
{
	if (m_isPlayerDead)
	{
		m_deadTimer += deltaseconds;
	}

	if (m_deadTimer >= 3.0f)
	{
		g_theGame->m_isLoseMode = true;
		g_theGame->m_isGameplayMode = false;
	}

	HandleInput();
	UpdateEntities(deltaseconds);
	CheckEntityVsBullet();
	PushEntitesVsEntities();
	PushEntitiesOutOfTiles();
	//DeleteGarbageEntites();
}

void Map::UpdateEntities(float deltaseconds)
{
	for (size_t index = 0; index < m_allEntites.size(); index++)
	{
		if (m_allEntites[index] && m_allEntites[index]->IsAlive())
		{
			m_allEntites[index]->Update(deltaseconds);

			if (m_allEntites[index]->m_type == ENTITY_TYPE_EVIL_SCORPIO && m_entitiesByType[ENTITY_TYPE_GOOD_PLAYER][0]->IsAlive())
			{
				Vec2 forward = m_entitiesByType[ENTITY_TYPE_GOOD_PLAYER][0]->GetPosition() - m_allEntites[index]->GetPosition();

				if (forward.GetLength() <= 10.0f)
				{
					if (HasLineOfSight(*m_allEntites[index], *m_entitiesByType[ENTITY_TYPE_GOOD_PLAYER][0]))
					{
						if (m_allEntites[index]->m_discoveryCounter < 1)
						{
							m_allEntites[index]->m_didDiscover = true;
						}

						m_allEntites[index]->m_hasLineOfSight = true;
						m_allEntites[index]->m_orientationDegrees = GetTurnedTowardDegrees(m_allEntites[index]->m_orientationDegrees, forward.GetNormalized().GetOrientationDegrees(), 360.0f * deltaseconds);
					}
					else
					{
						m_allEntites[index]->m_hasLineOfSight = false;
					}
				}

				if (m_allEntites[index]->m_didDiscover)
				{
					m_allEntites[index]->m_discoveryCounter++;
					SoundID discoveryID = g_theAudio->CreateOrGetSound("Data/Audio/Pause.mp3");
					g_theAudio->StartSound(discoveryID);

					m_allEntites[index]->m_didDiscover = false;
				}
			}
			else if (m_allEntites[index]->m_faction == FACTION_EVIL && m_allEntites[index]->m_type != ENTITY_TYPE_EVIL_BULLET && m_entitiesByType[ENTITY_TYPE_GOOD_PLAYER][0]->IsAlive())
			{
				Vec2 forward = m_entitiesByType[ENTITY_TYPE_GOOD_PLAYER][0]->GetPosition() - m_allEntites[index]->GetPosition();

				if (forward.GetLength() <= 10.0f)
				{
					if (HasLineOfSight(*m_allEntites[index], *m_entitiesByType[ENTITY_TYPE_GOOD_PLAYER][0]))
					{
						if (m_allEntites[index]->m_discoveryCounter < 1)
						{
							m_allEntites[index]->m_didDiscover = true;
						}

						m_allEntites[index]->m_lastPlayerPosition = m_entitiesByType[ENTITY_TYPE_GOOD_PLAYER][0]->GetPosition();
						Vec2 forwardNormal = m_allEntites[index]->m_lastPlayerPosition - m_allEntites[index]->GetPosition();

						m_allEntites[index]->m_hasLineOfSight = true;
						m_allEntites[index]->m_distanceToPlayer = (m_allEntites[index]->m_lastPlayerPosition - m_allEntites[index]->GetPosition()).GetLength();
						m_allEntites[index]->m_orientationDegrees = GetTurnedTowardDegrees(m_allEntites[index]->m_orientationDegrees, forwardNormal.GetOrientationDegrees(), 360.0f * deltaseconds);
					}
					else
					{
						if (m_allEntites[index]->m_distanceToPlayer > 0.5f)
						{
							Vec2 forwardNormal = m_allEntites[index]->m_lastPlayerPosition - m_allEntites[index]->GetPosition();
							m_allEntites[index]->m_distanceToPlayer = (m_allEntites[index]->m_lastPlayerPosition - m_allEntites[index]->GetPosition()).GetLength();
							m_allEntites[index]->m_orientationDegrees = GetTurnedTowardDegrees(m_allEntites[index]->m_orientationDegrees, forwardNormal.GetOrientationDegrees(), 360.0f * deltaseconds);
						}

						m_allEntites[index]->m_hasLineOfSight = false;
					}
				}

				if (m_allEntites[index]->m_didDiscover)
				{
					m_allEntites[index]->m_discoveryCounter++;
					SoundID discoveryID = g_theAudio->CreateOrGetSound("Data/Audio/Pause.mp3");
					g_theAudio->StartSound(discoveryID);

					m_allEntites[index]->m_didDiscover = false;
				}
			}
			else
			{
				m_allEntites[index]->m_hasLineOfSight = false;
			}
		}
	}
}

void Map::PlayerDead()
{
	m_isPlayerDead = true;
}

Vec2 Map::GetExitPosition() const
{
	return m_exitPosition;
}

MapDefinition Map::GetDefinition() const
{
	return m_definition;
}

void Map::RenderTiles()
{
	/*std::vector<Vertex_PCU> vertices;

	for (size_t tileIndex = 0; tileIndex < m_tiles.size(); tileIndex++)
	{
		AddVertsForTile(vertices, static_cast<int>(tileIndex), *m_terrainSpriteSheet);
	}*/

	TransformVertexArrayXY3D(static_cast<int>(m_mapCPUMesh.size()), m_mapCPUMesh.data(), 1.0f, 0.0f, Vec2(0.0f, 0.0f));

	g_theRenderer->SetModelConstants();
	g_theRenderer->SetBlendMode(BlendMode::ALPHA);
	g_theRenderer->SetDepthMode(DepthMode::ENABLED);
	g_theRenderer->SetRasterizerMode(RasterizerMode::SOLID_CULL_BACK);
	g_theRenderer->SetSamplerMode(SamplerMode::POINT_CLAMP);
	g_theRenderer->BindShader();
	g_theRenderer->BindTexture(&m_terrainSpriteSheet->GetTexture());
	g_theRenderer->DrawVertexBuffer(m_mapGPUMesh, (int)m_mapCPUMesh.size(), sizeof(Vertex_PCU));
	//g_theRenderer->DrawVertexArray(static_cast<int>(vertices.size()), vertices.data());
}

void Map::PushEntityOutOfTilePhysics(Entity& e, IntVec2 direction)
{
	int tileX = RoundDownToInt(e.GetPosition().x);
	int tileY = RoundDownToInt(e.GetPosition().y);

	int tileindex = (tileX + direction.x) + ((tileY + direction.y) * m_dimensions.x);
	
	if (IsTileSolid(IntVec2((tileX + direction.x), (tileY + direction.y))) || IsTileWater(IntVec2((tileX + direction.x), (tileY + direction.y))))
	{
		if(!e.m_canSwim)
			PushDiscOutOfAABB2D(e.m_position, e.m_physicsRadius, m_tiles[tileindex].GetBounds());
	}
}

void Map::RenderEntites()
{
	for (int entityType = 0; entityType < NUM_ENTITY_TYPES; entityType++)
	{
		for (size_t index = 0; index < m_entitiesByType[entityType].size(); index++)
		{
			if (m_entitiesByType[entityType][index] && m_entitiesByType[entityType][index]->IsAlive())
			{
				m_entitiesByType[entityType][index]->Render();
			}
		}
	}
}

void Map::PushEntitesVsEntities()
{
	for (size_t aIndex = 0; aIndex < m_allEntites.size(); aIndex++)
	{
		for (size_t bIndex = aIndex + 1; bIndex < m_allEntites.size(); bIndex++)
		{
			if (m_allEntites[aIndex]->IsAlive() && m_allEntites[bIndex]->IsAlive())
			{
				PushEntityVsEntity(*m_allEntites[aIndex], *m_allEntites[bIndex]);
			}
		}
	}
}

void Map::PushEntityVsEntity(Entity& a, Entity& b)
{
	bool canAPushB = a.m_doesPushEntities && b.m_isPushedByEntities;
	bool canBPushA = a.m_isPushedByEntities && b.m_doesPushEntities;

	if (!canBPushA && !canAPushB)
	{
		return;
	}
	else if (canAPushB && !canBPushA)
	{
		PushDiscOutOfDisc2D(b.m_position, b.m_physicsRadius, a.m_position, a.m_physicsRadius);
	}
	else if (canBPushA && !canAPushB)
	{
		PushDiscOutOfDisc2D(a.m_position, a.m_physicsRadius, b.m_position, b.m_physicsRadius);
	}
	else
	{
		PushDiscsOutOfEachOther2D(a.m_position, a.m_physicsRadius, b.m_position, b.m_physicsRadius);
	}
}

void Map::PushEntitiesOutOfTiles()
{
	for (size_t index = 0; index < m_allEntites.size(); index++)
	{
		Entity& entity = *m_allEntites[index];

		if (entity.m_type == ENTITY_TYPE_GOOD_PLAYER)
		{
			if (!m_isClipping)
			{
				PushEntityOutOfTilePhysics(entity, IntVec2::NORTH);
				PushEntityOutOfTilePhysics(entity, IntVec2::SOUTH);
				PushEntityOutOfTilePhysics(entity, IntVec2::EAST);
				PushEntityOutOfTilePhysics(entity, IntVec2::WEST);

				PushEntityOutOfTilePhysics(entity, IntVec2::NORTH_EAST);
				PushEntityOutOfTilePhysics(entity, IntVec2::NORTH_WEST);
				PushEntityOutOfTilePhysics(entity, IntVec2::SOUTH_EAST);
				PushEntityOutOfTilePhysics(entity, IntVec2::SOUTH_WEST);
			}
		}
		else
		{
			if (entity.m_isPushedByWalls)
			{
				PushEntityOutOfTilePhysics(entity, IntVec2::NORTH);
				PushEntityOutOfTilePhysics(entity, IntVec2::SOUTH);
				PushEntityOutOfTilePhysics(entity, IntVec2::EAST);
				PushEntityOutOfTilePhysics(entity, IntVec2::WEST);

				PushEntityOutOfTilePhysics(entity, IntVec2::NORTH_EAST);
				PushEntityOutOfTilePhysics(entity, IntVec2::NORTH_WEST);
				PushEntityOutOfTilePhysics(entity, IntVec2::SOUTH_EAST);
				PushEntityOutOfTilePhysics(entity, IntVec2::SOUTH_WEST);
			}
		}
	}
}

void Map::SpawnBullet(Vec2 position, float orientation, EntityType type)
{
	Entity* bullet = nullptr;

	if(type == ENTITY_TYPE_GOOD_BULLET || type == ENTITY_TYPE_EXPLOSION)
	{
		bullet = new Bullet(this, position, orientation, FACTION_GOOD, type);
		AddEntityToMap(*bullet);
	}
	else if(type == ENTITY_TYPE_EVIL_BULLET)
	{
		bullet = new Bullet(this, position, orientation, FACTION_EVIL, type);
		AddEntityToMap(*bullet);
	}
}

void Map::SpawnFlameThrower(Vec2 position, float orientation, EntityType type)
{
	Entity* muzzleFlash = new Explosion(this, position, orientation, 1.0f, type, 1.0f, SpriteAnimPlaybackType::ONCE, Vec2(2.5f, 2.5f));
	AddEntityToMap(*muzzleFlash);
}

void Map::SpawnExplosion(Vec2 position, float duration, float scale, EntityType type, Vec2 velocity)
{
	Entity* muzzleFlash = new Explosion(this, position, 0.0f, scale, type, duration, SpriteAnimPlaybackType::ONCE, velocity);
	AddEntityToMap(*muzzleFlash);
}

void Map::ResetDeathTimer()
{
	m_isPlayerDead = false;
	m_deadTimer = 0.0f;
}

bool Map::IsPointInSolid(Vec2 const& referencePoint)
{
	IntVec2 tileCoords = IntVec2(RoundDownToInt(referencePoint.x), RoundDownToInt(referencePoint.y));

	return IsTileSolid(tileCoords);
}

bool Map::IsPointInWater(Vec2 const& referencePoint)
{
	IntVec2 tileCoords = IntVec2(RoundDownToInt(referencePoint.x), RoundDownToInt(referencePoint.y));

	return IsTileWater(tileCoords);
}

bool Map::IsTileSolid(IntVec2 tileCoordinates)
{
	if(tileCoordinates.x < 0 || tileCoordinates.x > m_dimensions.x || tileCoordinates.y < 0 || tileCoordinates.y > m_dimensions.y)
		return true;

	int tileIndex = tileCoordinates.x + (tileCoordinates.y * m_dimensions.x);

	if (m_tiles[tileIndex].m_tileDef->m_isSolid)
	{
		return true;
	}

	return false;
}

bool Map::IsTileWater(IntVec2 tileCoordinates)
{
	if (tileCoordinates.x < 0 || tileCoordinates.x > m_dimensions.x || tileCoordinates.y < 0 || tileCoordinates.y > m_dimensions.y)
		return true;

	int tileIndex = tileCoordinates.x + (tileCoordinates.y * m_dimensions.x);

	if (m_tiles[tileIndex].m_tileDef->m_isWater)
	{
		return true;
	}

	return false;
}

bool Map::HasLineOfSight(Entity& a, Entity& b)
{
	Vec2 forward = b.GetPosition() - a.GetPosition();
	RaycastResult2D raycastResult;
	raycastResult = RaycastVsTiles(a.GetPosition(), forward.GetNormalized(), forward.GetLength());

	return !raycastResult.m_didImpact;
}

bool Map::IsHeatMapOn() const
{
	return m_isHeatMapOn;
}

RaycastResult2D Map::RaycastVsTiles(Vec2 startPos, Vec2 forwardImpactNormal, float maxLength)
{
	RaycastResult2D result;

	result.m_impactDist = maxLength;

	float oneStep = static_cast<float>(1.0f / 100.0f);
	Vec2 rayStepLength = forwardImpactNormal;
	rayStepLength.SetLength(oneStep);
	Vec2 stepPoint = startPos + rayStepLength;

	while (rayStepLength.GetLength() <= maxLength)
	{
		IntVec2 tileCoords = IntVec2(RoundDownToInt(stepPoint.x), RoundDownToInt(stepPoint.y));

		if (IsTileSolid(tileCoords) && !IsTileWater(tileCoords))
		{
			result.m_didImpact = true;
			result.m_impactDist = rayStepLength.GetLength();

			break;
		}

		rayStepLength.SetLength(rayStepLength.GetLength() + oneStep);

		stepPoint = startPos + rayStepLength;
	}

	return result;
}

void Map::CheckEntityVsBullet()
{
	for (size_t i = 0; i < m_entitiesByType[ENTITY_TYPE_EXPLOSION].size(); i++)
	{
		if (m_entitiesByType[ENTITY_TYPE_EXPLOSION][i] && m_entitiesByType[ENTITY_TYPE_EXPLOSION][i]->IsAlive())
		{
			for (size_t j = 0; j < m_allEntites.size(); j++)
			{
				if (m_allEntites[j]->m_faction == FACTION_EVIL && m_allEntites[j]->m_type != ENTITY_TYPE_EVIL_BULLET)
				{
					if (m_allEntites[j] && m_allEntites[j]->IsAlive())
					{
						if (DoDiscsOverlap(m_entitiesByType[ENTITY_TYPE_EXPLOSION][i]->m_position, m_entitiesByType[ENTITY_TYPE_EXPLOSION][i]->m_physicsRadius, m_allEntites[j]->m_position, m_allEntites[j]->m_physicsRadius))
						{
							g_theAudio->StartSound(g_theGame->m_enemyHit);
							m_allEntites[j]->ReactToBullet(*m_entitiesByType[ENTITY_TYPE_EXPLOSION][i]);
							break;
						}
					}
				}
			}
		}
	}

	for (size_t i = 0; i < m_entitiesByType[ENTITY_TYPE_GOOD_BULLET].size(); i++)
	{
		if (m_entitiesByType[ENTITY_TYPE_GOOD_BULLET][i] && m_entitiesByType[ENTITY_TYPE_GOOD_BULLET][i]->IsAlive())
		{
			for (size_t j = 0; j < m_allEntites.size(); j++)
			{
				if (m_allEntites[j]->m_faction == FACTION_EVIL && m_allEntites[j]->m_type != ENTITY_TYPE_EVIL_BULLET)
				{
					if (m_allEntites[j] && m_allEntites[j]->IsAlive())
					{
						if (DoDiscsOverlap(m_entitiesByType[ENTITY_TYPE_GOOD_BULLET][i]->m_position, m_entitiesByType[ENTITY_TYPE_GOOD_BULLET][i]->m_physicsRadius, m_allEntites[j]->m_position, m_allEntites[j]->m_physicsRadius))
						{
							g_theAudio->StartSound(g_theGame->m_enemyHit);
							m_allEntites[j]->ReactToBullet(*m_entitiesByType[ENTITY_TYPE_GOOD_BULLET][i]);
							break;
						}
					}
				}
			}
		}
	}

	for (size_t i = 0; i < m_entitiesByType[ENTITY_TYPE_EVIL_BULLET].size(); i++)
	{
		if (m_entitiesByType[ENTITY_TYPE_EVIL_BULLET][i]->IsAlive())
		{
			for (size_t j = 0; j < m_allEntites.size(); j++)
			{
				if (m_allEntites[j]->m_faction == FACTION_GOOD && m_allEntites[j]->m_type != ENTITY_TYPE_GOOD_BULLET)
				{
					if (m_allEntites[j]->IsAlive())
					{
						if (DoDiscsOverlap(m_entitiesByType[ENTITY_TYPE_EVIL_BULLET][i]->m_position, m_entitiesByType[ENTITY_TYPE_EVIL_BULLET][i]->m_physicsRadius, m_allEntites[j]->m_position, m_allEntites[j]->m_physicsRadius))
						{
							g_theAudio->StartSound(g_theGame->m_playerHit);
							m_allEntites[j]->ReactToBullet(*m_entitiesByType[ENTITY_TYPE_EVIL_BULLET][i]);
							break;
						}
					}
				}
			}
		}
	}
}

IntVec2 Map::GetTileCoordinates(Vec2 position)
{
	int tileX = RoundDownToInt(position.x);
	int tileY = RoundDownToInt(position.y);

	return IntVec2(tileX, tileY);
}

void Map::RenderMap()
{
	RenderTiles();
	RenderEntites();
}

IntVec2 Map::GetDimensions() const
{
	return m_dimensions;
}

Entity* Map::GetPlayerTank() const
{
	for (size_t index = 0; index < m_allEntites.size(); index++)
	{
		if (m_allEntites[index]->m_type == ENTITY_TYPE_GOOD_PLAYER)
		{
			return m_allEntites[index];
		}
	}

	return nullptr;
}
