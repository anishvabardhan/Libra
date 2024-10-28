#pragma once

#include "Engine/Math/Vec2.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Audio/AudioSystem.hpp"

#include "Game/Map.hpp"

typedef std::vector<Map*> MapList;

class Entity;
class BitmapFont;
class Clock;
class SpriteAnimDefinition;
class VertexBuffer;

enum GameTextureID
{
	DEFAULT = -1,
	ATTRACT_TEXTURE,
	GAME_OVER_TEXTURE,
	VICTORY_TEXTURE,
	GAME_TERRAIN_TEXTURE,
	GAME_EXPLOSION_TEXTURE,
	GAME_SCORPIO_BASE_TEXTURE,
	GAME_SCORPIO_TURRET_TEXTURE,
	GAME_PLAYER_BASE_TEXTURE,
	GAME_PLAYER_TURRET_TEXTURE,
	GAME_LEO_TEXTURE,
	GAME_ARIES_TEXTURE,
	GAME_CAPRICORN_TEXTURE,
	GAME_PLAYER_BULLET_TEXTURE,
	GAME_ENEMY_BULLET_TEXTURE,

	TOTAL_NUM_OF_TEXTURES
};

//enum GameSoundID
//{
//	DEFAULT = -1,
//	ATTRACT_SOUND,
//	WELCOME_SOUND,
//	VICTORY_SOUND,
//	GAME_TERRAIN_SOUND,
//	GAME_SCORPIO_BASE_SOUND,
//	GAME_SCORPIO_TURRET_SOUND,
//	GAME_PLAYER_BASE_SOUND,
//	GAME_PLAYER_TURRET_SOUND,
//	GAME_LEO_SOUND,
//	GAME_ARIES_SOUND,
//	GAME_PLAYER_BULLET_TEXTURE,
//	GAME_ENEMY_BULLET_TEXTURE,
//
//	TOTAL_NUM_OF_SOUNDS
//};

class Game
{
public:
	Camera*						m_worldCamera					= nullptr;
	Camera*						m_screenCamera					= nullptr;
	Map*						m_currentMap					= nullptr;
	int							m_currentMapIndex				= 0;
	MapList						m_maps;
	BitmapFont*					m_bitMapFont					= nullptr;
	Texture*					m_allTextures[TOTAL_NUM_OF_TEXTURES];
	std::vector<Vertex_PCU>		m_attractCPUMesh;
	std::vector<Vertex_PCU>		m_attractCPUMesh2;
	std::vector<Vertex_PCU>		m_attractCPUMesh3;
	std::vector<Vertex_PCU>		m_attractCPUMesh4;
	VertexBuffer*				m_attractGPUMesh				= nullptr;
	VertexBuffer*				m_attractGPUMesh2				= nullptr;
	VertexBuffer*				m_attractGPUMesh3				= nullptr;
	VertexBuffer*				m_attractGPUMesh4				= nullptr;
	Clock*						m_gameClock						= nullptr;
	//SoundID						m_allSoundIDs[TOTAL_NUM_OF_SOUNDS];
	SoundID						m_attractMusic;
	SoundID						m_welcomeMusic;
	SoundID						m_gameplayMusic;
	SoundID						m_pausedMusic;
	SoundID						m_unPausedMusic;
	SoundID						m_buttonClickSoundID;
	SoundID						m_gameOverMusic;
	SoundID						m_victoryMusic;
	SoundID						m_playerShoot;
	SoundID						m_enemyShoot;
	SoundID						m_enemyHit;
	SoundID						m_playerHit;
	SoundID						m_bulletRicochet;
	SoundID						m_bulletBounce;
	SoundPlaybackID				m_gameOverPlaybackMusic;
	SoundPlaybackID				m_victoryPlaybackMusic;
	SoundPlaybackID				m_attractPlaybackMusic;
	SoundPlaybackID				m_gameplayPlaybackMusic;
	Vec2						m_attractModePosition			= Vec2(500.0f, 400.0f);
	bool						m_isDebugCamera					= false;
	bool						m_isAttractMode					= true;
	bool						m_isGameplayMode				= false;
	bool						m_isVictoryMode					= false;
	bool						m_isLoseMode					= false;
	int							m_numTilesInViewVertically		= 0;
public:
								Game();
								~Game();

	void						StartUp();
	void						Shutdown();
	void						Update(float deltaseconds);
	void						Render() const ;
	void						RenderPauseMenu() ;
	void						RenderVictoryScreen() const;
	void						RenderLoseScreen() const;
	void						RenderAttractMode() const;
	void						LoadAssets();
	void						CreateMaps();
	void						CreateMap(MapDefinition definition);
	void						GoToNextLevel();
	void						HandleInput();
	void						UpdateFromController(float deltaseconds);
	void						UpdateAttractMode(float deltaseconds);
};