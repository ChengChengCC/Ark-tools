#pragma once 



#if DBG
#define dprintf DbgPrint
#else
#define dprintf
#endif

#include <ntifs.h>
#include "common.h"

#include "Process.h"

typedef enum _SYSTEM_INFORMATION_CLASS {    
	SystemBasicInformation,    
	SystemProcessorInformation,    
	SystemPerformanceInformation,    
	SystemTimeOfDayInformation,    
	SystemNotImplemented1,    
	SystemProcessesInformation,    
	SystemCallCounts,    
	SystemConfigurationInformation,    
	SystemProcessorTimes,    
	SystemGlobalFlag,    
	SystemNotImplemented2,    
	SystemModuleInformation,    
	SystemLockInformation,    
	SystemNotImplemented3,    
	SystemNotImplemented4,    
	SystemNotImplemented5,    
	SystemHandleInformation, //枚举系统中的全部句柄 
	SystemObjectInformation,    
	SystemPagefileInformation,    
	SystemInstructionEmulationCounts,    
	SystemInvalidInfoClass1,    
	SystemCacheInformation,    
	SystemPoolTagInformation,    
	SystemProcessorStatistics,    
	SystemDpcInformation,    
	SystemNotImplemented6,    
	SystemLoadImage,    
	SystemUnloadImage,    
	SystemTimeAdjustment,    
	SystemNotImplemented7,    
	SystemNotImplemented8,    
	SystemNotImplemented9,    
	SystemCrashDumpInformation,    
	SystemExceptionInformation,    
	SystemCrashDumpStateInformation,    
	SystemKernelDebuggerInformation,    
	SystemContextSwitchInformation,    
	SystemRegistryQuotaInformation,    
	SystemLoadAndCallImage,    
	SystemPrioritySeparation,    
	SystemNotImplemented10,    
	SystemNotImplemented11,    
	SystemInvalidInfoClass2,    
	SystemInvalidInfoClass3,    
	SystemTimeZoneInformation,    
	SystemLookasideInformation,    
	SystemSetTimeSlipEvent,    
	SystemCreateSession,    
	SystemDeleteSession,    
	SystemInvalidInfoClass4,    
	SystemRangeStartInformation,    
	SystemVerifierInformation,    
	SystemAddVerifier,    
	SystemSessionProcessesInformation    
} SYSTEM_INFORMATION_CLASS, *PSYSTEM_INFORMATION_CLASS;


typedef enum _OBJECT_INFO_CLASS {
	ObjectBasicInfo,
	ObjectNameInfo,
	ObjectTypeInfo,
	ObjectAllInfo,
	ObjectDataInfo
} OBJECT_INFO_CLASS, *POBJECT_INFO_CLASS;


#define MAX_OBJECT_NAME  50
#define MAX_PATH         500


typedef struct _HANDLE_INFO_
{
	ULONG_PTR Handle;
	ULONG_PTR Object;
	ULONG_PTR ReferenceCount;
	WCHAR ObjectName[MAX_OBJECT_NAME];
	WCHAR HandleName[MAX_PATH];
}HANDLE_INFO, *PHANDLE_INFO;

typedef struct _PROCESS_HANDLES_
{
	ULONG_PTR ulCount;
	HANDLE_INFO Handles[1];
}ALL_HANDLES, *PALL_HANDLES;




typedef
	NTSTATUS (NTAPI *pfnNtQueryObject)(HANDLE Handle,OBJECT_INFORMATION_CLASS ObjectInformationClass,
	PVOID ObjectInformation,
	ULONG ObjectInformationLength,
	PULONG ReturnLength);




NTSTATUS HsEnumProcessHandle(PVOID InBuffer, ULONG_PTR InSize, PVOID OutBuffer, ULONG_PTR OutSize);
NTSTATUS HsGetHandles(ULONG_PTR ulPid, ULONG_PTR EProcess, PALL_HANDLES OutHandles, ULONG_PTR ulCount);
VOID HsInsertHandleToList(PEPROCESS EProcess, HANDLE Handle, ULONG_PTR Object, PALL_HANDLES Handles);
VOID HsGetHandleObjectName(HANDLE hHandle,WCHAR* wzObjectName);
VOID HsGetHandleTypeName(HANDLE hHandle, ULONG_PTR Object,WCHAR* wzTypeName);
VOID HsInitHandleVariable();

NTSYSAPI
PIMAGE_NT_HEADERS
NTAPI
RtlImageNtHeader(PVOID Base);


extern 
	NTSTATUS 
	NtQuerySystemInformation(   
	IN ULONG SystemInformationClass,   
	IN PVOID SystemInformation,   
	IN ULONG SystemInformationLength,   
	OUT PULONG ReturnLength);

typedef 
ULONG_PTR 
(*pfnObGetObjectType)(PVOID pObject);





NTSTATUS 
	MapFileInUserSpace(IN LPWSTR lpszFileName,IN HANDLE ProcessHandle OPTIONAL,
	OUT PVOID *BaseAddress,
	OUT PSIZE_T ViewSize OPTIONAL);


