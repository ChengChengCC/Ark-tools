#ifndef _GETSSSDTFUNCADDRESS_H 
#define _GETSSSDTFUNCADDRESS_H

#include <ntifs.h>
#include <windef.h>

#endif

typedef struct _SYSTEM_SERVICE_TABLE_SSSDT64{
	PVOID  		ServiceTableBase; 
	PVOID  		ServiceCounterTableBase; 
	ULONG64  	NumberOfServices; 
	PVOID  		ParamTableBase; 
} SYSTEM_SERVICE_TABLE_SSSDT64, *PSYSTEM_SERVICE_TABLE_SSSDT64;

typedef struct _SYSTEM_SERVICE_TABLE_SSSDT32 {
	PVOID   ServiceTableBase;
	PVOID   ServiceCounterTableBase;
	ULONG32 NumberOfServices;
	PVOID   ParamTableBase;
} SYSTEM_SERVICE_TABLE_SSSDT32, *PSYSTEM_SERVICE_TABLE_SSSDT32;




ULONG_PTR GetKeServiceDescriptorTableShadow32WinXP();
ULONG_PTR GetKeServiceDescriptorTableShadow64Win7();
ULONG_PTR GetSSSDTFunctionAddress32WinXP(ULONG_PTR ulIndex,ULONG_PTR SSSDTDescriptor);
ULONG_PTR GetSSSDTFunctionAddress64Win7(ULONG_PTR ulIndex,ULONG_PTR SSSDTDescriptor);
ULONG_PTR GetSSSDTApi();



typedef
	ULONG_PTR
	(*pfnNtUserQueryWindow)(
	HWND hWnd, 
	ULONG_PTR Index);


typedef
	NTSTATUS
	(*pfnNtUserBuildHwndList)(
	HDESK hDesktop,
	HWND hwndParent,
	BOOLEAN bChildren,
	ULONG dwThreadId,
	ULONG lParam,
	HWND* pWnd,
	ULONG* pBufSize);
