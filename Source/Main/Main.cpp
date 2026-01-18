#include "stdafx.h"
#include "Controller.h"
#include "EventTimer.h"
#include "Font.h"
#include "Interface.h"
#include "Language.h"
#include "MapManager.h"
#include "MiniMap.h"
#include "MoveList.h"
#include "OptionsMenu.h"
#include "Patchs.h"
#include "PrintPlayer.h"
#include "Protect.h"
#include "Resolution.h"
#include "Sound.h"
#include "TrayMode.h"
#include "Util.h"
#include "Window.h"
#include "GuildLogo.h"
#include "CWideScreen.h"

HINSTANCE hins;

namespace
{
	bool IsInitEnabled(const char* key, int defaultValue = 1)
	{
		return GetPrivateProfileIntA("Debug", key, defaultValue, ".\\Config.ini") != 0;
	}

	using ExitProcessFn = void (WINAPI*)(UINT);
	using TerminateProcessFn = BOOL (WINAPI*)(HANDLE, UINT);
	using LoadLibraryAFn = HMODULE (WINAPI*)(LPCSTR);
	using LoadLibraryWFn = HMODULE (WINAPI*)(LPCWSTR);
	using CreateProcessAFn = BOOL (WINAPI*)(LPCSTR, LPSTR, LPSECURITY_ATTRIBUTES, LPSECURITY_ATTRIBUTES, BOOL, DWORD, LPVOID, LPCSTR, LPSTARTUPINFOA, LPPROCESS_INFORMATION);
	using CreateProcessWFn = BOOL (WINAPI*)(LPCWSTR, LPWSTR, LPSECURITY_ATTRIBUTES, LPSECURITY_ATTRIBUTES, BOOL, DWORD, LPVOID, LPCWSTR, LPSTARTUPINFOW, LPPROCESS_INFORMATION);
	using WinExecFn = UINT (WINAPI*)(LPCSTR, UINT);
	using ShellExecuteAFn = HINSTANCE (WINAPI*)(HWND, LPCSTR, LPCSTR, LPCSTR, LPCSTR, INT);
	using ShellExecuteWFn = HINSTANCE (WINAPI*)(HWND, LPCWSTR, LPCWSTR, LPCWSTR, LPCWSTR, INT);
	using MessageBoxAFn = int (WINAPI*)(HWND, LPCSTR, LPCSTR, UINT);
	using MessageBoxWFn = int (WINAPI*)(HWND, LPCWSTR, LPCWSTR, UINT);

	ExitProcessFn g_ExitProcess = nullptr;
	TerminateProcessFn g_TerminateProcess = nullptr;
	LoadLibraryAFn g_LoadLibraryA = nullptr;
	LoadLibraryWFn g_LoadLibraryW = nullptr;
	CreateProcessAFn g_CreateProcessA = nullptr;
	CreateProcessWFn g_CreateProcessW = nullptr;
	WinExecFn g_WinExec = nullptr;
	ShellExecuteAFn g_ShellExecuteA = nullptr;
	ShellExecuteWFn g_ShellExecuteW = nullptr;
	MessageBoxAFn g_MessageBoxA = nullptr;
	MessageBoxWFn g_MessageBoxW = nullptr;
	bool g_EarlyHooksInstalled = false;

	void WINAPI ExitProcessHook(UINT exitCode)
	{
		if (IsInitEnabled("BypassExitProcess", 0))
		{
			return;
		}

		if (g_ExitProcess)
		{
			g_ExitProcess(exitCode);
		}
	}

	BOOL WINAPI TerminateProcessHook(HANDLE hProcess, UINT exitCode)
	{
		if (IsInitEnabled("BypassExitProcess", 0) && hProcess == GetCurrentProcess())
		{
			return TRUE;
		}

		return g_TerminateProcess ? g_TerminateProcess(hProcess, exitCode) : FALSE;
	}

