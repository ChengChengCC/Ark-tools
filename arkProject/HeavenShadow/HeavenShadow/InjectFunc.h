#pragma once
#include "stdafx.h"





BOOL HsInjectDll(BOOL Is32Bit, CString* strDllPath, ULONG_PTR ProcessID);


BOOL HsInjectDllByRemoteThreadWin7(const TCHAR* wzDllFile, ULONG_PTR ProcessId, BOOL Is32Bit);





BOOL HsInjectDllByRemoteThreadWinXP(const TCHAR* wzDllFile, ULONG_PTR ProcessId);

BOOL HsInjectUpPrivilige();

DWORD WINAPI HsRemoteThreadInjectDll(CListCtrl *m_ListCtrl);