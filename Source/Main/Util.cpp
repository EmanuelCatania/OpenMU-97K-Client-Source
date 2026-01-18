#include "stdafx.h"
#include "Util.h"
#include "Offset.h"

namespace
{
	bool IsPatchSafetyEnabled()
	{
		static int cached = -1;
		if (cached < 0)
		{
			cached = GetPrivateProfileIntA("Debug", "EnablePatchSafety", 1, ".\\Config.ini");
		}
		return cached != 0;
	}

	bool IsOffsetInMainModule(DWORD offset, size_t size)
	{
		HMODULE module = GetModuleHandle(nullptr);
		if (module == nullptr)
		{
			return false;
		}

		auto base = reinterpret_cast<const BYTE*>(module);
		auto dosHeader = reinterpret_cast<const IMAGE_DOS_HEADER*>(base);
		if (dosHeader->e_magic != IMAGE_DOS_SIGNATURE)
		{
			return false;
		}

		auto ntHeaders = reinterpret_cast<const IMAGE_NT_HEADERS*>(base + dosHeader->e_lfanew);
		if (ntHeaders->Signature != IMAGE_NT_SIGNATURE)
		{
			return false;
		}

		DWORD imageStart = reinterpret_cast<DWORD>(base);
		DWORD imageEnd = imageStart + ntHeaders->OptionalHeader.SizeOfImage;
		DWORD endOffset = offset + static_cast<DWORD>(size);

		return offset >= imageStart && endOffset <= imageEnd;
	}

	bool CanPatchMemory(DWORD offset, size_t size)
	{
		if (!IsOffsetInMainModule(offset, size))
		{
			return false;
		}

		MEMORY_BASIC_INFORMATION mbi = {};
		if (VirtualQuery(reinterpret_cast<const void*>(offset), &mbi, sizeof(mbi)) == 0)
		{
			return false;
		}

		if (mbi.State != MEM_COMMIT)
		{
			return false;
		}

		if ((mbi.Protect & PAGE_GUARD) != 0 || (mbi.Protect & PAGE_NOACCESS) != 0)
		{
			return false;
		}

		DWORD regionEnd = reinterpret_cast<DWORD>(mbi.BaseAddress) + static_cast<DWORD>(mbi.RegionSize);
		DWORD endOffset = offset + static_cast<DWORD>(size);
		return endOffset <= regionEnd;
	}
}

void SetByte(DWORD offset, BYTE value)
{
	if (IsPatchSafetyEnabled() && !CanPatchMemory(offset, sizeof(BYTE)))
	{
		return;
	}

	DWORD OldProtect;

	VirtualProtect((void*)offset, 1, PAGE_EXECUTE_READWRITE, &OldProtect);

	*(BYTE*)(offset) = value;

	VirtualProtect((void*)offset, 1, OldProtect, &OldProtect);
}

void SetWord(DWORD offset, WORD value)
{
	if (IsPatchSafetyEnabled() && !CanPatchMemory(offset, sizeof(WORD)))
	{
		return;
	}

	DWORD OldProtect;

	VirtualProtect((void*)offset, 2, PAGE_EXECUTE_READWRITE, &OldProtect);

	*(WORD*)(offset) = value;

	VirtualProtect((void*)offset, 2, OldProtect, &OldProtect);
}

void SetDword(DWORD offset, DWORD value)
{
	if (IsPatchSafetyEnabled() && !CanPatchMemory(offset, sizeof(DWORD)))
	{
		return;
	}

	DWORD OldProtect;

	VirtualProtect((void*)offset, 4, PAGE_EXECUTE_READWRITE, &OldProtect);

	*(DWORD*)(offset) = value;

	VirtualProtect((void*)offset, 4, OldProtect, &OldProtect);
}

void SetFloat(DWORD offset, float value)
{
	if (IsPatchSafetyEnabled() && !CanPatchMemory(offset, sizeof(float)))
	{
		return;
	}

	DWORD OldProtect;

	VirtualProtect((void*)offset, 4, PAGE_EXECUTE_READWRITE, &OldProtect);

	*(float*)(offset) = value;

	VirtualProtect((void*)offset, 4, OldProtect, &OldProtect);
}

void SetDouble(DWORD offset, double value)
{
	if (IsPatchSafetyEnabled() && !CanPatchMemory(offset, sizeof(double)))
	{
		return;
	}

	DWORD OldProtect;

	VirtualProtect((void*)offset, 8, PAGE_EXECUTE_READWRITE, &OldProtect);

	*(double*)(offset) = value;

	VirtualProtect((void*)offset, 8, OldProtect, &OldProtect);
}

void SetCompleteHook(BYTE head, DWORD offset, ...)
{
	if (IsPatchSafetyEnabled() && !CanPatchMemory(offset, 5))
	{
		return;
	}

	DWORD OldProtect;

	VirtualProtect((void*)offset, 5, PAGE_EXECUTE_READWRITE, &OldProtect);

	if (head != 0xFF)
	{
		*(BYTE*)(offset) = head;
	}

	DWORD* function = &offset + 1;

	*(DWORD*)(offset + 1) = (*function) - (offset + 5);

	VirtualProtect((void*)offset, 5, OldProtect, &OldProtect);
}

void MemoryCpy(DWORD offset, void* value, DWORD size)
{
	if (IsPatchSafetyEnabled() && !CanPatchMemory(offset, size))
	{
		return;
	}

	DWORD OldProtect;

	VirtualProtect((void*)offset, size, PAGE_EXECUTE_READWRITE, &OldProtect);

	memcpy((void*)offset, value, size);

	VirtualProtect((void*)offset, size, OldProtect, &OldProtect);
}

