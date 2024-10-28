#pragma once

#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"

#include "Game/Entity.hpp"

#include <vector>

class VertexBuffer;

class Leo : public Entity
{
	bool					m_isMoving = false;
	bool					m_isTurning = false;
	bool					m_isDebugMode = false;
	bool					m_isShooting = false;
	bool					m_isDebugLine = false;
	float					m_nextBulletTimer = 0.0f;
	float					m_turnTimer = 0.0f;
	float					m_goalDegrees = 0.0f;
	VertexBuffer*			m_gpuMesh = nullptr;
	std::vector<Vertex_PCU> m_cpuMesh;
	RandomNumberGenerator	m_rand;
	Vec2					m_targetDirection = Vec2(0.0f, 0.0f);
public:
	Leo();
	Leo(Map* owner, Vec2 position, float orientation, EntityFaction faction, EntityType type);
	~Leo();

	bool					HasPlayerLineOfSight() const;

	void DebugMode() const;

	virtual void	Update(float deltaseconds) override;
	virtual void	Render() override;
	virtual void	Die() override;
};
