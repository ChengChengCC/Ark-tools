#include "Memory.h"
#include "GetFuncAddress.h"
#include <ntimage.h>

//////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////

extern BOOLEAN IsModuleInList(ULONG_PTR Base, ULONG_PTR Size, PALL_MODULES AllModules, ULONG_PTR ulCount);
extern VOID WalkerModuleList64(PLIST_ENTRY64 ListEntry, ULONG nType, PALL_MODULES AllModules, ULONG ulCount);
extern VOID WalkerModuleList32(PLIST_ENTRY32 ListEntry, ULONG nType, PALL_MODULES AllModules, ULONG ulCount);
extern NTSTATUS EnumDllModuleByPeb( PEPROCESS EProcess, PALL_MODULES AllModules, ULONG_PTR ulCount);


extern WIN_VERSION  WinVersion;

extern ULONG_PTR    ObjectTableOffsetOf_EPROCESS;
extern ULONG_PTR    ObjectHeaderSize;
extern ULONG_PTR    ObjectTypeOffsetOf_Object_Header;
extern ULONG_PTR    PreviousModeOffsetOf_KTHREAD;
extern ULONG_PTR    IndexOffset;
ULONG_PTR    SSDTDescriptor;
ULONG_PTR    ulIndex;
ULONG_PTR    SSDTFuncAddress;

pfnNtQueryVirtualMemory  NtQueryVirtualMemoryAddress = NULL;

extern ULONG_PTR    SSDTDescriptor;


NTSTATUS
	HsEnumProcessesModule(ULONG ulProcessID,PVOID OutBuffer,ULONG_PTR ulOutSize)
{
	NTSTATUS Status = STATUS_UNSUCCESSFUL;
	PEPROCESS EProcess = NULL;

	ULONG ulCount = (ulOutSize - sizeof(ALL_MODULES)) / sizeof(MODULE_INFO);

	HsInitMemoryVariable();

	if (ulProcessID)
	{
		Status = PsLookupProcessByProcessId((HANDLE)ulProcessID, &EProcess);

		if (!NT_SUCCESS(Status))
		{
			return Status;
		}
	}

	DbgPrint("Enter EnumProcessModule\r\n");

	if (HsIsRealProcess(EProcess))
	{
		PALL_MODULES AllModules = (PALL_MODULES)ExAllocatePool(PagedPool,ulOutSize);
		if (AllModules)
		{
			memset(AllModules, 0, ulOutSize);

			Status = EnumDllModuleByPeb(EProcess, AllModules, ulCount);

			if (ulCount >= AllModules->ulCount)
			{
				RtlCopyMemory(OutBuffer, AllModules, ulOutSize);
				Status = STATUS_SUCCESS;
			}
			else
			{
				Status = STATUS_BUFFER_TOO_SMALL;
			}

			ExFreePool(AllModules, 0);
			AllModules = NULL;
		}
	}

	if (NT_SUCCESS(Status))
	{
		ObfDereferenceObject(EProcess);
	}

	return Status;
}





//////////////////////////////////////////////////////////////////////////
NTSTATUS
HsEnumProcessesMemory(ULONG ulProcessID,PVOID OutBuffer,ULONG_PTR ulOutSize)
{
	NTSTATUS Status = STATUS_UNSUCCESSFUL;
	PEPROCESS EProcess = NULL;

	HsInitMemoryVariable();

	if (!PsLookupProcessByProcessId || !ObfDereferenceObject)
	{
		return STATUS_UNSUCCESSFUL;
	}

	if (ulProcessID)
	{
		Status = PsLookupProcessByProcessId((HANDLE)ulProcessID, &EProcess);
		if (!NT_SUCCESS(Status))
		{
			return Status;
		}
	}

	if (HsIsRealProcess(EProcess))
	{
		ULONG_PTR  ulCount = (ulOutSize - sizeof(ALL_MEMORYS)) / sizeof(MEMORY_INFO);
		DbgPrint("EnumProcessMemory\r\n");
		Status = GetMemorys(EProcess, (PALL_MEMORYS)OutBuffer,ulCount);
		if (NT_SUCCESS(Status))
		{
			if (ulCount >= ((PALL_MEMORYS)OutBuffer)->ulCount)
			{
				DbgPrint("GetSuccess\r\n");
				Status = STATUS_SUCCESS;
			}
			else
			{
				Status = STATUS_BUFFER_TOO_SMALL;
			}
		}
	}

	if (NT_SUCCESS(Status))
	{
		ObfDereferenceObject(EProcess);
	}

	return Status;
}


