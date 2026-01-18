#pragma once

#include "Enums.h"

class cWindow
{
public:

	cWindow();

	~cWindow();

	void WindowModeLoad(HINSTANCE hins);

	void ChangeWindowState(bool windowMode, bool borderless, int resolution);

private:

	static LONG WINAPI ChangeDisplaySettingsHook(DEVMODEA* lpDevMode, DWORD dwFlags);

	void ChangeDisplaySettingsFunction();

	void SetResolution(int res);

	static HWND StartWindow(HINSTANCE hCurrentInst, int nCmdShow);

	static LONG WINAPI MyWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

	static bool CreateOpenglWindow();

	HINSTANCE Instance;

public:

	bool m_WindowMode;

	bool m_Borderless;

	std::pair<WORD, WORD> iResolutionValues[MAX_RESOLUTION_VALUE];
};

extern cWindow gWindow;
