#pragma once
#include "stdafx.h"

#include "MyList.h"


#include "ProcessFunc.h"


#define ProcessBasicInformation 0




typedef LONG NTSTATUS;

typedef struct
{
	USHORT Length;
	USHORT MaximumLength;
	PWSTR  Buffer;
} UNICODE_STRING, *PUNICODE_STRING;

typedef struct  PROCESS_PARAMETERS32
{
	ULONG          AllocationSize;
	ULONG          ActualSize;
	ULONG          Flags;
	ULONG          Unknown1;
	UNICODE_STRING Unknown2;
	HANDLE         InputHandle;
	HANDLE         OutputHandle;
	HANDLE         ErrorHandle;
	UNICODE_STRING CurrentDirectory;
	HANDLE         CurrentDirectoryHandle;
	UNICODE_STRING SearchPaths;
	UNICODE_STRING ApplicationName;
	UNICODE_STRING CommandLine;
	PVOID          EnvironmentBlock;
	ULONG          Unknown[9];
	UNICODE_STRING Unknown3;
	UNICODE_STRING Unknown4;
	UNICODE_STRING Unknown5;
	UNICODE_STRING Unknown6;
} PROCESS_PARAMETERS32, *PPROCESS_PARAMETERS32;

typedef struct PROCESS_PARAMETERS64
{
	ULONG          AllocationSize;
	ULONG          ActualSize;
	ULONG          Flags;
	ULONG          Unknown1;
	PVOID64        ConsoleHandle;
	ULONG          ConsoleFlags;
	PVOID64        InputHandle;
	PVOID64		   OutputHandle;
	PVOID64        ErrorHandle;
	UNICODE_STRING CurrentDirectory;
	HANDLE         CurrentDirectoryHandle;
	UNICODE_STRING SearchPaths;
	UNICODE_STRING ApplicationName;
	UNICODE_STRING CommandLine;
	PVOID64        EnvironmentBlock;
	ULONG          Unknown[9];
	UNICODE_STRING Unknown3;
	UNICODE_STRING Unknown4;
	UNICODE_STRING Unknown5;
	UNICODE_STRING Unknown6;
}PROCESS_PARAMETERS64,*PPROCESS_PARAMETERS64;






typedef struct _PEB32 {
	BOOLEAN InheritedAddressSpace; 
	BOOLEAN ReadImageFileExecOptions;   
	BOOLEAN BeingDebugged;             
	union {
		BOOLEAN BitField;            
		struct {
			BOOLEAN ImageUsesLargePages : 1;
			BOOLEAN SpareBits : 7;
		};
	};
	LONG Mutant;
	PVOID ImageBaseAddress;
	PVOID Ldr;
	PVOID ProcessParameters;
}_PEB32, *_PPEB32;



typedef struct _PEB64 {
	BOOLEAN InheritedAddressSpace; 
	BOOLEAN ReadImageFileExecOptions;   
	BOOLEAN BeingDebugged;             
	union {
		BOOLEAN BitField;            
		struct {
			BOOLEAN ImageUsesLargePages : 1;
			BOOLEAN SpareBits : 7;
		};
	};
	LONG_PTR Mutant;
	PVOID ImageBaseAddress;
	PVOID Ldr;
	PVOID ProcessParameters;
}_PEB64, *_PPEB64;


typedef struct _PROCESS_BASIC_INFORMATION32 {
	NTSTATUS ExitStatus;
	ULONG32 PebBaseAddress;
	ULONG32 AffinityMask;
	ULONG BasePriority;
	ULONG32 UniqueProcessId;
	ULONG32 InheritedFromUniqueProcessId;
} PROCESS_BASIC_INFORMATION32;





typedef struct _PROCESS_BASIC_INFORMATION64 {
	NTSTATUS ExitStatus;
	ULONG32 Pad1;
	ULONG64 PebBaseAddress;
	ULONG64 AffinityMask;
	ULONG   BasePriority;
	ULONG32 Pad2;
	ULONG64 UniqueProcessId;
	ULONG64 InheritedFromUniqueProcessId;
} PROCESS_BASIC_INFORMATION64;


VOID HsInitProcessDetailList(CMyList *m_ListCtrl);

VOID HsLoadProcessDetailList(PHSPROCESSINFO ProcessInfo, CMyList *m_ListCtrl);

CString HsGetProcessPebAddress(DWORD dwPid);

CString HsGetFileDescription(CString strPath);

CString HsGetProcessCmdLine(DWORD dwPid);

CString HsGetProcessCurrentDirectory(DWORD dwPid);