NTSTATUS GetMemorys(PEPROCESS EProcess, PALL_MEMORYS Memorys, ULONG_PTR ulCount)
{
	NTSTATUS Status = STATUS_UNSUCCESSFUL;
	HANDLE hProcess = NULL;



	Status = ObOpenObjectByPointer(EProcess, 
		OBJ_KERNEL_HANDLE | OBJ_CASE_INSENSITIVE, 
		NULL, 
		GENERIC_ALL, 
		*PsProcessType, 
		KernelMode, 
		&hProcess
		);

	if (NT_SUCCESS(Status))
	{
		ULONG_PTR ulBase = 0;
		PETHREAD EThread = PsGetCurrentThread();
		CHAR PreMode = HsChangePreMode(EThread);

		while (ulBase < (ULONG_PTR)MM_HIGHEST_USER_ADDRESS)
		{
			MEMORY_BASIC_INFORMATION mbi;
			ULONG_PTR ulRet = 0;
			Status = NtQueryVirtualMemoryAddress(hProcess, 
				(PVOID)ulBase, 
				MemoryBasicInformation, 
				&mbi, 
				sizeof(MEMORY_BASIC_INFORMATION), 
				&ulRet);

			if (NT_SUCCESS(Status))
			{
				ULONG_PTR ulCurCnt = Memorys->ulCount;
				if (ulCount > ulCurCnt)
				{
					Memorys->Memorys[ulCurCnt].ulBase = ulBase;
					Memorys->Memorys[ulCurCnt].ulSize = mbi.RegionSize;
					Memorys->Memorys[ulCurCnt].ulProtect = mbi.Protect;
					Memorys->Memorys[ulCurCnt].ulState = mbi.State;
					Memorys->Memorys[ulCurCnt].ulType = mbi.Type;
				}

				Memorys->ulCount++;
				ulBase += mbi.RegionSize;
			}
			else
			{
				ulBase += PAGE_SIZE;
			}
		}

		NtClose(hProcess);
		HsRecoverPreMode(EThread,PreMode);
	}

	return Status;
}








VOID HsInitMemoryVariable()
{

	switch(WinVersion)
	{
	case WINDOWS_7:
		{
			PreviousModeOffsetOf_KTHREAD = 0x1f6;
			ObjectTableOffsetOf_EPROCESS = 0x200;
			IndexOffset = 4;
			SSDTDescriptor = GetKeServiceDescriptorTable64();
			ulIndex = GetSSDTApiFunIndex("NtQueryVirtualMemory");
			SSDTFuncAddress =  GetSSDTApiFunAddress(ulIndex,SSDTDescriptor);

			NtQueryVirtualMemoryAddress  = (pfnNtQueryVirtualMemory)SSDTFuncAddress;
			break;
		}

	case WINDOWS_XP:
		{

			ObjectHeaderSize = 0x18;
			ObjectTypeOffsetOf_Object_Header = 0x8;
			ObjectTableOffsetOf_EPROCESS = 0x0c4;
			PreviousModeOffsetOf_KTHREAD = 0x140;


			IndexOffset = 1;
			SSDTDescriptor = (ULONG_PTR)HsGetFunctionAddressByName(L"KeServiceDescriptorTable");
			//获得NtQueryObject函数的地址
			ulIndex = GetSSDTApiFunIndex("NtQueryVirtualMemory");


			SSDTFuncAddress =  GetSSDTApiFunAddress(ulIndex,SSDTDescriptor);

			NtQueryVirtualMemoryAddress  = (pfnNtQueryVirtualMemory)SSDTFuncAddress;
			break;
		}
	}
}