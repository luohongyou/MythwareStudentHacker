// dllmain.cpp : 定义 DLL 应用程序的入口点。

// 版权声明：
// Copyright (C) 2013-2022 ETO.                              All Rights Reserved.
// Copyright (C) 2018-2022 FreeSTD Inc.                      All Rights Reserved.
// Copyright (C) 2018-2022 Chengdu Zuosi Co.,Ltd.            All Rights Reserved.
// Copyright (C) 2020-2022 Coral Studio.                     All Rights Reserved.
// Copyright (C) 2022      North A516 Treaty Organization.   All Rights Reserved.
// Copyright (C) 2022      Sunny Orange Electronics Co.,Ltd. All Rights Reserved.

#include "pch.h"
#include <assert.h>
#include <tchar.h>
#include <stdlib.h>
#include "detours.h"
#pragma comment(lib , "detours.lib")

static HHOOK(WINAPI* poldHook)(
	_In_ int idHook,
	_In_ HOOKPROC lpfn,
	_In_opt_ HINSTANCE hmod,
	_In_ DWORD dwThreadId)
	= SetWindowsHookEx;
HHOOK WINAPI mySetWindowsHookEx(
	_In_ int idHook,
	_In_ HOOKPROC lpfn,
	_In_opt_ HINSTANCE hmod,
	_In_ DWORD dwThreadId)
{
	return 0;
};

int CursorCount = 0;
static int(WINAPI* poldHook2)(
	_In_ BOOL bShow) = ShowCursor;
int WINAPI myShowCursor(
	_In_ BOOL bShow)
{
	if (bShow)
		CursorCount++;
	else
		CursorCount--;
	return CursorCount;
}

static DWORD(WINAPI* poldHook3)(
	) = GetCurrentThreadId;
DWORD WINAPI myGetCurrentThreadId()
{
	return 0;
};
static HANDLE(WINAPI* poldHook3_1)(
	) = GetCurrentThread;
DWORD WINAPI myGetCurrentThread()
{
	return 0;
};
static DWORD(WINAPI* poldHook3_2)(
	) = GetCurrentProcessId;
DWORD WINAPI myGetCurrentProcessId()
{
	return 0;
};
static HANDLE(WINAPI* poldHook3_3)(
	) = GetCurrentProcess;
DWORD WINAPI myGetCurrentProcess()
{
	return 0;
};

static BOOL(WINAPI* poldHook4)(
	_In_ DWORD idAttach,
	_In_ DWORD idAttachTo,
	_In_ BOOL fAttach) = AttachThreadInput;
BOOL WINAPI myAttachThreadInput(
	_In_ DWORD idAttach,
	_In_ DWORD idAttachTo,
	_In_ BOOL fAttach)
{
	return 0;
}

static DWORD(WINAPI* poldHook5)(
	_In_ HWND hWnd,
	_Out_opt_ LPDWORD lpdwProcessId) = GetWindowThreadProcessId;
DWORD WINAPI myGetWindowThreadProcessId(
	_In_ HWND hWnd,
	_Out_opt_ LPDWORD lpdwProcessId)
{
	return 0;
};

static HWND(WINAPI* poldHook6)(
	) = GetForegroundWindow;
HWND WINAPI myGetForegroundWindow()
{
	return 0;
}
static BOOL(WINAPI* poldHook6_1)(
	_In_ HWND hWnd) = SetForegroundWindow;
BOOL WINAPI mySetForegroundWindow(
	_In_ HWND hWnd)
{
	return 0;
}
static HWND(WINAPI* poldHook6_2)(
	_In_ HWND hWnd) = SetActiveWindow;
HWND WINAPI mySetActiveWindow(
	_In_ HWND hWnd)
{
	return 0;
}
static HWND(WINAPI* poldHook6_3)(
	) = GetActiveWindow;
HWND WINAPI myGetActiveWindow()
{
	return 0;
}

static BOOL(WINAPI* poldHook7)(
	_In_ HWND hWnd,
	_In_opt_ HWND hWndInsertAfter,
	_In_ int X,
	_In_ int Y,
	_In_ int cx,
	_In_ int cy,
	_In_ UINT uFlags) = SetWindowPos;
BOOL WINAPI mySetWindowPos(
	_In_ HWND hWnd,
	_In_opt_ HWND hWndInsertAfter,
	_In_ int X,
	_In_ int Y,
	_In_ int cx,
	_In_ int cy,
	_In_ UINT uFlags)
{
	hWndInsertAfter = HWND_BOTTOM;
	DetourDetach((void**)&poldHook7, mySetWindowPos);
	SetWindowPos(hWnd, hWndInsertAfter, X, Y, cx, cy, uFlags);
	DetourAttach((void**)&poldHook7, mySetWindowPos);
	return 1;
}

static HBITMAP(WINAPI* poldHook8)(
	_In_ HDC hdc, 
	_In_ int cx, 
	_In_ int cy) = CreateCompatibleBitmap;
HBITMAP WINAPI myCreateCompatibleBitmap(
	_In_ HDC hdc, 
	_In_ int cx, 
	_In_ int cy)
{
	return NULL;
}

