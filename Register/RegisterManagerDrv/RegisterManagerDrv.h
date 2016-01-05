#pragma once

#include <ntifs.h>
#include <devioctl.h>
#include <ntimage.h>
#define SEC_IMAGE 0x01000000

typedef struct _OPEN_
{
	ACCESS_MASK DesiredAccess;
	POBJECT_ATTRIBUTES ObjectAttributes;
}OPEN,*POPEN;

typedef struct _ENUM_
{
	HANDLE hKey;
	ULONG Index;
	ULONG InformationClass;
	ULONG Length;
}ENUM,*PENUM;

typedef struct _ENUM_VALUE_
{
	PULONG RetLength;
	PVOID  ValueInfor;
}ENUM_VALUE, *PENUM_VALUE;


typedef struct  _CREATE_
{
	ACCESS_MASK DesiredAccess;
	POBJECT_ATTRIBUTES ObjectAttributes;
}CREATE,*PCREATE;



typedef struct _CREATE_VALUE_
{
	PHANDLE KeyHandle;
	PULONG  Disposition;
}CREATE_VALUE, *PCREATE_VALUE;

typedef struct _SET_KEY_VALUE_
{
	HANDLE hKey;
	PUNICODE_STRING ValueName;
	ULONG Type;
	PVOID Data;
	ULONG DataSize;
}SET_KEY_VALUE,*PSET_KEY_VALUE;


typedef struct _DELETE_   
{
	HANDLE  hKey;

}*PDELETE;


typedef  struct _DELETE_KEY_VALUE_ 
{
	HANDLE hKey;
	PUNICODE_STRING  uniValueName;

}DELETE_KEY_VALUE,*PDELETE_KEY_VALUE;


typedef struct  _RENAME_
{
	HANDLE hKey;
	PUNICODE_STRING  uniNewName;
}RENAME,*PRENAME;



typedef enum WIN_VERSION {
	WINDOWS_UNKNOW,
	WINDOWS_XP,
	WINDOWS_7,
	WINDOWS_8,
	WINDOWS_8_1
} WIN_VERSION;


typedef struct _SYSTEM_SERVICE_TABLE64{
	PVOID  		ServiceTableBase; 
	PVOID  		ServiceCounterTableBase; 
	ULONG64  	NumberOfServices; 
	PVOID  		ParamTableBase; 
} SYSTEM_SERVICE_TABLE64, *PSYSTEM_SERVICE_TABLE64;

typedef struct _SYSTEM_SERVICE_TABLE32 {
	PVOID   ServiceTableBase;
	PVOID   ServiceCounterTableBase;
	ULONG32 NumberOfServices;
	PVOID   ParamTableBase;
} SYSTEM_SERVICE_TABLE32, *PSYSTEM_SERVICE_TABLE32;

#define DEVICE_NAME  L"\\Device\\RegisterManagerDevice"
#define LINK_NAME    L"\\??\\RegisterManagerLink"


#define CTL_CREATE_KEY \
	CTL_CODE(FILE_DEVICE_UNKNOWN,0x833,METHOD_NEITHER,FILE_ANY_ACCESS)
#define CTL_OPEN_KEY \
	CTL_CODE(FILE_DEVICE_UNKNOWN,0x830,METHOD_NEITHER,FILE_ANY_ACCESS)
#define CTL_ENUM_KEY \
	CTL_CODE(FILE_DEVICE_UNKNOWN,0x831,METHOD_NEITHER,FILE_ANY_ACCESS)
#define CTL_ENUM_KEY_VALUE \
	CTL_CODE(FILE_DEVICE_UNKNOWN,0x832,METHOD_NEITHER,FILE_ANY_ACCESS)
#define CTL_SET_KEY_VALUE \
	CTL_CODE(FILE_DEVICE_UNKNOWN,0x834,METHOD_NEITHER,FILE_ANY_ACCESS)
#define CTL_DELETE_KEY \
	CTL_CODE(FILE_DEVICE_UNKNOWN,0x835,METHOD_NEITHER,FILE_ANY_ACCESS)
#define CTL_RENAME_KEY \
	CTL_CODE(FILE_DEVICE_UNKNOWN,0x836,METHOD_NEITHER,FILE_ANY_ACCESS)
#define CTL_DELETE_KEY_VALUE \
	CTL_CODE(FILE_DEVICE_UNKNOWN,0x837,METHOD_NEITHER,FILE_ANY_ACCESS)

NTSYSAPI PIMAGE_NT_HEADERS NTAPI RtlImageNtHeader(PVOID Base);

typedef NTSTATUS(NTAPI *pfnNtOpenKey)(PHANDLE KeyHandle,
	ACCESS_MASK DesiredAccess,
	POBJECT_ATTRIBUTES ObjectAttributes);

typedef NTSTATUS(NTAPI *pfnNtEnumerateKey)(
	HANDLE KeyHandle,
	ULONG Index,
	KEY_INFORMATION_CLASS KeyInformationClass,
	PVOID KeyInformation,
	ULONG Length,
	PULONG ResultLength);

typedef NTSTATUS(*pfnNtEnumerateValueKey)(
	HANDLE KeyHandle,
	ULONG Index,
	KEY_VALUE_INFORMATION_CLASS KeyValueInformationClass,
	PVOID KeyValueInformation,
	ULONG Length,
	PULONG ResultLength);


