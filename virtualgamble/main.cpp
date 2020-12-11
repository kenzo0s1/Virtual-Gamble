#include <stdexcept>

#include <thread>
#include <chrono>

#include "globaloperands.h"
#include "frontend/libraries/borderless/borderless.hpp"
#include "backend/configurations/configuration.h"

#include <oleidl.h>


float GenerateCrashSize(int min, int max)
{
	float modifier;

	switch(rand()%10)
	{
	case 0:modifier = min; break;
	case 1:modifier = max; break;
	case 3:modifier = min; break;
	case 4:modifier = max; break;
	case 5:modifier = min; break;
	case 6:modifier = max; break;
	case 7:modifier = min; break;
	case 8: modifier = max; break;
	case 9: modifier = min; break;
	case 10: modifier = max; break;
	case 2: modifier = 0; break;
	}
}

HANDLE WINAPI SecondThread()
{
	while (initialized_interface == false)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}

	for (;;)
	{

	}
}

int CALLBACK wWinMain(HINSTANCE /*inst*/, HINSTANCE /*prev*/, LPWSTR /*cmd*/, int /*show*/)
{
	CreateThread(0, 0, LPTHREAD_START_ROUTINE(SecondThread), 0, 0, 0);

	try
	{
		BorderlessWindow window;
	}
	catch (const std::exception& e)
	{
		::MessageBoxA(nullptr, e.what(), "QuarcCode Exception Handler", MB_OK | MB_ICONERROR);
	}
}
