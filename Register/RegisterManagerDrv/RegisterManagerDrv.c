#include "RegisterManagerDrv.h"

PDRIVER_OBJECT  g_DriverObject = NULL;
WIN_VERSION     WinVersion = WINDOWS_UNKNOW;
ULONG_PTR       SYSTEM_ADDRESS_START = 0;
ULONG_PTR       PreviousModeOffsetOf_KTHREAD = 0;
ULONG_PTR       IndexOffset = 0;
pfnNtEnumerateValueKey NtEnumerateValueKeyAddress = NULL;
pfnNtOpenKey    NtOpenKeyAddress = NULL;
pfnNtEnumerateKey      NtEnumerateKeyAddress = NULL;
pfnNtCreateKey         NtCreateKeyAddress = NULL;
pfnNtSetValueKey       NtSetValueKeyAddress = NULL;
pfnNtDeleteKey         NtDeleteKeyAddress = NULL;
pfnNtRenameKey         NtRenameKeyAddress = NULL;
pfnNtDeleteValueKey    NtDeleteValueKeyAddress = NULL;
ULONG_PTR		SSDTDescriptor = 0;
ULONG_PTR		ulIndex = 0;
ULONG_PTR		SSDTFuncAddress = 0;


HANDLE  hRoot = NULL;
NTSTATUS DriverEntry(PDRIVER_OBJECT  DriverObject,PUNICODE_STRING  RegisterPath)
{
	PDEVICE_OBJECT	DeviceObject;
	NTSTATUS		Status;
	ULONG			i;

	UNICODE_STRING	uniDeviceName;
	UNICODE_STRING	uniLinkName;

	RtlInitUnicodeString(&uniDeviceName,DEVICE_NAME);
	RtlInitUnicodeString(&uniLinkName,LINK_NAME);

	//创建设备对象;
	Status = IoCreateDevice(DriverObject,0,&uniDeviceName,FILE_DEVICE_UNKNOWN,0,FALSE,&DeviceObject);
	if (!NT_SUCCESS(Status))
	{
		return Status;
	}

	//创建符号链接;
	Status = IoCreateSymbolicLink(&uniLinkName,&uniDeviceName);

	for (i = 0; i<IRP_MJ_MAXIMUM_FUNCTION; i ++)
	{
		DriverObject->MajorFunction[i] = DefaultDispatch;
	}
	DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = ControlDispatch;
	DriverObject->DriverUnload = UnloadDriver;
	
#ifdef WIN64
/*	__asm
	{
		xchg rax,rbx
	}
*/	DbgPrint("X64: RegisterManager IS RUNNING!!!\n");
#else
/*	__asm
	{
		xor eax,eax
	}
*/	DbgPrint("X86: RegisterManager IS RUNNING!!!\n");
#endif
	

	g_DriverObject = DriverObject;
	WinVersion = GetWindowsVersion();
	SetGolbalMember();
	return STATUS_SUCCESS;
}



