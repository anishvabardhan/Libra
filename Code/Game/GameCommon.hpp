#pragma once

#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Audio/AudioSystem.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/EventSystem.hpp"

#include "Game/App.hpp"
#include "Game/Game.hpp"

#define UNUSED(x) (void)x
#define DELETE_PTR(x) if(x) { delete x; x = nullptr; }

extern Renderer*		g_theRenderer;
extern InputSystem*		g_theInputSystem;
extern AudioSystem*		g_theAudio;
extern App*				g_theApp;
extern Game*			g_theGame;
extern DevConsole*		g_theConsole;
extern EventSystem*		g_theEventSystem;

struct Vec2;
struct Rgba8;

void					DrawDebugRing(Vec2 const& center, float const& radius, float const& scale, float const& orientation, float const& thickness, Rgba8 const& color);
void					DrawDebugLine(Vec2 const& startPos, Vec2 const& endPos, float const& thickness, Rgba8 const& color);
void					DrawDebugDisc(Vec2 const& center, float radius, Rgba8 const& color);