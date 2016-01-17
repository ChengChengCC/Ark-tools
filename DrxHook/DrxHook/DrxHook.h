
#ifndef CXX_DRXHOOK_H
#define CXX_DRXHOOK_H



#include <ntifs.h>
#include <devioctl.h>
#endif	

typedef struct _SYSTEM_SERVICE_TABLE32 {
	ULONG_PTR*   ServiceTableBase;
	ULONG_PTR*   ServiceCounterTableBase;
	ULONG32 NumberOfServices;
	ULONG_PTR*   ParamTableBase;
} SYSTEM_SERVICE_TABLE32, *PSYSTEM_SERVICE_TABLE32;

typedef struct _SYSTEM_SERVICE_TABLE64{
	ULONG_PTR* 		ServiceTableBase; 
	ULONG_PTR* 		ServiceCounterTableBase; 
	ULONG64  		NumberOfServices; 
	ULONG_PTR* 		ParamTableBase; 
} SYSTEM_SERVICE_TABLE64, *PSYSTEM_SERVICE_TABLE64;

#ifndef _WIN64
#define		_SYSTEM_SERVICE_TABLE   _SYSTEM_SERVICE_TABLE64
#define		SYSTEM_SERVICE_TABLE	 SYSTEM_SERVICE_TABLE64
#define		PSYSTEM_SERVICE_TABLE	 PSYSTEM_SERVICE_TABLE64
#else
#define		_SYSTEM_SERVICE_TABLE   _SYSTEM_SERVICE_TABLE32
#define		SYSTEM_SERVICE_TABLE	 SYSTEM_SERVICE_TABLE32
#define		PSYSTEM_SERVICE_TABLE	 PSYSTEM_SERVICE_TABLE32
#endif



__declspec(dllimport) SYSTEM_SERVICE_TABLE KeServiceDescriptorTable;

//½á¹¹ÉùÃ÷
typedef struct _SIGNATURE_INFO{
	UCHAR	cSingature;
	int		Offset;
}SIGNATURE_INFO,*PSIGNATURE_INFO;


typedef struct _LDR_DATA_TABLE_ENTRY                         // 24 elements, 0x78 bytes (sizeof) 
{                                                                                                
	/*0x000*/     struct _LIST_ENTRY InLoadOrderLinks;       // 2 elements, 0x8 bytes (sizeof)   
	/*0x008*/     PVOID ExceptionTable;  
	/*0x00C*/	  ULONG ExceptionTableSize;
	/*0x010*/     struct _LIST_ENTRY InInitializationOrderLinks; // 2 elements, 0x8 bytes (sizeof)   
	/*0x018*/     VOID*        DllBase;                                                                        
	/*0x01C*/     VOID*        EntryPoint;                                                                     
	/*0x020*/     ULONG32      SizeOfImage;                                                                    
	/*0x024*/     struct _UNICODE_STRING FullDllName;             // 3 elements, 0x8 bytes (sizeof)   
	/*0x02C*/     struct _UNICODE_STRING BaseDllName;             // 3 elements, 0x8 bytes (sizeof)   
	/*0x034*/     ULONG32      Flags;                                                                          
	/*0x038*/     UINT16       LoadCount;                                                                      
	/*0x03A*/     UINT16       TlsIndex;                                                                       
	union                                                    // 2 elements, 0x8 bytes (sizeof)   
	{                                                                                            
    /*0x03C*/     struct _LIST_ENTRY HashLinks;           // 2 elements, 0x8 bytes (sizeof)   
		struct                                          // 2 elements, 0x8 bytes (sizeof)   
		{                                                                                        
			/*0x03C*/             VOID*        SectionPointer;                                                         
			/*0x040*/             ULONG32      CheckSum;                                                               
		};                                                                                       
	};                                                                                           
	union                                                    // 2 elements, 0x4 bytes (sizeof)   
	{                                                                                            
		/*0x044*/         ULONG32      TimeDateStamp;                                                              
		/*0x044*/         VOID*        LoadedImports;                                                              
	};                                                                                           
	/*0x048*/     VOID* EntryPointActivationContext;                                     
	/*0x04C*/     VOID*        PatchInformation;                                                               
	/*0x050*/     struct _LIST_ENTRY ForwarderLinks;                       // 2 elements, 0x8 bytes (sizeof)   
	/*0x058*/     struct _LIST_ENTRY ServiceTagLinks;                      // 2 elements, 0x8 bytes (sizeof)   
	/*0x060*/     struct _LIST_ENTRY StaticLinks;                          // 2 elements, 0x8 bytes (sizeof)   
	/*0x068*/     VOID*        ContextInformation;                                                             
	/*0x06C*/     ULONG32      OriginalBase;                                                                   
	/*0x070*/     union _LARGE_INTEGER LoadTime;                           // 4 elements, 0x8 bytes (sizeof)   
}LDR_DATA_TABLE_ENTRY, *PLDR_DATA_TABLE_ENTRY;


ULONG_PTR _stdcall
	FilterRtlDispatchException (
	IN PEXCEPTION_RECORD ExceptionRecord,
	IN PCONTEXT ContextRecord
	);
VOID HookRtlDispatchException();
VOID UnloadDriver(PDRIVER_OBJECT DriverObject);
PLDR_DATA_TABLE_ENTRY SearchDriver(PDRIVER_OBJECT pDriverObject,wchar_t *strDriverName);
BOOLEAN	Jmp_HookFunction(IN ULONG Destination,IN ULONG Source,IN UCHAR *Ori_Code);
VOID ResumeHookFunction(IN ULONG	Destination,IN UCHAR	*Ori_Code,IN ULONG	Length);
ULONG_PTR SearchAddressForSignFromPE(ULONG_PTR uStartBase,
                 ULONG_PTR uSearchLength,
                 SIGNATURE_INFO SignatureInfo[5]);
VOID WPOFF();
NTSTATUS  _stdcall FilterNtOpenProcess ();
VOID WPON();
VOID SetMonitor(PVOID Address);
VOID CancelMonitor(PVOID Address);