void MemorySet(DWORD offset, DWORD value, DWORD size)
{
	if (IsPatchSafetyEnabled() && !CanPatchMemory(offset, size))
	{
		return;
	}

	DWORD OldProtect;

	VirtualProtect((void*)offset, size, PAGE_EXECUTE_READWRITE, &OldProtect);

	memset((void*)offset, value, size);

	VirtualProtect((void*)offset, size, OldProtect, &OldProtect);
}

void VirtualizeOffset(DWORD offset, DWORD size)
{
	if (IsPatchSafetyEnabled() && !CanPatchMemory(offset, size))
	{
		return;
	}

	DWORD OldProtect;

	VirtualProtect((void*)offset, size, PAGE_EXECUTE_READWRITE, &OldProtect);

	DWORD HookAddr = (DWORD)malloc(size + 5);

	memcpy((void*)HookAddr, (void*)offset, size);

	*(BYTE*)(HookAddr + size) = 0xE9;

	*(DWORD*)(HookAddr + size + 1) = (offset + size) - ((HookAddr + size) + 5);

	*(BYTE*)(offset) = 0xE9;

	*(DWORD*)(offset + 1) = HookAddr - (offset + 5);

	memset((void*)(offset + 5), 0x90, (size - 5));

	VirtualProtect((void*)offset, size, OldProtect, &OldProtect);
}

void PacketArgumentEncrypt(char* out_buff, char* in_buff, int size)
{
	BYTE XorTable[3] = { 0xFC, 0xCF, 0xAB };

	for (int n = 0; n < size; n++)
	{
		out_buff[n] = in_buff[n] ^ XorTable[n % 3];
	}
}

char* ConvertModuleFileName(char* name)
{
	static char buff[MAX_PATH] = { 0 };

	for (int n = strlen(name); n > 0; n--)
	{
		if (name[n] == '\\')
		{
			strcpy_s(buff, sizeof(buff), &name[(n + 1)]);

			break;
		}
	}

	return buff;
}

SHORT WINAPI GetAsyncKeyStateCall(int key)
{
	if (GetForegroundWindow() != g_hWnd)
	{
		return 0;
	}

	return GetAsyncKeyState(key);
}

bool FileExists(char* name)
{
	if (CreateFile(name, 0, 0, 0, OPEN_EXISTING, 0, 0) == INVALID_HANDLE_VALUE)
	{
		return false;
	}

	return true;
};

int GetTextWidth(char* buff)
{
	SIZE sz;

	GetTextExtentPoint(m_hFontDC, buff, (int)strlen(buff), &sz);

	return (640 * sz.cx / WindowWidth);
}

int GetTextHeight(char* buff)
{
	SIZE sz;

	GetTextExtentPoint(m_hFontDC, buff, (int)strlen(buff), &sz);

	return (480 * sz.cy / WindowHeight);
}

int CenterTextPosX(char* buff, int PosX)
{
	SIZE sz;

	GetTextExtentPoint(m_hFontDC, buff, (int)strlen(buff), &sz);

	return (PosX - (((640 * sz.cx) / WindowWidth) >> 1));
}

int CenterTextPosY(const char* buff, int PosY)
{
	SIZE sz;

	GetTextExtentPoint(m_hFontDC, buff, (int)strlen(buff), &sz);

	return (PosY - (((480 * sz.cy) / WindowHeight) >> 1));
}

int GetTextPosX(char* buff, int PosX)
{
	HDC hdc = GetDC(g_hWnd);

	SIZE sz;

	GetTextExtentPoint(hdc, buff, strlen(buff), &sz);

	return (PosX - (640 * sz.cx / WindowWidth >> 1));
}

int GetTextPosY(char* buff, int PosY)
{
	HDC hdc = GetDC(g_hWnd);

	SIZE sz;

	GetTextExtentPoint(hdc, buff, strlen(buff), &sz);

	return (PosY - (480 * sz.cy / WindowHeight >> 1));
}

float ImgCenterScreenPosX(float Size)
{
	return (320.0f - (Size / 2.0f));
}

float ImgCenterScreenPosY(float Size)
{
	return (216.0f - (Size / 2.0f));
}

bool IsWorkZone(int X, int Y, int Width, int Height)
{
	if ((MouseX >= X && MouseX <= (X + Width)) && (MouseY >= Y && MouseY <= (Y + Height)))
	{
		return true;
	}

	return false;
}

void ConvertGold(double dGold, char* szText, int iDecimals)
{
	char szTemp[256] = { 0 };

	int iCipherCnt = 0;

	DWORD dwValueTemp = (DWORD)dGold;

	while (dwValueTemp / 1000 > 0)
	{
		iCipherCnt = iCipherCnt + 3;
		dwValueTemp = dwValueTemp / 1000;
	}

	sprintf_s(szTemp, "%d", dwValueTemp);

	std::string sText = szTemp;

	while (iCipherCnt > 0)
	{
		dwValueTemp = (DWORD)dGold;
		dwValueTemp = (dwValueTemp % (int)pow(10.0f, (float)iCipherCnt)) / (int)pow(10.0f, (float)(iCipherCnt - 3));

		sprintf_s(szTemp, ",%03d", dwValueTemp);

		sText += szTemp;

		iCipherCnt = iCipherCnt - 3;
	}

	if (iDecimals > 0)
	{
		dwValueTemp = (int)(dGold * pow(10.0f, (float)iDecimals)) % (int)pow(10.0f, (float)iDecimals);

		sprintf_s(szTemp, ".%d", dwValueTemp);

		sText += szTemp;
	}

	wsprintf(szText, "%s", sText.c_str());
}
