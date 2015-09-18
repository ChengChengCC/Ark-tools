/***************************************************************************************
* AUTHOR : 懒人
* DATE   : 2014-10-25
* MODULE : DpcTimerDrv.C
* 
* Command: 
*	Source of IOCTRL Sample Driver
*
* Description:
*		Demonstrates communications between USER and KERNEL.
*
****************************************************************************************
* Copyright (C) 2010 懒人.
****************************************************************************************/

//#######################################################################################
//# I N C L U D E S
//#######################################################################################


#include "DpcTimerDrv.h"

PDRIVER_OBJECT g_DriverObject = NULL;
PVOID Ntoskrnl_KLDR_DATA_TABLE_ENTRY = NULL;

//////////////////////////////////////////////////////////////////////////

//#######################################################################################
//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
//@@@@@@@@				D R I V E R   E N T R Y   P O I N T						 @@@@@@@@
//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
//#######################################################################################
NTSTATUS
DriverEntry(IN PDRIVER_OBJECT DriverObject, IN PUNICODE_STRING RegistryString)
{
	NTSTATUS  Status;
	UNICODE_STRING   uniDeviceName;
	UNICODE_STRING   uniLinkName;
	ULONG_PTR i = 0;
	PDEVICE_OBJECT   DeviceObject = NULL;
	RtlInitUnicodeString(&uniDeviceName,DEVICE_NAME);
	RtlInitUnicodeString(&uniLinkName,LINK_NAME);
	for (i=0;i<IRP_MJ_MAXIMUM_FUNCTION;i++)
	{
		DriverObject->MajorFunction[i] = DefaultDispatchFunction;
	}
	DriverObject->DriverUnload = UnloadDriver;
	DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = DispatchControl;
	//创建设备对象
	Status = IoCreateDevice(DriverObject,0,&uniDeviceName,FILE_DEVICE_UNKNOWN,0,FALSE,&DeviceObject);
	if (!NT_SUCCESS(Status))
	{
		return STATUS_UNSUCCESSFUL;
	}
	Status = IoCreateSymbolicLink(&uniLinkName,&uniDeviceName);
	if (!NT_SUCCESS(Status))
	{
		IoDeleteDevice(DeviceObject);
		return STATUS_UNSUCCESSFUL;
	}

	g_DriverObject = DriverObject;

	return STATUS_SUCCESS;

}


NTSTATUS DefaultDispatchFunction(PDEVICE_OBJECT  DeviceObject,PIRP Irp)
{
	Irp->IoStatus.Information = 0;
	Irp->IoStatus.Status = STATUS_SUCCESS;
	IoCompleteRequest(Irp,IO_NO_INCREMENT);
	return STATUS_SUCCESS;
}


