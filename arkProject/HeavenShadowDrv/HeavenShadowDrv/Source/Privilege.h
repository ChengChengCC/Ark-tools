#pragma once 



#if DBG
#define dprintf DbgPrint
#else
#define dprintf
#endif

#include <ntifs.h>
#include "common.h"

#include "Process.h"


typedef struct _PRIVILEGE_DATA_ 
{
	ULONG_PTR  ProcessID;
	TOKEN_PRIVILEGES TokenPrivileges;   //宏
}PRIVILEGEDATA,*PPRIVILEGEDATA;


typedef 
ULONG_PTR (*pfnObGetObjectType)(PVOID pObject);


NTSTATUS HsEnumProcessPrivilege(WCHAR* szOutBuffer,ULONG_PTR ProcessID,ULONG_PTR ulOutSize);


ULONG_PTR HsKeGetObjectType(PVOID Object);


//修改权限的值
NTSTATUS HsAdjustProcessTokenPrivileges(PPRIVILEGEDATA PrivilegeData, int* bFeedBack);