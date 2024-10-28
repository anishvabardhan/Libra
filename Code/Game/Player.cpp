#include "Game/Player.hpp"

#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Renderer/VertexBuffer.hpp"

#include "Game/GameCommon.hpp"

Player::Player()
	: Entity()
{
}

Player::Player(Map* owner, Vec2 position, float orientation, EntityFaction faction, EntityType type)
	: Entity(owner, position, orientation, faction, type)
{
	float playerSpeed = g_gameConfigBlackboard.GetValue("tankSpeed", 1.0f);

	m_weaponType = PlayerWeapon::BULLET;
	m_isPushedByEntities = true;
	m_doesPushEntities = true;
	m_isPushedByWalls = true;
	m_isHitByBullets = true;
	m_canSwim = false;
	m_velocity = Vec2(playerSpeed, playerSpeed);
	m_physicsRadius = g_gameConfigBlackboard.GetValue("tankPhysicsRadius", 1.0f);
	m_cosmeticRadius = g_gameConfigBlackboard.GetValue("tankCosmeticRadius", 1.0f);
	m_health = 5;

	m_gpuMesh = g_theRenderer->CreateVertexBuffer(sizeof(Vertex_PCU), std::wstring(L"Player"));
	m_gpuMesh2 = g_theRenderer->CreateVertexBuffer(sizeof(Vertex_PCU), std::wstring(L"Player"));
}

Player::~Player()
{
	DELETE_PTR(m_gpuMesh);
	DELETE_PTR(m_gpuMesh2);
}

void Player::HandleInput()
{
	m_targetDirection = Vec2(0.0f, 0.0f);
	m_turretAbsoluteDirection = Vec2(0.0f, 0.0f);
	m_isMoving = false;
	m_isTurretMoving = false;
	m_isShooting = false;

	if (g_theInputSystem->WasKeyJustPressed(KEYCODE_F2))
	{
		m_isInvincible = !m_isInvincible;
	}

	if (g_theInputSystem->IsKeyDown('W'))
	{
		m_targetDirection += Vec2(0.0f, 1.0f);
		m_isMoving = true;
	}

	if (g_theInputSystem->IsKeyDown('S'))
	{
		m_targetDirection += Vec2(0.0f, -1.0f);
		m_isMoving = true;
	}

	if (g_theInputSystem->IsKeyDown('A'))
	{
		m_targetDirection += Vec2(-1.0f, 0.0f);
		m_isMoving = true;
	}

	if (g_theInputSystem->IsKeyDown('D'))
	{
		m_targetDirection += Vec2(1.0f, 0.0f);
		m_isMoving = true;
	}

	if (g_theInputSystem->IsKeyDown('I'))
	{
		m_turretAbsoluteDirection += Vec2(0.0f, 1.0f);
		m_isTurretMoving = true;
	}

	if (g_theInputSystem->IsKeyDown('K'))
	{
		m_turretAbsoluteDirection += Vec2(0.0f, -1.0f);
		m_isTurretMoving = true;
	}

	if (g_theInputSystem->IsKeyDown('J'))
	{
		m_turretAbsoluteDirection += Vec2(-1.0f, 0.0f);
		m_isTurretMoving = true;
	}

	if (g_theInputSystem->IsKeyDown('L'))
	{
		m_turretAbsoluteDirection += Vec2(1.0f, 0.0f);
		m_isTurretMoving = true;
	}

	if (g_theInputSystem->WasKeyJustPressed(KEYCODE_F1))
	{
		m_isDebugMode = !m_isDebugMode;
	}

	if (g_theInputSystem->IsKeyDown(KEYCODE_SPACE))
	{
		m_isShooting = true;
	}

	if (g_theInputSystem->WasKeyJustPressed('1'))
	{
		m_weaponType = BULLET;
	}

	if (g_theInputSystem->WasKeyJustPressed('2'))
	{
		m_weaponType = FLAMETHROWER;
	}

	XboxController const& controller = g_theInputSystem->GetController(0);

	if (controller.GetLeftStick().GetMagnitude() > 0)
	{
		m_velocity = Vec2(g_gameConfigBlackboard.GetValue("tankSpeed", 1.0f) * controller.GetLeftStick().GetMagnitude(), g_gameConfigBlackboard.GetValue("tankSpeed", 1.0f) * controller.GetLeftStick().GetMagnitude());
		m_targetDirection = Vec2::MakeFromPolarDegrees(controller.GetLeftStick().GetOrientationDegrees());
		m_isMoving = true;
	}

	if (controller.GetRightStick().GetMagnitude() > 0)
	{
		m_turretOrientation = controller.GetRightStick().GetOrientationDegrees() - m_orientationDegrees;
		m_isTurretMoving = true;
	}
}

