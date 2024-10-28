#pragma once

#include "Engine/Core/EventSystem.hpp"

class Clock;

class App
{
public:
	double				m_previousFrameTime		= 0.0f;
	bool				m_isQuitting			= false;
	bool				m_isPaused				= false;
	bool				m_isSlowMo				= false;
	bool				m_isFastMo				= false;
	float				m_deltaTime				= 0.0f;
public:
						App();
						~App();

	void				StartUp();
	void				Run();
	void				ShutDown();

	void				InitializeGameConfigurations(char const* dataFilePath);
	bool				IsQuitting() const { return m_isQuitting; }
	void				HandleKeyPressed(unsigned char keyCode);
	void				HandleKeyReleased(unsigned char keyCode);
	void				HandleQuitRequested();
	void				RestartGame();

	float				GetDeltaTime() const;
	void				SetDeltaTime(float newDeltaTime);

	static bool			QuitApp(EventArgs& args);
private:
	void				RunFrame();
	void				BeginFrame();
	void				Update();
	void				Render() const;
	void				EndFrame();
};