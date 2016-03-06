#pragma once
#include "stdafx.h"

#include "MyList.h"

#include <afxtempl.h>
#include <vector>
#include <Strsafe.h>

#pragma comment(lib,"Version.lib")
using namespace std;

typedef struct _DRIVER_INFO_
{
	ULONG_PTR LodeOrder;
	ULONG_PTR Base;
	ULONG_PTR Size;
	ULONG_PTR DriverObject;
	ULONG_PTR DirverStartAddress;
	WCHAR wzDriverPath[MAX_PATH];
	WCHAR wzKeyName[MAX_PATH];
}DRIVER_INFO, *PDRIVER_INFO;

typedef struct _ALL_DRIVERS_
{
	ULONG_PTR ulCount;
	DRIVER_INFO Drivers[1];
}ALL_DRIVERS, *PALL_DRIVERS;



//////////////////////////////////////////////////////////////////////////

void HsInitModuleList(CListCtrl *m_ListCtrl);

DWORD WINAPI HsQueryModuleFunction(CListCtrl *m_ListCtrl);

void HsQueryModuleList(CListCtrl *m_ListCtrl);

void FixDriverPath(PDRIVER_INFO DriverInfor);

void AddModuleFileIcon(WCHAR* ModulePath);

void HsRemoveDriverModule(CListCtrl* m_ListCtrl);

BOOL SendIoControlUnloadDriver();