NTSTATUS ControlDispatch(PDEVICE_OBJECT  DeviceObject,PIRP Irp)
{

	NTSTATUS  Status = STATUS_SUCCESS;
	PIO_STACK_LOCATION   IrpSp;
	PVOID     InputBuffer  = NULL;
	PVOID     OutputBuffer = NULL;
	ULONG_PTR InputSize  = 0;
	ULONG_PTR OutputSize = 0;
	ULONG_PTR IoControlCode = 0;
	PETHREAD EThread = NULL;
	CHAR     PreMode = 0;
	IrpSp = IoGetCurrentIrpStackLocation(Irp);

	IrpSp = IoGetCurrentIrpStackLocation(Irp);
	InputBuffer = IrpSp->Parameters.DeviceIoControl.Type3InputBuffer;
	OutputBuffer = Irp->UserBuffer;
	InputSize = IrpSp->Parameters.DeviceIoControl.InputBufferLength;
	OutputSize  = IrpSp->Parameters.DeviceIoControl.OutputBufferLength;

	IoControlCode = IrpSp->Parameters.DeviceIoControl.IoControlCode;


	switch(IoControlCode)
	{
	
	case CTL_OPEN_KEY:
		{


			DbgPrint("应用层事件到达");

		
			Status  = RegOpenKey(InputBuffer,OutputBuffer);
		
			if (NT_SUCCESS(Status))
			{
				Irp->IoStatus.Information = OutputSize;
				Irp->IoStatus.Status = Status;
			}

			else
			{
				Irp->IoStatus.Information = 0;
				Irp->IoStatus.Status = Status;
			}
			

			break;
		
		}

	case CTL_ENUM_KEY:
		{


			Status = RegEnumerateKey(InputBuffer,OutputBuffer);


			if (NT_SUCCESS(Status))
			{
				Irp->IoStatus.Information = OutputSize;
				Irp->IoStatus.Status = Status;
			}

			else
			{
				Irp->IoStatus.Information = 0;
				Irp->IoStatus.Status = Status;
			}



			break;
		}

	case CTL_ENUM_KEY_VALUE:
		{

			Status = RegEnumerateValueKey(InputBuffer,OutputBuffer);


			if (NT_SUCCESS(Status))
			{
				Irp->IoStatus.Information = OutputSize;
				Irp->IoStatus.Status = Status;
			}

			else
			{
				Irp->IoStatus.Information = 0;
				Irp->IoStatus.Status = Status;
			}

			break;
		}

	case CTL_CREATE_KEY:
		{

			Status = RegCreateKey(InputBuffer,OutputBuffer);


			if (NT_SUCCESS(Status))
			{
				Irp->IoStatus.Information = OutputSize;
				Irp->IoStatus.Status = Status;
			}

			else
			{
				Irp->IoStatus.Information = 0;
				Irp->IoStatus.Status = Status;
			}

			break;
		}

	case CTL_SET_KEY_VALUE:
		{

			Status = RegSetValueKey(InputBuffer,OutputBuffer);


			if (NT_SUCCESS(Status))
			{
				Irp->IoStatus.Information = OutputSize;
				Irp->IoStatus.Status = Status;
			}

			else
			{
				Irp->IoStatus.Information = 0;
				Irp->IoStatus.Status = Status;
			}

			break;
		}

	case CTL_DELETE_KEY:
		{

			Status = RegDeleteKey(InputBuffer,OutputBuffer);


			if (NT_SUCCESS(Status))
			{
				Irp->IoStatus.Information = OutputSize;
				Irp->IoStatus.Status = Status;
			}

			else
			{
				Irp->IoStatus.Information = 0;
				Irp->IoStatus.Status = Status;
			}

			break;
		}

	case CTL_RENAME_KEY:
		{

			Status = RegRenameKey(InputBuffer,OutputBuffer);


			if (NT_SUCCESS(Status))
			{
				Irp->IoStatus.Information = OutputSize;
				Irp->IoStatus.Status = Status;
			}

			else
			{
				Irp->IoStatus.Information = 0;
				Irp->IoStatus.Status = Status;
			}

			break;
		}


	case CTL_DELETE_KEY_VALUE:
		{

			Status = RegDeleteValueKey(InputBuffer,OutputBuffer);


			if (NT_SUCCESS(Status))
			{
				Irp->IoStatus.Information = OutputSize;
				Irp->IoStatus.Status = Status;
			}

			else
			{
				Irp->IoStatus.Information = 0;
				Irp->IoStatus.Status = Status;
			}

			break;
		}
	default:
		{

			Irp->IoStatus.Status = STATUS_UNSUCCESSFUL;
			Irp->IoStatus.Information = 0;



			break;
		}
	}


	IoCompleteRequest(Irp,IO_NO_INCREMENT);

	return Status;
}

NTSTATUS RegOpenKey(PVOID InBuffer,PVOID OutBuffer)
{
	POPEN Open = (POPEN)InBuffer;
	ACCESS_MASK Mask = Open->DesiredAccess;
	POBJECT_ATTRIBUTES oa = Open->ObjectAttributes;
	PHANDLE KeyHandle = (PHANDLE)OutBuffer;
	NTSTATUS Status = STATUS_UNSUCCESSFUL;

	__try
	{
		ProbeForRead(oa,sizeof(OBJECT_ATTRIBUTES),1);
		Status = KernelOpenKey(KeyHandle,Mask,oa);

	
	}
	__except(1)
	{
		Status = STATUS_UNSUCCESSFUL;
	}	

	return Status;
}



NTSTATUS KernelOpenKey(OUT PHANDLE KeyHandle,
	IN ACCESS_MASK  DesiredAccess,
	IN POBJECT_ATTRIBUTES  ObjectAttributes
	)

{
	NTSTATUS Status = STATUS_UNSUCCESSFUL;
	PETHREAD EThread = NULL;
	CHAR PreMode = 0;

	if (!KeyHandle ||
		!ObjectAttributes)
	{
		return Status;
	}

	
	EThread = PsGetCurrentThread();
	PreMode = ChangePreMode(EThread);
	Status = NtOpenKeyAddress(KeyHandle, DesiredAccess, ObjectAttributes);
	RecoverPreMode(EThread, PreMode);
	

	return Status;	
}




