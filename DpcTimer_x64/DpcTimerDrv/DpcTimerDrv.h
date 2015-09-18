/***************************************************************************************
* AUTHOR : 懒人
* DATE   : 2014-10-25
* MODULE : DpcTimerDrv.H
*
* IOCTRL Sample Driver
*
* Description:
*		Demonstrates communications between USER and KERNEL.
*
****************************************************************************************
* Copyright (C) 2010 懒人.
****************************************************************************************/


#include <ntifs.h>
#include <devioctl.h>

#define CTL_GET_DPCTIMER \
	CTL_CODE(FILE_DEVICE_UNKNOWN,0x830,METHOD_NEITHER,FILE_ANY_ACCESS)
#define CTL_GET_DRIVER \
	CTL_CODE(FILE_DEVICE_UNKNOWN,0x831,METHOD_NEITHER,FILE_ANY_ACCESS)

#define CTL_REMOVEDPCTIMER \
	CTL_CODE(FILE_DEVICE_UNKNOWN,0x832,METHOD_NEITHER,FILE_ANY_ACCESS)

#define DEVICE_NAME   L"\\Device\\DPCTimer"
#define LINK_NAME 	  L"\\DosDevices\\DPCTimerLink"

#define MAX_PATH  260

#define SYSTEM_ADDRESS_START32 0x80000000
#define SYSTEM_ADDRESS_START64 0x80000000000

typedef struct _REMOVE_DPCTIMER
{
	ULONG_PTR     TimerObject;
}REMOVE_DPCTIMER,*PREMOVE_DPCTIMER;

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


typedef struct _DPC_TIMER_
{
	ULONG_PTR TimerObject;
	ULONG_PTR Period;			// 周期
	ULONG_PTR TimeDispatch;
	ULONG_PTR Dpc;
}DPC_TIMER, *PDPC_TIMER;


typedef struct _DPC_TIMER_INFOR_
{
	ULONG ulCnt;
	ULONG ulRetCnt;
	DPC_TIMER DpcTimer[1];
}DPC_TIMER_INFOR, *PDPC_TIMER_INFOR;


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
#define SYSTEM_ADDRESS_START    SYSTEM_ADDRESS_START64
#else
#define PKLDR_DATA_TABLE_ENTRY  PKLDR_DATA_TABLE_ENTRY32
#define SYSTEM_ADDRESS_START	SYSTEM_ADDRESS_START32
#endif

VOID UnloadDriver(PDRIVER_OBJECT DriverObject);
PVOID GetFunctionAddressByName(WCHAR *wzFunction);
NTSTATUS DefaultDispatchFunction(PDEVICE_OBJECT  DeviceObject,PIRP Irp);
NTSTATUS EnumDpcTimer(PVOID OutBuffer);
NTSTATUS DispatchControl(PDEVICE_OBJECT  DeviceObject,PIRP Irp);
NTSTATUS GetDpcTimerInformation_x64(PDPC_TIMER_INFOR DpcTimerInfor);
VOID FindKiWaitFunc(PULONG64 *KiWaitNeverAddr, PULONG64 *KiWaitAlwaysAddr);
KDPC* TransTimerDpcEx(
	IN PKTIMER InTimer,
	IN ULONGLONG InKiWaitNever,
	IN ULONGLONG InKiWaitAlways);

VOID EnumDriverByLdrDataTableEntry(PALL_DRIVERS DriversInfor, ULONG_PTR ulCount);
NTSTATUS EnumDrivers(PVOID OutBuffer, ULONG OutSize);
BOOLEAN GetKernelLdrDataTableEntry(PDRIVER_OBJECT DriverObject);
BOOLEAN IsUnicodeStringValid(PUNICODE_STRING uniString);
NTSTATUS RemoveDPCTimer(PVOID InBuffer);