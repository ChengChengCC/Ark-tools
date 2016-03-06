#pragma once 


#if DBG
#define dprintf DbgPrint
#else
#define dprintf
#endif

#include <ntifs.h>
#include <ntimage.h>
#include "common.h"

#define MAX_PATH 60

//////////////////////////////////////////////////////////////////////////

enum HS_KERNEL_KERNELFILE
{
	HS_KERNEL_KERNELFILE_NTOSKRNL_IAT,
	HS_KERNEL_KERNELFILE_NTOSKRNL_EAT,
	HS_KERNEL_KERNELFILE_WIN32K_IAT,
	HS_KERNEL_KERNELFILE_WIN32K_EAT,
	HS_KERNEL_KERNELFILE_HALDLL_IAT,
	HS_KERNEL_KERNELFILE_HALDLL_EAT,
};

//////////////////////////////////////////////////////////////////////////


typedef struct _IAT_INFO_
{
	ULONG_PTR CurFuncAddress;
	ULONG_PTR OriFuncAddress;
	CHAR  szFunctionName[MAX_PATH];
	CHAR  szModuleName[MAX_PATH];
}IAT_INFO, *PIAT_INFO;

typedef struct _MODULE_IAT_
{
	ULONG_PTR ulCount;
	IAT_INFO  Data[1];
}MODULE_IAT, *PMODULE_IAT;

typedef struct _EAT_INFO_
{
	ULONG_PTR CurFuncAddress;
	ULONG_PTR OriFuncAddress;
	CHAR  szFunctionName[MAX_PATH];
}EAT_INFO, *PEAT_INFO;

typedef struct _MODULE_EAT_
{
	ULONG_PTR ulCount;
	EAT_INFO  Data[1];
}MODULE_EAT, *PMODULE_EAT;





#define MakePtr(a,b,c) ((a)((char*)b+c))

#define SystemModuleInformation 11

typedef struct  _FILE_INFOR_
{
	char  szFileFullName[512];
	char*  szFileData;
	PVOID BaseAddress;
	ULONG_PTR Size;
}FILE_INFOR,*PFILE_INFOR;


typedef struct _SYSTEM_MODULE_INFORMATION_ENTRY64
{
	ULONG Reserved[4];  
	PVOID Base;
	ULONG Size;
	ULONG Flags;
	USHORT Index;
	USHORT NameLength;
	USHORT LoadCount;
	USHORT ModuleNameOffset;
	char ImageName[256];
} SYSTEM_MODULE_INFORMATION_ENTRY64, *PSYSTEM_MODULE_INFORMATION_ENTRY64;



typedef struct _SYSTEM_MODULE_INFORMATION_ENTRY32
{
	ULONG  Reserved[2];  
	ULONG  Base;        
	ULONG  Size;         
	ULONG  Flags;        
	USHORT Index;       
	USHORT Unknown;     
	USHORT LoadCount;   
	USHORT ModuleNameOffset;
	CHAR   ImageName[256];   
} SYSTEM_MODULE_INFORMATION_ENTRY32, *PSYSTEM_MODULE_INFORMATION_ENTRY32;



#ifdef _WIN64
#define SYSTEM_MODULE_INFORMATION_ENTRY SYSTEM_MODULE_INFORMATION_ENTRY64
#else
#define SYSTEM_MODULE_INFORMATION_ENTRY SYSTEM_MODULE_INFORMATION_ENTRY32
#endif


typedef struct _SYSTEM_MODULE_INFORMATION{          //模块链结构
	ULONG ulCount;
	SYSTEM_MODULE_INFORMATION_ENTRY smi[1];
}SYSTEM_MODULE_INFORMATION, *PSYSTEM_MODULE_INFORMATION;


NTSYSAPI
	NTSTATUS
	NtQuerySystemInformation(IN ULONG SystemInformationClass,IN PVOID SystemInformation,
	IN ULONG SystemInformationLength,
	OUT PULONG ReturnLength);




BOOLEAN  ReadFileData(PFILE_INFOR FileInfor);

PVOID
	GetDirectoryAddr(PUCHAR AddressBase,USHORT DirectoryIndex,
	ULONG_PTR* ulSize,ULONG_PTR* ulDiff,BOOLEAN IsFile);

PIMAGE_SECTION_HEADER
	GetSectionHeaderFromRva(ULONG RVA,PIMAGE_NT_HEADERS NtHeader);  //判断RVA是在那个节表当中


NTSTATUS HsEnumKernelFileFunc(int KernelFile, PVOID OutputBuffer, ULONG_PTR ulOutputLen);

NTSTATUS HsQueryKernelFileFuncIAT(PVOID OutputBuffer, ULONG_PTR ulOutputLen, char* szModuleFile);

NTSTATUS HsQueryKernelFileFuncEAT(PVOID OutputBuffer, ULONG_PTR ulOutputLen, char* szModuleFile);

NTSTATUS HsEnumEATTable(PVOID  KernelBase,PMODULE_EAT OutBuffer, ULONG_PTR OutSize, char* szModuleFileName);

NTSTATUS HsEnumIATTable(PMODULE_IAT OutBuffer, ULONG_PTR OutSize);

BOOLEAN	GetModuleInforKernelFile(PULONG_PTR ulKernelBase,PULONG_PTR ulKernelSize,char* szWin32kFullName, char* szModuleFile);

BOOLEAN GetModuleInfor(char* szModuleName,char* szFullName, SYSTEM_MODULE_INFORMATION_ENTRY* Temp);

//////////////////////////////////////////////////////////////////////////


VOID HsInitKrnlFileGlobalVariable();

PFILE_INFOR CreateFileData(SYSTEM_MODULE_INFORMATION_ENTRY* ModuleInfor,char* ModuleName);

BOOLEAN GetAddrOfExportFuncAddr(
	UCHAR*  Base,				  
	CHAR*	FunctionName,	      
	ULONG_PTR*  AddrOfExportFuncAddr, BOOLEAN IsFile);



NTSYSAPI
	NTSTATUS
	NTAPI
	ZwQuerySystemInformation(
	IN ULONG_PTR SystemInformationClass,
	IN OUT PVOID SystemInformation,
	IN ULONG SystemInformationLength,
	OUT PULONG ReturnLength OPTIONAL
	);