NTSTATUS RegEnumerateKey(PVOID InBuffer,PVOID OutBuffer)
{
	PENUM Enum = (PENUM)InBuffer;
	HANDLE KeyHandle = Enum->hKey;
	ULONG Index = Enum->Index;
	KEY_INFORMATION_CLASS KeyInformationClass = Enum->InformationClass;
	ULONG Length = Enum->Length;
	PENUM_VALUE EnumValue = (PENUM_VALUE)OutBuffer;
	ULONG ulReturn = 0;
	return KernelEnumerateKey(KeyHandle, Index, KeyInformationClass,EnumValue->ValueInfor,Length,EnumValue->RetLength);
}



NTSTATUS KernelEnumerateKey(IN HANDLE  KeyHandle,
	IN ULONG  Index,
	IN KEY_INFORMATION_CLASS  KeyInformationClass,
	OUT PVOID  KeyInformation,
	IN ULONG  Length,
	OUT PULONG  ResultLength
	)
{
	NTSTATUS Status = STATUS_UNSUCCESSFUL;
	PETHREAD EThread = NULL;
	CHAR PreMode = 0;
	
	if (!KeyHandle)
	{
		return Status;
	}



	EThread = PsGetCurrentThread();
	PreMode = ChangePreMode(EThread);;
	Status = NtEnumerateKeyAddress(KeyHandle, Index, KeyInformationClass, KeyInformation, Length, ResultLength);
	RecoverPreMode(EThread, PreMode);
	if (Status == STATUS_BUFFER_TOO_SMALL)
	{
		DbgPrint("STATUS_BUFFER_TOO_SMALL\r\n");
	}
	else if (STATUS_NO_MORE_ENTRIES == Status)
	{
		DbgPrint("STATUS_NO_MORE_ENTRIES\r\n");
	}

	else if (Status==STATUS_ACCESS_VIOLATION)
	{
		DbgPrint("STATUS_ACCESS_VIOLATION  %d\r\n",RtlNtStatusToDosError(Status));
	}
	
	return Status;
}




NTSTATUS RegEnumerateValueKey(PVOID InBuffer,PVOID OutBuffer)
{
	PENUM Enum = (PENUM)InBuffer;
	HANDLE KeyHandle = Enum->hKey;
	ULONG Index = Enum->Index;
	KEY_VALUE_INFORMATION_CLASS KeyValueInformationClass = Enum->InformationClass;
	ULONG Length = Enum->Length;
	PENUM_VALUE EnumValue = (PENUM_VALUE)OutBuffer;

	return KernelEnumerateValueKey(KeyHandle, Index, KeyValueInformationClass, EnumValue->ValueInfor, Length, EnumValue->RetLength);
}


NTSTATUS KernelEnumerateValueKey(IN HANDLE  KeyHandle,
	IN ULONG  Index,
	IN KEY_VALUE_INFORMATION_CLASS  KeyValueInformationClass,
	OUT PVOID  KeyValueInformation,
	IN ULONG  Length,
	OUT PULONG  ResultLength
	)
{
	NTSTATUS Status = STATUS_UNSUCCESSFUL;
	PETHREAD EThread = NULL;
	CHAR PreMode = 0;
	

	if (!KeyHandle)
	{
		return Status;
	}

	EThread  =  PsGetCurrentThread();
	PreMode = ChangePreMode(EThread);

	Status = NtEnumerateValueKeyAddress(KeyHandle, Index, KeyValueInformationClass, KeyValueInformation, Length, ResultLength);

	if (Status == STATUS_BUFFER_TOO_SMALL)
	{
		DbgPrint("STATUS_BUFFER_TOO_SMALL\r\n");
	}
	else if (STATUS_NO_MORE_ENTRIES == Status)
	{
		DbgPrint("STATUS_NO_MORE_ENTRIES\r\n");
	}

	else if (Status==STATUS_ACCESS_VIOLATION)
	{
		DbgPrint("STATUS_ACCESS_VIOLATION  %d\r\n",RtlNtStatusToDosError(Status));
	}

	RecoverPreMode(EThread, PreMode);
	return Status;
}


NTSTATUS RegCreateKey(PVOID InBuffer,PVOID OutBuffer)
{
	PCREATE Create = (PCREATE)InBuffer;
	ACCESS_MASK DesiredAccess = Create->DesiredAccess;
	POBJECT_ATTRIBUTES oa = Create->ObjectAttributes;

	PCREATE_VALUE CreateValue = (PCREATE_VALUE)OutBuffer;
	PHANDLE  KeyHandle = CreateValue->KeyHandle;
	PULONG   Disposition = CreateValue->Disposition;
	return KernelCreateKey(KeyHandle, DesiredAccess, oa, 0, NULL, REG_OPTION_NON_VOLATILE, Disposition);
}



