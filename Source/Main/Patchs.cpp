#include "stdafx.h"
#include "Patchs.h"
#include "Protect.h"
#include "Util.h"

__declspec(naked) void ReduceCPU()
{
	static DWORD JmpBack = 0x00526A60;

	__asm
	{
		Push 1;

		Call Dword Ptr Ds : [0x00552128] ; //Sleep

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
}