NTSTATUS DispatchControl(PDEVICE_OBJECT  DeviceObject,PIRP Irp)
{
	NTSTATUS  Status = STATUS_SUCCESS;
	PIO_STACK_LOCATION   IrpSp;
	PVOID     InputBuffer  = NULL;
	PVOID     OutputBuffer = NULL;
	ULONG_PTR InputSize  = 0;
	ULONG_PTR OutputSize = 0;
	ULONG_PTR IoControlCode = 0;

	IrpSp = IoGetCurrentIrpStackLocation(Irp);
	InputBuffer = IrpSp->Parameters.DeviceIoControl.Type3InputBuffer;
	OutputBuffer = Irp->UserBuffer;
	InputSize = IrpSp->Parameters.DeviceIoControl.InputBufferLength;
	OutputSize  = IrpSp->Parameters.DeviceIoControl.OutputBufferLength;
	IoControlCode = IrpSp->Parameters.DeviceIoControl.IoControlCode;
	switch(IoControlCode)
	{

	case CTL_GET_DPCTIMER:
		{
			EnumDpcTimer(OutputBuffer);
			Irp->IoStatus.Information = 0;
			Status = Irp->IoStatus.Status = Status;
			break;
		}

	case CTL_REMOVEDPCTIMER:
		{

			Status =  RemoveDPCTimer(InputBuffer);
			Irp->IoStatus.Information = 0;
			Status = Irp->IoStatus.Status = Status;
			break;
		}

	case CTL_GET_DRIVER:
		{
			GetKernelLdrDataTableEntry(g_DriverObject);
			Status = EnumDrivers(OutputBuffer,OutputSize);
			Irp->IoStatus.Information = 0;
			Status = Irp->IoStatus.Status = Status;
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



NTSTATUS EnumDpcTimer(PVOID OutBuffer)
{
	NTSTATUS Status = STATUS_UNSUCCESSFUL;
	PDPC_TIMER_INFOR DpcTimerInfor = (PDPC_TIMER_INFOR)OutBuffer;
	GetDpcTimerInformation_x64(DpcTimerInfor);
	if (DpcTimerInfor->ulCnt >= DpcTimerInfor->ulRetCnt)
	{
		Status = STATUS_SUCCESS;
	}

	return Status;
}


KDPC* TransTimerDpcEx(
	IN PKTIMER InTimer,
	IN ULONGLONG InKiWaitNever,
	IN ULONGLONG InKiWaitAlways)
{
	ULONGLONG			RDX = (ULONGLONG)InTimer->Dpc;
	RDX ^= InKiWaitNever;
	RDX = _rotl64(RDX, (UCHAR)(InKiWaitNever & 0xFF));
	RDX ^= (ULONGLONG)InTimer;
	RDX = _byteswap_uint64(RDX);
	RDX ^= InKiWaitAlways;
	return (KDPC*)RDX;
}


NTSTATUS GetDpcTimerInformation_x64(PDPC_TIMER_INFOR DpcTimerInfor)
{
	ULONG CPUNumber = KeNumberProcessors;   //系统变量
	PUCHAR CurrentKPRCBAddress = NULL;            
	PUCHAR CurrentTimerTableEntry = NULL;
	PLIST_ENTRY CurrentEntry = NULL;
	PLIST_ENTRY NextEntry = NULL;
	PULONG64    KiWaitAlways = NULL;
	PULONG64    KiWaitNever  = NULL;
	int i = 0;
	int j = 0;
	int n = 0;
	PKTIMER Timer;
	typedef struct _KTIMER_TABLE_ENTRY
	{
		ULONG64			Lock;
		LIST_ENTRY		Entry;
		ULARGE_INTEGER	Time;
	} KTIMER_TABLE_ENTRY, *PKTIMER_TABLE_ENTRY;

	for(j=0; j<CPUNumber; j++)
	{
		KeSetSystemAffinityThread(j+1);   //使当前线程运行在第一个处理器上
		CurrentKPRCBAddress=(PUCHAR)__readmsr(0xC0000101) + 0x20;
		KeRevertToUserAffinityThread();   //恢复线程运行的处理器
		
		CurrentTimerTableEntry=(PUCHAR)(*(ULONG64*)CurrentKPRCBAddress + 0x2200 + 0x200);
		FindKiWaitFunc(&KiWaitNever,&KiWaitAlways);  //找KiWaitAlways 函数的地址
		for(i=0; i<0x100; i++)
		{
			CurrentEntry = (PLIST_ENTRY)(CurrentTimerTableEntry + sizeof(KTIMER_TABLE_ENTRY) * i + 8);
			NextEntry = CurrentEntry->Blink;
			if( MmIsAddressValid(CurrentEntry) && MmIsAddressValid(CurrentEntry) )
			{
				while( NextEntry != CurrentEntry )
				{
					PKDPC RealDpc;
					//获得首地址
					Timer = CONTAINING_RECORD(NextEntry,KTIMER,TimerListEntry);
					RealDpc=TransTimerDpcEx(Timer,*KiWaitNever,*KiWaitAlways);
					if( MmIsAddressValid(Timer)&&MmIsAddressValid(RealDpc)&&MmIsAddressValid(RealDpc->DeferredRoutine))
					{				
						if (DpcTimerInfor->ulCnt > DpcTimerInfor->ulRetCnt)
						{
							DpcTimerInfor->DpcTimer[n].Dpc = (ULONG64)RealDpc;
							DpcTimerInfor->DpcTimer[n].Period = Timer->Period;
							DpcTimerInfor->DpcTimer[n].TimeDispatch = (ULONG64)RealDpc->DeferredRoutine;
							DpcTimerInfor->DpcTimer[n].TimerObject = (ULONG64)Timer;
							n++;
						}					
						DpcTimerInfor->ulRetCnt++;					
					}
					NextEntry = NextEntry->Blink;
				}
			}
		}
	}
}



VOID FindKiWaitFunc(PULONG64 *KiWaitNeverAddr, PULONG64 *KiWaitAlwaysAddr)
{
	long Temp;
	PUCHAR StartAddress,i;
	UNICODE_STRING  uniFuncName;
	WCHAR wzFunName[] = L"KeSetTimer";
	RtlInitUnicodeString(&uniFuncName,wzFunName);
	StartAddress = (PUCHAR)MmGetSystemRoutineAddress(&uniFuncName);
	for(i=StartAddress; i<StartAddress+0xFF; i++)
	{
		if(*i==0x48 && *(i+1)==0x8B && *(i+2)==0x05)
		{
			memcpy(&Temp,i+3,4);
			*KiWaitNeverAddr=(PULONG64)((ULONGLONG)Temp + (ULONGLONG)i + 7);
			i=i+7;
			memcpy(&Temp,i+3,4);
			*KiWaitAlwaysAddr=(PULONG64)((ULONGLONG)Temp + (ULONGLONG)i + 7);
			return;
		}
	}
}

BOOLEAN GetKernelLdrDataTableEntry(PDRIVER_OBJECT DriverObject)  
{
	BOOLEAN bRet = FALSE;
	if (DriverObject)
	{
		PKLDR_DATA_TABLE_ENTRY Entry = NULL, FirstEntry = NULL;
		WCHAR wzNtoskrnl[] = L"ntoskrnl.exe";                                      
		int nLen = wcslen(wzNtoskrnl) * sizeof(WCHAR);
		FirstEntry = Entry = (PKLDR_DATA_TABLE_ENTRY)DriverObject->DriverSection;  

		while((PKLDR_DATA_TABLE_ENTRY)Entry->InLoadOrderLinks.Flink != FirstEntry)
		{

			if (Entry->BaseDllName.Buffer																			&& 
				nLen == Entry->BaseDllName.Length																	&&
				MmIsAddressValid((PVOID)Entry->BaseDllName.Buffer)													&&
				!_wcsnicmp(wzNtoskrnl,(WCHAR*)Entry->BaseDllName.Buffer, nLen / sizeof(WCHAR)))
			{
				Ntoskrnl_KLDR_DATA_TABLE_ENTRY = (PVOID)Entry;
				bRet = TRUE;
				break;
			}

			Entry = (PKLDR_DATA_TABLE_ENTRY)Entry->InLoadOrderLinks.Flink;
		}

		// 如果没找到ntoskrnl,那么使用自己
		if (!bRet)
		{
			Ntoskrnl_KLDR_DATA_TABLE_ENTRY = (PVOID)FirstEntry;
			bRet = TRUE;
		}
	}

	return bRet;
}




NTSTATUS EnumDrivers(PVOID OutBuffer, ULONG OutSize)
{
	NTSTATUS Status = STATUS_UNSUCCESSFUL;
	PALL_DRIVERS DriversInfor = (PALL_DRIVERS)OutBuffer;
	ULONG ulCount = (OutSize - sizeof(ALL_DRIVERS)) / sizeof(DRIVER_INFO);
	if (!OutBuffer)
	{
		return STATUS_INVALID_PARAMETER;
	}

	EnumDriverByLdrDataTableEntry(DriversInfor,ulCount);
	if (ulCount >= DriversInfor->ulCount)
	{
		Status = STATUS_SUCCESS;
	}
	else
	{
		Status = STATUS_BUFFER_TOO_SMALL;
	}

	return Status;
}




VOID EnumDriverByLdrDataTableEntry(PALL_DRIVERS DriversInfor, ULONG_PTR ulCount)
{
	PKLDR_DATA_TABLE_ENTRY Entry = NULL, FirstEntry = NULL;
	ULONG nMax = PAGE_SIZE;
	ULONG i = 0;
	KIRQL OldIrql;

	FirstEntry = Entry = (PKLDR_DATA_TABLE_ENTRY)Ntoskrnl_KLDR_DATA_TABLE_ENTRY;

	if (!FirstEntry || !DriversInfor)
	{
		return;
	}
	OldIrql = KeRaiseIrqlToDpcLevel();
	__try
	{
		do
		{
			if ((ULONG_PTR)Entry->DllBase > SYSTEM_ADDRESS_START && Entry->SizeOfImage > 0)
			{
				ULONG_PTR Temp = DriversInfor->ulCount;
				if (ulCount > Temp)
				{

					DriversInfor->Drivers[Temp].LodeOrder = ++i;
					DriversInfor->Drivers[Temp].Base = (ULONG_PTR)Entry->DllBase;
					DriversInfor->Drivers[Temp].Size = Entry->SizeOfImage;

					if (IsUnicodeStringValid(&(Entry->FullDllName)))
					{
						wcsncpy(DriversInfor->Drivers[Temp].wzDriverPath, Entry->FullDllName.Buffer, Entry->FullDllName.Length);
					}
					else if (IsUnicodeStringValid(&(Entry->BaseDllName)))
					{
						wcsncpy(DriversInfor->Drivers[Temp].wzDriverPath, Entry->BaseDllName.Buffer, Entry->BaseDllName.Length);
					}
				}
				DriversInfor->ulCount++;
			}

			Entry = (PKLDR_DATA_TABLE_ENTRY)Entry->InLoadOrderLinks.Flink;

		}while(Entry && Entry != FirstEntry && nMax--);	
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		DbgPrint("LdrDataTable Exception!\r\n");
	}

	KeLowerIrql(OldIrql);
}

BOOLEAN IsUnicodeStringValid(PUNICODE_STRING uniString)
{
	BOOLEAN bRet = FALSE;

	__try
	{
		if (uniString->Length > 0	&&
			uniString->Buffer		&&
			MmIsAddressValid(uniString->Buffer) &&
			MmIsAddressValid(&uniString->Buffer[uniString->Length / sizeof(WCHAR) - 1]))
		{
			bRet = TRUE;
		}

	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{	
		bRet = FALSE;
	}

	return bRet;
}



PVOID GetFunctionAddressByName(WCHAR *wzFunction)
{
	UNICODE_STRING uniFunction;  
	PVOID AddrBase = NULL;

	if (wzFunction && wcslen(wzFunction) > 0)
	{
		RtlInitUnicodeString(&uniFunction, wzFunction);      //常量指针
		AddrBase = MmGetSystemRoutineAddress(&uniFunction);  //在System 进程  第一个模块  Ntosknrl.exe  ExportTable
	}

	return AddrBase;
}



NTSTATUS RemoveDPCTimer(PVOID InBuffer)
{
	PREMOVE_DPCTIMER Temp = (PREMOVE_DPCTIMER)InBuffer;
	ULONG_PTR TimerObject = Temp->TimerObject;
	if (TimerObject&&MmIsAddressValid((PVOID)TimerObject))
	{

		if (KeCancelTimer((PKTIMER)TimerObject))
		{
			return STATUS_SUCCESS;
		}
	}
	return STATUS_UNSUCCESSFUL;
}



VOID UnloadDriver(PDRIVER_OBJECT DriverObject)
{
	UNICODE_STRING  uniLinkName;
	PDEVICE_OBJECT  CurrentDeviceObject;
	PDEVICE_OBJECT  NextDeviceObject;

	RtlInitUnicodeString(&uniLinkName,LINK_NAME);
	IoDeleteSymbolicLink(&uniLinkName);
	if (DriverObject->DeviceObject!=NULL)
	{
		CurrentDeviceObject = DriverObject->DeviceObject;
		while(CurrentDeviceObject!=NULL)
		{
			NextDeviceObject  = CurrentDeviceObject->NextDevice;
			IoDeleteDevice(CurrentDeviceObject);
			CurrentDeviceObject = NextDeviceObject;
		}
	}
	DbgPrint("UnloadDriver\r\n");
}