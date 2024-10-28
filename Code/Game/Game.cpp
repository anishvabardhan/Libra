#include "Game/Game.hpp"
#include "Game/GameCommon.hpp"

#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Core/Image.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Core/SimpleTriangleFont.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/Clock.hpp"
#include "Engine/Renderer/Texture.hpp"
#include "Engine/Renderer/BitmapFont.hpp"
#include "Engine/Renderer/VertexBuffer.hpp"
#include "Engine/Renderer/SpriteAnimDefinition.hpp"

#include "Game/Map.hpp"
#include "Game/Entity.hpp"

#include <math.h>

Game::Game()
{
}

Game::~Game()
{
}

void Game::StartUp()
{
	m_gameClock = new Clock(Clock::GetSystemClock());

	m_bitMapFont = g_theRenderer->CreateOrGetBitmapFont("Data/Textures/SquirrelFixedFont.png");

	LoadAssets();

	g_theAudio->StartSound(m_welcomeMusic);

	m_attractPlaybackMusic = g_theAudio->StartSound(m_attractMusic, true);

	m_screenCamera = new Camera();
	m_worldCamera = new Camera();

	m_screenCamera->m_normalizedViewport = AABB2::ZERO_TO_ONE;
	m_worldCamera->m_normalizedViewport = AABB2::ZERO_TO_ONE;

	m_attractGPUMesh = g_theRenderer->CreateVertexBuffer(sizeof(Vertex_PCU), std::wstring(L"Game"));
	m_attractGPUMesh2 = g_theRenderer->CreateVertexBuffer(sizeof(Vertex_PCU), std::wstring(L"Game"));
	m_attractGPUMesh3 = g_theRenderer->CreateVertexBuffer(sizeof(Vertex_PCU), std::wstring(L"Game"));
	m_attractGPUMesh4 = g_theRenderer->CreateVertexBuffer(sizeof(Vertex_PCU), std::wstring(L"Game"));

	AddVertsForAABB2D(m_attractCPUMesh, AABB2(Vec2(0.0f, 0.0f), Vec2(1600.0f, 800.0f)), Rgba8(255, 255, 255, 255));
	g_theRenderer->CopyCPUToGPU(m_attractCPUMesh.data(), m_attractCPUMesh.size() * sizeof(Vertex_PCU), m_attractGPUMesh);

	AddVertsForAABB2D(m_attractCPUMesh2, AABB2(100.0f, 300.0f, 500.0, 500.0f), Rgba8(0, 0, 0, 255));
	g_theRenderer->CopyCPUToGPU(m_attractCPUMesh2.data(), m_attractCPUMesh2.size() * sizeof(Vertex_PCU), m_attractGPUMesh2);

	Vec2 loc = Vec2(100.0f, 200.0f);
	m_bitMapFont->AddVertsForText2D(m_attractCPUMesh3, loc, 50.0f, "HELLO WORLD", Rgba8(255, 0, 0, 255));
	m_bitMapFont->AddVertsForTextInBox2D(m_attractCPUMesh3, AABB2(100.0f, 300.0f, 500.0, 500.0f), 25.0f, "HELLO WORLD ME YOU", Rgba8::WHITE, 1.0f, Vec2(0.5f, 0.5f));
	g_theRenderer->CopyCPUToGPU(m_attractCPUMesh3.data(), m_attractCPUMesh3.size() * sizeof(Vertex_PCU), m_attractGPUMesh3);

	MapDefinition::InitializeDef();

	CreateMaps();
}

