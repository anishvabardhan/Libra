#include "Game/Capricorn.hpp"

#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/MathUtils.hpp"

#include "Game/GameCommon.hpp"

Capricorn::Capricorn()
{
}

Capricorn::Capricorn(Map* owner, Vec2 position, float orientation, EntityFaction faction, EntityType type)
	: Entity(owner, position, orientation, faction, type)
{
	m_velocity = Vec2(g_gameConfigBlackboard.GetValue("leoSpeed", 1.0f), g_gameConfigBlackboard.GetValue("leoSpeed", 1.0f));
	m_isPushedByEntities = true;
	m_doesPushEntities = true;
	m_isPushedByWalls = true;
	m_isHitByBullets = true;
	m_canSwim = true;
	m_physicsRadius = g_gameConfigBlackboard.GetValue("leoPhysicsRadius", 1.0f);
	m_cosmeticRadius = g_gameConfigBlackboard.GetValue("leoCosmeticRadius", 1.0f);
	m_health = 10;

	m_rand = RandomNumberGenerator();
}

Capricorn::~Capricorn()
{
}

bool Capricorn::HasPlayerLineOfSight() const
{
	Vec2 turretForward = Vec2::MakeFromPolarDegrees(m_orientationDegrees, m_cosmeticRadius);

	Vec2 forward = m_owner->GetPlayerTank()->GetPosition() - m_position - turretForward;
	RaycastResult2D raycastResult;
	raycastResult = m_owner->RaycastVsTiles(m_position + turretForward, forward.GetNormalized(), forward.GetLength());

	return !raycastResult.m_didImpact;
}

void Capricorn::DebugMode() const
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

void Capricorn::Update(float deltaseconds)
{
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
}

void Capricorn::Render()
{
	std::vector<Vertex_PCU> bodyVertices;

	AABB2 tankBody = AABB2(-0.5f, -0.5f, 0.5f, 0.5f);

	AddVertsForAABB2D(bodyVertices, tankBody, Rgba8(255, 255, 255, 255));


	TransformVertexArrayXY3D(static_cast<int>(bodyVertices.size()), bodyVertices.data(), 1.0f, m_orientationDegrees, m_position);
	g_theRenderer->SetModelConstants();
	g_theRenderer->BindTexture(g_theGame->m_allTextures[GAME_CAPRICORN_TEXTURE]);
	g_theRenderer->DrawVertexArray(static_cast<int>(bodyVertices.size()), bodyVertices.data());

	if (m_isDebugMode)
	{
		DebugMode();
	}
}

void Capricorn::Die()
{
	m_isDead = true;
}
