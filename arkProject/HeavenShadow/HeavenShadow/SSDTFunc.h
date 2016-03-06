#pragma once
#include "stdafx.h"

typedef struct _SSDT_INFO
{
	ULONG_PTR	Id;
	ULONG_PTR	CurAddr;
	ULONG_PTR	OriAddr;
	CHAR	    szFuncName[80];
}SSDT_INFO, *PSSDT_INFO;

typedef struct _RESUME_DATA_ 
{
	ULONG ulIndex;
	ULONG_PTR ulFuncAddress;
}RESUME_DATA,*PRESUME_DATA;

VOID HsInitSSDTList(CListCtrl *m_ListCtrl);

VOID HsLoadSSDTList(CListCtrl *m_ListCtrl);

VOID HsQuerySSDTList(CListCtrl *m_ListCtrl);

BOOL GetNtosImageBase();

BOOL GetNtosNameAndBase();

BOOL GetTempNtosName();

BOOL GetKiServiceTable();


BOOL InitDataOfSSDT();

BOOL EnumSSDT(SSDT_INFO* si);

ULONG_PTR HsGetFunctionOriginalAddress(ULONG_PTR dwIndex);

BOOL SendIoCodeSSDT(DWORD dwFuncIndex,PULONG_PTR SSDTCurAddr);

BOOL EnumDriver();

CString GetDriverPath(ULONG_PTR Address);

void HsResumeSSDTHook(CListCtrl* m_ListCtrl);