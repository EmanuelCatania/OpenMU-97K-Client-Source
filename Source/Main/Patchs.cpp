#include "stdafx.h"
#include "Patchs.h"
#include "Offset.h"
#include "Protect.h"
#include "ServerList.h"
#include "Util.h"

static const double kTargetFrameMs = 33.0; // ~30 FPS
static LARGE_INTEGER g_QpcFreq = {};
static LARGE_INTEGER g_LastQpc = {};
static bool g_QpcReady = false;
static bool g_TimerResolutionSet = false;

typedef unsigned int(WINAPI* TimeBeginPeriodFn)(unsigned int);
typedef unsigned int(WINAPI* TimeEndPeriodFn)(unsigned int);
static TimeBeginPeriodFn g_TimeBeginPeriod = nullptr;
static TimeEndPeriodFn g_TimeEndPeriod = nullptr;

static void EnsureTimerResolution()
{
	if (g_TimerResolutionSet)
	{
		return;
	}

	HMODULE winmm = LoadLibraryA("winmm.dll");
	if (!winmm)
	{
		return;
	}

	g_TimeBeginPeriod = (TimeBeginPeriodFn)GetProcAddress(winmm, "timeBeginPeriod");
	g_TimeEndPeriod = (TimeEndPeriodFn)GetProcAddress(winmm, "timeEndPeriod");
	if (g_TimeBeginPeriod && g_TimeEndPeriod)
	{
		g_TimeBeginPeriod(1);
		g_TimerResolutionSet = true;
	}
}

static void EnsureQpc()
{
	if (g_QpcReady)
	{
		return;
	}

	if (QueryPerformanceFrequency(&g_QpcFreq) && g_QpcFreq.QuadPart > 0)
	{
		QueryPerformanceCounter(&g_LastQpc);
		g_QpcReady = true;
	}
}

extern "C" __declspec(noinline) void __stdcall FrameLimiter()
{
	EnsureTimerResolution();
	EnsureQpc();
	if (!g_QpcReady)
	{
		return;
	}

	LARGE_INTEGER now = {};
	QueryPerformanceCounter(&now);

	double elapsedMs = (double)(now.QuadPart - g_LastQpc.QuadPart) * 1000.0 / (double)g_QpcFreq.QuadPart;
	if (elapsedMs < kTargetFrameMs)
	{
		double remainingMs = kTargetFrameMs - elapsedMs;
		if (remainingMs > 1.5)
		{
			Sleep((DWORD)(remainingMs - 1.0));
		}

		do
		{
			QueryPerformanceCounter(&now);
			elapsedMs = (double)(now.QuadPart - g_LastQpc.QuadPart) * 1000.0 / (double)g_QpcFreq.QuadPart;
		} while (elapsedMs < kTargetFrameMs);
	}

	g_LastQpc = now;
}

__declspec(naked) void ReduceCPU()
{
	static DWORD JmpBack = 0x00526A60;

	__asm
	{
		Call FrameLimiter
		Call Dword Ptr Ds : [0x00552198] ; //GetTickCount
		Jmp[JmpBack];
	}
}

void ReduceRam(LPVOID lpThreadParameter)
{
	HANDLE v1;

	HANDLE v2;

	while (TRUE)
	{
		Sleep(5000);

		v1 = GetCurrentProcess();

		SetProcessWorkingSetSize(v1, 0xFFFFFFFF, 0xFFFFFFFF);

		v2 = GetCurrentProcess();

		SetThreadPriority(v2, -2);
	}
}

static DWORD WINAPI FixClassNamesThread(LPVOID lpParameter)
{
	for (int n = 0; n < 200; n++)
	{
		if (GlobalText[20][0] != '\0')
		{
			break;
		}

		Sleep(50);
	}

	strcpy_s(GlobalText[44], sizeof(GlobalText[44]), "Magic Gladiator");
	strcpy_s(GlobalText[45], sizeof(GlobalText[45]), "Duel Master");
	strcpy_s(GlobalText[46], sizeof(GlobalText[46]), "Duel Master");
	strcpy_s(GlobalText[20], sizeof(GlobalText[20]), "Dark Wizard");
	strcpy_s(GlobalText[24], sizeof(GlobalText[24]), "Soul Master");
	strcpy_s(GlobalText[21], sizeof(GlobalText[21]), "Dark Knight");
	strcpy_s(GlobalText[25], sizeof(GlobalText[25]), "Blade Knight");
	strcpy_s(GlobalText[22], sizeof(GlobalText[22]), "Fairy Elf");
	strcpy_s(GlobalText[26], sizeof(GlobalText[26]), "Muse Elf");
	strcpy_s(GlobalText[23], sizeof(GlobalText[23]), "Magic Gladiator");
	strcpy_s(GlobalText[27], sizeof(GlobalText[27]), "Duel Master");

	return 0;
}

void InitPatchs()
{
	SetByte(0x00558EA8, 0xA0); // Accent

	SetByte(0x00406F36, 0xEB); // Crack return

	SetByte(0x00406F5F, 0xEB); // Crack error messagebox

	SetByte(0x00406F9B, 0xEB); // Crack error messagebox

	SetByte(0x0041ECB5, 0xEB); // Crack mu.exe

	SetByte(0x0041ED25, 0xEB); // Crack config.ini

	SetByte(0x0041ED5E, 0xEB); // Crack config.ini

	SetByte(0x0041EFB5, 0xEB); // Crack gg init

	SetByte(0x0053D90B, 0xEB); // Crack gameguard

	SetByte(0x0055961C, (gProtect.m_MainInfo.ClientVersion[0] + 1)); // Version

	SetByte(0x0055961D, (gProtect.m_MainInfo.ClientVersion[2] + 2)); // Version

	SetByte(0x0055961E, (gProtect.m_MainInfo.ClientVersion[3] + 3)); // Version

	SetByte(0x0055961F, (gProtect.m_MainInfo.ClientVersion[5] + 4)); // Version

	SetByte(0x00559620, (gProtect.m_MainInfo.ClientVersion[6] + 5)); // Version

	SetWord(0x005615BC, (gProtect.m_MainInfo.IpAddressPort)); // IpAddressPort

	SetDword(0x005267B2, (DWORD)gProtect.m_MainInfo.ScreenShotPath); //Screenshot

	MemoryCpy(0x00558ED8, gProtect.m_MainInfo.IpAddress, sizeof(gProtect.m_MainInfo.IpAddress)); // IpAddress

	MemoryCpy(0x00559624, gProtect.m_MainInfo.ClientSerial, sizeof(gProtect.m_MainInfo.ClientSerial)); // ClientSerial

	MemorySet(0x004127B0, 0x90, 5); // Remove MuError.log

	SetByte(0x0041F0AC, ANTIALIASED_QUALITY); //Font Quality

	SetByte(0x0041F0ED, ANTIALIASED_QUALITY); //Font Quality

	SetByte(0x0041F12E, ANTIALIASED_QUALITY); //Font Quality

	SetCompleteHook(0xE9, 0x00526A5A, &ReduceCPU);

	CreateThread(0, 0, (LPTHREAD_START_ROUTINE)ReduceRam, 0, 0, 0);

	CreateThread(0, 0, FixClassNamesThread, 0, 0, 0);
}