NTSTATUS 
	KernelCreateKey(
	OUT PHANDLE  KeyHandle,
	IN ACCESS_MASK  DesiredAccess,
	IN POBJECT_ATTRIBUTES  ObjectAttributes,
	IN ULONG  TitleIndex,
	IN PUNICODE_STRING  Class  OPTIONAL,
	IN ULONG  CreateOptions,
	OUT PULONG  Disposition  OPTIONAL
	)
{
	
	PETHREAD EThread = NULL;
	CHAR PreMode = 0;
	NTSTATUS Status = STATUS_UNSUCCESSFUL;

	if (!KeyHandle)
	{
		return Status;
	}

	EThread = PsGetCurrentThread();
	PreMode = ChangePreMode(EThread);

	Status = NtCreateKeyAddress(KeyHandle, DesiredAccess, ObjectAttributes, TitleIndex, Class, CreateOptions, Disposition);

	RecoverPreMode(EThread, PreMode);
	return Status;
}


NTSTATUS RegSetValueKey(PVOID InBuffer,PVOID OutBuffer)
{
	PSET_KEY_VALUE SetKeyValue = (PSET_KEY_VALUE)InBuffer;
	HANDLE KeyHandle = SetKeyValue->hKey;
	PUNICODE_STRING uniValueName = (PUNICODE_STRING)(SetKeyValue->ValueName);
	ULONG Type = SetKeyValue->Type;
	PVOID Data = SetKeyValue->Data;
	ULONG DataSize = SetKeyValue->DataSize;
	return KernelSetValueKey(KeyHandle, uniValueName, 0, Type, Data, DataSize);
}

NTSTATUS 
	KernelSetValueKey(IN HANDLE  KeyHandle,
	IN PUNICODE_STRING  ValueName,
	IN ULONG  TitleIndex  OPTIONAL,
	IN ULONG  Type,
	IN PVOID  Data,
	IN ULONG  DataSize
	)
{
	
	PETHREAD EThread = NULL;
	CHAR PreMode = 0;
	NTSTATUS Status = STATUS_UNSUCCESSFUL;

	if (!KeyHandle)
	{
		return Status;
	}

	EThread = PsGetCurrentThread();
	PreMode = ChangePreMode(EThread);

	Status = NtSetValueKeyAddress(KeyHandle, ValueName, TitleIndex, Type, Data, DataSize);

	RecoverPreMode(EThread, PreMode);
	return Status;
}

NTSTATUS RegDeleteKey(PVOID InBuffer,PVOID OutBuffer)
{
	PDELETE Delete = (PDELETE)InBuffer;
	HANDLE KeyHandle = Delete->hKey;

	return KernelDeleteKey(KeyHandle);
}



NTSTATUS 
	KernelDeleteKey(
	IN HANDLE  KeyHandle
	)
{
	
	PETHREAD EThread = NULL;
	CHAR PreMode = 0;
	NTSTATUS Status = STATUS_UNSUCCESSFUL;

	if (!KeyHandle)
	{
		return Status;
	}

	EThread = PsGetCurrentThread();
	PreMode = ChangePreMode(EThread);

	Status = NtDeleteKeyAddress(KeyHandle);

	RecoverPreMode(EThread, PreMode);
	return Status;
}


NTSTATUS RegRenameKey(PVOID InBuffer,PVOID OutBuffer)
{
	PRENAME Rename = (PRENAME)InBuffer;
	HANDLE KeyHandle = Rename->hKey;
	PUNICODE_STRING uniNewName = (PUNICODE_STRING)(Rename->uniNewName);


	return KernelRenameKey(KeyHandle, uniNewName);
}




NTSTATUS KernelRenameKey(IN HANDLE KeyHandle, IN PUNICODE_STRING uniNewName)
{
	
	PETHREAD EThread = NULL;
	CHAR PreMode = 0;
	NTSTATUS Status = STATUS_UNSUCCESSFUL;

	if (!KeyHandle)
	{
		return Status;
	}

	EThread = PsGetCurrentThread();
	PreMode = ChangePreMode(EThread);

	Status = NtRenameKeyAddress(KeyHandle, uniNewName);

	RecoverPreMode(EThread, PreMode);
	return Status;
}


NTSTATUS RegDeleteValueKey(PVOID InBuffer,PVOID OutBuffer)
{
	PDELETE_KEY_VALUE DeleteKeyValue = (PDELETE_KEY_VALUE)InBuffer;
	HANDLE KeyHandle = DeleteKeyValue->hKey;
	PUNICODE_STRING uniValueName = (PUNICODE_STRING)(DeleteKeyValue->uniValueName);


	return KernelDeleteValueKey(KeyHandle, uniValueName);
}


