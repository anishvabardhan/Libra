#include "Game/Leo.hpp"

#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Renderer/VertexBuffer.hpp"

#include "Game/GameCommon.hpp"

Leo::Leo()
{
}

Leo::Leo(Map* owner, Vec2 position, float orientation, EntityFaction faction, EntityType type)
	: Entity(owner, position, orientation, faction, type)
{
	m_velocity = Vec2(g_gameConfigBlackboard.GetValue("leoSpeed", 1.0f), g_gameConfigBlackboard.GetValue("leoSpeed", 1.0f));
	m_isPushedByEntities = true;
	m_doesPushEntities = true;
	m_isPushedByWalls = true;
	m_isHitByBullets = true;
	m_canSwim = false;
	m_physicsRadius = g_gameConfigBlackboard.GetValue("leoPhysicsRadius", 1.0f);
	m_cosmeticRadius = g_gameConfigBlackboard.GetValue("leoCosmeticRadius", 1.0f);
	m_health = 10;

	m_rand = RandomNumberGenerator();

	m_distanceToTargetMap = new TileHeatMap(m_owner->GetDimensions());

	m_gpuMesh = g_theRenderer->CreateVertexBuffer(sizeof(Vertex_PCU), std::wstring(L"Leo"));
}

Leo::~Leo()
{
	DELETE_PTR(m_gpuMesh);
}

bool Leo::HasPlayerLineOfSight() const
{
	Vec2 turretForward = Vec2::MakeFromPolarDegrees(m_orientationDegrees, m_cosmeticRadius);

	Vec2 forward = m_owner->GetPlayerTank()->GetPosition() - m_position - turretForward;
	RaycastResult2D raycastResult;
	raycastResult = m_owner->RaycastVsTiles(m_position + turretForward, forward.GetNormalized(), forward.GetLength());

	return !raycastResult.m_didImpact;
}

void Leo::DebugMode() const
{
	Vec2 normal = GetForwardNormal();
	normal.SetLength(m_cosmeticRadius);

	DrawDebugRing(m_position, m_physicsRadius, 1.0f, m_orientationDegrees, 0.03f, Rgba8(0, 255, 255, 255));
	DrawDebugRing(m_position, m_cosmeticRadius, 1.0f, m_orientationDegrees, 0.03f, Rgba8(255, 0, 255, 255));
	DrawDebugLine(m_position, m_position + (1.5f * normal).GetRotated90Degrees(), 0.03f, Rgba8(0, 255, 0, 255));
	DrawDebugLine(m_position, m_position + (1.5f * normal), 0.03f, Rgba8(255, 0, 0, 255));

	if (m_isDebugLine)
	{
		DrawDebugLine(m_position, m_lastPlayerPosition, 0.02f, Rgba8(0, 0, 0, 255));
		DrawDebugDisc(m_lastPlayerPosition, 0.1f, Rgba8(0, 0, 0, 255));
	}
}