static BOOL(WINAPI* poldHook9)(
	_In_ HDC hdc,
	_In_ int x, _In_ int y,
	_In_ int cx, _In_ int cy,
	_In_opt_ HDC hdcSrc,
	_In_ int x1, _In_ int y1,
	_In_ DWORD rop) = BitBlt;
BOOL WINAPI myBitBlt(
	_In_ HDC hdc,
	_In_ int x, _In_ int y,
	_In_ int cx, _In_ int cy,
	_In_opt_ HDC hdcSrc,
	_In_ int x1, _In_ int y1,
	_In_ DWORD rop)
{
	return 0;
}

static VOID(WINAPI* poldHook10)(
	_In_ DWORD dwFlags,
	_In_ DWORD dx,
	_In_ DWORD dy,
	_In_ DWORD dwData,
	_In_ ULONG_PTR dwExtraInfo) = mouse_event;
VOID WINAPI mymouse_event(
	_In_ DWORD dwFlags,
	_In_ DWORD dx,
	_In_ DWORD dy,
	_In_ DWORD dwData,
	_In_ ULONG_PTR dwExtraInfo)
{
	return;
}
static UINT(WINAPI* poldHook10_1)(
	_In_ UINT cInputs,
	_In_reads_(cInputs) LPINPUT pInputs,
	_In_ int cbSize) = SendInput;
UINT WINAPI mySendInput(
	_In_ UINT cInputs,
	_In_reads_(cInputs) LPINPUT pInputs,
	_In_ int cbSize)
{
	return cInputs;
}
static BOOL(WINAPI* poldHook10_2)(
	_In_ int X,
	_In_ int Y) = SetCursorPos;
BOOL WINAPI mySetCursorPos(
	_In_ int X,
	_In_ int Y)
{
	return 1;
}

void Hook()
{
	DetourRestoreAfterWith();
	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	DetourAttach((void**)&poldHook, mySetWindowsHookEx);
	DetourAttach((void**)&poldHook2, myShowCursor);
	// DetourAttach((void**)&poldHook3, myGetCurrentThreadId);
	// DetourAttach((void**)&poldHook3_1, myGetCurrentThread);
	// DetourAttach((void**)&poldHook3_2, myGetCurrentProcessId);
	// DetourAttach((void**)&poldHook3_3, myGetCurrentProcess);
	// DetourAttach((void**)&poldHook4, myAttachThreadInput);
	DetourAttach((void**)&poldHook5, myGetWindowThreadProcessId);
	// DetourAttach((void**)&poldHook6, myGetForegroundWindow);
	// DetourAttach((void**)&poldHook6_1, mySetForegroundWindow);
	// DetourAttach((void**)&poldHook6_2, mySetActiveWindow);
	// DetourAttach((void**)&poldHook6_3, myGetActiveWindow);
	// DetourAttach((void**)&poldHook7, mySetWindowPos);
	// DetourAttach((void**)&poldHook8, myCreateCompatibleBitmap);
	// DetourAttach((void**)&poldHook9, myBitBlt);
	// DetourAttach((void**)&poldHook10, mymouse_event);
	DetourAttach((void**)&poldHook10_1, mySendInput);
	// DetourAttach((void**)&poldHook10_2, mySetCursorPos);
	DetourTransactionCommit();
}
void UnHook()
{
	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	DetourDetach((void**)&poldHook, mySetWindowsHookEx);
	DetourDetach((void**)&poldHook2, myShowCursor);
	// DetourDetach((void**)&poldHook3, myGetCurrentThreadId);
	// DetourDetach((void**)&poldHook3_1, myGetCurrentThread);
	// DetourDetach((void**)&poldHook3_2, myGetCurrentProcessId);
	// DetourDetach((void**)&poldHook3_3, myGetCurrentProcess);
	// DetourDetach((void**)&poldHook4, myAttachThreadInput);
	DetourDetach((void**)&poldHook5, myGetWindowThreadProcessId);
	// DetourDetach((void**)&poldHook6, myGetForegroundWindow);
	// DetourDetach((void**)&poldHook6_1, mySetForegroundWindow);
	// DetourDetach((void**)&poldHook6_2, mySetActiveWindow);
	// DetourDetach((void**)&poldHook6_3, myGetActiveWindow);
	// DetourDetach((void**)&poldHook7, mySetWindowPos);
	// DetourDetach((void**)&poldHook8, myCreateCompatibleBitmap);
	// DetourDetach((void**)&poldHook9, myBitBlt);
	// DetourDetach((void**)&poldHook10, mymouse_event);
	DetourDetach((void**)&poldHook10_1, mySendInput);
	// DetourDetach((void**)&poldHook10_2, mySetCursorPos);
	DetourTransactionCommit();
}

BOOL APIENTRY DllMain(HANDLE hModule,
                      DWORD ul_reason_for_call,
                      LPVOID lpReserved
                     )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		Hook();
		break;
	case DLL_PROCESS_DETACH:
		UnHook();
		break;
	}
	return TRUE;
}