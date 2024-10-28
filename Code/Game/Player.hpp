#pragma once

#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Audio/AudioSystem.hpp"

#include "Game/Entity.hpp"

#include <vector>

class VertexBuffer;

enum PlayerWeapon
{
	BULLET,
	FLAMETHROWER,

	TOTAL_WEAPONS
};

class Player : public Entity
{
	PlayerWeapon			m_weaponType;
	std::vector<Vertex_PCU> m_bodyCPUMesh;
	std::vector<Vertex_PCU> m_turretCPUMesh;
	VertexBuffer*			m_gpuMesh = nullptr;
	VertexBuffer*			m_gpuMesh2 = nullptr;
	bool					m_isMoving = false;
	bool					m_isTurretMoving = false;
	bool					m_isDebugMode = false;
	bool					m_isInvincible = false;
	bool					m_isShooting = false;
	float					m_nextProjectileTimer = 0.0f;
	float					m_turretOrientation = 0.0f;
	float					m_projectileShootTimer[TOTAL_WEAPONS] = {0.1f, 0.05f};
	Vec2					m_targetDirection = Vec2(0.0f, 0.0f);
	Vec2					m_turretAbsoluteDirection = Vec2(0.0f, 0.0f);
public:
							Player();
							Player(Map* owner, Vec2 position, float orientation, EntityFaction faction, EntityType type);
							~Player();

	void					HandleInput();
	void					DebugMode() const;

	virtual void			Update(float deltaseconds) override;
	virtual void			Render() override;
	virtual void			Die() override;
	virtual void			ReactToBullet(Entity& bullet) override;
};