	bool PatchImportFunction(HMODULE module, const char* dllName, const char* funcName, void* newFunc, void** oldFunc)
	{
		if (!module)
		{
			return false;
		}

		auto base = reinterpret_cast<BYTE*>(module);
		auto dosHeader = reinterpret_cast<IMAGE_DOS_HEADER*>(base);
		if (dosHeader->e_magic != IMAGE_DOS_SIGNATURE)
		{
			return false;
		}

		auto ntHeaders = reinterpret_cast<IMAGE_NT_HEADERS*>(base + dosHeader->e_lfanew);
		if (ntHeaders->Signature != IMAGE_NT_SIGNATURE)
		{
			return false;
		}

		auto& dir = ntHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT];
		if (!dir.VirtualAddress)
		{
			return false;
		}

		auto importDesc = reinterpret_cast<IMAGE_IMPORT_DESCRIPTOR*>(base + dir.VirtualAddress);
		for (; importDesc->Name; ++importDesc)
		{
			const char* name = reinterpret_cast<const char*>(base + importDesc->Name);
			if (_stricmp(name, dllName) != 0)
			{
				continue;
			}

			auto thunk = reinterpret_cast<IMAGE_THUNK_DATA*>(base + importDesc->FirstThunk);
			auto origThunk = importDesc->OriginalFirstThunk
				? reinterpret_cast<IMAGE_THUNK_DATA*>(base + importDesc->OriginalFirstThunk)
				: thunk;

			for (; origThunk->u1.AddressOfData; ++origThunk, ++thunk)
			{
				if (IMAGE_SNAP_BY_ORDINAL(origThunk->u1.Ordinal))
				{
					continue;
				}

				auto importByName = reinterpret_cast<IMAGE_IMPORT_BY_NAME*>(base + origThunk->u1.AddressOfData);
				if (strcmp(reinterpret_cast<const char*>(importByName->Name), funcName) != 0)
				{
					continue;
				}

				DWORD oldProtect;
				if (!VirtualProtect(&thunk->u1.Function, sizeof(ULONG_PTR), PAGE_EXECUTE_READWRITE, &oldProtect))
				{
					return false;
				}

				if (oldFunc)
				{
					*oldFunc = reinterpret_cast<void*>(thunk->u1.Function);
				}

				thunk->u1.Function = reinterpret_cast<ULONG_PTR>(newFunc);

				VirtualProtect(&thunk->u1.Function, sizeof(ULONG_PTR), oldProtect, &oldProtect);
				return true;
			}
		}

