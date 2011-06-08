#include <windows.h>
#include <shlwapi.h>
#include <stdio.h>
#include "GameModule.hpp"
#include "RedVex.hpp"

PluginInfo Info;
RedVexInfo *Funcs;

#define _MAX_FILE (_MAX_PATH + _MAX_FNAME)

char path[_MAX_PATH];
char db[_MAX_FILE];

int __stdcall DllMain(HINSTANCE instance, int reason, void* reserved)
{
	switch(reason)
	{
		case DLL_PROCESS_ATTACH:
			GetModuleFileName(instance, path, MAX_PATH);
			PathRemoveFileSpec(path);
			
			sprintf_s(db, "%s\\%s", path, "AntiSpam.db");
			break;

		default:
			break;
	}

	return 1;
}

void __stdcall FreePlugin(PluginInfo* Info)
{
}

IModule *__stdcall CreateModule(IProxy* proxy, ModuleKind Kind)
{
	switch(Kind)
	{
		case GameModule:
				return new AntiSpamGameModule(proxy);
	}

	return 0;
}

extern "C"
{
	__declspec(dllexport) PluginInfo* __stdcall InitPlugin(RedVexInfo *funcs)
	{
		Funcs = funcs;

		Info.Name = "AntiSpam 1.0 Beta";
		Info.Author = "Zoxc";
		Info.SDKVersion = 3;
		Info.Destroy = (DestroyPlugin)&FreePlugin;
		Info.Create = &CreateModule;
		
		bayes = Bayes(db);

		return &Info;
	}
}