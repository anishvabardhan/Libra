#include "Game/Scorpio.hpp"

#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Renderer/VertexBuffer.hpp"

#include "Game/GameCommon.hpp"
#include "Game/Game.hpp"

Scorpio::Scorpio()
{
}

Scorpio::Scorpio(Map* owner, Vec2 position, float orientation, EntityFaction faction, EntityType type)
	: Entity(owner, position, orientation, faction, type)
{
	m_isPushedByEntities = false;
	m_doesPushEntities = true;
	m_isPushedByWalls = true;
	m_isHitByBullets = true;
	m_hasLineOfSight = true;
	m_canSwim = false;	
	m_physicsRadius = g_gameConfigBlackboard.GetValue("scorpioPhysicsRadius", 1.0f);
	m_cosmeticRadius = g_gameConfigBlackboard.GetValue("scorpioCosmeticRadius", 1.0f);
	m_health = 10;

	m_gpuMesh = g_theRenderer->CreateVertexBuffer(sizeof(Vertex_PCU), std::wstring(L"Scorpio"));
	m_gpuMesh2 = g_theRenderer->CreateVertexBuffer(sizeof(Vertex_PCU), std::wstring(L"Scorpio"));
	m_gpuMesh3 = g_theRenderer->CreateVertexBuffer(sizeof(Vertex_PCU), std::wstring(L"Scorpio"));
}

Scorpio::~Scorpio()
{
	DELETE_PTR(m_gpuMesh);
	DELETE_PTR(m_gpuMesh2);
	DELETE_PTR(m_gpuMesh3);
}

bool Scorpio::HasPlayerLineOfSight() const
{
	Vec2 turretForward = Vec2::MakeFromPolarDegrees(m_orientationDegrees, m_cosmeticRadius);

	Vec2 forward = m_owner->GetPlayerTank()->GetPosition() - m_position - turretForward;
	RaycastResult2D raycastResult;
	raycastResult = m_owner->RaycastVsTiles(m_position + turretForward, forward.GetNormalized(), forward.GetLength());

	return !raycastResult.m_didImpact;
}

void Scorpio::DebugMode() const
{
	Vec2 normal = GetForwardNormal();
	normal.SetLength(m_cosmeticRadius);

	Vec2 turretForwardNormal = Vec2::MakeFromPolarDegrees(m_orientationDegrees, m_cosmeticRadius);

	DrawDebugLine(m_position, m_position + turretForwardNormal, 0.15f, Rgba8(0, 0, 255, 255));
	DrawDebugRing(m_position, m_physicsRadius, 1.0f, m_orientationDegrees, 0.03f, Rgba8(0, 255, 255, 255));
	DrawDebugRing(m_position, m_cosmeticRadius, 1.0f, m_orientationDegrees, 0.03f, Rgba8(255, 0, 255, 255));
	DrawDebugLine(m_position, m_position + (1.5f * normal).GetRotated90Degrees(), 0.03f, Rgba8(0, 255, 0, 255));
	DrawDebugLine(m_position, m_position + (1.5f * normal), 0.03f, Rgba8(255, 0, 0, 255));

	if (HasPlayerLineOfSight())
	{
		DrawDebugLine(m_position, m_owner->GetPlayerTank()->GetPosition(), 0.02f, Rgba8(0, 0, 0, 255));
	}
}

