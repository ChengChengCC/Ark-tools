#pragma once
#include "stdafx.h"

typedef struct _SSSDT_INFO
{
	ULONG_PTR	Id;
	ULONG_PTR	CurAddr;
	ULONG_PTR	OriAddr;
	CHAR	    szFuncName[100];
}SSSDT_INFO, *PSSSDT_INFO;


VOID HsInitSSSDTList(CListCtrl *m_ListCtrl);

VOID HsLoadSSSDTList(CListCtrl *m_ListCtrl);

VOID HsQuerySSSDTList(CListCtrl *m_ListCtrl);

BOOL EnumSSSDT(SSSDT_INFO* si, CListCtrl* m_ListCtrl);

VOID SendIoCodeSSSDT(DWORD dwFuncIndex,PULONG_PTR SSSDTCurAddr);

ULONG_PTR GetOriSSSDTAddress(ULONG_PTR Index);

VOID SendIoCodeWin32kServiceTable();