NTSTATUS 
	KernelDeleteValueKey(IN HANDLE  KeyHandle,
	IN PUNICODE_STRING  uniValueName
	)
{
	
	PETHREAD EThread = NULL;
	CHAR PreMode = 0;
	NTSTATUS Status = STATUS_UNSUCCESSFUL;

	if (!KeyHandle)
	{
		return Status;
	}

	EThread = PsGetCurrentThread();
	PreMode = ChangePreMode(EThread);

	Status = NtDeleteValueKeyAddress(KeyHandle,uniValueName);

	RecoverPreMode(EThread, PreMode);
	return Status;
}


NTSTATUS DefaultDispatch(PDEVICE_OBJECT  DeviceObject,PIRP Irp)
{
	Irp->IoStatus.Status = STATUS_SUCCESS;
	Irp->IoStatus.Information = 0;
	IoCompleteRequest(Irp,IO_NO_INCREMENT);

	return STATUS_SUCCESS;
}

VOID UnloadDriver(PDRIVER_OBJECT DriverObject)
{
	UNICODE_STRING  uniLinkName;
	PDEVICE_OBJECT	NextObject = NULL;
	PDEVICE_OBJECT  CurrentObject = NULL;
	RtlInitUnicodeString(&uniLinkName,LINK_NAME);

	IoDeleteSymbolicLink(&uniLinkName);
	CurrentObject = DriverObject->DeviceObject;
	while (CurrentObject != NULL) 
	{
	
		NextObject = CurrentObject->NextDevice;
		IoDeleteDevice(CurrentObject);
		CurrentObject = NextObject;
	}

#ifdef WIN64
	DbgPrint("X64: RegisterManager IS STOPPED!!!\n");
#else
	DbgPrint("X86: RegisterManager IS STOPPED!!!\n");
#endif

	return;
}


CHAR ChangePreMode(PETHREAD EThread)
{

	CHAR PreMode = *(PCHAR)((ULONG_PTR)EThread + PreviousModeOffsetOf_KTHREAD);
	*(PCHAR)((ULONG_PTR)EThread + PreviousModeOffsetOf_KTHREAD) = KernelMode;
	return PreMode;
}

VOID RecoverPreMode(PETHREAD EThread, CHAR PreMode)
{

	*(PCHAR)((ULONG_PTR)EThread + PreviousModeOffsetOf_KTHREAD) = PreMode;
}



