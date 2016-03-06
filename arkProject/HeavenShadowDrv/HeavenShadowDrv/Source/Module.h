/**************************************************************************************
* AUTHOR : HeavenShadow
* DATE   : 2014-10-28
* MODULE : Module.h
*
* Command: 
*	模块大功能的主文件
*
* Description:
*	与模块相关的所有功能集合文件
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


#define MAX_PATH 260


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



typedef struct _KLDR_DATA_TABLE_ENTRY64 {
	LIST_ENTRY64 InLoadOrderLinks;
	ULONG64 __Undefined1;
	ULONG64 __Undefined2;
	ULONG64 __Undefined3;
	ULONG64 NonPagedDebugInfo;
	ULONG64 DllBase;
	ULONG64 EntryPoint;
	ULONG SizeOfImage;
	UNICODE_STRING64 FullDllName;
	UNICODE_STRING64 BaseDllName;
	ULONG   Flags;
	USHORT  LoadCount;
	USHORT  __Undefined5;
	ULONG64 __Undefined6;
	ULONG   CheckSum;
	ULONG   __padding1;
	ULONG   TimeDateStamp;
	ULONG   __padding2;
} KLDR_DATA_TABLE_ENTRY64, *PKLDR_DATA_TABLE_ENTRY64;




typedef struct _KLDR_DATA_TABLE_ENTRY32 {
	LIST_ENTRY32 InLoadOrderLinks;
	ULONG __Undefined1;
	ULONG __Undefined2;
	ULONG __Undefined3;
	ULONG NonPagedDebugInfo;
	ULONG DllBase;
	ULONG EntryPoint;
	ULONG SizeOfImage;
	UNICODE_STRING32 FullDllName;
	UNICODE_STRING32 BaseDllName;
	ULONG Flags;
	USHORT LoadCount;
	USHORT __Undefined5;
	ULONG  __Undefined6;
	ULONG  CheckSum;
	ULONG  TimeDateStamp;
} KLDR_DATA_TABLE_ENTRY32, *PKLDR_DATA_TABLE_ENTRY32;


#ifdef _WIN64
#define PKLDR_DATA_TABLE_ENTRY  PKLDR_DATA_TABLE_ENTRY64
#else
#define PKLDR_DATA_TABLE_ENTRY  PKLDR_DATA_TABLE_ENTRY32
#endif


#define NUMBER_HASH_BUCKETS 37
typedef struct _OBJECT_DIRECTORY_ENTRY
{
	struct _OBJECT_DIRECTORY_ENTRY *ChainLink;
	PVOID Object;
	ULONG HashValue;
} OBJECT_DIRECTORY_ENTRY, *POBJECT_DIRECTORY_ENTRY;

typedef struct _OBJECT_DIRECTORY
{
	struct _OBJECT_DIRECTORY_ENTRY *HashBuckets[NUMBER_HASH_BUCKETS];
} OBJECT_DIRECTORY, *POBJECT_DIRECTORY;



typedef
	NTSTATUS
	(*pfnNtOpenDirectoryObject)(PHANDLE  DirectoryHandle,ACCESS_MASK  DesiredAccess,POBJECT_ATTRIBUTES  ObjectAttributes);

typedef 
	ULONG_PTR 
	(*pfnObGetObjectType)(PVOID Object);

//////////////////////////////////////////////////////////////////////////

NTSTATUS HsDispatchControlForModule(PIO_STACK_LOCATION  IrpSp, PVOID OutputBuffer, ULONG_PTR* ulRet);




// 枚举系统驱动模块列表
NTSTATUS HsEnumSystemModuleList(PVOID OutBuffer, ULONG OutSize);




VOID EnumDriverByLdrDataTableEntry(PALL_DRIVERS DriversInfor, ULONG_PTR ulCount);

BOOLEAN GetKernelLdrDataTableEntry(PDRIVER_OBJECT DriverObject);

VOID EnumDriversByWalkerDirectoryObject(PALL_DRIVERS DriversInfor, ULONG_PTR ulCount);

VOID WalkerDirectoryObject(PALL_DRIVERS DriversInfor, PVOID DirectoryObject, ULONG_PTR ulCount);

POBJECT_TYPE KeGetObjectType(PVOID Object);

BOOLEAN IsDriverInList(PALL_DRIVERS DriversInfor, PDRIVER_OBJECT DriverObject, ULONG_PTR ulCount);

VOID InsertDriver(PALL_DRIVERS DriversInfor, PDRIVER_OBJECT DriverObject, ULONG_PTR ulCount);

BOOLEAN IsUnicodeStringValid(PUNICODE_STRING uniString);



NTSTATUS HsUnloadDriverModule(ULONG_PTR InBuffer, ULONG_PTR InSize);

VOID HsInitModuleGlobalVariable();


//判断一个驱动是否为真的驱动对象
BOOLEAN IsRealDriverObject(PDRIVER_OBJECT DriverObject);

NTSTATUS PspUnloadDriver(PDRIVER_OBJECT DriverObject);

VOID HaveDriverUnloadThread(PVOID lParam);

VOID NotHaveDriverUnloadThread(IN PVOID lParam);







