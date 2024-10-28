#include "Game/HealthBar.hpp"

#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Renderer/VertexBuffer.hpp"

#include "Game/GameCommon.hpp"

HealthBar::HealthBar(Map* owner, Entity* entityOwner, Vec2 position, int maxHealth)
	: Entity(owner, position, 0.0f, FACTION_NEUTRAL, ENTITY_TYPE_HEALTHBAR), m_entityOwner(entityOwner), m_maxHealth(maxHealth)
{
	m_currentHealth = static_cast<float>(m_maxHealth);

	m_gpuMesh = g_theRenderer->CreateVertexBuffer(sizeof(Vertex_PCU), std::wstring(L"HealthBar"));
	m_gpuMesh2 = g_theRenderer->CreateVertexBuffer(sizeof(Vertex_PCU), std::wstring(L"HealthBar"));
}

HealthBar::~HealthBar()
{
	DELETE_PTR(m_gpuMesh);
	DELETE_PTR(m_gpuMesh2)
}

void HealthBar::Update(float deltaseconds)
{
	UNUSED(deltaseconds);

	if (!m_entityOwner->IsAlive())
	{
		Die();
	}

	m_position = m_entityOwner->GetPosition();

	m_currentHealth = static_cast<float>(m_entityOwner->GetHealth());

	m_cpuMesh.clear();

	AABB2 boxBounds = AABB2(Vec2(0.0f, 0.0f), Vec2(static_cast<float>(m_maxHealth / m_maxHealth), 0.15f));

	AddVertsForAABB2D(m_cpuMesh, boxBounds, Rgba8(255, 0, 0, 255));

	TransformVertexArrayXY3D(static_cast<int>(m_cpuMesh.size()), m_cpuMesh.data(), m_scale, m_orientationDegrees, Vec2(m_position.x - 0.5f, m_position.y + 0.35f));

	g_theRenderer->CopyCPUToGPU(m_cpuMesh.data(), (int)m_cpuMesh.size() * sizeof(Vertex_PCU), m_gpuMesh);

	m_cpuMesh2.clear();

	AABB2 healthBounds = AABB2(Vec2(0.0f, 0.0f), Vec2(static_cast<float>(m_currentHealth / m_maxHealth), 0.15f));

	AddVertsForAABB2D(m_cpuMesh2, healthBounds, Rgba8(0, 255, 0, 255));

	TransformVertexArrayXY3D(static_cast<int>(m_cpuMesh2.size()), m_cpuMesh2.data(), m_scale, m_orientationDegrees, Vec2(m_position.x - 0.5f, m_position.y + 0.35f));
	g_theRenderer->CopyCPUToGPU(m_cpuMesh2.data(), (int)m_cpuMesh2.size() * sizeof(Vertex_PCU), m_gpuMesh2);

}

void HealthBar::Render()
{
	//std::vector<Vertex_PCU> boxVerts;

	/*AABB2 boxBounds = AABB2(Vec2(0.0f, 0.0f), Vec2(static_cast<float>(m_maxHealth / m_maxHealth), 0.15f));

	AddVertsForAABB2D(boxVerts, boxBounds, Rgba8(255, 0, 0, 255));

	TransformVertexArrayXY3D(static_cast<int>(boxVerts.size()), boxVerts.data(), m_scale, m_orientationDegrees, Vec2(m_position.x - 0.5f, m_position.y + 0.35f));
	*/g_theRenderer->SetModelConstants();
	g_theRenderer->BindTexture(nullptr);
	g_theRenderer->SetBlendMode(BlendMode::ALPHA);
	g_theRenderer->SetDepthMode(DepthMode::ENABLED);
	g_theRenderer->SetRasterizerMode(RasterizerMode::SOLID_CULL_BACK);
	g_theRenderer->SetSamplerMode(SamplerMode::POINT_CLAMP);
	g_theRenderer->DrawVertexBuffer(m_gpuMesh, (int)m_cpuMesh.size(), sizeof(Vertex_PCU));
	//g_theRenderer->DrawVertexArray(static_cast<int>(boxVerts.size()), boxVerts.data());

	//std::vector<Vertex_PCU> healthVerts;
/*
	AABB2 healthBounds = AABB2(Vec2(0.0f, 0.0f), Vec2(static_cast<float>(m_currentHealth / m_maxHealth), 0.15f));

	AddVertsForAABB2D(healthVerts, healthBounds, Rgba8(0, 255, 0, 255));

	TransformVertexArrayXY3D(static_cast<int>(healthVerts.size()), healthVerts.data(), m_scale, m_orientationDegrees, Vec2(m_position.x - 0.5f, m_position.y + 0.35f));
	*/g_theRenderer->SetModelConstants();
	g_theRenderer->SetBlendMode(BlendMode::ALPHA);
	g_theRenderer->SetDepthMode(DepthMode::ENABLED);
	g_theRenderer->SetRasterizerMode(RasterizerMode::SOLID_CULL_BACK);
	g_theRenderer->SetSamplerMode(SamplerMode::POINT_CLAMP);
	g_theRenderer->BindTexture(nullptr);
	g_theRenderer->DrawVertexBuffer(m_gpuMesh2, (int)m_cpuMesh2.size(), sizeof(Vertex_PCU));
	//g_theRenderer->DrawVertexArray(static_cast<int>(healthVerts.size()), healthVerts.data());
}

void HealthBar::Die()
{
	m_isDead = true;
}
