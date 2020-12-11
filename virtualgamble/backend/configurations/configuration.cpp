#include "configuration.h"

#define _CRT_SECURE_NO_WARNINGS
#include "../../globaloperands.h"

QuarcSaver::QuarcSaver()
{

}

std::string GetTempDataPath()
{
	return getenv("APPDATA");
}

void QuarcSaver::SaveSettings()
{
	CreateDirectoryA(std::string(GetTempDataPath() + "//QuarcCode").c_str(), NULL);

	Saver::InitPath(std::string(GetTempDataPath() + "//QuarcCode//data.qcd").c_str());
}

void QuarcSaver::LoadSettings()
{
	CreateDirectoryA(std::string(GetTempDataPath() + "//QuarcCode").c_str(), NULL);

	Saver::InitPath(std::string(GetTempDataPath() + "//QuarcCode//data.qcd").c_str());
}

QuarcSaver qSaver;