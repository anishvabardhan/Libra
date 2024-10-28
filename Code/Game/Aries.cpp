#include "Game/Aries.hpp"

#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Renderer/VertexBuffer.hpp"

#include "Game/GameCommon.hpp"

Aries::Aries()
{
}

Aries::Aries(Map* owner, Vec2 position, float orientation, EntityFaction faction, EntityType type)
	: Entity(owner, position, orientation, faction, type)
{
	m_velocity = Vec2(g_gameConfigBlackboard.GetValue("ariesSpeed", 1.0f), g_gameConfigBlackboard.GetValue("ariesSpeed", 1.0f));
	m_isPushedByEntities = true;
	m_doesPushEntities = true;
	m_isPushedByWalls = true;
	m_isHitByBullets = true;
	m_canSwim = false;
	m_physicsRadius = g_gameConfigBlackboard.GetValue("ariesPhysicsRadius", 1.0f);
	m_cosmeticRadius = g_gameConfigBlackboard.GetValue("ariesCosmeticRadius", 1.0f);
	m_health = 5;

	m_rand = RandomNumberGenerator();

	m_gpuMesh = g_theRenderer->CreateVertexBuffer(sizeof(Vertex_PCU), std::wstring(L"Aries"));
}

Aries::~Aries()
{
	DELETE_PTR(m_gpuMesh);
}

void Aries::DebugMode() const
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

void Aries::RenderShield() const
{
	float thetaDegrees = 360.0f / 50.0f;

	Vertex_PCU vertices[300];

	for (int index = 0; index < 50; index++)
	{
		float theta1 = index * thetaDegrees;
		float theta2 = (index + 1) * thetaDegrees;

		if (theta2 <= 45.0f)
		{
			Vec2 vert1 = Vec2(m_physicsRadius * CosDegrees(theta1), m_physicsRadius * SinDegrees(theta1));
			Vec2 vert2 = Vec2(m_physicsRadius * CosDegrees(theta2), m_physicsRadius * SinDegrees(theta2));
			Vec2 vert3 = Vec2((m_physicsRadius + 0.05f) * CosDegrees(theta1), (m_physicsRadius + 0.05f) * SinDegrees(theta1));
			Vec2 vert4 = Vec2((m_physicsRadius + 0.05f) * CosDegrees(theta2), (m_physicsRadius + 0.05f) * SinDegrees(theta2));

			vertices[6 * index] = Vertex_PCU(vert1.x, vert1.y, 255, 165, 0, 200);
			vertices[6 * index + 1] = Vertex_PCU(vert3.x, vert3.y, 255, 165, 0, 200);
			vertices[6 * index + 2] = Vertex_PCU(vert4.x, vert4.y, 255, 165, 0, 200);
			vertices[6 * index + 3] = Vertex_PCU(vert4.x, vert4.y, 255, 165, 0, 200);
			vertices[6 * index + 4] = Vertex_PCU(vert2.x, vert2.y, 255, 165, 0, 200);
			vertices[6 * index + 5] = Vertex_PCU(vert1.x, vert1.y, 255, 165, 0, 200);
		}

		if (theta2 >= 322.2f && theta2 <= 360.0f)
		{
			Vec2 vert1 = Vec2(m_physicsRadius * CosDegrees(theta1), m_physicsRadius * SinDegrees(theta1));
			Vec2 vert2 = Vec2(m_physicsRadius * CosDegrees(theta2), m_physicsRadius * SinDegrees(theta2));
			Vec2 vert3 = Vec2((m_physicsRadius + 0.05f) * CosDegrees(theta1), (m_physicsRadius + 0.05f) * SinDegrees(theta1));
			Vec2 vert4 = Vec2((m_physicsRadius + 0.05f) * CosDegrees(theta2), (m_physicsRadius + 0.05f) * SinDegrees(theta2));

			vertices[6 * index] = Vertex_PCU(vert1.x, vert1.y, 255, 165, 0, 200);
			vertices[6 * index + 1] = Vertex_PCU(vert3.x, vert3.y, 255, 165, 0, 200);
			vertices[6 * index + 2] = Vertex_PCU(vert4.x, vert4.y, 255, 165, 0, 200);
			vertices[6 * index + 3] = Vertex_PCU(vert4.x, vert4.y, 255, 165, 0, 200);
			vertices[6 * index + 4] = Vertex_PCU(vert2.x, vert2.y, 255, 165, 0, 200);
			vertices[6 * index + 5] = Vertex_PCU(vert1.x, vert1.y, 255, 165, 0, 200);
		}
	}

	TransformVertexArrayXY3D(300, vertices, 1.0f, m_orientationDegrees, m_position);
	g_theRenderer->SetModelConstants();
	g_theRenderer->BindTexture(nullptr);
	g_theRenderer->DrawVertexArray(300, vertices);
}