void Game::Shutdown()
{
	g_theAudio->StopSound(m_attractPlaybackMusic);
	g_theAudio->StopSound(m_gameplayPlaybackMusic);

	DELETE_PTR(m_attractGPUMesh);
	DELETE_PTR(m_attractGPUMesh2);
	DELETE_PTR(m_attractGPUMesh3);
	DELETE_PTR(m_attractGPUMesh4);

	DELETE_PTR(m_worldCamera);
	DELETE_PTR(m_screenCamera);
	DELETE_PTR(m_gameClock);

	for (int i = 0; i < TOTAL_NUM_OF_TEXTURES; i++)
	{
		DELETE_PTR(m_allTextures[i]);
	}
	
	for (size_t i = 0; i < m_maps.size(); i++)
	{
		if (m_maps[i])
		{
			DELETE_PTR(m_maps[i]);
		}
	}

	m_currentMap = nullptr;

	m_maps.clear();
}

void Game::Update(float deltaseconds)
{
	if (m_isAttractMode)
	{
		UpdateAttractMode(deltaseconds);

 		m_screenCamera->SetOrthoView(Vec2(0.0f, 0.0f), Vec2(g_gameConfigBlackboard.GetValue("screenSizeX", 20.0f), g_gameConfigBlackboard.GetValue("screenSizeY", 20.0f)));
	}
	else if(m_isGameplayMode)
	{
		if (IsPointInsideDisc2D(m_currentMap->GetExitPosition(), m_currentMap->GetPlayerTank()->GetPosition(), m_currentMap->GetPlayerTank()->m_physicsRadius))
		{
			GoToNextLevel();
		}

		m_currentMap->Update(deltaseconds);

		if (!m_isDebugCamera)
		{
			Vec2 camCenter = m_currentMap->GetPlayerTank()->GetPosition();

			camCenter.x = GetClamped(camCenter.x, g_gameConfigBlackboard.GetValue("worldSizeX", 20.0f) * 0.5f, m_currentMap->GetDefinition().m_dimensions.x - (g_gameConfigBlackboard.GetValue("worldSizeX", 20.0f) * 0.5f));
			camCenter.y = GetClamped(camCenter.y, g_gameConfigBlackboard.GetValue("worldSizeY", 20.0f) * 0.5f, m_currentMap->GetDefinition().m_dimensions.y - (g_gameConfigBlackboard.GetValue("worldSizeY", 20.0f) * 0.5f));

			m_worldCamera->SetOrthoView(Vec2(0.0f, 0.0f), Vec2(g_gameConfigBlackboard.GetValue("worldSizeX", 20.0f), g_gameConfigBlackboard.GetValue("worldSizeY", 20.0f)));
			m_worldCamera->SetCenter(camCenter);
		}
		else
		{
			m_numTilesInViewVertically = static_cast<int>((m_currentMap->GetDimensions().y > m_currentMap->GetDimensions().x * 0.5f) ? m_currentMap->GetDimensions().y : m_currentMap->GetDimensions().x * 0.5f);

			m_worldCamera->SetOrthoView(Vec2(0.0f, 0.0f), Vec2(static_cast<float>(m_numTilesInViewVertically * 2.0f), static_cast<float>(m_numTilesInViewVertically)));
		}
	}
	
	HandleInput();
	UpdateFromController(deltaseconds);
}

void Game::Render() const
{
	if (m_isAttractMode)
	{
		g_theRenderer->BeginCamera(*m_screenCamera);

		RenderAttractMode();

		g_theRenderer->EndCamera(*m_screenCamera);
	}
	else if(m_isGameplayMode)
	{
		g_theRenderer->BeginCamera(*m_worldCamera);

		m_currentMap->RenderMap();

		g_theRenderer->EndCamera(*m_worldCamera);
	}
	else if (m_isVictoryMode)
	{
		g_theRenderer->BeginCamera(*m_screenCamera);

		RenderVictoryScreen();

		g_theRenderer->EndCamera(*m_screenCamera);
	}
	else if (m_isLoseMode)
	{
		g_theRenderer->BeginCamera(*m_screenCamera);

		RenderLoseScreen();

		g_theRenderer->EndCamera(*m_screenCamera);
	}
}