void Scorpio::RenderLineOfSight() const
{/*
	Vec2 turretForward = Vec2::MakeFromPolarDegrees(m_orientationDegrees, m_cosmeticRadius);

	RaycastResult2D raycast =m_owner->RaycastVsTiles(m_position + turretForward, turretForward.GetNormalized(), 10.0f);

	Vec2 lineOfSightEnd = Vec2::MakeFromPolarDegrees(m_orientationDegrees, raycast.m_impactDist);

	Vec2 direction = m_position + turretForward + lineOfSightEnd - m_position - turretForward;
	Vec2 forward = direction.GetNormalized();

	forward.SetLength(0.05f / 2.0f);

	Vec2 left = forward.GetRotated90Degrees();

	Vec2 endLeft = m_position + turretForward + lineOfSightEnd + forward + left;
	Vec2 endRight = m_position + turretForward + lineOfSightEnd + forward - left;
	Vec2 startLeft = m_position + turretForward - forward + left;
	Vec2 startRight = m_position + turretForward - forward - left;

	Vertex_PCU vertices[6];

	vertices[0] = Vertex_PCU(startLeft.x, startLeft.y, 255, 0, 0, 255);
	vertices[1] = Vertex_PCU(startRight.x, startRight.y, 255, 0, 0, 255);
	vertices[2] = Vertex_PCU(endRight.x, endRight.y, 255, 0, 0, 0);
	vertices[3] = Vertex_PCU(endRight.x, endRight.y, 255, 0, 0, 0);
	vertices[4] = Vertex_PCU(endLeft.x, endLeft.y, 255, 0, 0, 0);
	vertices[5] = Vertex_PCU(startLeft.x, startLeft.y, 255, 0, 0, 255);

	TransformVertexArrayXY3D(6, vertices, 1.0f, 0.0f, Vec2(0.0f, 0.0f));*/
	g_theRenderer->SetModelConstants();
	g_theRenderer->SetBlendMode(BlendMode::ALPHA);
	g_theRenderer->SetDepthMode(DepthMode::ENABLED);
	g_theRenderer->SetRasterizerMode(RasterizerMode::SOLID_CULL_BACK);
	g_theRenderer->SetSamplerMode(SamplerMode::POINT_CLAMP);
	g_theRenderer->BindTexture(nullptr);
	g_theRenderer->DrawVertexBuffer(m_gpuMesh3, (int)m_sightCPUMesh.size(), sizeof(Vertex_PCU));

	//g_theRenderer->DrawVertexArray(6, vertices);
}

void Scorpio::Update(float deltaseconds)
{
	m_isShooting = false;

	if (m_health == 0)
	{
		m_owner->SpawnExplosion(m_position, 0.75f, 1.5f, ENTITY_TYPE_EXPLOSION);
		Die();
	}

	if (g_theInputSystem->WasKeyJustPressed(KEYCODE_F1))
	{
		m_isDebugMode = !m_isDebugMode;
	}
	
	m_orientationDegrees += 15.0f * deltaseconds;
	
	Vec2 forwardNormal = GetForwardNormal();

	if (m_owner->GetPlayerTank()->IsAlive())
	{
		if (IsPointInsideDirectedSector2D(m_owner->GetPlayerTank()->GetPosition(), m_position, forwardNormal, 10.0f, 10.0f))
		{
			if (HasPlayerLineOfSight())
			{
				m_isShooting = true;
			}
		}
	}

	if (m_isShooting)
	{
		Vec2 turretForwardNormal = Vec2::MakeFromPolarDegrees(m_orientationDegrees, m_physicsRadius - 0.1f);

		m_nextBulletTimer += deltaseconds;

		if (m_nextBulletTimer >= 0.3f)
		{
			g_theAudio->StartSound(g_theGame->m_enemyShoot);
			m_owner->SpawnBullet(m_position + turretForwardNormal, m_orientationDegrees, ENTITY_TYPE_EVIL_BULLET);
			m_nextBulletTimer = 0.0f;
		}
	}

	m_bodyCPUMesh.clear();
	m_turretCPUMesh.clear();

	AABB2 tankBody = AABB2(-0.5f, -0.5f, 0.5f, 0.5f);
	AABB2 tankTurret = AABB2(-0.5f, -0.5f, 0.5f, 0.5f);

	AddVertsForAABB2D(m_bodyCPUMesh, tankBody, Rgba8(255, 255, 255, 255));
	AddVertsForAABB2D(m_turretCPUMesh, tankTurret, Rgba8(255, 255, 255, 255));

	TransformVertexArrayXY3D(static_cast<int>(m_bodyCPUMesh.size()), m_bodyCPUMesh.data(), 1.0f, 0.0f, m_position);
	TransformVertexArrayXY3D(static_cast<int>(m_turretCPUMesh.size()), m_turretCPUMesh.data(), 1.0f, m_orientationDegrees, m_position);

	g_theRenderer->CopyCPUToGPU(m_bodyCPUMesh.data(), (int)m_bodyCPUMesh.size() * sizeof(Vertex_PCU), m_gpuMesh);
	g_theRenderer->CopyCPUToGPU(m_turretCPUMesh.data(), (int)m_turretCPUMesh.size() * sizeof(Vertex_PCU), m_gpuMesh2);

	m_sightCPUMesh.clear();

	Vec2 turretForward = Vec2::MakeFromPolarDegrees(m_orientationDegrees, m_cosmeticRadius);

	RaycastResult2D raycast =m_owner->RaycastVsTiles(m_position + turretForward, turretForward.GetNormalized(), 10.0f);

	Vec2 lineOfSightEnd = Vec2::MakeFromPolarDegrees(m_orientationDegrees, raycast.m_impactDist);

	Vec2 direction = m_position + turretForward + lineOfSightEnd - m_position - turretForward;
	Vec2 forward = direction.GetNormalized();

	forward.SetLength(0.05f / 2.0f);

	Vec2 left = forward.GetRotated90Degrees();

	Vec2 endLeft = m_position + turretForward + lineOfSightEnd + forward + left;
	Vec2 endRight = m_position + turretForward + lineOfSightEnd + forward - left;
	Vec2 startLeft = m_position + turretForward - forward + left;
	Vec2 startRight = m_position + turretForward - forward - left;

	//Vertex_PCU vertices[6];

	m_sightCPUMesh.push_back(Vertex_PCU(startLeft.x, startLeft.y, 255, 0, 0, 255));
	m_sightCPUMesh.push_back(Vertex_PCU(startRight.x, startRight.y, 255, 0, 0, 255));
	m_sightCPUMesh.push_back(Vertex_PCU(endRight.x, endRight.y, 255, 0, 0, 0));
	m_sightCPUMesh.push_back(Vertex_PCU(endRight.x, endRight.y, 255, 0, 0, 0));
	m_sightCPUMesh.push_back(Vertex_PCU(endLeft.x, endLeft.y, 255, 0, 0, 0));
	m_sightCPUMesh.push_back(Vertex_PCU(startLeft.x, startLeft.y, 255, 0, 0, 255));

	TransformVertexArrayXY3D(6, m_sightCPUMesh.data(), 1.0f, 0.0f, Vec2(0.0f, 0.0f));

	g_theRenderer->CopyCPUToGPU(m_sightCPUMesh.data(), (int)m_sightCPUMesh.size() * sizeof(Vertex_PCU), m_gpuMesh3);
}

