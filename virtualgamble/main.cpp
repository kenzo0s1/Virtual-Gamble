#include <stdexcept>

#include "frontend/libraries/borderless/borderless.hpp"
#include "backend/configurations/configuration.h"

#include <oleidl.h>

int CALLBACK wWinMain(HINSTANCE /*inst*/, HINSTANCE /*prev*/, LPWSTR /*cmd*/, int /*show*/) {
	try
	{
		BorderlessWindow window;
	}
	catch (const std::exception& e)
	{
		::MessageBoxA(nullptr, e.what(), "QuarcCode Exception Handler", MB_OK | MB_ICONERROR);
	}
}