void Game::RenderPauseMenu()
{/*
	std::vector<Vertex_PCU> verts;

	if (m_isDebugCamera)
	{
		AABB2 pauseMenuBounds = AABB2(m_currentMap->GetPlayerTank()->GetPosition().x - 200.0f * 0.5f, m_currentMap->GetPlayerTank()->GetPosition().y - 100.0f * 0.5f, m_currentMap->GetPlayerTank()->GetPosition().x + 200.0f * 0.5f, m_currentMap->GetPlayerTank()->GetPosition().y + 100.0f * 0.5f);
		pauseMenuBounds.SetCenter(Vec2(GetClamped(m_currentMap->GetPlayerTank()->GetPosition().x, 200.0f * 0.5f, m_currentMap->GetDefinition().m_dimensions.x - (200.0f * 0.5f)), GetClamped(m_currentMap->GetPlayerTank()->GetPosition().y, 100.0f * 0.5f, m_currentMap->GetDefinition().m_dimensions.y - (100.0f * 0.5f))));
		
		AddVertsForAABB2D(verts, pauseMenuBounds, Rgba8(0, 0, 0, 127), Vec2::ZERO, Vec2::ONE);
	}
	else
	{
		AABB2 pauseMenuBounds = AABB2(m_currentMap->GetPlayerTank()->GetPosition().x - g_gameConfigBlackboard.GetValue("worldSizeX", 20.0f) * 0.5f, m_currentMap->GetPlayerTank()->GetPosition().y - g_gameConfigBlackboard.GetValue("worldSizeY", 20.0f) * 0.5f, m_currentMap->GetPlayerTank()->GetPosition().x + g_gameConfigBlackboard.GetValue("worldSizeX", 20.0f) * 0.5f, m_currentMap->GetPlayerTank()->GetPosition().y + g_gameConfigBlackboard.GetValue("worldSizeY", 20.0f) * 0.5f);
		pauseMenuBounds.SetCenter(Vec2(GetClamped(m_currentMap->GetPlayerTank()->GetPosition().x, g_gameConfigBlackboard.GetValue("worldSizeX", 20.0f) * 0.5f, m_currentMap->GetDefinition().m_dimensions.x - (g_gameConfigBlackboard.GetValue("worldSizeX", 20.0f) * 0.5f)), GetClamped(m_currentMap->GetPlayerTank()->GetPosition().y, g_gameConfigBlackboard.GetValue("worldSizeY", 20.0f) * 0.5f, m_currentMap->GetDefinition().m_dimensions.y - (g_gameConfigBlackboard.GetValue("worldSizeY", 20.0f) * 0.5f))));

		AddVertsForAABB2D(verts, pauseMenuBounds, Rgba8(0, 0, 0, 127), Vec2::ZERO, Vec2::ONE);
	}*/
	
	g_theRenderer->SetModelConstants();
	g_theRenderer->BindTexture(nullptr);
	g_theRenderer->BindShader();
	g_theRenderer->SetBlendMode(BlendMode::ALPHA);
	g_theRenderer->SetDepthMode(DepthMode::ENABLED);
	g_theRenderer->SetRasterizerMode(RasterizerMode::SOLID_CULL_BACK);
	g_theRenderer->SetSamplerMode(SamplerMode::POINT_CLAMP);
	g_theRenderer->DrawVertexBuffer(m_attractGPUMesh4, (int)m_attractCPUMesh4.size(), sizeof(Vertex_PCU));
	//g_theRenderer->DrawVertexArray(static_cast<int>(verts.size()), verts.data());
}

void Game::RenderVictoryScreen() const
{
	std::vector<Vertex_PCU> verts1;

	AddVertsForAABB2D(verts1, AABB2(Vec2(0.0f, 0.0f), Vec2(1600.0f, 800.0f)), Rgba8(255, 255, 255, 255));

	g_theRenderer->SetModelConstants();
	g_theRenderer->BindTexture(m_allTextures[VICTORY_TEXTURE]);
	g_theRenderer->DrawVertexArray(static_cast<int>(verts1.size()), verts1.data());
}