		return false;
	}

	void InstallExitHooks()
	{
		if (!IsInitEnabled("BypassExitProcess", 0))
		{
			return;
		}

		HMODULE exe = GetModuleHandle(nullptr);
		PatchImportFunction(exe, "KERNEL32.dll", "ExitProcess", ExitProcessHook, reinterpret_cast<void**>(&g_ExitProcess));
		PatchImportFunction(exe, "KERNEL32.dll", "TerminateProcess", TerminateProcessHook, reinterpret_cast<void**>(&g_TerminateProcess));
	}

	bool PatchFunctionRet(void* function, WORD stackBytes)
	{
		if (!function)
		{
			return false;
		}

		DWORD oldProtect = 0;
		if (!VirtualProtect(function, 3, PAGE_EXECUTE_READWRITE, &oldProtect))
		{
			return false;
		}

		auto bytes = reinterpret_cast<BYTE*>(function);
		bytes[0] = 0xC2; // ret imm16
		bytes[1] = static_cast<BYTE>(stackBytes & 0xFF);
		bytes[2] = static_cast<BYTE>((stackBytes >> 8) & 0xFF);

		VirtualProtect(function, 3, oldProtect, &oldProtect);
		return true;
	}

	void InstallHardExitHooks()
	{
		if (!IsInitEnabled("HardBypassExitProcess", 0))
		{
			return;
		}

		HMODULE kernel32 = GetModuleHandleA("KERNEL32.dll");
		HMODULE ntdll = GetModuleHandleA("ntdll.dll");

		PatchFunctionRet(kernel32 ? GetProcAddress(kernel32, "ExitProcess") : nullptr, 4);
		PatchFunctionRet(kernel32 ? GetProcAddress(kernel32, "TerminateProcess") : nullptr, 8);
		PatchFunctionRet(ntdll ? GetProcAddress(ntdll, "RtlExitUserProcess") : nullptr, 4);
		PatchFunctionRet(ntdll ? GetProcAddress(ntdll, "NtTerminateProcess") : nullptr, 8);
	}

	void AppendLogLine(const char* text)
	{
		if (!text || !*text)
		{
			return;
		}

		HANDLE file = CreateFileA(".\\gg_load.log", FILE_APPEND_DATA, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
		if (file == INVALID_HANDLE_VALUE)
		{
			return;
		}

		DWORD written = 0;
		WriteFile(file, text, static_cast<DWORD>(strlen(text)), &written, nullptr);
		CloseHandle(file);
	}

	bool ShouldBlockLibrary(const char* name)
	{
		if (!IsInitEnabled("BlockGameGuard", 0))
		{
			return false;
		}

		if (!name)
		{
			return false;
		}

		char lower[260] = {};
		size_t len = strlen(name);
		if (len >= sizeof(lower))
		{
			len = sizeof(lower) - 1;
		}

		for (size_t i = 0; i < len; ++i)
		{
			char c = name[i];
			lower[i] = (c >= 'A' && c <= 'Z') ? (c - 'A' + 'a') : c;
		}

		return strstr(lower, "gameguard") != nullptr
			|| strstr(lower, "nprotect") != nullptr
			|| strstr(lower, "npgg") != nullptr
			|| strstr(lower, ".des") != nullptr;
	}

	bool ShouldBlockProcessName(const char* name)
	{
		if (!IsInitEnabled("BlockGGProcess", 0) && !IsInitEnabled("BlockGameGuard", 0))
		{
			return false;
		}

		if (!name)
		{
			return false;
		}

		char lower[260] = {};
		size_t len = strlen(name);
		if (len >= sizeof(lower))
		{
			len = sizeof(lower) - 1;
		}

		for (size_t i = 0; i < len; ++i)
		{
			char c = name[i];
			lower[i] = (c >= 'A' && c <= 'Z') ? (c - 'A' + 'a') : c;
		}

		return strstr(lower, "gameguard") != nullptr
			|| strstr(lower, "nprotect") != nullptr
			|| strstr(lower, "npgg") != nullptr;
	}

	void FillFakeProcessInfo(LPPROCESS_INFORMATION lpProcessInformation)
	{
		if (!lpProcessInformation)
		{
			return;
		}

		ZeroMemory(lpProcessInformation, sizeof(PROCESS_INFORMATION));

		HANDLE processHandle = nullptr;
		HANDLE threadHandle = nullptr;
		DuplicateHandle(GetCurrentProcess(), GetCurrentProcess(), GetCurrentProcess(), &processHandle, 0, FALSE, DUPLICATE_SAME_ACCESS);
		DuplicateHandle(GetCurrentProcess(), GetCurrentThread(), GetCurrentProcess(), &threadHandle, 0, FALSE, DUPLICATE_SAME_ACCESS);

		lpProcessInformation->hProcess = processHandle;
		lpProcessInformation->hThread = threadHandle;
		lpProcessInformation->dwProcessId = GetCurrentProcessId();
		lpProcessInformation->dwThreadId = GetCurrentThreadId();
	}

	BOOL WINAPI CreateProcessAHook(LPCSTR lpApplicationName, LPSTR lpCommandLine, LPSECURITY_ATTRIBUTES lpProcessAttributes,
		LPSECURITY_ATTRIBUTES lpThreadAttributes, BOOL bInheritHandles, DWORD dwCreationFlags, LPVOID lpEnvironment,
		LPCSTR lpCurrentDirectory, LPSTARTUPINFOA lpStartupInfo, LPPROCESS_INFORMATION lpProcessInformation)
	{
		if (IsInitEnabled("HookCreateProcess", 0))
		{
			char line[1024];
			sprintf_s(line, sizeof(line), "CreateProcessA: app=\"%s\" cmd=\"%s\"\r\n",
				lpApplicationName ? lpApplicationName : "", lpCommandLine ? lpCommandLine : "");
			AppendLogLine(line);

			if (ShouldBlockProcessName(lpApplicationName) || ShouldBlockProcessName(lpCommandLine))
			{
				AppendLogLine("CreateProcessA blocked\r\n");
				if (IsInitEnabled("FakeGGProcessSuccess", 0))
				{
					FillFakeProcessInfo(lpProcessInformation);
					return TRUE;
				}

				SetLastError(ERROR_ACCESS_DENIED);
				return FALSE;
			}
		}

		return g_CreateProcessA
			? g_CreateProcessA(lpApplicationName, lpCommandLine, lpProcessAttributes, lpThreadAttributes, bInheritHandles,
				dwCreationFlags, lpEnvironment, lpCurrentDirectory, lpStartupInfo, lpProcessInformation)
			: FALSE;
	}

	BOOL WINAPI CreateProcessWHook(LPCWSTR lpApplicationName, LPWSTR lpCommandLine, LPSECURITY_ATTRIBUTES lpProcessAttributes,
		LPSECURITY_ATTRIBUTES lpThreadAttributes, BOOL bInheritHandles, DWORD dwCreationFlags, LPVOID lpEnvironment,
		LPCWSTR lpCurrentDirectory, LPSTARTUPINFOW lpStartupInfo, LPPROCESS_INFORMATION lpProcessInformation)
	{
		if (IsInitEnabled("HookCreateProcess", 0))
		{
			char app[512] = {};
			char cmd[512] = {};
			if (lpApplicationName)
			{
				WideCharToMultiByte(CP_ACP, 0, lpApplicationName, -1, app, static_cast<int>(sizeof(app)), nullptr, nullptr);
			}
			if (lpCommandLine)
			{
				WideCharToMultiByte(CP_ACP, 0, lpCommandLine, -1, cmd, static_cast<int>(sizeof(cmd)), nullptr, nullptr);
			}

			char line[1200];
			sprintf_s(line, sizeof(line), "CreateProcessW: app=\"%s\" cmd=\"%s\"\r\n", app, cmd);
			AppendLogLine(line);

			if (ShouldBlockProcessName(app) || ShouldBlockProcessName(cmd))
			{
				AppendLogLine("CreateProcessW blocked\r\n");
				if (IsInitEnabled("FakeGGProcessSuccess", 0))
				{
					FillFakeProcessInfo(lpProcessInformation);
					return TRUE;
				}

				SetLastError(ERROR_ACCESS_DENIED);
				return FALSE;
			}
		}

		return g_CreateProcessW
			? g_CreateProcessW(lpApplicationName, lpCommandLine, lpProcessAttributes, lpThreadAttributes, bInheritHandles,
				dwCreationFlags, lpEnvironment, lpCurrentDirectory, lpStartupInfo, lpProcessInformation)
			: FALSE;
	}

	UINT WINAPI WinExecHook(LPCSTR lpCmdLine, UINT uCmdShow)
	{
		if (IsInitEnabled("HookCreateProcess", 0))
		{
			char line[768];
			sprintf_s(line, sizeof(line), "WinExec: cmd=\"%s\"\r\n", lpCmdLine ? lpCmdLine : "");
			AppendLogLine(line);

			if (ShouldBlockProcessName(lpCmdLine))
			{
				AppendLogLine("WinExec blocked\r\n");
				return IsInitEnabled("FakeGGProcessSuccess", 0) ? 33 : 0;
			}
		}

		return g_WinExec ? g_WinExec(lpCmdLine, uCmdShow) : 0;
	}

	HINSTANCE WINAPI ShellExecuteAHook(HWND hwnd, LPCSTR lpOperation, LPCSTR lpFile, LPCSTR lpParameters, LPCSTR lpDirectory, INT nShowCmd)
	{
		const int kShellExecAccessDenied = 5;
		if (IsInitEnabled("HookCreateProcess", 0))
		{
			char line[1024];
			sprintf_s(line, sizeof(line), "ShellExecuteA: file=\"%s\" params=\"%s\"\r\n",
				lpFile ? lpFile : "", lpParameters ? lpParameters : "");
			AppendLogLine(line);

			if (ShouldBlockProcessName(lpFile) || ShouldBlockProcessName(lpParameters))
			{
				AppendLogLine("ShellExecuteA blocked\r\n");
				return reinterpret_cast<HINSTANCE>(IsInitEnabled("FakeGGProcessSuccess", 0) ? 33 : kShellExecAccessDenied);
			}
		}

		return g_ShellExecuteA ? g_ShellExecuteA(hwnd, lpOperation, lpFile, lpParameters, lpDirectory, nShowCmd) : nullptr;
	}

	HINSTANCE WINAPI ShellExecuteWHook(HWND hwnd, LPCWSTR lpOperation, LPCWSTR lpFile, LPCWSTR lpParameters, LPCWSTR lpDirectory, INT nShowCmd)
	{
		const int kShellExecAccessDenied = 5;
		if (IsInitEnabled("HookCreateProcess", 0))
		{
			char file[512] = {};
			char params[512] = {};
			if (lpFile)
			{
				WideCharToMultiByte(CP_ACP, 0, lpFile, -1, file, static_cast<int>(sizeof(file)), nullptr, nullptr);
			}
			if (lpParameters)
			{
				WideCharToMultiByte(CP_ACP, 0, lpParameters, -1, params, static_cast<int>(sizeof(params)), nullptr, nullptr);
			}

			char line[1200];
			sprintf_s(line, sizeof(line), "ShellExecuteW: file=\"%s\" params=\"%s\"\r\n", file, params);
			AppendLogLine(line);

			if (ShouldBlockProcessName(file) || ShouldBlockProcessName(params))
			{
				AppendLogLine("ShellExecuteW blocked\r\n");
				return reinterpret_cast<HINSTANCE>(IsInitEnabled("FakeGGProcessSuccess", 0) ? 33 : kShellExecAccessDenied);
			}
		}

		return g_ShellExecuteW ? g_ShellExecuteW(hwnd, lpOperation, lpFile, lpParameters, lpDirectory, nShowCmd) : nullptr;
	}

	HMODULE WINAPI LoadLibraryAHook(LPCSTR lpLibFileName)
	{
		if (IsInitEnabled("HookLoadLibrary", 0))
		{
			if (lpLibFileName)
			{
				char line[512];
				sprintf_s(line, sizeof(line), "LoadLibraryA: %s\r\n", lpLibFileName);
				AppendLogLine(line);
			}

			if (ShouldBlockLibrary(lpLibFileName))
			{
				AppendLogLine("LoadLibraryA blocked\r\n");
				return nullptr;
			}
		}

		return g_LoadLibraryA ? g_LoadLibraryA(lpLibFileName) : nullptr;
	}

	HMODULE WINAPI LoadLibraryWHook(LPCWSTR lpLibFileName)
	{
		if (IsInitEnabled("HookLoadLibrary", 0))
		{
			char name[512] = {};
			if (lpLibFileName)
			{
				WideCharToMultiByte(CP_ACP, 0, lpLibFileName, -1, name, static_cast<int>(sizeof(name)), nullptr, nullptr);
				char line[768];
				sprintf_s(line, sizeof(line), "LoadLibraryW: %s\r\n", name);
				AppendLogLine(line);
			}

			if (ShouldBlockLibrary(name))
			{
				AppendLogLine("LoadLibraryW blocked\r\n");
				return nullptr;
			}
		}

		return g_LoadLibraryW ? g_LoadLibraryW(lpLibFileName) : nullptr;
	}

	void InstallLoadLibraryHooks()
	{
		if (!IsInitEnabled("HookLoadLibrary", 0))
		{
			return;
		}

		HMODULE exe = GetModuleHandle(nullptr);
		PatchImportFunction(exe, "KERNEL32.dll", "LoadLibraryA", LoadLibraryAHook, reinterpret_cast<void**>(&g_LoadLibraryA));
		PatchImportFunction(exe, "KERNEL32.dll", "LoadLibraryW", LoadLibraryWHook, reinterpret_cast<void**>(&g_LoadLibraryW));
	}

	void InstallCreateProcessHooks()
	{
		if (!IsInitEnabled("HookCreateProcess", 0))
		{
			return;
		}

		HMODULE exe = GetModuleHandle(nullptr);
		PatchImportFunction(exe, "KERNEL32.dll", "CreateProcessA", CreateProcessAHook, reinterpret_cast<void**>(&g_CreateProcessA));
		PatchImportFunction(exe, "KERNEL32.dll", "CreateProcessW", CreateProcessWHook, reinterpret_cast<void**>(&g_CreateProcessW));
		PatchImportFunction(exe, "KERNEL32.dll", "WinExec", WinExecHook, reinterpret_cast<void**>(&g_WinExec));
		PatchImportFunction(exe, "SHELL32.dll", "ShellExecuteA", ShellExecuteAHook, reinterpret_cast<void**>(&g_ShellExecuteA));
		PatchImportFunction(exe, "SHELL32.dll", "ShellExecuteW", ShellExecuteWHook, reinterpret_cast<void**>(&g_ShellExecuteW));
	}

	bool IsGGMessageText(const char* text)
	{
		if (!text)
		{
			return false;
		}

		return strstr(text, "1003-153") != nullptr
			|| strstr(text, "GameGuard") != nullptr
			|| strstr(text, "webzen") != nullptr
			|| strstr(text, "muonline") != nullptr;
	}

	int WINAPI MessageBoxAHook(HWND hWnd, LPCSTR lpText, LPCSTR lpCaption, UINT uType)
	{
		if (IsInitEnabled("HookMessageBox", 0))
		{
			if (lpText)
			{
				char line[1024];
				sprintf_s(line, sizeof(line), "MessageBoxA: %s | %s\r\n", lpCaption ? lpCaption : "", lpText);
				AppendLogLine(line);
			}

			if (IsInitEnabled("SuppressGGMessage", 0) && IsGGMessageText(lpText))
			{
				return IDOK;
			}
		}

		return g_MessageBoxA ? g_MessageBoxA(hWnd, lpText, lpCaption, uType) : IDOK;
	}

	int WINAPI MessageBoxWHook(HWND hWnd, LPCWSTR lpText, LPCWSTR lpCaption, UINT uType)
	{
		if (IsInitEnabled("HookMessageBox", 0))
		{
			char text[1024] = {};
			char caption[256] = {};
			if (lpText)
			{
				WideCharToMultiByte(CP_ACP, 0, lpText, -1, text, static_cast<int>(sizeof(text)), nullptr, nullptr);
			}
			if (lpCaption)
			{
				WideCharToMultiByte(CP_ACP, 0, lpCaption, -1, caption, static_cast<int>(sizeof(caption)), nullptr, nullptr);
			}

			if (*text)
			{
				char line[1400];
				sprintf_s(line, sizeof(line), "MessageBoxW: %s | %s\r\n", caption, text);
				AppendLogLine(line);
			}

			if (IsInitEnabled("SuppressGGMessage", 0) && IsGGMessageText(text))
			{
				return IDOK;
			}
		}

		return g_MessageBoxW ? g_MessageBoxW(hWnd, lpText, lpCaption, uType) : IDOK;
	}

	void InstallMessageBoxHooks()
	{
		if (!IsInitEnabled("HookMessageBox", 0))
		{
			return;
		}

		HMODULE exe = GetModuleHandle(nullptr);
		PatchImportFunction(exe, "USER32.dll", "MessageBoxA", MessageBoxAHook, reinterpret_cast<void**>(&g_MessageBoxA));
		PatchImportFunction(exe, "USER32.dll", "MessageBoxW", MessageBoxWHook, reinterpret_cast<void**>(&g_MessageBoxW));
	}

	void InstallEarlyHooks()
	{
		if (!IsInitEnabled("EnableDebugHooks", 0))
		{
			return;
		}

		if (g_EarlyHooksInstalled)
		{
			return;
		}

		g_EarlyHooksInstalled = true;
		InstallExitHooks();
		InstallHardExitHooks();
		InstallLoadLibraryHooks();
		InstallCreateProcessHooks();
		InstallMessageBoxHooks();
	}
}

