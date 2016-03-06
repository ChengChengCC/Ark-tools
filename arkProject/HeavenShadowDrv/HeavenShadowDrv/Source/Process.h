/**************************************************************************************
* AUTHOR : HeavenShadow
* DATE   : 2014-10-28
* MODULE : Process.h
*
* Command: 
*	进程大功能的主文件
*
* Description:
*	与进程相关的所有功能集合文件
*
****************************************************************************************
* Copyright (C) 2015 HeavenShadow.
****************************************************************************************/

#pragma once 


#if DBG
#define dprintf DbgPrint
#else
#define dprintf
#endif

#include <ntifs.h>
#include "common.h"



typedef struct HS_PROCESSINFO
{
	WCHAR       Name[100];
	WCHAR       Path[260];
	WCHAR       CompanyName[100];
	INT         UserAccess;
	ULONG_PTR   Pid;
	ULONG_PTR   PPid;
	ULONG_PTR   Eprocess;
	LONGLONG    CreateTime;
}HSPROCESSINFO, *PHSPROCESSINFO;



typedef struct _CONTROL_AREA64
{
	PVOID64 Segment;
	PVOID64 p1;
	PVOID64 p2;
	ULONG64 NumberOfSectionReferences;
	ULONG64 NumberOfPfnReferences;
	ULONG64 NumberOfMappedViews;
	ULONG64 NumberOfUserReferences;
	union
	{
		ULONG LongFlags;
		ULONG Flags;
	} u;
	PVOID64 FilePointer;
} CONTROL_AREA64, *PCONTROL_AREA64;



typedef struct _CONTROL_AREA
{
	PVOID Segment;
	LIST_ENTRY DereferenceList;
	ULONG NumberOfSectionReferences;
	ULONG NumberOfPfnReferences;
	ULONG NumberOfMappedViews;
	ULONG NumberOfSystemCacheViews;
	ULONG NumberOfUserReferences;
	union
	{
		ULONG LongFlags;
		ULONG Flags;
	} u;
	PFILE_OBJECT FilePointer;
} CONTROL_AREA, *PCONTROL_AREA;




typedef struct _SEGMENT64
{
	PVOID64 ControlArea;
	ULONG TotalNumberOfPtes;
	ULONG NonExtendedPtes;
	ULONG Spare0;
}SEGMENT64,*PSEGMENT64;


typedef struct _SEGMENT
{
	struct _CONTROL_AREA *ControlArea;
	ULONG TotalNumberOfPtes;
	ULONG NonExtendedPtes;
	ULONG Spare0;
} SEGMENT, *PSEGMENT;




typedef struct _SECTION_OBJECT
{
	PVOID StartingVa;
	PVOID EndingVa;
	PVOID Parent;
	PVOID LeftChild;
	PVOID RightChild;
	PSEGMENT Segment;
} SECTION_OBJECT, *PSECTION_OBJECT;


typedef struct _SECTION_OBJECT64
{
	PVOID64 StartingVa;
	PVOID64 EndingVa;
	PVOID64 Parent;
	PVOID64 LeftChild;
	PVOID64 RightChild;
	PVOID64 Segment;
} SECTION_OBJECT64, *PSECTION_OBJECT64;


//////////////////////////////////////////////////////////////////////////
//句柄结构
typedef struct _SYSTEM_HANDLE_TABLE_ENTRY_INFO{
	USHORT UniqueProcessId;
	USHORT CreatorBackTraceIndex;
	UCHAR ObjectTypeIndex;
	UCHAR HandleAttributes;
	USHORT HandleValue;
	PVOID Object;
	ULONG GrantedAccess;
} SYSTEM_HANDLE_TABLE_ENTRY_INFO, *PSYSTEM_HANDLE_TABLE_ENTRY_INFO;

typedef struct _SYSTEM_HANDLE_INFORMATION{
	ULONG NumberOfHandles;
	SYSTEM_HANDLE_TABLE_ENTRY_INFO Handles[1];
} SYSTEM_HANDLE_INFORMATION, *PSYSTEM_HANDLE_INFORMATION;


typedef struct _HS_HANDLE_INFO{
	UCHAR  ObjectTypeIndex;
	USHORT HandleValue;
	WCHAR  HandleObjectName[100];
	WCHAR  HandleTypeName[100];
	PVOID  Object;
	ULONG_PTR HandleCount;
}HSHANDLEINFO, *PHSHANDLEINFO;

typedef struct _HS_HANDLE {
	ULONG NumberOfHandles;
	HSHANDLEINFO Handles[1];
}HSHANDLE, *PHSHANDLE;


//////////////////////////////////////////////////////////////////////////


typedef struct _MODULE_INFO_
{
	ULONG_PTR Base;
	ULONG_PTR Size;
	WCHAR Path[260]; 
}MODULE_INFO, *PMODULE_INFO;

typedef struct _ALL_MODULES_
{
	ULONG_PTR   ulCount;
	MODULE_INFO Modules[1];
}ALL_MODULES, *PALL_MODULES;

typedef struct _PEB_LDR_DATA32 
{
	ULONG Length;
	BOOLEAN Initialized;
	HANDLE SsHandle;
	LIST_ENTRY InLoadOrderModuleList;
	LIST_ENTRY InMemoryOrderModuleList;
	LIST_ENTRY InInitializationOrderModuleList;
	PVOID EntryInProgress;
} PEB_LDR_DATA32, *PPEB_LDR_DATA32;