void Game::RenderLoseScreen() const
{
	std::vector<Vertex_PCU> verts1;

	AddVertsForAABB2D(verts1, AABB2(Vec2(0.0f, 0.0f), Vec2(1600.0f, 800.0f)), Rgba8(255, 255, 255, 255));

	g_theRenderer->SetModelConstants();
	g_theRenderer->BindTexture(m_allTextures[GAME_OVER_TEXTURE]);
	g_theRenderer->DrawVertexArray(static_cast<int>(verts1.size()), verts1.data());
}

void Game::CreateMap(MapDefinition definition)
{
	Map* map = new Map(definition);

	m_maps.push_back(map);
}

void Game::GoToNextLevel()
{
	m_currentMapIndex++;
	if (m_currentMapIndex < 3)
	{
		delete m_currentMap;
		m_currentMap = m_maps[m_currentMapIndex];
	}
	else
	{
		m_victoryPlaybackMusic = g_theAudio->StartSound(m_victoryMusic, true);
		g_theAudio->StopSound(m_gameplayPlaybackMusic);
		m_isVictoryMode = true;
		m_isGameplayMode = false;
	}
}

void Game::RenderAttractMode() const
{
	g_theRenderer->SetModelConstants();
	g_theRenderer->BindTexture(m_allTextures[ATTRACT_TEXTURE]);
	g_theRenderer->BindShader();
	g_theRenderer->SetBlendMode(BlendMode::ALPHA);
	g_theRenderer->SetDepthMode(DepthMode::ENABLED);
	g_theRenderer->SetRasterizerMode(RasterizerMode::SOLID_CULL_BACK);
	g_theRenderer->SetSamplerMode(SamplerMode::POINT_CLAMP);
	g_theRenderer->DrawVertexBuffer(m_attractGPUMesh, (int)m_attractCPUMesh.size(), sizeof(Vertex_PCU));

	g_theRenderer->SetModelConstants();
	g_theRenderer->BindTexture(nullptr);
	g_theRenderer->BindShader();
	g_theRenderer->SetBlendMode(BlendMode::ALPHA);
	g_theRenderer->SetDepthMode(DepthMode::ENABLED);
	g_theRenderer->SetRasterizerMode(RasterizerMode::SOLID_CULL_BACK);
	g_theRenderer->SetSamplerMode(SamplerMode::POINT_CLAMP);
	g_theRenderer->DrawVertexBuffer(m_attractGPUMesh2, (int)m_attractCPUMesh2.size(), sizeof(Vertex_PCU));

	g_theRenderer->SetModelConstants();
	g_theRenderer->BindTexture(&m_bitMapFont->GetTexture());
	g_theRenderer->BindShader();
	g_theRenderer->SetBlendMode(BlendMode::ALPHA);
	g_theRenderer->SetDepthMode(DepthMode::ENABLED);
	g_theRenderer->SetRasterizerMode(RasterizerMode::SOLID_CULL_BACK);
	g_theRenderer->SetSamplerMode(SamplerMode::POINT_CLAMP);
	g_theRenderer->DrawVertexBuffer(m_attractGPUMesh3, (int)m_attractCPUMesh3.size(), sizeof(Vertex_PCU));
}

