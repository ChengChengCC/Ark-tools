

#ifndef CXX_INLINEHOOKSSSDT_H
#define CXX_INLINEHOOKSSSDT_H



#include <ntifs.h>
#include <WINDEF.H>

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

#define DEVICE_NAME   L"\\Device\\InlineHookSSSDTDevice"
#define LINK_NAME 	  L"\\DosDevices\\InlineHookSSSDTLink"

#define IOCTL_GET_SSSDTSERVERICE	CTL_CODE(FILE_DEVICE_UNKNOWN, 0x801, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_INLINEHOOK_SSSDT \
	CTL_CODE(FILE_DEVICE_UNKNOWN,0x830,METHOD_BUFFERED,FILE_ANY_ACCESS)

#define IOCTL_INLINEUNHOOK_SSSDT \
	CTL_CODE(FILE_DEVICE_UNKNOWN,0x831,METHOD_BUFFERED,FILE_ANY_ACCESS)


NTSTATUS ControlPassThrough(PDEVICE_OBJECT  DeviceObject,PIRP Irp);
NTSTATUS DefaultPassThrough(PDEVICE_OBJECT  DeviceObject,PIRP Irp);
VOID UnloadDriver(PDRIVER_OBJECT DriverObject);

PVOID 
	GetFunctionAddressByNameFromNtosExport(WCHAR *wzFunctionName);
PVOID GetKeShadowServiceDescriptorTable32();
PVOID GetKeShadowServiceDescriptorTable64();
PVOID GetSSSDTFunctionAddress64(ULONG ulIndex);
PVOID GetSSSDTFunctionAddress32(ULONG ulIndex);

typedef ULONG_PTR (*pfnNtUserQueryWindow)(HWND WindowHandle, ULONG_PTR TypeInformation);
typedef ULONG_PTR (*pfnNtUserPostMessage)(HWND WindowHandle,UINT uMsg,WPARAM wParam,LPARAM lParam);

VOID WPOFF();
VOID WPON();

BOOLEAN  InlineHookSSSDTWin7(PVOID OriginalFunctionAddress,PVOID NewFucntionAddress,ULONG ulPatchSize,
	PVOID* OrigianlFunctionCode);
VOID InlineUnHookSSSDTWin7(PVOID OriginalFunctionAddress,PVOID OrigianlFunctionCode,ULONG ulPatchSize);



BOOL InlineHookSSSDTWinXP(PVOID OriginalFunctionAddress,PVOID NewFunctionAddress,ULONG ulPatchSize,PVOID* OrigianlFunctionCode);
VOID InlineUnHookSSSDTWinXP(PVOID OriginalFunctionAddress,PVOID OrigianlFunctionCode,ULONG ulPatchSize);

ULONG_PTR Fake_NtUserPostMessageAddress(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void LDE_Init64() ;
ULONG GetPatchSize64(PUCHAR Address);
#endif
