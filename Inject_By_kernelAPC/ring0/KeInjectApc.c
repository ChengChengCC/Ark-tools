

#ifndef CXX_KEINJECTAPC_H
#	include "KeInjectApc.h"
#endif


ULONG ApcStateOffset; 
PLDR_LOAD_DLL LdrLoadDll; 


NTSTATUS DriverEntry(PDRIVER_OBJECT DriverObject,PUNICODE_STRING pRegistryPath)
{
	NTSTATUS Status;
	PDEVICE_OBJECT DeviceObject;
	PEPROCESS Process;
	PETHREAD Thread;
	PKAPC_STATE ApcState;

	PVOID KdVersionBlock,NtdllBase;
	PULONG ptr,Functions,Names;
	PUSHORT Ordinals;

	PLDR_DATA_TABLE_ENTRY MmLoadedUserImageList,ModuleEntry;
	ULONG i;

	PIMAGE_DOS_HEADER pIDH;
	PIMAGE_NT_HEADERS pINH;
	PIMAGE_EXPORT_DIRECTORY pIED;

	UNICODE_STRING   uniDeviceName;
	UNICODE_STRING   uniLinkName;

	RtlInitUnicodeString(&uniDeviceName,DEVICE_NAME);

	RtlInitUnicodeString(&uniLinkName,LINK_NAME);

	for (i=0;i<IRP_MJ_MAXIMUM_FUNCTION;i++)
	{
		DriverObject->MajorFunction[i] = DefaultPassThrough;
	}
	DriverObject->DriverUnload = UnloadDriver;

	DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = DriverDispatch;

	//创建设备对象
	Status = IoCreateDevice(DriverObject,0,&uniDeviceName,FILE_DEVICE_UNKNOWN,0,FALSE,&DeviceObject);

	if (!NT_SUCCESS(Status))
	{

		return Status;
	}

	Status = IoCreateSymbolicLink(&uniLinkName,&uniDeviceName);

	if (!NT_SUCCESS(Status))
	{
		IoDeleteDevice(DeviceObject);

		return Status;
	}

	//使当前线程运行在第一个处理器上
	KeSetSystemAffinityThread(1);
	KdVersionBlock=(PVOID)__readfsdword(0x34); //得到KdVersionBlock
	KeRevertToUserAffinityThread();//恢复线程运行的处理器
	MmLoadedUserImageList=*(PLDR_DATA_TABLE_ENTRY*)((PUCHAR)KdVersionBlock+0x228); // Get the MmLoadUserImageList

	/*
	kd> !pcr
	KPCR for Processor 0 at 83f3ec00:


	kd> dt _kpcr 83f3ec00
	+0x034 KdVersionBlock   : 0x83f3dc00 Void

	kd> dd 0x83f3dc00+0x228
	83f3de28  83f5de38 00000000 83e5dfa8 00000000
	83f3de38  00000000 00000000 83f7d8c0 00000000
	83f3de48  83f7d560 00000000 83f5d84c 00000000


	kd> dd 83f5de38
	83f5de38  8706b1e8 877cb660 00000000 00000000
	83f5de48  00000000 00000000 00040107 00000000
	83f5de58  865d0690 865d0690 c0403188 0007ff7e

	kd> dt _LDR_DATA_TABLE_ENTRY 8706b1e8
	nt!_LDR_DATA_TABLE_ENTRY
	+0x000 InLoadOrderLinks : _LIST_ENTRY [ 0x8713b4e0 - 0x83f5de38 ]
	+0x008 InMemoryOrderLinks : _LIST_ENTRY [ 0x0 - 0x0 ]
	+0x010 InInitializationOrderLinks : _LIST_ENTRY [ 0x0 - 0x0 ]
	+0x018 DllBase          : 0x77ce0000 Void
	+0x01c EntryPoint       : (null) 
	+0x020 SizeOfImage      : 0x13c000
	+0x024 FullDllName      : _UNICODE_STRING "\Windows\System32\ntdll.dll"
	+0x02c BaseDllName      : _UNICODE_STRING ""
	+0x034 Flags            : 0
	+0x038 LoadCount        : 1
	+0x03a TlsIndex         : 0
	+0x03c HashLinks        : _LIST_ENTRY [ 0x0 - 0x1490d9 ]
	+0x03c SectionPointer   : (null) 
	+0x040 CheckSum         : 0x1490d9
	+0x044 TimeDateStamp    : 0
	+0x044 LoadedImports    : (null) 
	+0x048 EntryPointActivationContext : (null) 
	+0x04c PatchInformation : (null) 
	+0x050 ForwarderLinks   : _LIST_ENTRY [ 0x0 - 0x0 ]
	+0x058 ServiceTagLinks  : _LIST_ENTRY [ 0x0 - 0x57005c ]
	+0x060 StaticLinks      : _LIST_ENTRY [ 0x6e0069 - 0x6f0064 ]
	+0x068 ContextInformation : 0x00730077 Void
	+0x06c OriginalBase     : 0x53005c
	+0x070 LoadTime         : _LARGE_INTEGER 0x650074`00730079

	*/
	DbgPrint("KdVersionBlock address: %#x",KdVersionBlock);
	DbgPrint("MmLoadedUserImageList address: %#x",MmLoadedUserImageList);

	ModuleEntry=(PLDR_DATA_TABLE_ENTRY)MmLoadedUserImageList->InLoadOrderLinks.Flink; //第一模块
	NtdllBase=ModuleEntry->DllBase; //ntdll基地址

	DbgPrint("ntdll base address: %#x",NtdllBase);

	pIDH=(PIMAGE_DOS_HEADER)NtdllBase;
	pINH=(PIMAGE_NT_HEADERS)((PUCHAR)NtdllBase+pIDH->e_lfanew);
	pIED=(PIMAGE_EXPORT_DIRECTORY)((PUCHAR)NtdllBase+pINH->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress);

	Functions=(PULONG)((PUCHAR)NtdllBase+pIED->AddressOfFunctions);
	Names=(PULONG)((PUCHAR)NtdllBase+pIED->AddressOfNames);

	Ordinals=(PUSHORT)((PUCHAR)NtdllBase+pIED->AddressOfNameOrdinals);

	//搜索LdrLoadDll
	for(i=0;i<pIED->NumberOfNames;i++)
	{
		if(!strcmp((char*)NtdllBase+Names[i],"LdrLoadDll"))
		{
			LdrLoadDll=(PLDR_LOAD_DLL)((PUCHAR)NtdllBase+Functions[Ordinals[i]]);
			break;
		}
	}

	DbgPrint("LdrLoadDll address: %#x",LdrLoadDll);

	Process=PsGetCurrentProcess();
	Thread=PsGetCurrentThread();

	ptr=(PULONG)Thread;

	//确定ApcState在EThread中的偏移
	for(i=0;i<512;i++)
	{
		if(ptr[i]==(ULONG)Process)
		{
			ApcState=CONTAINING_RECORD(&ptr[i],KAPC_STATE,Process); 
			ApcStateOffset=(ULONG)ApcState-(ULONG)Thread; 
			break;
		}
	}

	DbgPrint("ApcState offset: %#x",ApcStateOffset);
	DbgPrint("DLL injection driver loaded.");
	return STATUS_SUCCESS;
}