WIN_VERSION GetWindowsVersion()
{
	RTL_OSVERSIONINFOEXW osverInfo = {sizeof(osverInfo)}; 
	pfnRtlGetVersion RtlGetVersion = NULL;
	WIN_VERSION WinVersion;
	WCHAR wzRtlGetVersion[] = L"RtlGetVersion";

	RtlGetVersion = GetFunctionAddressByName(wzRtlGetVersion);    
	if (RtlGetVersion)
	{
		RtlGetVersion((PRTL_OSVERSIONINFOW)&osverInfo); 
	} 
	else 
	{
		PsGetVersion(&osverInfo.dwMajorVersion, &osverInfo.dwMinorVersion, &osverInfo.dwBuildNumber, NULL);
	}

	DbgPrint("Build Number: %d\r\n", osverInfo.dwBuildNumber);

	if (osverInfo.dwMajorVersion == 5 && osverInfo.dwMinorVersion == 1) 
	{
		DbgPrint("WINDOWS_XP\r\n");
		WinVersion = WINDOWS_XP;
	}
	else if (osverInfo.dwMajorVersion == 6 && osverInfo.dwMinorVersion == 1)
	{
		DbgPrint("WINDOWS 7\r\n");
		WinVersion = WINDOWS_7;
	}
	else if (osverInfo.dwMajorVersion == 6 && 
		osverInfo.dwMinorVersion == 2 &&
		osverInfo.dwBuildNumber == 9200)
	{
		DbgPrint("WINDOWS 8\r\n");
		WinVersion = WINDOWS_8;
	}
	else if (osverInfo.dwMajorVersion == 6 && 
		osverInfo.dwMinorVersion == 3 && 
		osverInfo.dwBuildNumber == 9600)
	{
		DbgPrint("WINDOWS 8.1\r\n");
		WinVersion = WINDOWS_8_1;
	}
	else
	{
		DbgPrint("WINDOWS_UNKNOW\r\n");
		WinVersion = WINDOWS_UNKNOW;
	}

	return WinVersion;
}
VOID SetGolbalMember()
{
	switch(WinVersion)
	{
	case WINDOWS_XP:
		{
			SYSTEM_ADDRESS_START = 0x80000000;


			PreviousModeOffsetOf_KTHREAD = 0x140;
			IndexOffset = 1;
			SSDTDescriptor = (ULONG_PTR)GetFunctionAddressByName(L"KeServiceDescriptorTable");
			//获得NtQueryObject函数的地址
			ulIndex = GetSSDTApiFunIndex("NtOpenKey");
			SSDTFuncAddress =  GetSSDTApiFunAddress(ulIndex,SSDTDescriptor);
			NtOpenKeyAddress  = (pfnNtOpenKey)SSDTFuncAddress;


			ulIndex = GetSSDTApiFunIndex("NtEnumerateKey");
			SSDTFuncAddress =  GetSSDTApiFunAddress(ulIndex,SSDTDescriptor);
			NtEnumerateKeyAddress  = (pfnNtEnumerateKey)SSDTFuncAddress;

			
			ulIndex = GetSSDTApiFunIndex("NtEnumerateValueKey");
			SSDTFuncAddress =  GetSSDTApiFunAddress(ulIndex,SSDTDescriptor);
			NtEnumerateValueKeyAddress  = (pfnNtEnumerateValueKey)SSDTFuncAddress;


			ulIndex = GetSSDTApiFunIndex("NtCreateKey");
			SSDTFuncAddress =  GetSSDTApiFunAddress(ulIndex,SSDTDescriptor);
			NtCreateKeyAddress  = (pfnNtCreateKey)SSDTFuncAddress;



			ulIndex = GetSSDTApiFunIndex("NtSetValueKey");
			SSDTFuncAddress =  GetSSDTApiFunAddress(ulIndex,SSDTDescriptor);
			NtSetValueKeyAddress  = (pfnNtSetValueKey)SSDTFuncAddress;


			ulIndex = GetSSDTApiFunIndex("NtDeleteKey");
			SSDTFuncAddress =  GetSSDTApiFunAddress(ulIndex,SSDTDescriptor);
			NtDeleteKeyAddress  = (pfnNtDeleteKey)SSDTFuncAddress;


			ulIndex = GetSSDTApiFunIndex("NtRenameKey");     
			SSDTFuncAddress =  GetSSDTApiFunAddress(ulIndex,SSDTDescriptor);
			NtRenameKeyAddress  = (pfnNtRenameKey)SSDTFuncAddress;

			ulIndex = GetSSDTApiFunIndex("NtDeleteValueKey");     
			SSDTFuncAddress =  GetSSDTApiFunAddress(ulIndex,SSDTDescriptor);
			NtDeleteValueKeyAddress  = (pfnNtDeleteValueKey)SSDTFuncAddress;

			break;
		}
	case WINDOWS_7:
		{

			PreviousModeOffsetOf_KTHREAD = 0x1f6;
			IndexOffset = 4;
			SSDTDescriptor = GetKeServiceDescriptorTable64();

			ulIndex = GetSSDTApiFunIndex("NtOpenKey");
			SSDTFuncAddress =  GetSSDTApiFunAddress(ulIndex,SSDTDescriptor);
			NtOpenKeyAddress  = (pfnNtOpenKey)SSDTFuncAddress;

			ulIndex = GetSSDTApiFunIndex("NtEnumerateKey");
			SSDTFuncAddress =  GetSSDTApiFunAddress(ulIndex,SSDTDescriptor);
			NtEnumerateKeyAddress  = (pfnNtEnumerateKey)SSDTFuncAddress;

			ulIndex = GetSSDTApiFunIndex("NtEnumerateValueKey");
			SSDTFuncAddress =  GetSSDTApiFunAddress(ulIndex,SSDTDescriptor);
			NtEnumerateValueKeyAddress  = (pfnNtEnumerateValueKey)SSDTFuncAddress;


			ulIndex = GetSSDTApiFunIndex("NtCreateKey");
			SSDTFuncAddress =  GetSSDTApiFunAddress(ulIndex,SSDTDescriptor);
			NtCreateKeyAddress  = (pfnNtCreateKey)SSDTFuncAddress;

			ulIndex = GetSSDTApiFunIndex("NtSetValueKey");
			SSDTFuncAddress =  GetSSDTApiFunAddress(ulIndex,SSDTDescriptor);
			NtSetValueKeyAddress  = (pfnNtSetValueKey)SSDTFuncAddress;


			ulIndex = GetSSDTApiFunIndex("NtDeleteKey");
			SSDTFuncAddress =  GetSSDTApiFunAddress(ulIndex,SSDTDescriptor);
			NtDeleteKeyAddress  = (pfnNtDeleteKey)SSDTFuncAddress;

			ulIndex = GetSSDTApiFunIndex("NtRenameKey");
			SSDTFuncAddress =  GetSSDTApiFunAddress(ulIndex,SSDTDescriptor);
			NtRenameKeyAddress  = (pfnNtRenameKey)SSDTFuncAddress;

			ulIndex = GetSSDTApiFunIndex("NtDeleteValueKey");     
			SSDTFuncAddress =  GetSSDTApiFunAddress(ulIndex,SSDTDescriptor);
			NtDeleteValueKeyAddress  = (pfnNtDeleteValueKey)SSDTFuncAddress;
			SYSTEM_ADDRESS_START = 0x80000000000;
			break;
		}
	}
}