void Player::DebugMode() const
{
	Vec2 normal = GetForwardNormal();
	normal.SetLength(m_cosmeticRadius);

	Vec2 turretForwardNormal = Vec2::MakeFromPolarDegrees(m_orientationDegrees + m_turretOrientation, m_cosmeticRadius);
	Vec2 targetForwardNormal = Vec2::MakeFromPolarDegrees(m_targetDirection.GetOrientationDegrees(), m_cosmeticRadius);
	
	DrawDebugRing(m_position, m_physicsRadius, 1.0f, m_orientationDegrees, 0.03f, Rgba8(0, 255, 255, 255));
	DrawDebugRing(m_position, m_cosmeticRadius, 1.0f, m_orientationDegrees, 0.03f, Rgba8(255, 0, 255, 255));
	DrawDebugLine(m_position, m_position + turretForwardNormal, 0.15f, Rgba8(0, 0, 255, 255));
	DrawDebugLine(m_position, m_position + (1.5f * normal).GetRotated90Degrees(), 0.03f, Rgba8(0, 255, 0, 255));
	DrawDebugLine(m_position, m_position + (1.5f * normal), 0.03f, Rgba8(255, 0, 0, 255));

    if (g_theInputSystem->IsKeyDown('W') || g_theInputSystem->IsKeyDown('D') || g_theInputSystem->IsKeyDown('A') || g_theInputSystem->IsKeyDown('S'))
	{
		DrawDebugLine(m_position, m_position + (1.75f * normal), 0.025f, Rgba8(255, 255, 0, 255));
		DrawDebugLine(m_position + (1.85f * targetForwardNormal), m_position + (2.0f * targetForwardNormal), 0.05f, Rgba8(255, 0, 0, 255));
	}
}

void Player::Update(float deltaseconds)
{
	if (m_health == 0)
	{
		g_theGame->m_gameOverPlaybackMusic = g_theAudio->StartSound(g_theGame->m_gameOverMusic);
		m_owner->SpawnExplosion(m_position, 0.75f, 1.5f, ENTITY_TYPE_EXPLOSION);
		m_owner->PlayerDead();
		Die();
	}

	HandleInput();

	if (m_isMoving)
	{
		m_orientationDegrees = GetTurnedTowardDegrees(m_orientationDegrees, m_targetDirection.GetOrientationDegrees(), 360.0f * deltaseconds);
		m_position += m_velocity * GetForwardNormal() * deltaseconds;
	}

	if (m_isTurretMoving)
	{
		m_turretOrientation = GetTurnedTowardDegrees(m_turretOrientation + m_orientationDegrees, m_turretAbsoluteDirection.GetOrientationDegrees(), 360.0f * deltaseconds) - m_orientationDegrees;
	}

	if (m_isShooting)
	{
		Vec2 turretForwardNormal = Vec2::MakeFromPolarDegrees(m_orientationDegrees + m_turretOrientation, m_physicsRadius + 0.1f);

		m_nextProjectileTimer += deltaseconds;

		if (m_nextProjectileTimer >= m_projectileShootTimer[m_weaponType])
		{
			switch (m_weaponType)
			{
			case BULLET:
			{
				g_theAudio->StartSound(g_theGame->m_playerShoot);

				m_owner->SpawnBullet(m_position + turretForwardNormal, m_turretOrientation + m_orientationDegrees, ENTITY_TYPE_GOOD_BULLET);
				m_owner->SpawnExplosion(m_position + turretForwardNormal, 0.5f, 0.25f, ENTITY_TYPE_EXPLOSION);
				break;
			}
			case FLAMETHROWER:
			{
				RandomNumberGenerator random = RandomNumberGenerator();

				m_owner->SpawnFlameThrower(m_position + turretForwardNormal, m_turretOrientation + m_orientationDegrees + random.RollRandomFloatInRange(-15.0f, 15.0f), ENTITY_TYPE_EXPLOSION);
				break;
			}
			}
			m_nextProjectileTimer = 0.0f;
		}
	}

	m_bodyCPUMesh.clear();
	m_turretCPUMesh.clear();

	AABB2 tankBody = AABB2(-0.5f, -0.5f, 0.5f, 0.5f);
	AABB2 tankTurret = AABB2(-0.5f, -0.5f, 0.5f, 0.5f);

	AddVertsForAABB2D(m_bodyCPUMesh, tankBody, Rgba8(255, 255, 255, 255), Vec2::ZERO, Vec2::ONE);
	AddVertsForAABB2D(m_turretCPUMesh, tankTurret, Rgba8(255, 255, 255, 255), Vec2::ZERO, Vec2::ONE);
	TransformVertexArrayXY3D(static_cast<int>(m_bodyCPUMesh.size()), m_bodyCPUMesh.data(), 1.0f, m_orientationDegrees, m_position);
	TransformVertexArrayXY3D(static_cast<int>(m_turretCPUMesh.size()), m_turretCPUMesh.data(), 1.0f, m_turretOrientation + m_orientationDegrees, m_position);

	g_theRenderer->CopyCPUToGPU(m_bodyCPUMesh.data(), (int)m_bodyCPUMesh.size() * sizeof(Vertex_PCU), m_gpuMesh);
	g_theRenderer->CopyCPUToGPU(m_turretCPUMesh.data(), (int)m_turretCPUMesh.size() * sizeof(Vertex_PCU), m_gpuMesh2);
}

