#pragma once

#include "Game/Entity.hpp"

#include <vector>

class VertexBuffer;
class Explosion;

class Bullet : public Entity
{
	std::vector<Vertex_PCU> m_cpuMesh;
	VertexBuffer*			m_gpuMesh			= nullptr;
	bool					m_isDebugMode		= false;
	bool					m_isReflected		= true;
	float					m_aliveDuration		= 0.0f;
public:
	Bullet(Map* owner, Vec2 position, float orientation, EntityFaction faction, EntityType type);
	~Bullet();

	virtual void Update(float deltaseconds) override;
	virtual void Render() override;
	virtual void Die() override;

	void DebugMode() const;
};