void Scorpio::Render()
{/*
	std::vector<Vertex_PCU> bodyVertices;
	std::vector<Vertex_PCU> turretVertices;

	AABB2 tankBody = AABB2(-0.5f, -0.5f, 0.5f, 0.5f);
	AABB2 tankTurret = AABB2(-0.5f, -0.5f, 0.5f, 0.5f);

	AddVertsForAABB2D(bodyVertices, tankBody, Rgba8(255, 255, 255, 255));
	AddVertsForAABB2D(turretVertices, tankTurret, Rgba8(255, 255, 255, 255));

	TransformVertexArrayXY3D(static_cast<int>(bodyVertices.size()), bodyVertices.data(), 1.0f, 0.0f, m_position);*/
	g_theRenderer->SetModelConstants();
	g_theRenderer->SetBlendMode(BlendMode::ALPHA);
	g_theRenderer->SetDepthMode(DepthMode::ENABLED);
	g_theRenderer->SetRasterizerMode(RasterizerMode::SOLID_CULL_BACK);
	g_theRenderer->SetSamplerMode(SamplerMode::POINT_CLAMP);
	g_theRenderer->BindTexture(g_theGame->m_allTextures[GAME_SCORPIO_BASE_TEXTURE]);
	g_theRenderer->DrawVertexBuffer(m_gpuMesh, (int)m_bodyCPUMesh.size(), sizeof(Vertex_PCU));

	//g_theRenderer->DrawVertexArray(static_cast<int>(bodyVertices.size()), bodyVertices.data());

	//TransformVertexArrayXY3D(static_cast<int>(turretVertices.size()), turretVertices.data(), 1.0f, m_orientationDegrees, m_position);
	g_theRenderer->SetModelConstants();
	g_theRenderer->SetBlendMode(BlendMode::ALPHA);
	g_theRenderer->SetDepthMode(DepthMode::ENABLED);
	g_theRenderer->SetRasterizerMode(RasterizerMode::SOLID_CULL_BACK);
	g_theRenderer->SetSamplerMode(SamplerMode::POINT_CLAMP);
	g_theRenderer->BindTexture(g_theGame->m_allTextures[GAME_SCORPIO_TURRET_TEXTURE]);
	g_theRenderer->DrawVertexBuffer(m_gpuMesh2, (int)m_turretCPUMesh.size(), sizeof(Vertex_PCU));

	//g_theRenderer->DrawVertexArray(static_cast<int>(turretVertices.size()), turretVertices.data());

	RenderLineOfSight();

	if (m_isDebugMode)
	{
		DebugMode();
	}
}

void Scorpio::Die()
{
	m_isDead = true;
}
