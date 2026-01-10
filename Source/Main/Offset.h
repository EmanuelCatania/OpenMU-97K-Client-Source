#pragma once

#define SceneFlag *(int*)0x005615C0

#define FontHeight *(int*)0x07D78080

#define m_Resolution *(int*)0x055C9E38

#define WindowWidth *(int*)0x0056156C

#define WindowHeight *(int*)0x00561570

#define g_fScreenRate_x *(float*)0x55C9B70

#define g_fScreenRate_y *(float*)0x55C9B74

#define g_hWnd *(HWND*)0x055C9FFC

#define g_hDC *(HDC*)0x055CA004

#define g_hRC *(HGLRC*)0x055CA008

#define WndProc (WNDPROC)(0x4149D0)

#define pDrawMessage ((int(__cdecl*)(LPCSTR Text, int Mode)) 0x0047FAE0)