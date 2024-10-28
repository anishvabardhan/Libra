#include "Game/Bullet.hpp"

#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Renderer/VertexBuffer.hpp"

#include "Game/GameCommon.hpp"
#include "Game/Explosion.hpp"

#include <vector>

Bullet::Bullet(Map* owner, Vec2 position, float orientation, EntityFaction faction, EntityType type)
	: Entity(owner, position, orientation, faction, type)
{
	switch (type)
	{
	case ENTITY_TYPE_GOOD_BULLET:
	{
		m_velocity = Vec2(g_gameConfigBlackboard.GetValue("bulletSpeed", 1.0f), g_gameConfigBlackboard.GetValue("bulletSpeed", 1.0f));
		m_color = Rgba8::WHITE;
		m_physicsRadius = g_gameConfigBlackboard.GetValue("bulletPhysicsRadius", 1.0f);
		m_cosmeticRadius = g_gameConfigBlackboard.GetValue("bulletCosmeticRadius", 1.0f);
		m_isPushedByEntities = false;
		m_doesPushEntities = false;
		m_canSwim = true;
		m_health = 3;
		m_aliveDuration = 2.0f;
		break;
	}
	case ENTITY_TYPE_EVIL_BULLET:
	{
		m_velocity = Vec2(g_gameConfigBlackboard.GetValue("bulletSpeed", 1.0f), g_gameConfigBlackboard.GetValue("bulletSpeed", 1.0f));
		m_color = Rgba8::WHITE;
		m_physicsRadius = g_gameConfigBlackboard.GetValue("bulletPhysicsRadius", 1.0f);
		m_cosmeticRadius = g_gameConfigBlackboard.GetValue("bulletCosmeticRadius", 1.0f);
		m_isPushedByEntities = false;
		m_doesPushEntities = false;
		m_canSwim = true;
		m_health = 3;
		m_aliveDuration = 2.0f;
		break;
	}
	}

	m_gpuMesh = g_theRenderer->CreateVertexBuffer(sizeof(Vertex_PCU), std::wstring(L"Bullet"));
}

Bullet::~Bullet()
{
	DELETE_PTR(m_gpuMesh);
}

void Bullet::Update(float deltaseconds)
{
	m_aliveTime += deltaseconds;

	if (g_theInputSystem->WasKeyJustPressed(KEYCODE_F1))
	{
		m_isDebugMode = !m_isDebugMode;
	}

	if (m_health <= 0 || m_aliveTime > m_aliveDuration)
	{
		m_aliveTime = 0.0f;
		Die();
	}

	float oneStep = static_cast<float>(1.0f / 10.0f);
	Vec2 forwardNormal = GetForwardNormal();
	Vec2 oneStepRay = GetForwardNormal();
	oneStepRay.SetLength(oneStep);

	Vec2 currentPosition = m_position;
	Vec2 nextPosition = currentPosition + oneStepRay;

	if (m_owner->IsPointInSolid(nextPosition) && !m_owner->IsPointInWater(nextPosition))
	{
		TakeDamage(1);
		IntVec2 tileCoords = m_owner->GetTileCoordinates(nextPosition);

		Vec2 tileMins = Vec2(static_cast<float>(tileCoords.x), static_cast<float>(tileCoords.y));
		Vec2 tileMaxs = Vec2(static_cast<float>(tileCoords.x + 1), static_cast<float>(tileCoords.y + 1));

		AABB2 tileBounds = AABB2(tileMins, tileMaxs);

		Vec2 impactNormal = (currentPosition - tileBounds.GetNearestPoint(currentPosition)).GetNormalized();

		g_theAudio->StartSound(g_theGame->m_bulletRicochet);
		Vec2 reflected = forwardNormal.GetReflected(impactNormal);
		m_orientationDegrees = reflected.GetOrientationDegrees();
	}

	m_position += m_velocity * GetForwardNormal() * deltaseconds;

	switch (m_type)
	{
	case ENTITY_TYPE_GOOD_BULLET:
	{
		m_cpuMesh.clear();

		AddVertsForAABB2D(m_cpuMesh, AABB2(-0.1f, -0.04f, 0.1f, 0.04f), m_color);

		TransformVertexArrayXY3D(static_cast<int>(m_cpuMesh.size()), m_cpuMesh.data(), 1.0f, m_orientationDegrees, m_position);

		g_theRenderer->CopyCPUToGPU(m_cpuMesh.data(), (int)m_cpuMesh.size() * sizeof(Vertex_PCU), m_gpuMesh);

		break;
	}
	case ENTITY_TYPE_EVIL_BULLET:
	{
		m_cpuMesh.clear();

		AddVertsForAABB2D(m_cpuMesh, AABB2(-0.1f, -0.04f, 0.1f, 0.04f), m_color);

		TransformVertexArrayXY3D(static_cast<int>(m_cpuMesh.size()), m_cpuMesh.data(), 1.0f, m_orientationDegrees, m_position);

		g_theRenderer->CopyCPUToGPU(m_cpuMesh.data(), (int)m_cpuMesh.size() * sizeof(Vertex_PCU), m_gpuMesh);

		break;
	}
	}
}

