#include "stdafx.h"
#include "Controller.h"
#include "Patchs.h"
#include "Protect.h"
#include "Resolution.h"
#include "TrayMode.h"
#include "Util.h"
#include "Window.h"

HINSTANCE hins;

extern "C" _declspec(dllexport) void _cdecl EntryProc()
{
	if (gProtect.ReadMainFile("main.emu") == 0)
	{
		MessageBoxA(NULL, "Read file incorrect or not exists", "Error", MB_OK);

		ExitProcess(0);
	}

	gProtect.CheckLauncher();
	
	gProtect.CheckInstance();

	gProtect.CheckClientFile();

	gProtect.CheckPluginFile();

	//InitConsole();

	InitPatchs();

	InitResolution();
}

BOOL APIENTRY DllMain(HMODULE hModule,DWORD ul_reason_for_call,LPVOID lpReserved)
{
	switch(ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
		{
			hins = (HINSTANCE)hModule;

			gWindow.WindowModeLoad(hins);

			gController.Load(hins);

			gTrayMode.Init(hins);

			break;
		}

		case DLL_PROCESS_DETACH:
		{
			break;
		}

		case DLL_THREAD_ATTACH:
		{
			break;
		}

		case DLL_THREAD_DETACH:
		{
			break;
		}
	}

	return 1;
}