#include "stdafx.h"
#include "Interface.h"
#include "EventTimer.h"
#include "MiniMap.h"
#include "MoveList.h"
#include "Offset.h"
#include "Util.h"
#include "CWideScreen.h"

Interface gInterface;

namespace
{
	bool IsInterfaceEnabled(const char* key, int defaultValue = 1)
	{
		return GetPrivateProfileIntA("Interface", key, defaultValue, ".\\Config.ini") != 0;
	}

	void EnsureWizardRingModelLoaded()
	{
		static bool loaded = false;
		if (loaded)
		{
			return;
		}

		loaded = true;

		char modelDir[] = "Data\\Item\\";
		char modelName[] = "MagicRing";
		char textureDir[] = "Item\\";

		AccessModel(MODEL_HELPER + 20, modelDir, modelName, 0);
		OpenTexture(MODEL_HELPER + 20, textureDir, GL_NEAREST, true);
	}
}

void Interface::Init()
{
	if (IsInterfaceEnabled("HookLoadImages", 1))
	{
		SetCompleteHook(0xE8, 0x00510DEA, &Interface::LoadImagesHook);
	}
	if (IsInterfaceEnabled("HookUpdateMouse", 1))
	{
		SetCompleteHook(0xE8, 0x005254B2, &Interface::UpdateWindowsMouseHook);
	}
	if (IsInterfaceEnabled("HookRenderWindows", 1))
	{
		SetCompleteHook(0xE8, 0x00525CEC, &Interface::RenderWindowsHook);
	}
}

void Interface::LoadImagesHook()
{
	((void(__cdecl*)())0x0050EB80)(); // OpenImages
	EnsureWizardRingModelLoaded();
	if (IsInterfaceEnabled("LoadWDecorOld", 1))
	{
		const char* decorFile = IsInterfaceEnabled("UseWDecorOldOZT", 1)
			? "Interface\\WDecorOld.OZT"
			: "Interface\\WDecorOld.tga";
		OpenTGA(decorFile, WDecorOldTEXTURE, GL_NEAREST, GL_CLAMP, false, true);
	}
}

void Interface::UpdateWindowsMouseHook()
{
	UpdateWindowsMouse();
	if (SceneFlag == MAIN_SCENE && IsInterfaceEnabled("UpdateMiniMapMouse", 1))
	{
		gMiniMap.UpdateMouse();
	}
	if (SceneFlag == MAIN_SCENE && IsInterfaceEnabled("UpdateMoveListMouse", 1))
	{
		gMoveList.UpdateMouse();
	}
	if (SceneFlag == MAIN_SCENE && IsInterfaceEnabled("UpdateEventTimerMouse", 1))
	{
		gEventTimer.UpdateMouse();
	}
}

void Interface::RenderWindowsHook()
{
	((void(_cdecl*)())0x004C3530)();
	if (SceneFlag == MAIN_SCENE && IsInterfaceEnabled("RenderMiniMap", 1))
	{
		gMiniMap.Render();
	}
	if (SceneFlag == MAIN_SCENE && IsInterfaceEnabled("RenderMoveList", 1))
	{
		gMoveList.Render();
	}
	if (SceneFlag == MAIN_SCENE && IsInterfaceEnabled("RenderEventTimer", 1))
	{
		gEventTimer.Render();
	}
}