void Player::Render()
{
	/*AABB2 tankBody = AABB2(-0.5f, -0.5f, 0.5f, 0.5f);
	AABB2 tankTurret = AABB2(-0.5f, -0.5f, 0.5f, 0.5f);

	AddVertsForAABB2D(bodyVertices, tankBody, Rgba8(255, 255, 255, 255), Vec2::ZERO, Vec2::ONE);
	AddVertsForAABB2D(turretVertices, tankTurret, Rgba8(255, 255, 255, 255), Vec2::ZERO, Vec2::ONE);*/

	g_theRenderer->SetModelConstants();
	g_theRenderer->SetBlendMode(BlendMode::ALPHA);
	g_theRenderer->SetDepthMode(DepthMode::ENABLED);
	g_theRenderer->SetRasterizerMode(RasterizerMode::SOLID_CULL_BACK);
	g_theRenderer->SetSamplerMode(SamplerMode::POINT_CLAMP);
	g_theRenderer->BindTexture(g_theGame->m_allTextures[GAME_PLAYER_BASE_TEXTURE]);
	g_theRenderer->BindShader();
	g_theRenderer->DrawVertexBuffer(m_gpuMesh, (int)m_bodyCPUMesh.size(), sizeof(Vertex_PCU));
	//g_theRenderer->DrawVertexArray(static_cast<int>(bodyVertices.size()), bodyVertices.data());


	g_theRenderer->BindTexture(g_theGame->m_allTextures[GAME_PLAYER_TURRET_TEXTURE]);
	g_theRenderer->SetBlendMode(BlendMode::ALPHA);
	g_theRenderer->SetDepthMode(DepthMode::ENABLED);
	g_theRenderer->SetRasterizerMode(RasterizerMode::SOLID_CULL_BACK);
	g_theRenderer->SetSamplerMode(SamplerMode::POINT_CLAMP);
	g_theRenderer->SetModelConstants();
	g_theRenderer->BindShader();
	g_theRenderer->DrawVertexBuffer(m_gpuMesh2, (int)m_turretCPUMesh.size(), sizeof(Vertex_PCU));
	//g_theRenderer->DrawVertexArray(static_cast<int>(turretVertices.size()), turretVertices.data());

	if (m_isInvincible)
	{
		DrawDebugRing(m_position, m_cosmeticRadius, 1.0f, m_orientationDegrees, 0.03f, Rgba8(255, 255, 255, 255));
	}

	if (m_isDebugMode)
	{
		DebugMode();
	}
}

void Player::Die()
{
	m_isDead = true;
}

void Player::ReactToBullet(Entity& bullet)
{
	if (!m_isInvincible)
	{
		TakeDamage(1);
	}

	bullet.Die();
}