typedef struct _PEB_LDR_DATA64 
{
	ULONG Length;
	BOOLEAN Initialized;
	HANDLE SsHandle;
	LIST_ENTRY64 InLoadOrderModuleList;
	LIST_ENTRY64 InMemoryOrderModuleList;
	LIST_ENTRY64 InInitializationOrderModuleList;
	PVOID EntryInProgress;
	BOOLEAN  ShutdownInProgress;
	PVOID    ShutdownThreadId;
} PEB_LDR_DATA64, *PPEB_LDR_DATA64;


typedef struct _LDR_DATA_TABLE_ENTRY64
{
	LIST_ENTRY64	InLoadOrderLinks;
	LIST_ENTRY64	InMemoryOrderLinks;
	LIST_ENTRY64	InInitializationOrderLinks;
	PVOID			DllBase;
	PVOID			EntryPoint;
	ULONG			SizeOfImage;
	UNICODE_STRING	FullDllName;
	UNICODE_STRING 	BaseDllName;
	ULONG			Flags;
	USHORT			LoadCount;
	USHORT			TlsIndex;
	PVOID			SectionPointer;
	ULONG			CheckSum;
	PVOID			LoadedImports;
	PVOID			EntryPointActivationContext;
	PVOID			PatchInformation;
	LIST_ENTRY64	ForwarderLinks;
	LIST_ENTRY64	ServiceTagLinks;
	LIST_ENTRY64	StaticLinks;
	PVOID			ContextInformation;
	ULONG64			OriginalBase;
	LARGE_INTEGER	LoadTime;
} LDR_DATA_TABLE_ENTRY64, *PLDR_DATA_TABLE_ENTRY64;



typedef struct _LDR_DATA_TABLE_ENTRY {
	LIST_ENTRY InLoadOrderLinks;
	LIST_ENTRY InMemoryOrderLinks;
	LIST_ENTRY InInitializationOrderLinks;
	PVOID DllBase;
	PVOID EntryPoint;
	ULONG SizeOfImage;
	UNICODE_STRING FullDllName;
	UNICODE_STRING BaseDllName;
	ULONG Flags;
	USHORT LoadCount;
	USHORT TlsIndex;
	union {
		LIST_ENTRY HashLinks;
		struct {
			PVOID SectionPointer;
			ULONG CheckSum;
		};
	};
	union {
		struct {
			ULONG TimeDateStamp;
		};
		struct {
			PVOID LoadedImports;
		};
	};
} LDR_DATA_TABLE_ENTRY,*PLDR_DATA_TABLE_ENTRY;

//////////////////////////////////////////////////////////////////////////



NTKERNELAPI VOID NTAPI KeAttachProcess(PEPROCESS Process);
NTKERNELAPI VOID NTAPI KeDetachProcess();




/***************************************************************************************
* NAME:			HsDispatchControlForProcess
*
* DESCRIPTION:	自定义的进程大功能DispatchControl判断函数，由主DispatchControl调用
*					
* PARAMETERS:		IrpSp				IN		IrpSp栈的地址
*					OutputBuffer		IN		应用层UserBuffer的地址
*                   ulRet               Out     返回长度的值的地址
*
* RETURNS:		NTSTATUS
*
* NOTES:		在这个函数中，只传入上述两个参数，其他参数不传入。
				返回结果到主DIspatchControl函数中。
****************************************************************************************/
NTSTATUS HsDispatchControlForProcess(PIO_STACK_LOCATION  IrpSp, PVOID OutputBuffer, ULONG_PTR* ulRet);


/***************************************************************************************
* NAME:			HsEnumSystemProcessList
*
* DESCRIPTION:	查询系统进程列表的例程。通过该函数将进城信息List写入应用层UserBuffer中。
*					
* PARAMETERS:		OutputBuffer		IN		应用层UserBuffer的地址
*
* RETURNS:		NTSTATUS
****************************************************************************************/
NTSTATUS HsEnumSystemProcessList(ULONG_PTR ulBasePid, PHSPROCESSINFO plProcessList, ULONG_PTR* ulRet);





VOID HsInitProcessGlobalVariable();


BOOLEAN HsGetProcessPathBySectionObject(ULONG_PTR ulProcessID,WCHAR* wzProcessPath);
BOOLEAN HsGetPathByFileObject(PFILE_OBJECT FileObject, WCHAR* wzPath);


BOOLEAN HsIsRealProcess(PEPROCESS EProcess);
BOOLEAN HsIsProcessDie(PEPROCESS EProcess);

ULONG_PTR HsGetParentProcessIdByEProcess(PEPROCESS EProcess);

PEPROCESS HsGetIdleEProcess();

NTSTATUS HsGetSystemProcessCount(ULONG_PTR* ulRetCount);



//内存清零法结束进程
NTSTATUS HsKillProcessByZeroMemory(ULONG_PTR ProcessID, int* bFeedBack);


//获取进程创建时间
BOOLEAN
HsGetProcessCreateTime(ULONG_PTR ProcessID,LONGLONG* OutputBuffer);