NTSTATUS DefaultPassThrough(PDEVICE_OBJECT  DeviceObject,PIRP Irp)
{
	Irp->IoStatus.Information = 0;
	Irp->IoStatus.Status = STATUS_SUCCESS;

	IoCompleteRequest(Irp,IO_NO_INCREMENT);

	return STATUS_SUCCESS;
}


void UnloadDriver(PDRIVER_OBJECT DriverObject)
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


void NTAPI InjectDllApc(PVOID NormalContext,PVOID SystemArgument1,PVOID SystemArgument2)
{
	PKINJECT inject=(PKINJECT)NormalContext;

	inject->LdrLoadDll(NULL,NULL,&inject->DllName,&inject->DllBase);
	inject->Executed=TRUE;
}


void NTAPI KernelRoutine(PKAPC apc,PKNORMAL_ROUTINE* NormalRoutine,PVOID* NormalContext,\
	PVOID* SystemArgument1,PVOID* SystemArgument2)
{
	ExFreePool(apc);
}

BOOLEAN InjectDll(PINJECT_INFO InjectInfo)
{
	PEPROCESS Process;
	PETHREAD Thread;
	PKINJECT mem;
    ULONG size;
	PKAPC_STATE ApcState;
	PKAPC apc;
	PVOID buffer;
	PSYSTEM_PROCESS_INFO pSpi;
	LARGE_INTEGER delay;
	buffer=ExAllocatePool(NonPagedPool,1024*1024); 
	if(!buffer)
	{
		DbgPrint("Error: Unable to allocate memory for the process thread list.");
		return FALSE;
	}

	//5	SystemProcessInformation,
	if(!NT_SUCCESS(ZwQuerySystemInformation(5,buffer,1024*1024,NULL)))
	{
		DbgPrint("Error: Unable to query process thread list.");

		ExFreePool(buffer);
		return FALSE;
	}

	pSpi=(PSYSTEM_PROCESS_INFO)buffer;

	//找到目标进程
	while(pSpi->NextEntryOffset)
	{
		if(pSpi->UniqueProcessId==InjectInfo->ProcessId)
		{
			DbgPrint("Target thread found. TID: %d",pSpi->Threads[0].ClientId.UniqueThread);
			break;
		}

		pSpi=(PSYSTEM_PROCESS_INFO)((PUCHAR)pSpi+pSpi->NextEntryOffset);
	}

	// 引用目标进程EProcess,
	if(!NT_SUCCESS(PsLookupProcessByProcessId(InjectInfo->ProcessId,&Process)))
	{
		DbgPrint("Error: Unable to reference the target process.");
		ExFreePool(buffer);
		return FALSE;
	}

	DbgPrint("Process name: %s",PsGetProcessImageFileName(Process));
	DbgPrint("EPROCESS address: %#x",Process);

	//目标进程主线程
	if(!NT_SUCCESS(PsLookupThreadByThreadId(pSpi->Threads[0].ClientId.UniqueThread,&Thread)))
	{
		DbgPrint("Error: Unable to reference the target thread.");
		ObDereferenceObject(Process); 
		ExFreePool(buffer); 
		return FALSE;
	}

	DbgPrint("ETHREAD address: %#x",Thread);

	ExFreePool(buffer); 
	//切入到目标进程
	KeAttachProcess(Process); 

	mem=NULL;
	size=4096;

	//在目标进程申请内存
	if(!NT_SUCCESS(ZwAllocateVirtualMemory(NtCurrentProcess(),(PVOID*)&mem,0,&size,MEM_COMMIT|MEM_RESERVE,PAGE_EXECUTE_READWRITE)))
	{
		DbgPrint("Error: Unable to allocate memory in the target process.");
		KeDetachProcess(); 

		ObDereferenceObject(Process);
		ObDereferenceObject(Thread); 
		return FALSE;
	}

	DbgPrint("Memory allocated at %#x",mem);
	mem->LdrLoadDll=LdrLoadDll; 
	wcscpy(mem->Buffer,InjectInfo->DllName); 
	RtlInitUnicodeString(&mem->DllName,mem->Buffer); 
	ApcState=(PKAPC_STATE)((PUCHAR)Thread+ApcStateOffset); 
	ApcState->UserApcPending=TRUE;   
	memcpy((PKINJECT)(mem+1),InjectDllApc,(ULONG)KernelRoutine-(ULONG)InjectDllApc); 
	DbgPrint("APC code address: %#x",(PKINJECT)(mem+1));

	//申请apc对象
	apc=(PKAPC)ExAllocatePool(NonPagedPool,sizeof(KAPC)); 

	if(!apc)
	{
		DbgPrint("Error: Unable to allocate the APC object.");
		size=0;
		ZwFreeVirtualMemory(NtCurrentProcess(),(PVOID*)&mem,&size,MEM_RELEASE);  
		KeDetachProcess();
		ObDereferenceObject(Process); 
		ObDereferenceObject(Thread); 
		return FALSE;
	}

	KeInitializeApc(apc,
		Thread,    //目标进程主线程
		OriginalApcEnvironment,   //目标apcz状态
		KernelRoutine,  //内核apc总入口
		NULL,       //Rundown Rounine=NULL
		(PKNORMAL_ROUTINE)((PKINJECT)mem+1),   //用户空间的总apc
		UserMode,   //插入到用户apc队列
		mem); // 自己的apc队列

	DbgPrint("Inserting APC to target thread");

	// 插入apc队列
	if(!KeInsertQueueApc(apc,NULL,NULL,IO_NO_INCREMENT))
	{
		DbgPrint("Error: Unable to insert APC to target thread.");
		size=0;
		ZwFreeVirtualMemory(NtCurrentProcess(),(PVOID*)&mem,&size,MEM_RELEASE); 
		KeDetachProcess(); 
		ObDereferenceObject(Process); 
		ObDereferenceObject(Thread); 
		ExFreePool(apc); 
		return FALSE;
	}

	delay.QuadPart=-100*10000;
	while(!mem->Executed)
	{
		KeDelayExecutionThread(KernelMode,FALSE,&delay);  //等待apc执行 
	}
	if(!mem->DllBase)
	{
		DbgPrint("Error: Unable to inject DLL into target process.");
		size=0;
		ZwFreeVirtualMemory(NtCurrentProcess(),(PVOID*)&mem,&size,MEM_RELEASE);
		KeDetachProcess();
		ObDereferenceObject(Process);
		ObDereferenceObject(Thread);
		return FALSE;
	}

	DbgPrint("DLL injected at %#x",mem->DllBase);
	size=0;
	ZwFreeVirtualMemory(NtCurrentProcess(),(PVOID*)&mem,&size,MEM_RELEASE); 
	ObDereferenceObject(Process); 
	ObDereferenceObject(Thread); 
	return TRUE;
}

