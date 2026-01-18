#include "stdafx.h"
#include "Camera.h"
#include "Controller.h"
#include "EventTimer.h"
#include "MathLib.h"
#include "MiniMap.h"
#include "MoveList.h"
#include "Offset.h"
#include "resource.h"
#include "Util.h"
#include "TrayMode.h"

Controller gController;

void Controller::Load(HINSTANCE hins)
{
	this->Instance = hins;

	if (!this->MouseHook)
	{
		this->MouseHook = SetWindowsHookEx(WH_MOUSE, this->Mouse, this->Instance, GetCurrentThreadId());

		if (!this->MouseHook)
		{
			return;
		}
	}

	if (!this->KeyboardHook)
	{
		this->KeyboardHook = SetWindowsHookEx(WH_KEYBOARD, this->Keyboard, this->Instance, GetCurrentThreadId());

		if (!this->KeyboardHook)
		{
			return;
		}
	}

	SetDword(0x00552398, (DWORD)&GetAsyncKeyStateHook);

	SetCompleteHook(0xE8, 0x005259DD, &Controller::CheckKeyboardKeys);
}

LRESULT Controller::Mouse(int nCode, WPARAM wParam, LPARAM lParam)
{
	if (nCode == HC_ACTION)
	{
		MOUSEHOOKSTRUCTEX* HookStruct = (MOUSEHOOKSTRUCTEX*)lParam;

		switch (wParam)
		{
			case WM_MOUSEMOVE:
			{
				if (GetForegroundWindow() == g_hWnd)
				{
					gCamera.Move(HookStruct);
				}

				break;
			}

			case WM_MBUTTONDOWN:
			{
				if (GetForegroundWindow() == g_hWnd)
				{
					gCamera.SetIsMove(1);

					gCamera.SetCursorX(HookStruct->pt.x);

					gCamera.SetCursorY(HookStruct->pt.y);
				}

				break;
			}

			case WM_MBUTTONUP:
			{
				if (GetForegroundWindow() == g_hWnd)
				{
					gCamera.SetIsMove(0);
				}

				break;
			}

			case WM_MOUSEWHEEL:
			{
				if (GetForegroundWindow() == g_hWnd)
				{
					gCamera.Zoom(HookStruct);
				}

				break;
			}

			default:
			{
				break;
			}
		}
	}

	return CallNextHookEx(gController.MouseHook, nCode, wParam, lParam);
}

LRESULT Controller::Keyboard(int nCode, WPARAM wParam, LPARAM lParam)
{
	if (nCode == HC_ACTION)
	{
		if (((DWORD)lParam & (1 << 30)) != 0 && ((DWORD)lParam & (1 << 31)) != 0)
		{
			switch (wParam)
			{
				case VK_F10:
				{
					if (GetForegroundWindow() == g_hWnd)
					{
						gCamera.Toggle();
					}

					break;
				}

				case VK_F11:
				{
					if (GetForegroundWindow() == g_hWnd)
					{
						gCamera.Restore();
					}

					break;
				}

				case VK_F12:
				{
					if (GetForegroundWindow() == g_hWnd)
					{
						gTrayMode.Toggle();
					}

					break;
				}

				default:
				{
					break;
				}
			}
		}
	}

	return CallNextHookEx(gController.KeyboardHook, nCode, wParam, lParam);
}

SHORT WINAPI Controller::GetAsyncKeyStateHook(int key)
{
	if (GetForegroundWindow() != g_hWnd)
	{
		return 0;
	}

	return GetAsyncKeyState(key);
}

void Controller::CheckKeyboardKeys()
{
	if ((GetAsyncKeyStateCall('M') >> 0x0F)) // M Pressed
	{
		if (!KeyState['M'])
		{
			KeyState['M'] = 1;

			gMoveList.Toggle();
		}
	}
	else
	{
		KeyState['M'] = 0;
	}

	if ((GetAsyncKeyStateCall('H') >> 0x0F)) // H Pressed
	{
		if (!KeyState['H'])
		{
			KeyState['H'] = 1;

			gEventTimer.Toggle();
		}
	}
	else
	{
		KeyState['H'] = 0;
	}

	if ((GetAsyncKeyStateCall(VK_TAB) >> 0x0F)) // Tab Pressed
	{
		if (!KeyState[VK_TAB])
		{
			KeyState[VK_TAB] = 1;

			gMiniMap.Toggle();
		}
	}
	else
	{
		KeyState[VK_TAB] = 0;
	}

	if ((GetAsyncKeyStateCall(VK_SPACE) >> 0x0F)) // Space Pressed
	{
		if (!KeyState[VK_SPACE])
		{
			KeyState[VK_SPACE] = 1;

			if (!InputEnable && !MouseOnWindow)
			{
				for (int i = 0; i < 1000; ++i)
				{
					DWORD ItemObject = (0x07E127F8 + 516 * i) + 72; // Items[i].Object

					if (*(BYTE*)(ItemObject + 0) && *(BYTE*)(ItemObject + 352)) // Live + Visible
					{
						vec3_t vDir;

						VectorSubtract((float*)(ItemObject + 16), (float*)(Hero + 16), vDir);

						if (sqrt(vDir[0] * vDir[0] + vDir[1] * vDir[1] + vDir[2] * vDir[2]) <= 300.0f)
						{
							*(BYTE*)(Hero + 749) = 1; // MovementType = MOVEMENT_GET

							ItemKey = i;

							Action(Hero, ItemObject);

							*(BYTE*)(Hero + 749) = 0; // MovementType = MOVEMENT_MOVE

							break;
						}
					}
				}
			}
		}
	}
	else
	{
		KeyState[VK_SPACE] = 0;
	}

	((void(__cdecl*)())0x004C04A0)(); // Original function
}