PVOID GetFunctionAddressByName(WCHAR *wzFunction)
{
	UNICODE_STRING uniFunction;  
	PVOID AddrBase = NULL;

	if (wzFunction && wcslen(wzFunction) > 0)
	{
		RtlInitUnicodeString(&uniFunction, wzFunction);     
		AddrBase = MmGetSystemRoutineAddress(&uniFunction); 
	}

	return AddrBase;
}


ULONG_PTR GetKeServiceDescriptorTable64()
{
	PUCHAR StartSearchAddress = (PUCHAR)__readmsr(0xC0000082);
	PUCHAR EndSearchAddress = StartSearchAddress + 0x500;
	PUCHAR i = NULL;
	UCHAR b1=0,b2=0,b3=0;
	ULONG_PTR Temp = 0;
	ULONG_PTR Address = 0;
	for(i=StartSearchAddress;i<EndSearchAddress;i++)
	{
		if( MmIsAddressValid(i) && MmIsAddressValid(i+1) && MmIsAddressValid(i+2) )
		{
			b1=*i;
			b2=*(i+1);
			b3=*(i+2);
			if( b1==0x4c && b2==0x8d && b3==0x15 ) //4c8d15
			{
				memcpy(&Temp,i+3,4);
				Address = (ULONG_PTR)Temp + (ULONG_PTR)i + 7;
				return Address;
			}
		}
	}
	return 0;
}




LONG GetSSDTApiFunIndex(IN LPSTR lpszFunName)
{
	LONG Index = -1;
	NTSTATUS Status = STATUS_UNSUCCESSFUL;
	PVOID    MapBase = NULL;
	PIMAGE_NT_HEADERS  NtHeader;
	PIMAGE_EXPORT_DIRECTORY ExportTable;
	ULONG*  FunctionAddresses;
	ULONG*  FunctionNames;
	USHORT* FunIndexs;
	ULONG   ulFunIndex;
	ULONG   i;
	CHAR*   FunName;
	SIZE_T  ViewSize=0;
	ULONG_PTR FunAddress;
	WCHAR wzNtdll[] = L"\\SystemRoot\\System32\\ntdll.dll";

	Status = MapFileInUserSpace(wzNtdll, NtCurrentProcess(), &MapBase, &ViewSize);
	if (!NT_SUCCESS(Status))
	{

		return STATUS_UNSUCCESSFUL;

	}
	else
	{
		__try{
			NtHeader = RtlImageNtHeader(MapBase);
			if (NtHeader && NtHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress){
				ExportTable =(IMAGE_EXPORT_DIRECTORY *)((ULONG_PTR)MapBase + NtHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress);
				FunctionAddresses = (ULONG*)((ULONG_PTR)MapBase + ExportTable->AddressOfFunctions);
				FunctionNames = (ULONG*)((ULONG_PTR)MapBase + ExportTable->AddressOfNames);
				FunIndexs = (USHORT*)((ULONG_PTR)MapBase + ExportTable->AddressOfNameOrdinals);
				for(i = 0; i < ExportTable->NumberOfNames; i++)
				{
					FunName = (LPSTR)((ULONG_PTR)MapBase + FunctionNames[i]);
					if (_stricmp(FunName, lpszFunName) == 0) 
					{
						ulFunIndex = FunIndexs[i]; 
						FunAddress = (ULONG_PTR)((ULONG_PTR)MapBase + FunctionAddresses[ulFunIndex]);
						Index=*(ULONG*)(FunAddress+IndexOffset);
						break;
					}
				}
			}
		}__except(EXCEPTION_EXECUTE_HANDLER)
		{
			;
		}
	}

	if (Index == -1)
	{
		DbgPrint("%s Get Index Error\n", lpszFunName);
	}

	ZwUnmapViewOfSection(NtCurrentProcess(), MapBase);
	return Index;
}