void Game::LoadAssets()
{
	m_allTextures[GAME_TERRAIN_TEXTURE]			= g_theRenderer->CreateOrGetTextureFromFile("Data/Textures/Terrain_8x8.png");
	m_allTextures[ATTRACT_TEXTURE]				= g_theRenderer->CreateOrGetTextureFromFile("Data/Textures/AttractScreen.png");
	m_allTextures[GAME_SCORPIO_BASE_TEXTURE]	= g_theRenderer->CreateOrGetTextureFromFile("Data/Textures/EnemyTurretBase.png");
	m_allTextures[GAME_SCORPIO_TURRET_TEXTURE]	= g_theRenderer->CreateOrGetTextureFromFile("Data/Textures/EnemyCannon.png");
	m_allTextures[GAME_OVER_TEXTURE]			= g_theRenderer->CreateOrGetTextureFromFile("Data/Textures/YouDiedScreen.png");
	m_allTextures[VICTORY_TEXTURE]				= g_theRenderer->CreateOrGetTextureFromFile("Data/Textures/VictoryScreen.jpg");
	m_allTextures[GAME_PLAYER_BASE_TEXTURE]		= g_theRenderer->CreateOrGetTextureFromFile("Data/Textures/PlayerTankBase.png");
	m_allTextures[GAME_PLAYER_TURRET_TEXTURE]	= g_theRenderer->CreateOrGetTextureFromFile("Data/Textures/PlayerTankTop.png");
	m_allTextures[GAME_LEO_TEXTURE]				= g_theRenderer->CreateOrGetTextureFromFile("Data/Textures/EnemyTank3.png");
	m_allTextures[GAME_ARIES_TEXTURE]			= g_theRenderer->CreateOrGetTextureFromFile("Data/Textures/EnemyTank2.png");
	m_allTextures[GAME_CAPRICORN_TEXTURE]		= g_theRenderer->CreateOrGetTextureFromFile("Data/Textures/EnemyTank1.png");
	m_allTextures[GAME_ENEMY_BULLET_TEXTURE]	= g_theRenderer->CreateOrGetTextureFromFile("Data/Textures/EnemyBolt.png");
	m_allTextures[GAME_PLAYER_BULLET_TEXTURE]	= g_theRenderer->CreateOrGetTextureFromFile("Data/Textures/FriendlyBolt.png");
	m_allTextures[GAME_EXPLOSION_TEXTURE]		= g_theRenderer->CreateOrGetTextureFromFile("Data/Textures/Explosion_5x5.png");

 	m_welcomeMusic								= g_theAudio->CreateOrGetSound("Data/Audio/Welcome.mp3");
	m_attractMusic								= g_theAudio->CreateOrGetSound("Data/Audio/AttractMusic.mp3");
	m_gameplayMusic								= g_theAudio->CreateOrGetSound("Data/Audio/GameplayMusic.mp3");
	m_gameOverMusic								= g_theAudio->CreateOrGetSound("Data/Audio/GameOver.mp3");
	m_victoryMusic								= g_theAudio->CreateOrGetSound("Data/Audio/Victory.mp3");
	m_playerShoot								= g_theAudio->CreateOrGetSound("Data/Audio/PlayerShootNormal.ogg");
	m_enemyShoot								= g_theAudio->CreateOrGetSound("Data/Audio/EnemyShoot.wav");
	m_buttonClickSoundID						= g_theAudio->CreateOrGetSound("Data/Audio/Click.mp3");
	m_pausedMusic								= g_theAudio->CreateOrGetSound("Data/Audio/Pause.mp3");
	m_unPausedMusic								= g_theAudio->CreateOrGetSound("Data/Audio/Unpause.mp3");
	m_playerHit									= g_theAudio->CreateOrGetSound("Data/Audio/PlayerHit.wav");
	m_enemyHit									= g_theAudio->CreateOrGetSound("Data/Audio/EnemyHit.wav");
	m_bulletRicochet							= g_theAudio->CreateOrGetSound("Data/Audio/BulletRicochet.wav");
	m_bulletBounce								= g_theAudio->CreateOrGetSound("Data/Audio/BulletRicochet2.wav");
}

void Game::CreateMaps()
{
	for (int index = 0; index < g_gameConfigBlackboard.GetValue("totalMaps", 0.0f); index++)
	{
		CreateMap(MapDefinition::s_definitions[index]);
	}

	m_currentMap = m_maps[0];
}