extern "C" _declspec(dllexport) void _cdecl EntryProc()
{
	if (IsInitEnabled("DisableEntryProc", 0))
	{
		return;
	}

	if (!IsInitEnabled("SkipProtect", 0))
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
	}

	//InitConsole();

	if (!IsInitEnabled("SkipPatchs", 0))
	{
		InitPatchs();
	}

	InstallEarlyHooks();

	if (IsInitEnabled("ShowEntryProc", 0))
	{
		MessageBoxA(NULL, "EntryProc reached", "Debug", MB_OK);
	}

	if (IsInitEnabled("InitFont"))
	{
		gFont.Init();
		if (IsInitEnabled("ShowEntryProcSteps", 0))
		{
			MessageBoxA(NULL, "InitFont done", "Debug", MB_OK);
		}
	}
	if (IsInitEnabled("InitLanguage"))
	{
		gLanguage.Init();
		if (IsInitEnabled("ShowEntryProcSteps", 0))
		{
			MessageBoxA(NULL, "InitLanguage done", "Debug", MB_OK);
		}
	}
	if (IsInitEnabled("InitWideScreen"))
	{
		gCWideScreen.Load();
		if (IsInitEnabled("ShowEntryProcSteps", 0))
		{
			MessageBoxA(NULL, "InitWideScreen done", "Debug", MB_OK);
		}
	}
	if (IsInitEnabled("InitInterface"))
	{
		gInterface.Init();
		if (IsInitEnabled("ShowEntryProcSteps", 0))
		{
			MessageBoxA(NULL, "InitInterface done", "Debug", MB_OK);
		}
	}
	if (IsInitEnabled("InitMiniMap"))
	{
		gMiniMap.Init();
		if (IsInitEnabled("ShowEntryProcSteps", 0))
		{
			MessageBoxA(NULL, "InitMiniMap done", "Debug", MB_OK);
		}
	}
	if (IsInitEnabled("InitSound"))
	{
		gSound.Init();
		if (IsInitEnabled("ShowEntryProcSteps", 0))
		{
			MessageBoxA(NULL, "InitSound done", "Debug", MB_OK);
		}
	}
	if (IsInitEnabled("InitPrintPlayer"))
	{
		gPrintPlayer.Init();
		if (IsInitEnabled("ShowEntryProcSteps", 0))
		{
			MessageBoxA(NULL, "InitPrintPlayer done", "Debug", MB_OK);
		}
	}
	if (IsInitEnabled("InitGuildLogo"))
	{
		gGuildLogo.Init();
		if (IsInitEnabled("ShowEntryProcSteps", 0))
		{
			MessageBoxA(NULL, "InitGuildLogo done", "Debug", MB_OK);
		}
	}

	if (IsInitEnabled("InitResolution"))
	{
		InitResolution();
		if (IsInitEnabled("ShowEntryProcSteps", 0))
		{
			MessageBoxA(NULL, "InitResolution done", "Debug", MB_OK);
		}
	}

	if (IsInitEnabled("ShowEntryProcSteps", 0))
	{
		MessageBoxA(NULL, "EntryProc done", "Debug", MB_OK);
	}
}

BOOL APIENTRY DllMain(HMODULE hModule,DWORD ul_reason_for_call,LPVOID lpReserved)
{
	switch(ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
		{
			hins = (HINSTANCE)hModule;

			InstallEarlyHooks();

			if (IsInitEnabled("ShowDllLoad", 0))
			{
				MessageBoxA(NULL, "Main.dll loaded", "Debug", MB_OK);
			}

			if (IsInitEnabled("InitWindowMode", 1))
			{
				gWindow.WindowModeLoad(hins);
			}

			if (IsInitEnabled("InitController", 1))
			{
				gController.Load(hins);
			}

			if (IsInitEnabled("InitTrayMode", 1))
			{
				gTrayMode.Init(hins);
			}

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
