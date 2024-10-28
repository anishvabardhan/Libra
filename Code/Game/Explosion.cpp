#include "Game/Explosion.hpp"

#include "Engine/Renderer/VertexBuffer.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Core/EngineCommon.hpp"

#include "Game/GameCommon.hpp"

Explosion::Explosion(Map* owner, Vec2 position, float orientation, float scale, EntityType type, float animDuration, SpriteAnimPlaybackType playback, Vec2 velocity)
	: Entity(owner, position, orientation, FACTION_NEUTRAL, type)
{
	m_scale = scale;
	m_velocity = velocity;
	m_health = 1;
	m_explosionSpriteSheet = new SpriteSheet(*g_theGame->m_allTextures[GAME_EXPLOSION_TEXTURE], IntVec2(5, 5));
	m_explosionAnim = new SpriteAnimDefinition(*m_explosionSpriteSheet, 0, 24, animDuration, playback);
	m_physicsRadius = g_gameConfigBlackboard.GetValue("flamethrowerPhysicsRadius", 1.0f);
	m_cosmeticRadius = g_gameConfigBlackboard.GetValue("flamethrowerCosmeticRadius", 1.0f);

	m_gpuMesh = g_theRenderer->CreateVertexBuffer(sizeof(Vertex_PCU), std::wstring(L"Explosion"));
}

Explosion::~Explosion()
{
	DELETE_PTR(m_gpuMesh);
}

void Explosion::Update(float deltaseconds)
{
	if (g_theInputSystem->WasKeyJustPressed(KEYCODE_F1))
	{
		m_isDebugMode = !m_isDebugMode;
	}

	if (m_health <= 0)
	{
		m_aliveTime = 0.0f;
		Die();
	}

	m_explosionAnim->Update(deltaseconds);

	float oneStep = static_cast<float>(1.0f / 10.0f);
	Vec2 forwardNormal = GetForwardNormal();
	Vec2 oneStepRay = GetForwardNormal();
	oneStepRay.SetLength(oneStep);

	Vec2 currentPosition = m_position;
	Vec2 nextPosition = currentPosition + oneStepRay;

	if (m_owner->IsPointInSolid(nextPosition) && !m_owner->IsPointInWater(nextPosition))
	{
		TakeDamage(1);
	}

	m_position += m_velocity * GetForwardNormal() * deltaseconds;

	m_cpuMesh.clear();

	Vec2 uvMins, uvMaxs;
	m_explosionAnim->GetSpriteDefAtTime(m_explosionAnim->GetElapsedTime()).GetUVs(uvMins, uvMaxs);

	AddVertsForAABB2D(m_cpuMesh, AABB2(Vec2(-0.5f, -0.5f), Vec2(0.5f, 0.5f)), Rgba8(255, 255, 255, 255), uvMins, uvMaxs);

	TransformVertexArrayXY3D(static_cast<int>(m_cpuMesh.size()), m_cpuMesh.data(), m_scale, m_orientationDegrees, m_position);

	g_theRenderer->CopyCPUToGPU(m_cpuMesh.data(), (int)m_cpuMesh.size() * sizeof(Vertex_PCU), m_gpuMesh);
}

void Explosion::Render()
{
	//std::vector<Vertex_PCU> explosionVerts;

	//Vec2 uvMins, uvMaxs;
	//m_explosionAnim->GetSpriteDefAtTime(m_explosionAnim->GetElapsedTime()).GetUVs(uvMins, uvMaxs);

	//AddVertsForAABB2D(explosionVerts, AABB2(Vec2(-0.5f, -0.5f), Vec2(0.5f, 0.5f)), Rgba8(255, 255, 255, 255), uvMins, uvMaxs);

	//TransformVertexArrayXY3D(static_cast<int>(explosionVerts.size()), explosionVerts.data(), m_scale, m_orientationDegrees, m_position);

	g_theRenderer->SetModelConstants();
	g_theRenderer->BindTexture(&m_explosionSpriteSheet->GetTexture());
	g_theRenderer->BindShader();
	g_theRenderer->SetBlendMode(BlendMode::ADDITIVE);
	g_theRenderer->SetDepthMode(DepthMode::ENABLED);
	g_theRenderer->SetRasterizerMode(RasterizerMode::SOLID_CULL_BACK);
	g_theRenderer->SetSamplerMode(SamplerMode::POINT_CLAMP);
	g_theRenderer->DrawVertexBuffer(m_gpuMesh, (int)m_cpuMesh.size(), sizeof(Vertex_PCU));

	if (m_isDebugMode)
	{
		DebugMode();
	}
}

void Explosion::Die()
{
	m_isDead = true;
}

void Explosion::DebugMode() const
{
	Vec2 normal = GetForwardNormal();
	normal.SetLength(m_cosmeticRadius);

	DrawDebugRing(m_position, m_physicsRadius, 1.0f, m_orientationDegrees, 0.01f, Rgba8(0, 255, 255, 255));
	DrawDebugRing(m_position, m_cosmeticRadius, 1.0f, m_orientationDegrees, 0.01f, Rgba8(255, 0, 255, 255));
	DrawDebugLine(m_position, m_position + (1.5f * normal).GetRotated90Degrees(), 0.01f, Rgba8(0, 255, 0, 255));
	DrawDebugLine(m_position, m_position + (1.5f * normal), 0.01f, Rgba8(255, 0, 0, 255));
}