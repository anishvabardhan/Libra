#pragma once

#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"

#include "Game/Entity.hpp"

#include <vector>

class VertexBuffer;

class Aries : public Entity
{
	bool m_isMoving = false;
	bool m_isTurning = false;
	bool m_isDebugMode = false;
	bool m_isDebugLine = false;
	float m_turnTimer = 0.0f;
	float m_goalDegrees = 0.0f;
	RandomNumberGenerator m_rand;
	std::vector<Vertex_PCU> m_cpuMesh;
	VertexBuffer* m_gpuMesh = nullptr;
	Vec2 m_targetDirection = Vec2(0.0f, 0.0f);
public:
	Aries();
	Aries(Map* owner, Vec2 position, float orientation, EntityFaction faction, EntityType type);
	~Aries();

	void DebugMode() const;
	void RenderShield() const;
	bool					HasPlayerLineOfSight() const;

	virtual void	Update(float deltaseconds) override;
	virtual void	Render() override;
	virtual void	Die() override;
	virtual void	ReactToBullet(Entity& bullet) override;
};