typedef NTSTATUS (*pfnNtCreateKey)(
	OUT PHANDLE  KeyHandle,
	IN ACCESS_MASK  DesiredAccess,
	IN POBJECT_ATTRIBUTES  ObjectAttributes,
	IN ULONG  TitleIndex,
	IN PUNICODE_STRING  Class  OPTIONAL,
	IN ULONG  CreateOptions,
	OUT PULONG  Disposition  OPTIONAL);

typedef NTSTATUS (*pfnNtSetValueKey)(
	IN HANDLE  KeyHandle,
	IN PUNICODE_STRING  ValueName,
	IN ULONG  TitleIndex  OPTIONAL,
	IN ULONG  Type,
	IN PVOID  Data,
	IN ULONG  DataSize);

typedef NTSTATUS (*pfnNtDeleteKey)(
	IN HANDLE  KeyHandle);

typedef NTSTATUS(*pfnNtRenameKey)(
	IN HANDLE KeyHandle,
	IN PUNICODE_STRING uniNewName);

typedef NTSTATUS (*pfnNtDeleteValueKey)(
	IN HANDLE  KeyHandle,
	IN PUNICODE_STRING  uniValueName);

VOID UnloadDriver(PDRIVER_OBJECT DriverObject);
NTSTATUS DefaultDispatch(PDEVICE_OBJECT  DeviceObject,PIRP Irp);
NTSTATUS ControlDispatch(PDEVICE_OBJECT  DeviceObject,PIRP Irp);

typedef NTSTATUS (*pfnRtlGetVersion)(OUT PRTL_OSVERSIONINFOW lpVersionInformation);
PVOID GetFunctionAddressByName(WCHAR *wzFunction);
WIN_VERSION GetWindowsVersion();
VOID SetGolbalMember();
LONG GetSSDTApiFunIndex(IN LPSTR lpszFunName);
NTSTATUS MapFileInUserSpace(IN LPWSTR lpszFileName,
	IN HANDLE ProcessHandle OPTIONAL,
	OUT PVOID *BaseAddress,
	OUT PSIZE_T ViewSize OPTIONAL);

ULONG_PTR GetSSDTApiFunAddress(ULONG_PTR ulIndex,ULONG_PTR SSDTDescriptor);
ULONG_PTR GetSSDTFunctionAddress64(ULONG_PTR ulIndex,ULONG_PTR SSDTDescriptor);
ULONG_PTR GetSSDTFunctionAddress32(ULONG_PTR ulIndex,ULONG_PTR SSDTDescriptor);
ULONG_PTR GetKeServiceDescriptorTable64();

VOID RecoverPreMode(PETHREAD EThread, CHAR PreMode);
CHAR ChangePreMode(PETHREAD EThread);
NTSTATUS RegOpenKey(PVOID InBuffer,PVOID OutBuffer);
NTSTATUS KernelOpenKey(OUT PHANDLE KeyHandle,
	IN ACCESS_MASK  DesiredAccess,
	IN POBJECT_ATTRIBUTES  ObjectAttributes);

NTSTATUS RegEnumerateKey(PVOID InBuffer,PVOID OutBuffer);
NTSTATUS KernelEnumerateKey(IN HANDLE  KeyHandle,
	IN ULONG  Index,
	IN KEY_INFORMATION_CLASS  KeyInformationClass,
	OUT PVOID  KeyInformation,
	IN ULONG  Length,
	OUT PULONG  ResultLength);

NTSTATUS RegEnumerateValueKey(PVOID InBuffer,PVOID OutBuffer);
NTSTATUS KernelEnumerateValueKey(IN HANDLE KeyHandle,
	IN ULONG  Index,
	IN KEY_VALUE_INFORMATION_CLASS  KeyValueInformationClass,
	OUT PVOID  KeyValueInformation,
	IN ULONG  Length,
	OUT PULONG  ResultLength);

NTSTATUS RegCreateKey(PVOID InBuffer,PVOID OutBuffer);
NTSTATUS KernelCreateKey(
	OUT PHANDLE  KeyHandle,
	IN ACCESS_MASK  DesiredAccess,
	IN POBJECT_ATTRIBUTES  ObjectAttributes,
	IN ULONG  TitleIndex,
	IN PUNICODE_STRING  Class  OPTIONAL,
	IN ULONG  CreateOptions,
	OUT PULONG  Disposition  OPTIONAL);
NTSTATUS RegSetValueKey(PVOID InBuffer,PVOID OutBuffer);
NTSTATUS KernelSetValueKey(IN HANDLE  KeyHandle,
	IN PUNICODE_STRING  ValueName,
	IN ULONG  TitleIndex  OPTIONAL,
	IN ULONG  Type,
	IN PVOID  Data,
	IN ULONG  DataSize);
NTSTATUS RegDeleteKey(PVOID InBuffer,PVOID OutBuffer);
NTSTATUS KernelDeleteKey(
	IN HANDLE  KeyHandle);
NTSTATUS RegRenameKey(PVOID InBuffer,PVOID OutBuffer);
NTSTATUS KernelRenameKey(IN HANDLE KeyHandle, IN PUNICODE_STRING uniNewName);
NTSTATUS RegDeleteValueKey(PVOID InBuffer,PVOID OutBuffer);
NTSTATUS KernelDeleteValueKey(IN HANDLE  KeyHandle,
	IN PUNICODE_STRING  uniValueName);

