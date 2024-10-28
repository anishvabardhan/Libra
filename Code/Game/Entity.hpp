#pragma once

#include "Engine/Math/Vec2.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Core/TileHeatMap.hpp"

class Map;

enum EntityFaction
{
	FACTION_UNKNOWN = -1,
	FACTION_GOOD,
	FACTION_NEUTRAL,
	FACTION_EVIL,

	NUM_FACTION_TYPES
};

enum EntityType
{
	ENTITY_TYPE_UNKNOWN = -1,
	ENTITY_TYPE_EVIL_SCORPIO, 
	ENTITY_TYPE_EVIL_LEO, 
	ENTITY_TYPE_EVIL_ARIES, 
	ENTITY_TYPE_EVIL_CAPRICORN,
	ENTITY_TYPE_GOOD_PLAYER, 
	ENTITY_TYPE_GOOD_BULLET, 
	ENTITY_TYPE_EVIL_BULLET,
	ENTITY_TYPE_GUIDED_MISSILE,
	ENTITY_TYPE_EXPLOSION,
	ENTITY_TYPE_HEALTHBAR,

	NUM_ENTITY_TYPES
};

class Entity
{
public:
	EntityType		m_type;
	EntityFaction	m_faction;
	Map*			m_owner					= nullptr;
	TileHeatMap*	m_distanceToTargetMap	= nullptr;
	Vec2			m_targetPosition;
	Vec2			m_position;
	Vec2			m_velocity;
	float			m_aliveTime				= 0.0f;
	float			m_distanceToPlayer		= 0.0f;
	float			m_orientationDegrees	= 0.0f;
	float			m_scale					= 1.0f;
	float			m_angularVelocity		= 0.0f;
	float			m_physicsRadius			= 1.0f;
	float			m_cosmeticRadius		= 2.0f;
	int				m_health				= 3;
	int				m_discoveryCounter		= 0;
	Rgba8			m_color					= Rgba8(0, 0, 0, 255);
	bool			m_canSwim				= false;
	bool			m_isDead				= false;
	bool			m_isHeatMapOn			= false;
	bool			m_isGarbage				= false;
	bool			m_didDiscover = false;
	bool			m_isPushedByEntities	= false;
	bool			m_doesPushEntities		= false;
	bool			m_isPushedByWalls		= false;
	bool			m_isHitByBullets		= false;
	bool			m_hasLineOfSight		= false;
	Vec2			m_lastPlayerPosition;

public:
					Entity() = default;
					Entity(Map* owner, Vec2 startPos, float orientation, EntityFaction faction, EntityType type);
	virtual			~Entity();

	virtual void	Update(float deltaseconds) = 0;
	virtual void	Render() = 0;
	virtual void	ReactToBullet(Entity& bullet);
	virtual void	Die();

	void			TakeDamage(int damagePoints);
	bool			IsOffScreen();
	bool			IsAlive() const;
	bool			IsGarbage() const;
	EntityType		GetType() const;
	Vec2			GetPosition() const;
	Vec2			GetVelocity() const;
	float			GetOrientation() const;
	Vec2			GetForwardNormal() const;
	float			GetAliveTime() const;
	int				GetHealth() const;
};