void Leo::Update(float deltaseconds)
{
	m_owner->PopulateDistanceField(*m_distanceToTargetMap, IntVec2(static_cast<int>(m_owner->GetPlayerTank()->GetPosition().x), static_cast<int>(m_owner->GetPlayerTank()->GetPosition().y)), 999999.0f);

	if (m_health == 0)
	{
		m_owner->SpawnExplosion(m_position, 0.75f, 1.5f, ENTITY_TYPE_EXPLOSION);
		Die();
	}

	if (HasPlayerLineOfSight())
	{
		m_isDebugLine = true;
	}

	if (g_theInputSystem->WasKeyJustPressed(KEYCODE_F1))
	{
		m_isDebugMode = !m_isDebugMode;
	}

	if (!m_hasLineOfSight)
	{
		if (m_distanceToPlayer <= 0.5f)
		{
			m_isDebugLine = false;

			m_turnTimer += deltaseconds;

			if (m_turnTimer > 1.0f)
			{
				m_isTurning = true;

				m_goalDegrees = m_rand.RollRandomFloatInRange(0.0f, 360.0f);
				m_turnTimer = 0.0f;
			}

			if (m_isTurning)
			{
				m_orientationDegrees = GetTurnedTowardDegrees(m_orientationDegrees, m_goalDegrees, 360.0f * deltaseconds);

				if (m_orientationDegrees == m_goalDegrees)
				{
					m_isTurning = false;
				}
			}
		}
	}
	else
	{
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
			Vec2 turretForwardNormal = Vec2::MakeFromPolarDegrees(m_orientationDegrees, m_physicsRadius);

			m_nextBulletTimer += deltaseconds;

			if (m_nextBulletTimer >= m_rand.RollRandomFloatInRange(1.0f, 1.3f))
			{
				g_theAudio->StartSound(g_theGame->m_enemyShoot);
				m_owner->SpawnBullet(m_position + turretForwardNormal, m_orientationDegrees, ENTITY_TYPE_EVIL_BULLET);
				m_nextBulletTimer = 0.0f;
			}
		}
	}

	m_position += m_velocity * GetForwardNormal() * deltaseconds;

	m_cpuMesh.clear();

	AABB2 tankBody = AABB2(-0.5f, -0.5f, 0.5f, 0.5f);

	AddVertsForAABB2D(m_cpuMesh, tankBody, Rgba8(255, 255, 255, 255));

	TransformVertexArrayXY3D(static_cast<int>(m_cpuMesh.size()), m_cpuMesh.data(), 1.0f, m_orientationDegrees, m_position);

	g_theRenderer->CopyCPUToGPU(m_cpuMesh.data(), (int)m_cpuMesh.size() * sizeof(Vertex_PCU), m_gpuMesh);
}

void Leo::Render()
{
	if (m_owner->m_isHeatMapOn)
	{
		float maxHeatValue = 0.0f;

		for (size_t index = 0; index < m_distanceToTargetMap->m_values.size(); index++)
		{
			if (m_distanceToTargetMap->m_values[index] != 999999.f && maxHeatValue < m_distanceToTargetMap->m_values[index])
				maxHeatValue = m_distanceToTargetMap->m_values[index];
		}

		std::vector<Vertex_PCU> heatVertices;

		m_distanceToTargetMap->AddVertsForDebugDraw(heatVertices, AABB2(Vec2::ZERO, Vec2(static_cast<float>(m_owner->GetDimensions().x), static_cast<float>(m_owner->GetDimensions().y))), FloatRange(0.0f, maxHeatValue), Rgba8(0, 0, 0, 255), Rgba8::WHITE);

		g_theRenderer->BindTexture(nullptr);
		TransformVertexArrayXY3D(static_cast<int>(heatVertices.size()), heatVertices.data(), 1.0f, 0.0f, Vec2(0.0f, 0.0f));
		g_theRenderer->SetModelConstants();
		g_theRenderer->DrawVertexArray(static_cast<int>(heatVertices.size()), heatVertices.data());
	}

	//std::vector<Vertex_PCU> bodyVertices;

	//AABB2 tankBody = AABB2(-0.5f, -0.5f, 0.5f, 0.5f);

	//AddVertsForAABB2D(bodyVertices, tankBody, Rgba8(255, 255, 255, 255));

	//TransformVertexArrayXY3D(static_cast<int>(bodyVertices.size()), bodyVertices.data(), 1.0f, m_orientationDegrees, m_position);
	g_theRenderer->SetModelConstants();
	g_theRenderer->SetBlendMode(BlendMode::ALPHA);
	g_theRenderer->SetDepthMode(DepthMode::ENABLED);
	g_theRenderer->SetRasterizerMode(RasterizerMode::SOLID_CULL_BACK);
	g_theRenderer->SetSamplerMode(SamplerMode::POINT_CLAMP);
	g_theRenderer->BindTexture(g_theGame->m_allTextures[GAME_LEO_TEXTURE]);
	g_theRenderer->BindShader();
	g_theRenderer->DrawVertexBuffer(m_gpuMesh, (int)m_cpuMesh.size(), sizeof(Vertex_PCU));
	//g_theRenderer->DrawVertexArray(static_cast<int>(bodyVertices.size()), bodyVertices.data());

	if (m_isDebugMode)
	{
		DebugMode();
	}
}

void Leo::Die()
{
	m_isDead = true;
}