NTSTATUS DriverDispatch(PDEVICE_OBJECT DeviceObject,PIRP Irp)
{
	PIO_STACK_LOCATION io;
	PINJECT_INFO InjectInfo;
	NTSTATUS  Status = STATUS_SUCCESS;
	PIO_STACK_LOCATION   IrpSp;
	PVOID     InputBuffer  = NULL;
	PVOID     OutputBuffer = NULL;
	ULONG_PTR InputSize  = 0;
	ULONG_PTR OutputSize = 0;
	ULONG_PTR IoControlCode = 0;

	IrpSp = IoGetCurrentIrpStackLocation(Irp);
 	InputBuffer = OutputBuffer = Irp->AssociatedIrp.SystemBuffer;
 	InputSize = IrpSp->Parameters.DeviceIoControl.InputBufferLength;
 	OutputSize  = IrpSp->Parameters.DeviceIoControl.OutputBufferLength;
	IoControlCode = IrpSp->Parameters.DeviceIoControl.IoControlCode;

	switch(IoControlCode)
	{
	case CTL_KEINJECTAPC:

		InjectInfo=(PINJECT_INFO)InputBuffer;

		if(!InjectInfo)
		{
			Status=STATUS_INSUFFICIENT_RESOURCES;
			break;
		}

		if(!InjectDll(InjectInfo))
		{
			Status=STATUS_UNSUCCESSFUL;
			break;
		}

		Status=STATUS_SUCCESS;
		Irp->IoStatus.Information=0;

		break;

	default:
		Status=STATUS_INVALID_DEVICE_REQUEST;
		break;
	}

	Irp->IoStatus.Status=Status;

	IoCompleteRequest(Irp,IO_NO_INCREMENT);
	return Status;
}