void Bullet::Render()
{
	//std::vector<Vertex_PCU> vertices;

	switch (m_type)
	{
	case ENTITY_TYPE_GOOD_BULLET:
	{
		//AddVertsForAABB2D(vertices, AABB2(-0.1f, -0.04f, 0.1f, 0.04f), m_color);

		//g_theRenderer->SetBlendMode(BlendMode::ALPHA);
		//TransformVertexArrayXY3D(static_cast<int>(vertices.size()), vertices.data(), 1.0f, m_orientationDegrees, m_position);
		g_theRenderer->SetModelConstants();
		g_theRenderer->SetBlendMode(BlendMode::ALPHA);
		g_theRenderer->SetDepthMode(DepthMode::ENABLED);
		g_theRenderer->SetRasterizerMode(RasterizerMode::SOLID_CULL_BACK);
		g_theRenderer->SetSamplerMode(SamplerMode::POINT_CLAMP);
		g_theRenderer->BindShader();
		g_theRenderer->BindTexture(g_theGame->m_allTextures[GAME_PLAYER_BULLET_TEXTURE]);
		g_theRenderer->DrawVertexBuffer(m_gpuMesh, (int)m_cpuMesh.size(), sizeof(Vertex_PCU));
		//g_theRenderer->DrawVertexArray(static_cast<int>(vertices.size()), vertices.data());
		//g_theRenderer->SetBlendMode(BlendMode::ALPHA);

		break;
	}
	case ENTITY_TYPE_EVIL_BULLET:
	{
		//AddVertsForAABB2D(vertices, AABB2(-0.1f, -0.04f, 0.1f, 0.04f), m_color);

		//g_theRenderer->SetBlendMode(BlendMode::ALPHA);
		//TransformVertexArrayXY3D(static_cast<int>(vertices.size()), vertices.data(), 1.0f, m_orientationDegrees, m_position);
		g_theRenderer->SetModelConstants();
		g_theRenderer->SetBlendMode(BlendMode::ALPHA);
		g_theRenderer->SetDepthMode(DepthMode::ENABLED);
		g_theRenderer->SetRasterizerMode(RasterizerMode::SOLID_CULL_BACK);
		g_theRenderer->SetSamplerMode(SamplerMode::POINT_CLAMP);
		g_theRenderer->BindShader();
		g_theRenderer->BindTexture(g_theGame->m_allTextures[GAME_ENEMY_BULLET_TEXTURE]);
		g_theRenderer->DrawVertexBuffer(m_gpuMesh, (int)m_cpuMesh.size(), sizeof(Vertex_PCU));
		//g_theRenderer->DrawVertexArray(static_cast<int>(vertices.size()), vertices.data());
		//g_theRenderer->SetBlendMode(BlendMode::ALPHA);

		break;
	}
	}

	if (m_isDebugMode)
	{
		DebugMode();
	}
}

void Bullet::Die()
{
	m_isDead = true;
}

void Bullet::DebugMode() const
{
	Vec2 normal = GetForwardNormal();
	normal.SetLength(m_cosmeticRadius);

	DrawDebugRing(m_position, m_physicsRadius, 1.0f, m_orientationDegrees, 0.01f, Rgba8(0, 255, 255, 255));
	DrawDebugRing(m_position, m_cosmeticRadius, 1.0f, m_orientationDegrees, 0.01f, Rgba8(255, 0, 255, 255));
	DrawDebugLine(m_position, m_position + (1.5f * normal).GetRotated90Degrees(), 0.01f, Rgba8(0, 255, 0, 255));
	DrawDebugLine(m_position, m_position + (1.5f * normal), 0.01f, Rgba8(255, 0, 0, 255));
}
