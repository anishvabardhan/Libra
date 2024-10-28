#pragma once

#include "Game/Entity.hpp"

#include <vector>

class Map;
class VertexBuffer;
struct Vec2;

class HealthBar : public Entity
{
	int						m_maxHealth;
	float					m_currentHealth;
	Entity*					m_entityOwner = nullptr;
	std::vector<Vertex_PCU> m_cpuMesh;
	std::vector<Vertex_PCU> m_cpuMesh2;
	VertexBuffer*			m_gpuMesh = nullptr;
	VertexBuffer*			m_gpuMesh2 = nullptr;
public:
	HealthBar(Map* owner, Entity* entityOwner, Vec2 position, int maxHealth);
	~HealthBar();

	virtual void	Update(float deltaseconds) override;
	virtual void	Render() override;
	virtual void	Die() override;
};