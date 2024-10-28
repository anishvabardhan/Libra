#pragma once

#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/RaycastUtils.hpp"
#include "Engine/Core/Vertex_PCU.hpp"

#include "Game/Tile.hpp"
#include "Game/Entity.hpp"

#include <string>
#include <vector>

typedef std::vector<Entity*> EntityList;

class Game;
class Explosion;
class SpriteSheet;
class Texture;
class VertexBuffer;
class TileHeatMap;

enum DistanceFieldMap
{
	UNKNOWN = -1,
	LAND,
	LAND_AND_WATER,

	TOTAL_MAPS
};

struct MapDefinition
{
	std::string					m_name				= "";
	IntVec2						m_dimensions		= IntVec2::ZERO;
	std::string					m_fillType			= "";
	std::string					m_borderType		= "";
	std::string					m_bunkerFloorType	= "";
	std::string					m_bunkerWallType	= "";
	std::string					m_mapStartType		= "";
	std::string					m_mapEndType		= "";
	std::string					m_wormType1			= "";
	std::string					m_wormType2			= "";
	int							m_numOfWorms1		= 0;
	int							m_worm1length		= 0;
	int							m_numOfWorms2		= 0;
	int							m_worm2length		= 0;
	int							m_numOfLeo			= 0;
	int							m_numOfAries		= 0;
	int							m_numOfScorpio		= 0;
	int							m_numOfCapricorn	= 0;

	static MapDefinition		s_definitions[3];

	static void					InitializeDef();
};

class Map
{
	EntityList					m_allEntites;
	EntityList					m_entitiesByType[NUM_ENTITY_TYPES];
	SpriteSheet*				m_terrainSpriteSheet = nullptr;
	std::vector<Tile>			m_tiles;
	std::vector<Vertex_PCU>		m_mapCPUMesh;
	VertexBuffer*				m_mapGPUMesh = nullptr;
	MapDefinition				m_definition;
	IntVec2						m_dimensions;
	TileHeatMap*				m_tileHeatMap = nullptr;
	bool						m_isClipping = false;
	bool						m_isPlayerDead = false;
	float						m_deadTimer = 0.0f;
	int							m_mapGenerationCounter = 0;
	int							m_enemyCounter = 0;
	Vec2						m_exitPosition;
public:
	bool						m_isHeatMapOn = false;

								Map();
								Map(IntVec2 dimensions);
								Map(MapDefinition definition);
								~Map();

	Entity*						SpawnNewEntity( EntityType type, Vec2 const& position, float orientationDegrees );
	void						PopulateDistanceField( TileHeatMap& out_distanceField, IntVec2 startCoords, float maxCost, bool treatWaterAsSolid=true );
	void						SpawnBullet(Vec2 position, float orientation, EntityType type);
	void						SpawnFlameThrower(Vec2 position, float orientation, EntityType type);
	void						SpawnExplosion(Vec2 position, float duration, float scale, EntityType type, Vec2 velocity = Vec2::ZERO);
	void						RespawnPlayer();
	void						DeleteGarbageEntites();
	void						InitializeMap();
	void						InitializeLeo();
	void						InitializeAries();
	void						InitializeScorpio();
	void						InitializeCapicorn();
	void						AddEntityToMap(Entity& e);
	void						ResetDeathTimer();
	void						RemoveEntityFromMap(Entity& e);
	void						AddVertsForTile(std::vector<Vertex_PCU>& verts, int tileIndex, SpriteSheet const& spriteSheet);
	void						HandleInput();
	void						PushEntitesVsEntities();
	void						PushEntityVsEntity(Entity& a, Entity& b);
	void						PushEntitiesOutOfTiles();
	void						PushEntityOutOfTilePhysics(Entity& e, IntVec2 direction);
	bool						IsPointInSolid(Vec2 const& referencePoint);
	bool						IsPointInWater(Vec2 const& referencePoint);
	bool						IsTileSolid(IntVec2 tileCoordinates);
	bool						IsTileWater(IntVec2 tileCoordinates);
	bool						HasLineOfSight(Entity& a, Entity& b);
	bool						IsHeatMapOn() const;
	RaycastResult2D				RaycastVsTiles(Vec2 startPos, Vec2 forwardImpactNormal, float maxLength);
	void						CheckEntityVsBullet();
	void						Update(float deltaseconds);
	void						UpdateEntities(float deltaseconds);
	void						RenderTiles();
	void						RenderEntites();
	void						RenderMap();
	void						PlayerDead();
	IntVec2						GetTileCoordinates(Vec2 referencePosition);
	IntVec2						GetDimensions() const;
	Vec2						GetExitPosition() const;
	MapDefinition				GetDefinition() const;
	Entity*						GetPlayerTank() const;
};