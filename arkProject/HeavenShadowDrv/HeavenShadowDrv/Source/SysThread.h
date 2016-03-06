#pragma once 


#if DBG
#define dprintf DbgPrint
#else
#define dprintf
#endif

#include <ntifs.h>
#include "common.h"
#include "Thread.h"


typedef struct _HANDLE_TABLE64
{
	PVOID64 TableCode;
	PVOID64 QuotaProcess;
	PVOID64 UniqueProcessID;
	PVOID64 HandleLock;
	LIST_ENTRY HandleTableList;
	PVOID64    HandleContentionEvent;
	PVOID64    DebugInfo;
	ULONG      ExtraInfoPages;
	ULONG      Flags;
	ULONG      FirstFreeHandle;
	PVOID64    LastFreeHandleEntry;
	ULONG      HandleCount;
	ULONG      NextHandleNeedingPool;
	ULONG      HandleCountHighWatermark;
}HANDLE_TABLE64,*PHANDLE_TABLE64;



typedef struct _HANDLE_TABLE32
{
	PVOID TableCode;
	PVOID QuotaProcess;
	PVOID UniqueProcessID;
	ULONG HandleLock[4];
	LIST_ENTRY HandleTableList;
	PVOID    HandleContentionEvent;
	PVOID    DebugInfo;
	ULONG    ExtraInfoPages;
	ULONG    FirstFree;
	ULONG    LastFree;
	ULONG    NextHandleNeedingPool;
	ULONG    HandleCount;
	ULONG    Flags;
}HANDLE_TABLE32,*PHANDLE_TABLE32;

#ifdef _WIN64
#define PHANDLE_TABLE PHANDLE_TABLE64
#else
#define PHANDLE_TABLE PHANDLE_TABLE32
#endif

typedef struct _HANDLE_TABLE_ENTRY64 
{
	union {
		PVOID64 Object;
		ULONG ObAttributes;
		PVOID64 InfoTable;
		ULONG_PTR Value;
	};
	union {
		union {
			ULONG GrantedAccess;
			struct {
				USHORT GrantedAccessIndex;
				USHORT CreatorBackTraceIndex;
			};
		};
		ULONG NextFreeTableEntry;
	};

} HANDLE_TABLE_ENTRY64, *PHANDLE_TABLE_ENTRY64;


typedef struct _HANDLE_TABLE_ENTRY32 
{
	union {
		PVOID Object;
		ULONG ObAttributes;
		PVOID InfoTable;
		ULONG_PTR Value;
	};
	union {
		union {
			ULONG GrantedAccess;
			struct {
				USHORT GrantedAccessIndex;
				USHORT CreatorBackTraceIndex;
			};
		};
		ULONG NextFreeTableEntry;
	};

} HANDLE_TABLE_ENTRY32, *PHANDLE_TABLE_ENTRY32;


#ifdef _WIN64
#define PHANDLE_TABLE_ENTRY PHANDLE_TABLE_ENTRY64
#else
#define PHANDLE_TABLE_ENTRY PHANDLE_TABLE_ENTRY32
#endif



//////////////////////////////////////////////////////////////////////////


NTSTATUS HsEnumSysThread(PVOID OutBuffer, ULONG_PTR OutSize);

VOID HsSetGolbalMemberSysThread();

ULONG_PTR HsGetPspCidTableValue();

VOID ScanHandleTableToFindThread(PEPROCESS EProcess, PALL_THREADS AllThreads, ULONG_PTR ulCnt);

NTSTATUS EnumTable1(ULONG_PTR uTableCode, PEPROCESS EProcess, PALL_THREADS AllThreads, ULONG_PTR ulCnt);

NTSTATUS EnumTable2(ULONG_PTR uTableCode, PEPROCESS EProcess, PALL_THREADS AllThreads, ULONG_PTR ulCnt);

NTSTATUS EnumTable3(ULONG_PTR uTableCode, PEPROCESS EProcess, PALL_THREADS AllThreads, ULONG_PTR ulCnt);


