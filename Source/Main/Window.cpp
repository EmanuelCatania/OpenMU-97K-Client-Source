#include "StdAfx.h"
#include "Window.h"
#include "Offset.h"
#include "Protect.h"
#include "resource.h"
#include "TrayMode.h"
#include "Util.h"
#include "Font.h"
#include "Camera.h"
#include "CWideScreen.h"

cWindow	gWindow;

cWindow::cWindow()
{
	this->iResolutionValues[R640x480] = std::make_pair<WORD, WORD>(640, 480);
	this->iResolutionValues[R800x600] = std::make_pair<WORD, WORD>(800, 600);
	this->iResolutionValues[R1024x768] = std::make_pair<WORD, WORD>(1024, 768);
	this->iResolutionValues[R1280x1024] = std::make_pair<WORD, WORD>(1280, 1024);
	this->iResolutionValues[R1280x720] = std::make_pair<WORD, WORD>(1280, 720);
	this->iResolutionValues[R1366x768] = std::make_pair<WORD, WORD>(1366, 768);
	this->iResolutionValues[R1600x900] = std::make_pair<WORD, WORD>(1600, 900);
	this->iResolutionValues[R1920x1080] = std::make_pair<WORD, WORD>(1920, 1080);

	this->m_Borderless = false;

	HKEY Key;

	if (RegOpenKey(HKEY_CURRENT_USER, "Software\\Webzen\\MU\\Config", &Key) == ERROR_SUCCESS)
	{
		DWORD Size = sizeof(int);

		DWORD windowMode = 0;
		if (RegQueryValueEx(Key, "WindowMode", nullptr, nullptr, (LPBYTE)(&windowMode), &Size) != ERROR_SUCCESS)
		{
			windowMode = 0;
		}

		this->m_WindowMode = (windowMode != 0);

		RegCloseKey(Key);
	}
	else
	{
		this->m_WindowMode = false;
	}

	if (GetPrivateProfileIntA("Debug", "SkipWindowCtorInit", 0, ".\\Config.ini") == 0)
	{
		SetResolution(R1024x768);
	}
}

cWindow::~cWindow()
{

}

void cWindow::WindowModeLoad(HINSTANCE hins)
{
	this->Instance = hins;

	SetDword(0x0055237C, (DWORD)&this->ChangeDisplaySettingsHook);

	SetCompleteHook(0xE9, 0x0041ED79, 0x0041EEC6);

	SetCompleteHook(0xE9, 0x0041DFF0, &this->StartWindow);

	SetCompleteHook(0xE9, 0x0041DE30, &cWindow::CreateOpenglWindow);

	if (this->m_WindowMode)
	{
		SetCompleteHook(0xE9, 0x0041F617, 0x00421B0B);
	}
}

LONG WINAPI cWindow::ChangeDisplaySettingsHook(DEVMODEA* lpDevMode, DWORD dwFlags)
{
	if (!gWindow.m_WindowMode)
	{
		return ChangeDisplaySettingsA(lpDevMode, dwFlags);
	}

	return DISP_CHANGE_SUCCESSFUL;
}

void cWindow::SetResolution(int res)
{
	if (res < R640x480 || res >= MAX_RESOLUTION_VALUE)
	{
		res = R1024x768;
	}

	m_Resolution = res;
	WindowWidth = this->iResolutionValues[res].first;
	WindowHeight = this->iResolutionValues[res].second;
	if (gCWideScreen.Scale <= 0.0f)
	{
		gCWideScreen.Scale = 1.0f;
	}

	if (res >= R1280x720)
	{
		gCWideScreen.numW = 853.33f * gCWideScreen.Scale;
		gCWideScreen.numW2 = 480.0f * gCWideScreen.Scale;
		gCWideScreen.RENDER_ITEMVIEW_FAR = 2000.0f * gCWideScreen.Scale;
		gCWideScreen.y_MainUi = gCWideScreen.numW2 - 48.0f;
		gCWideScreen.WCenter = (gCWideScreen.numW - 640.0f) / 2.0f;
	}
	else
	{
		gCWideScreen.numW = 640.0f;
		gCWideScreen.numW2 = 480.0f;
		gCWideScreen.RENDER_ITEMVIEW_FAR = 2000.0f;
		gCWideScreen.y_MainUi = 432.0f;
		gCWideScreen.WCenter = 0.0f;
	}

	g_fScreenRate_x = (float)WindowWidth / gCWideScreen.numW;
	g_fScreenRate_y = (float)WindowHeight / gCWideScreen.numW2;
	OpenglWindowWidth = WindowWidth;
	OpenglWindowHeight = WindowHeight;
}