NTSTATUS 
MapFileInUserSpace(IN LPWSTR lpszFileName,IN HANDLE ProcessHandle OPTIONAL,
	OUT PVOID *BaseAddress,
	OUT PSIZE_T ViewSize OPTIONAL)
{
	NTSTATUS Status = STATUS_INVALID_PARAMETER;
	HANDLE   hFile = NULL;
	HANDLE   hSection = NULL;
	OBJECT_ATTRIBUTES oa;
	SIZE_T MapViewSize = 0;
	IO_STATUS_BLOCK Iosb;
	UNICODE_STRING uniFileName;

	if (!lpszFileName || !BaseAddress){
		return Status;
	}

	RtlInitUnicodeString(&uniFileName, lpszFileName);
	InitializeObjectAttributes(&oa,
		&uniFileName,
		OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
		NULL,
		NULL
		);

	Status = IoCreateFile(&hFile,
		GENERIC_READ | SYNCHRONIZE,
		&oa,
		&Iosb,
		NULL,
		FILE_ATTRIBUTE_NORMAL,
		FILE_SHARE_READ,
		FILE_OPEN,
		FILE_SYNCHRONOUS_IO_NONALERT,
		NULL,
		0,
		CreateFileTypeNone,
		NULL,
		IO_NO_PARAMETER_CHECKING
		);

	if (!NT_SUCCESS(Status))
	{
		DbgPrint("ZwCreateFile Failed! Error=%08x\n",Status);
		return Status;
	}

	oa.ObjectName = NULL;
	Status = ZwCreateSection(&hSection,
		SECTION_QUERY | SECTION_MAP_READ,
		&oa,
		NULL,
		PAGE_WRITECOPY,
		SEC_IMAGE,
		hFile
		);
	ZwClose(hFile);
	if (!NT_SUCCESS(Status))
	{
		DbgPrint("ZwCreateSection Failed! Error=%08x\n",Status);
		return Status;

	}

	if (!ProcessHandle){
		ProcessHandle = NtCurrentProcess();
	}

	Status = ZwMapViewOfSection(hSection, 
		ProcessHandle, 
		BaseAddress, 
		0, 
		0, 
		0, 
		ViewSize ? ViewSize : &MapViewSize, 
		ViewUnmap, 
		0, 
		PAGE_WRITECOPY
		);
	ZwClose(hSection);
	if (!NT_SUCCESS(Status))
	{
		DbgPrint("ZwMapViewOfSection Failed! Error=%08x\n",Status);
		return Status;
	}

	return Status;
}




ULONG_PTR GetSSDTApiFunAddress(ULONG_PTR ulIndex,ULONG_PTR SSDTDescriptor)
{
	ULONG_PTR  SSDTFuncAddress = 0;
	switch(WinVersion)
	{
	case WINDOWS_7:
		{
			SSDTFuncAddress = GetSSDTFunctionAddress64(ulIndex,SSDTDescriptor);
			break;
		}

	case WINDOWS_XP:
		{
			SSDTFuncAddress = GetSSDTFunctionAddress32(ulIndex,SSDTDescriptor);
			break;
		}
	}
}


ULONG_PTR GetSSDTFunctionAddress32(ULONG_PTR ulIndex,ULONG_PTR SSDTDescriptor)
{
	ULONG_PTR ServiceTableBase= 0 ;
	PSYSTEM_SERVICE_TABLE32 SSDT = (PSYSTEM_SERVICE_TABLE32)SSDTDescriptor;

	ServiceTableBase=(ULONG_PTR)(SSDT ->ServiceTableBase);

	return (ULONG_PTR)(((ULONG*)ServiceTableBase)[(ULONG)ulIndex]);
}

ULONG_PTR GetSSDTFunctionAddress64(ULONG_PTR ulIndex,ULONG_PTR SSDTDescriptor)
{
	LONG dwTemp=0;
	ULONG_PTR qwTemp=0;
	ULONG_PTR ServiceTableBase= 0 ;
	ULONG_PTR FuncAddress =0;
	PSYSTEM_SERVICE_TABLE64 SSDT = (PSYSTEM_SERVICE_TABLE64)SSDTDescriptor;
	ServiceTableBase=(ULONG_PTR)(SSDT ->ServiceTableBase);
	qwTemp = ServiceTableBase + 4 * ulIndex;
	dwTemp = *(PLONG)qwTemp;
	dwTemp = dwTemp>>4;
	FuncAddress = ServiceTableBase + (ULONG_PTR)dwTemp;
	return FuncAddress;
}



