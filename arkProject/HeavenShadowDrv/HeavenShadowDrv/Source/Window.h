#pragma once 



#if DBG
#define dprintf DbgPrint
#else
#define dprintf
#endif

#include <ntifs.h>
#include <WINDEF.H>
#include "GetSSSDTFuncAddress.h"
#include "common.h"
#include "Process.h"



typedef struct _WND_INFO_
{
	HWND  hWnd;
	ULONG uPid;
	ULONG uTid;
}WND_INFO, *PWND_INFO;

typedef struct _ALL_WNDS_
{
	ULONG nCnt;
	WND_INFO WndInfo[1];
}ALL_WNDS, *PALL_WNDS;



NTSTATUS 
	HsEnumProcessWindow(
	PVOID InBuffer, 
	ULONG_PTR InSize, 
	PVOID OutBuffer, 
	ULONG_PTR OutLen);