void cWindow::ChangeWindowState(bool windowMode, bool borderless, int resolution)
{
	bool modeChanged = (windowMode != this->m_WindowMode) || (borderless != this->m_Borderless);
	bool resolutionChanged = (resolution != m_Resolution);

	if (resolutionChanged)
	{
		SetResolution(resolution);
		gCWideScreen.Load();
	}

	if (modeChanged)
	{
		if (!windowMode)
		{
			ChangeDisplaySettingsFunction();
		}
		else if (windowMode != this->m_WindowMode)
		{
			ChangeDisplaySettings(NULL, 0);
		}

		this->m_WindowMode = windowMode;
		this->m_Borderless = borderless;
	}
	else if (resolutionChanged && !this->m_WindowMode)
	{
		ChangeDisplaySettingsFunction();
	}

	RECT rc = { 0, 0, WindowWidth, WindowHeight };

	LONG posX = (GetSystemMetrics(SM_CXSCREEN) / 2) - (WindowWidth / 2);
	LONG posY = (GetSystemMetrics(SM_CYSCREEN) / 2) - (WindowHeight / 2);

	LONG style;
	LONG exStyle;

	if (this->m_WindowMode)
	{
		style = this->m_Borderless
			? (WS_POPUP | WS_VISIBLE)
			: (WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_VISIBLE);

		exStyle = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
	}
	else
	{
		style = WS_POPUP | WS_VISIBLE;
		exStyle = WS_EX_APPWINDOW;
	}

	SetWindowLongPtr(g_hWnd, GWL_STYLE, style);
	SetWindowLongPtr(g_hWnd, GWL_EXSTYLE, exStyle);

	AdjustWindowRect(&rc, style, FALSE);

	SetWindowPos(g_hWnd, NULL, posX, posY, rc.right - rc.left, rc.bottom - rc.top,
		SWP_SHOWWINDOW | SWP_NOZORDER | SWP_FRAMECHANGED);

	MoveWindow(g_hWnd, posX, posY, rc.right - rc.left, rc.bottom - rc.top, TRUE);

	gCamera.SetCurrentValue();
	gFont.ReloadFont();
}

void cWindow::ChangeDisplaySettingsFunction()
{
	DEVMODE DevMode;

	int nModes = 0;

	while (EnumDisplaySettings(NULL, nModes, &DevMode))
	{
		nModes++;
	}

	DEVMODE* pDevmodes = new DEVMODE[nModes + 1];

	nModes = 0;

	while (EnumDisplaySettings(NULL, nModes, &pDevmodes[nModes]))
	{
		nModes++;
	}

	DWORD dwBitsPerPel = 32;

	for (int n1 = 0; n1 < nModes; n1++)
	{
		if (pDevmodes[n1].dmBitsPerPel == 24 || pDevmodes[n1].dmBitsPerPel == 32)
		{
			dwBitsPerPel = pDevmodes[n1].dmBitsPerPel;

			break;
		}
	}

	for (int n2 = 0; n2 < nModes; n2++)
	{
		if (pDevmodes[n2].dmPelsWidth == WindowWidth && pDevmodes[n2].dmPelsHeight == WindowHeight && pDevmodes[n2].dmBitsPerPel == dwBitsPerPel)
		{
			ChangeDisplaySettings(&pDevmodes[n2], 0);

			break;
		}
	}

	delete[] pDevmodes;
}

HWND cWindow::StartWindow(HINSTANCE hCurrentInst, int nCmdShow)
{
	char* windowName = "MU";

	WNDCLASS wndClass;

	wndClass.style = CS_OWNDC | CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW;

	wndClass.lpfnWndProc = gWindow.MyWndProc;

	wndClass.cbClsExtra = 0;

	wndClass.cbWndExtra = 0;

	wndClass.hInstance = hCurrentInst;

	wndClass.hIcon = LoadIcon(gWindow.Instance, MAKEINTRESOURCE(IDI_ICON));

	wndClass.hCursor = LoadCursor(NULL, IDC_ARROW);

	wndClass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);

	wndClass.lpszMenuName = NULL;

	wndClass.lpszClassName = windowName;

	RegisterClass(&wndClass);

	HWND hWnd;

	if (!gWindow.m_WindowMode)
	{
		hWnd = CreateWindowExA(WS_EX_APPWINDOW | WS_EX_TOPMOST, windowName, gProtect.m_MainInfo.WindowName, WS_POPUP, 0, 0, WindowWidth, WindowHeight, NULL, NULL, hCurrentInst, NULL);

		gWindow.ChangeDisplaySettingsFunction();
	}
	else
	{
		RECT rc = { 0, 0, WindowWidth, WindowHeight };

		AdjustWindowRect(&rc, WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_BORDER | WS_CLIPCHILDREN, NULL);

		hWnd = CreateWindowA(windowName, gProtect.m_MainInfo.WindowName, WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_BORDER | WS_CLIPCHILDREN, (GetSystemMetrics(SM_CXSCREEN) - rc.right) / 2, (GetSystemMetrics(SM_CYSCREEN) - rc.bottom) / 2, rc.right, rc.bottom + 28, NULL, NULL, hCurrentInst, NULL);
	}

	return hWnd;
}

