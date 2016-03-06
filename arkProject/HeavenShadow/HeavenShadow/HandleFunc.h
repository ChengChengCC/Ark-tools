#pragma once
#include "stdafx.h"
#include <vector>
#include <afxtempl.h>
#include "MyList.h"
using namespace std;

#include "ProcessFunc.h"



#define MAX_OBJECT_NAME  50
#define MAX_PATH2        500


typedef struct _HANDLE_INFO_
{
	ULONG_PTR Handle;
	ULONG_PTR Object;
	ULONG_PTR ReferenceCount;
	WCHAR ObjectName[MAX_OBJECT_NAME];
	WCHAR HandleName[MAX_PATH2];
}HANDLE_INFO, *PHANDLE_INFO;

typedef struct _PROCESS_HANDLES_
{
	ULONG_PTR ulCount;
	HANDLE_INFO Handles[1];
}ALL_HANDLES, *PALL_HANDLES;




VOID HsInitProcessHandleList(CMyList *m_ListCtrl);

VOID HsQueryProcessHandle(CMyList *m_ListCtrl);
