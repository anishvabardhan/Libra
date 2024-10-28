#pragma once

#include "Game/Entity.hpp"

#include "Engine/Renderer/SpriteAnimDefinition.hpp"

#include <vector>

class VertexBuffer;
class SpriteSheet;

class Explosion : public Entity
{
	bool					m_isDebugMode			= false;
	SpriteSheet*			m_explosionSpriteSheet	= nullptr;
	SpriteAnimDefinition*	m_explosionAnim			= nullptr;
	std::vector<Vertex_PCU> m_cpuMesh;
	VertexBuffer*			m_gpuMesh				= nullptr;
public:
	Explosion(Map* owner, Vec2 position, float orientation, float scale, EntityType type, float animDuration, SpriteAnimPlaybackType playback, Vec2 velocity);
	~Explosion();

	virtual void Update(float deltaseconds) override;
	virtual void Render() override;
	virtual void Die() override;

	void DebugMode() const;
};