LRESULT WINAPI cWindow::MyWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
		case WM_NCACTIVATE:
		{
			return 0;
		}
	}

	return CallWindowProc(WndProc, hwnd, msg, wParam, lParam);
}

typedef HGLRC(WINAPI* wglCreateContextAttribsARBProc)(HDC, HGLRC, const int*);
typedef BOOL(WINAPI* wglSwapIntervalEXTProc)(int);

static HGLRC CreateCompatContext33(HDC hdc)
{
	HGLRC legacy = wglCreateContext(hdc);
	if (!legacy)
	{
		return nullptr;
	}

	if (!wglMakeCurrent(hdc, legacy))
	{
		wglDeleteContext(legacy);
		return nullptr;
	}

	wglCreateContextAttribsARBProc createAttribs =
		(wglCreateContextAttribsARBProc)wglGetProcAddress("wglCreateContextAttribsARB");
	if (!createAttribs)
	{
		return legacy;
	}

	const int WGL_CONTEXT_MAJOR_VERSION_ARB = 0x2091;
	const int WGL_CONTEXT_MINOR_VERSION_ARB = 0x2092;
	const int WGL_CONTEXT_PROFILE_MASK_ARB = 0x9126;
	const int WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB = 0x00000002;

	const int attribs[] =
	{
		WGL_CONTEXT_MAJOR_VERSION_ARB, 3,
		WGL_CONTEXT_MINOR_VERSION_ARB, 3,
		WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB,
		0
	};

	HGLRC modern = createAttribs(hdc, 0, attribs);
	if (!modern)
	{
		return legacy;
	}

	wglMakeCurrent(NULL, NULL);
	wglDeleteContext(legacy);
	wglMakeCurrent(hdc, modern);

	return modern;
}

bool cWindow::CreateOpenglWindow()
{
	PIXELFORMATDESCRIPTOR pfd = {};

	pfd.nSize = sizeof(pfd);
	pfd.nVersion = 1;
	pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = 32;
	pfd.cDepthBits = 32;

	if (!(g_hDC = GetDC(g_hWnd)))
	{
		MessageBox(NULL, "OpenGL Get DC Error.", "OpenGL Error", MB_OK | MB_ICONEXCLAMATION);
		return false;
	}

	GLuint PixelFormat = 0;
	if (!(PixelFormat = ChoosePixelFormat(g_hDC, &pfd)))
	{
		MessageBox(NULL, "OpenGL Choose Pixel Format Error.", "OpenGL Error", MB_OK | MB_ICONEXCLAMATION);
		return false;
	}

	if (!SetPixelFormat(g_hDC, PixelFormat, &pfd))
	{
		MessageBox(NULL, "OpenGL Set Pixel Format Error.", "OpenGL Error", MB_OK | MB_ICONEXCLAMATION);
		return false;
	}

	g_hRC = CreateCompatContext33(g_hDC);
	if (!g_hRC)
	{
		MessageBox(NULL, "OpenGL Create Context Error.", "OpenGL Error", MB_OK | MB_ICONEXCLAMATION);
		return false;
	}

	if (!wglMakeCurrent(g_hDC, g_hRC))
	{
		MessageBox(NULL, "OpenGL Make Current Error.", "OpenGL Error", MB_OK | MB_ICONEXCLAMATION);
		return false;
	}

	wglSwapIntervalEXTProc swapInterval =
		(wglSwapIntervalEXTProc)wglGetProcAddress("wglSwapIntervalEXT");
	if (swapInterval)
	{
		swapInterval(0);
	}

	ShowWindow(g_hWnd, SW_SHOW);
	SetForegroundWindow(g_hWnd);
	SetFocus(g_hWnd);

	return true;
}
