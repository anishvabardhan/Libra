#include "Game/Entity.hpp"

#include "Engine/Core/EngineCommon.hpp"

#include "Game/GameCommon.hpp"

#include <math.h>

Entity::Entity(Map* owner, Vec2 startPos, float orientation, EntityFaction faction, EntityType type)
	: m_owner(owner), m_position(startPos), m_orientationDegrees(orientation), m_faction(faction), m_type(type)
{
}

Entity::~Entity()
{
}

void Entity::Update(float deltaseconds)
{
	UNUSED(deltaseconds);
}

void Entity::Render()
{
}

void Entity::ReactToBullet(Entity& bullet)
{
	TakeDamage(1);
	bullet.Die();
}

void Entity::Die()
{
}

void Entity::TakeDamage(int damagePoints)
{
	m_health -= damagePoints;
}

bool Entity::IsOffScreen()
{
	if((m_position.x >= g_gameConfigBlackboard.GetValue("worldSizeX", 20.0f) + m_cosmeticRadius) || (m_position.x <= -m_cosmeticRadius))
		return true;

	if ((m_position.y >= g_gameConfigBlackboard.GetValue("worldSizeY", 20.0f) + m_cosmeticRadius) || (m_position.y <= -m_cosmeticRadius))
		return true;

	return false;
}

bool Entity::IsAlive() const
{
	return !m_isDead;
}

bool Entity::IsGarbage() const
{
	return m_isGarbage;
}

EntityType Entity::GetType() const
{
	return m_type;
}

Vec2 Entity::GetPosition() const
{
	return m_position;
}

Vec2 Entity::GetVelocity() const
{
	return m_velocity;
}

float Entity::GetOrientation() const
{
	return m_orientationDegrees;
}

Vec2 Entity::GetForwardNormal() const
{
	Vec2 radians;

	radians.x = m_orientationDegrees * (3.14f / 180.0f);
	radians.y = m_orientationDegrees * (3.14f / 180.0f);

	return Vec2(cosf(radians.x), sinf(radians.y));
}

float Entity::GetAliveTime() const
{
	return m_aliveTime;
}

int Entity::GetHealth() const
{
	return m_health;
}
