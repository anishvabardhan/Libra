#define WIN32_LEAN_AND_MEAN		
#include <windows.h>
#include "Game/App.hpp"

#define UNUSED(x) (void)(x);

extern App* g_theApp;

//-----------------------------------------------------------------------------------------------
int WINAPI WinMain( _In_ HINSTANCE applicationInstanceHandle, _In_opt_ HINSTANCE previousInstance, _In_ LPSTR commandLineString, _In_ int nShowCmd)
{
	UNUSED(applicationInstanceHandle);
	UNUSED(previousInstance);
	UNUSED(commandLineString);
	UNUSED(nShowCmd);

	g_theApp = new App();
	g_theApp->StartUp();
	g_theApp->Run();
	g_theApp->ShutDown();
	delete g_theApp;
	g_theApp = nullptr;
	
	return 0;
}


