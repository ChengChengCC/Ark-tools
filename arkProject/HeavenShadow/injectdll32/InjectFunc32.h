#pragma once
#include "stdafx.h"
#include <atlstr.h>

typedef enum _WIN_VERSION
{
	WindowsNT,
	Windows2000,
	WindowsXP,
	Windows2003,
	WindowsVista,
	Windows7,
	Windows8,
	Windows8_1,
	Windows10,
	WinUnknown
}WIN_VERSION;

DWORD HsRemoteThreadInjectDll32(WCHAR* ProcessInfo);
BOOL __stdcall HsIs32BitFile(const WCHAR * pwszFullPath);

WIN_VERSION  GetWindowsVersion();
BOOL HsInjectUpPrivilige();	//XP
BOOL HsInjectDllByRemoteThreadWinXP(const TCHAR* wzDllFile, ULONG_PTR ProcessId);
BOOL HsInjectDllByRemoteThreadWin7(const TCHAR* wzDllFile, ULONG_PTR ProcessId, BOOL Is32Bit);
BOOL HsInjectDll(BOOL Is32Bit, CString* strDllPath, ULONG_PTR ProcessID);
BOOL HsDebugPrivilege(const WCHAR *pName, BOOL bEnable);

void PopFileInitialize (HWND hwnd);
BOOL PopFileOpenDlg (HWND hwnd, PTSTR pstrFileName, PTSTR pstrTitleName);