void Game::HandleInput()
{
	if (!g_theConsole->IsOpen())
	{
		if (m_isAttractMode)
		{
			if (g_theInputSystem->WasKeyJustPressed('P'))
			{
				g_theAudio->StartSound(m_buttonClickSoundID);

				if (!m_currentMap)
				{
					m_currentMap = m_maps[0];
				}

				m_isAttractMode = false;
				m_isGameplayMode = true;
				g_theAudio->StopSound(m_attractPlaybackMusic);
				m_gameplayPlaybackMusic = g_theAudio->StartSound(m_gameplayMusic, true);
			}
		}
		else if (m_isGameplayMode)
		{
			if (g_theInputSystem->WasKeyJustPressed(KEYCODE_F4))
			{
				g_theAudio->StartSound(m_buttonClickSoundID);

				m_isDebugCamera = !m_isDebugCamera;

				if (m_isDebugCamera)
				{
					AABB2 pauseMenuBounds = AABB2(m_currentMap->GetPlayerTank()->GetPosition().x - g_gameConfigBlackboard.GetValue("worldSizeX", 20.0f) * 0.5f, m_currentMap->GetPlayerTank()->GetPosition().y - g_gameConfigBlackboard.GetValue("worldSizeY", 20.0f) * 0.5f, m_currentMap->GetPlayerTank()->GetPosition().x + g_gameConfigBlackboard.GetValue("worldSizeX", 20.0f) * 0.5f, m_currentMap->GetPlayerTank()->GetPosition().y + g_gameConfigBlackboard.GetValue("worldSizeY", 20.0f) * 0.5f);
					pauseMenuBounds.SetCenter(Vec2(GetClamped(m_currentMap->GetPlayerTank()->GetPosition().x, g_gameConfigBlackboard.GetValue("worldSizeX", 20.0f) * 0.5f, m_currentMap->GetDefinition().m_dimensions.x - (g_gameConfigBlackboard.GetValue("worldSizeX", 20.0f) * 0.5f)), GetClamped(m_currentMap->GetPlayerTank()->GetPosition().y, g_gameConfigBlackboard.GetValue("worldSizeY", 20.0f) * 0.5f, m_currentMap->GetDefinition().m_dimensions.y - (g_gameConfigBlackboard.GetValue("worldSizeY", 20.0f) * 0.5f))));

					AddVertsForAABB2D(m_attractCPUMesh4, pauseMenuBounds, Rgba8(0, 0, 0, 127), Vec2::ZERO, Vec2::ONE);
					g_theRenderer->CopyCPUToGPU(m_attractCPUMesh4.data(), m_attractCPUMesh4.size() * sizeof(Vertex_PCU), m_attractGPUMesh4);
				}
				else
				{
					AABB2 pauseMenuBounds = AABB2(m_currentMap->GetPlayerTank()->GetPosition().x - g_gameConfigBlackboard.GetValue("worldSizeX", 20.0f) * 0.5f, m_currentMap->GetPlayerTank()->GetPosition().y - g_gameConfigBlackboard.GetValue("worldSizeY", 20.0f) * 0.5f, m_currentMap->GetPlayerTank()->GetPosition().x + g_gameConfigBlackboard.GetValue("worldSizeX", 20.0f) * 0.5f, m_currentMap->GetPlayerTank()->GetPosition().y + g_gameConfigBlackboard.GetValue("worldSizeY", 20.0f) * 0.5f);
					pauseMenuBounds.SetCenter(Vec2(GetClamped(m_currentMap->GetPlayerTank()->GetPosition().x, g_gameConfigBlackboard.GetValue("worldSizeX", 20.0f) * 0.5f, m_currentMap->GetDefinition().m_dimensions.x - (g_gameConfigBlackboard.GetValue("worldSizeX", 20.0f) * 0.5f)), GetClamped(m_currentMap->GetPlayerTank()->GetPosition().y, g_gameConfigBlackboard.GetValue("worldSizeY", 20.0f) * 0.5f, m_currentMap->GetDefinition().m_dimensions.y - (g_gameConfigBlackboard.GetValue("worldSizeY", 20.0f) * 0.5f))));

					AddVertsForAABB2D(m_attractCPUMesh4, pauseMenuBounds, Rgba8(0, 0, 0, 127), Vec2::ZERO, Vec2::ONE);
					g_theRenderer->CopyCPUToGPU(m_attractCPUMesh4.data(), m_attractCPUMesh4.size() * sizeof(Vertex_PCU), m_attractGPUMesh4);
				}

				TransformVertexArrayXY3D(static_cast<int>(m_attractCPUMesh4.size()), m_attractCPUMesh4.data(), 1.0f, 0.0f, Vec2(0.0f, 0.0f));
			}

			if (g_theApp->m_isPaused && !g_theApp->m_isSlowMo && !g_theApp->m_isFastMo)
			{
				if (g_theInputSystem->WasKeyJustPressed(KEYCODE_ESC))
				{
					g_theAudio->StartSound(m_buttonClickSoundID);

					//if (m_currentMap)
					//{
					//	delete m_currentMap;
					//	m_currentMap = nullptr;
					//}

					for (size_t i = 0; i < m_maps.size(); i++)
					{
						if (m_maps[i])
						{
							DELETE_PTR(m_maps[i]);
						}
					}

					m_maps.clear();

					m_currentMap = nullptr;

					m_isAttractMode = true;
					m_isGameplayMode = false;
					g_theApp->m_isPaused = false;
					m_attractPlaybackMusic = g_theAudio->StartSound(m_attractMusic, true);
					g_theAudio->StopSound(m_gameplayPlaybackMusic);

					AddVertsForAABB2D(m_attractCPUMesh, AABB2(Vec2(0.0f, 0.0f), Vec2(1600.0f, 800.0f)), Rgba8(255, 255, 255, 255));
					g_theRenderer->CopyCPUToGPU(m_attractCPUMesh.data(), m_attractCPUMesh.size() * sizeof(Vertex_PCU), m_attractGPUMesh);

					AddVertsForAABB2D(m_attractCPUMesh2, AABB2(100.0f, 300.0f, 500.0, 500.0f), Rgba8(0, 0, 0, 255));
					g_theRenderer->CopyCPUToGPU(m_attractCPUMesh2.data(), m_attractCPUMesh2.size() * sizeof(Vertex_PCU), m_attractGPUMesh2);

					Vec2 loc = Vec2(100.0f, 200.0f);
					m_bitMapFont->AddVertsForText2D(m_attractCPUMesh3, loc, 50.0f, "HELLO WORLD", Rgba8(255, 0, 0, 255));
					m_bitMapFont->AddVertsForTextInBox2D(m_attractCPUMesh3, AABB2(100.0f, 300.0f, 500.0, 500.0f), 25.0f, "HELLO WORLD ME YOU", Rgba8::WHITE, 1.0f, Vec2(0.5f, 0.5f));
					g_theRenderer->CopyCPUToGPU(m_attractCPUMesh3.data(), m_attractCPUMesh3.size() * sizeof(Vertex_PCU), m_attractGPUMesh3);

				}

				if (g_theInputSystem->WasKeyJustPressed('P'))
				{
					g_theAudio->StartSound(m_buttonClickSoundID);
					g_theAudio->StartSound(g_theGame->m_pausedMusic);

					g_theAudio->SetSoundPlaybackSpeed(m_gameplayPlaybackMusic, 0.0f);
				}
			}

			if (!g_theApp->m_isPaused && g_theApp->m_isSlowMo && !g_theApp->m_isFastMo)
			{
				if (g_theInputSystem->WasKeyJustPressed('T'))
				{
					g_theAudio->SetSoundPlaybackSpeed(m_gameplayPlaybackMusic, 0.5f);
				}
			}

			if (!g_theApp->m_isPaused && !g_theApp->m_isSlowMo && g_theApp->m_isFastMo)
			{
				if (g_theInputSystem->WasKeyJustPressed('Y'))
				{
					g_theAudio->SetSoundPlaybackSpeed(m_gameplayPlaybackMusic, 4.0f);
				}
			}

			if (!g_theApp->m_isPaused && !g_theApp->m_isSlowMo && !g_theApp->m_isFastMo)
			{
				if (g_theInputSystem->WasKeyJustPressed('P'))
				{
					g_theAudio->StartSound(m_buttonClickSoundID);
					g_theAudio->StartSound(g_theGame->m_unPausedMusic);

					g_theAudio->SetSoundPlaybackSpeed(m_gameplayPlaybackMusic, 1.0f);
				}

				if (g_theInputSystem->WasKeyJustReleased('T'))
				{
					g_theAudio->SetSoundPlaybackSpeed(m_gameplayPlaybackMusic, 1.0f);
				}

				if (g_theInputSystem->WasKeyJustReleased('Y'))
				{
					g_theAudio->SetSoundPlaybackSpeed(m_gameplayPlaybackMusic, 1.0f);
				}
			}
		}
		else if (m_isLoseMode)
		{
			if (g_theInputSystem->WasKeyJustPressed('N'))
			{
				g_theAudio->StartSound(m_buttonClickSoundID);

				m_isGameplayMode = true;
				m_isLoseMode = false;
				g_theAudio->StopSound(m_gameOverPlaybackMusic);
				m_currentMap->ResetDeathTimer();
				m_currentMap->RespawnPlayer();
			}

			if (g_theInputSystem->WasKeyJustPressed(KEYCODE_ESC))
			{
				g_theAudio->StartSound(m_buttonClickSoundID);

				m_isGameplayMode = false;
				m_isLoseMode = false;
				m_isAttractMode = true;
			}
		}
		else if (m_isVictoryMode)
		{
			if (g_theInputSystem->WasKeyJustPressed('N') || g_theInputSystem->WasKeyJustPressed('P') || g_theInputSystem->WasKeyJustPressed(KEYCODE_ESC) || g_theInputSystem->WasKeyJustPressed(KEYCODE_SPACE))
			{
				g_theAudio->StartSound(m_buttonClickSoundID);

				m_isGameplayMode = false;
				m_isVictoryMode = false;
				m_isAttractMode = true;
				g_theAudio->StopSound(m_victoryPlaybackMusic);
			}
		}
	}
}