bool Aries::HasPlayerLineOfSight() const
{
	Vec2 turretForward = Vec2::MakeFromPolarDegrees(m_orientationDegrees, m_cosmeticRadius);

	Vec2 forward = m_owner->GetPlayerTank()->GetPosition() - m_position - turretForward;
	RaycastResult2D raycastResult;
	raycastResult = m_owner->RaycastVsTiles(m_position + turretForward, forward.GetNormalized(), forward.GetLength());

	return !raycastResult.m_didImpact;
}

void Aries::Update(float deltaseconds)
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
	
	m_position += m_velocity * GetForwardNormal() * deltaseconds;

	m_cpuMesh.clear();

	AABB2 tankBody = AABB2(-0.5f, -0.5f, 0.5f, 0.5f);

	AddVertsForAABB2D(m_cpuMesh, tankBody, Rgba8(255, 255, 255, 255), Vec2::ZERO, Vec2::ONE);

	TransformVertexArrayXY3D(static_cast<int>(m_cpuMesh.size()), m_cpuMesh.data(), 1.0f, m_orientationDegrees, m_position);

	g_theRenderer->CopyCPUToGPU(m_cpuMesh.data(), (int)m_cpuMesh.size() * sizeof(Vertex_PCU), m_gpuMesh);
}

void Aries::Render()
{
	//std::vector<Vertex_PCU> bodyVertices;

	//AABB2 tankBody = AABB2(-0.5f, -0.5f, 0.5f, 0.5f);

	//AddVertsForAABB2D(bodyVertices, tankBody, Rgba8(255, 255, 255, 255), Vec2::ZERO, Vec2::ONE);

	//TransformVertexArrayXY3D(static_cast<int>(bodyVertices.size()), bodyVertices.data(), 1.0f, m_orientationDegrees, m_position);
	g_theRenderer->SetModelConstants();
	g_theRenderer->SetBlendMode(BlendMode::ALPHA);
	g_theRenderer->SetDepthMode(DepthMode::ENABLED);
	g_theRenderer->SetRasterizerMode(RasterizerMode::SOLID_CULL_BACK);
	g_theRenderer->SetSamplerMode(SamplerMode::POINT_CLAMP);
	g_theRenderer->BindShader();
	g_theRenderer->BindTexture(g_theGame->m_allTextures[GAME_ARIES_TEXTURE]);
	g_theRenderer->DrawVertexBuffer(m_gpuMesh, (int)m_cpuMesh.size(), sizeof(Vertex_PCU));
	//g_theRenderer->DrawVertexArray(static_cast<int>(m_cpuMesh.size()), m_cpuMesh.data());

	//RenderShield();

	if (m_isDebugMode)
	{
		DebugMode();
	}
}

void Aries::Die()
{
	m_isDead = true;
}

void Aries::ReactToBullet(Entity& bullet)
{
	Vec2 forwardNormal = GetForwardNormal();
	Vec2 bulletVector = bullet.m_position - m_position;
	Vec2 bulletForwardNormal = bullet.GetForwardNormal();

	float angleBetweenVectors = GetAngleDegreesBetweenVectors2D(forwardNormal, bulletVector);

	if (angleBetweenVectors <= 45.0f)
	{
		g_theAudio->StartSound(g_theGame->m_bulletBounce);
		Vec2 reflected = bulletForwardNormal.GetReflected(bulletVector.GetNormalized());
  		bullet.m_orientationDegrees = reflected.GetOrientationDegrees();
	}
	else
	{
		g_theAudio->StartSound(g_theGame->m_enemyHit);
		TakeDamage(1);
		bullet.Die();
	}
}
