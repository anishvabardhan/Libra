#pragma once

#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Audio/AudioSystem.hpp"

#include "Game/Entity.hpp"

#include <vector>

class VertexBuffer;

class Scorpio : public Entity
{
	bool					m_isDebugMode = false;
	bool					m_isShooting = false;
	float					m_nextBulletTimer = 0.0f;
	std::vector<Vertex_PCU> m_bodyCPUMesh;
	std::vector<Vertex_PCU> m_turretCPUMesh;
	std::vector<Vertex_PCU> m_sightCPUMesh;
	VertexBuffer*			m_gpuMesh = nullptr;
	VertexBuffer*			m_gpuMesh2 = nullptr;
	VertexBuffer*			m_gpuMesh3 = nullptr;
	RandomNumberGenerator	m_rand;
public:
							Scorpio();
							Scorpio(Map* owner, Vec2 position, float orientation, EntityFaction faction, EntityType type);
							~Scorpio();

	bool					HasPlayerLineOfSight() const;

	void					DebugMode() const;
	void					RenderLineOfSight() const;

	virtual void			Update(float deltaseconds) override;
	virtual void			Render() override;
	virtual void			Die() override;
};