void Game::UpdateAttractMode(float deltaseconds)
{
	UNUSED(deltaseconds);

}

void Game::UpdateFromController(float deltaseconds)
{
	UNUSED(deltaseconds);

	XboxController const& controller = g_theInputSystem->GetController(0);

	if (m_isAttractMode)
	{
		if (controller.WasButtonJustPressed(XboxButtonID::BUTTON_B))
		{
			g_theApp->HandleQuitRequested();
		}

		if (controller.WasButtonJustPressed(XboxButtonID::BUTTON_START))
		{
			m_isAttractMode = false;
			m_isGameplayMode = true;
			g_theAudio->StopSound(m_attractPlaybackMusic);
			m_gameplayPlaybackMusic = g_theAudio->StartSound(m_gameplayMusic, true);
		}
	}
	else
	{
		if (g_theApp->m_isPaused && !g_theApp->m_isSlowMo && !g_theApp->m_isFastMo)
		{
			if (controller.WasButtonJustPressed(XboxButtonID::BUTTON_B))
			{
				m_isAttractMode = true;
				m_isGameplayMode = false;
				m_attractPlaybackMusic = g_theAudio->StartSound(m_attractMusic, true);
				g_theAudio->StopSound(m_gameplayPlaybackMusic);
			}
		}
	}
}