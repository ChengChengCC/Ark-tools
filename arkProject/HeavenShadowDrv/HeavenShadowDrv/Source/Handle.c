#include "Handle.h"
#include "GetFuncAddress.h"
#include <ntimage.h>

pfnNtQueryObject  NtQueryObjectAddress = NULL;


#define SystemHandleInformation 16
#define SEC_IMAGE 0x01000000

extern
	WIN_VERSION  WinVersion;

extern ULONG_PTR ulBuildNumber;

extern ULONG_PTR ObjectHeaderSize;

extern ULONG_PTR PreviousModeOffsetOf_KTHREAD;
extern ULONG_PTR ObjectTypeOffsetOf_Object_Header;
extern ULONG_PTR ObjectTableOffsetOf_EPROCESS;



NTSTATUS HsEnumProcessHandle(PVOID InBuffer, ULONG_PTR InSize, PVOID OutBuffer, ULONG_PTR OutSize)
{
	NTSTATUS Status = STATUS_UNSUCCESSFUL, LookupStatus = STATUS_UNSUCCESSFUL;

	PEPROCESS EProcess = NULL;
	ULONG ulPid = 0;
	ULONG_PTR ulCount = (OutSize - sizeof(ALL_HANDLES)) / sizeof(HANDLE_INFO);

	HsInitHandleVariable();
	NtQueryObjectAddress = (pfnNtQueryObject)HsGetFuncAddress("NtQueryObject");


	if (!InBuffer ||
		!OutBuffer)
	{
		return STATUS_INVALID_PARAMETER;
	}

	ulPid = (ULONG)InBuffer;

	if (ulPid)
	{
		LookupStatus = PsLookupProcessByProcessId((HANDLE)ulPid, &EProcess);
		if (!NT_SUCCESS(LookupStatus) || !EProcess)
		{
			return STATUS_UNSUCCESSFUL;
		}
	}


	if (HsIsRealProcess(EProcess))
	{
		Status = HsGetHandles(ulPid, (ULONG_PTR)EProcess, (PALL_HANDLES)OutBuffer, ulCount);
	}

	if (NT_SUCCESS(LookupStatus))
	{
		ObfDereferenceObject(EProcess);
	}

	return Status;
}




NTSTATUS HsGetHandles(ULONG_PTR ulPid, ULONG_PTR EProcess, PALL_HANDLES OutHandles, ULONG_PTR ulCount)
{
	NTSTATUS  Status = STATUS_UNSUCCESSFUL;
	ULONG_PTR ulRet= 0x10000;
	PETHREAD EThread = NULL;
	CHAR PreMode = 0;


	EThread = PsGetCurrentThread();
	PreMode = HsChangePreMode(EThread);

	do 
	{
		PVOID Buffer = ExAllocatePool(PagedPool,ulRet);
		if (Buffer)
		{
			memset(Buffer, 0, ulRet);
			Status = NtQuerySystemInformation(SystemHandleInformation, Buffer, ulRet, &ulRet);
			if (NT_SUCCESS(Status))
			{
				PSYSTEM_HANDLE_INFORMATION Handles = (PSYSTEM_HANDLE_INFORMATION)Buffer;
				ULONG i = 0;
				for (i = 0; i < Handles->NumberOfHandles; i++)
				{
					if (ulPid == Handles->Handles[i].UniqueProcessId)
					{
						if (ulCount > OutHandles->ulCount)
						{
							HsInsertHandleToList((PEPROCESS)EProcess, 
								(HANDLE)Handles->Handles[i].HandleValue, 
								(ULONG_PTR)Handles->Handles[i].Object, 
								OutHandles);
						}

						OutHandles->ulCount++;
					}
				}
			}

			ExFreePool(Buffer);
			ulRet *= 2;
		}
	} while (Status == STATUS_INFO_LENGTH_MISMATCH);

	HsRecoverPreMode(EThread, PreMode);

	if (NT_SUCCESS(Status))
	{
		if (ulCount >= OutHandles->ulCount)
		{
			Status = STATUS_SUCCESS;
		}
		else
		{
			Status = STATUS_BUFFER_TOO_SMALL;
		}
	}

	return Status;
}





VOID HsInsertHandleToList(PEPROCESS EProcess, HANDLE Handle, ULONG_PTR Object, PALL_HANDLES Handles)
{

	BOOLEAN bAttach = FALSE;
	KAPC_STATE as;
	PHANDLE_INFO Buffer = NULL;

	if (Object && 
		MmIsAddressValid((PVOID)Object) && 
		(Buffer = (PHANDLE_INFO)ExAllocatePool(NonPagedPool,sizeof(HANDLE_INFO))) != NULL)
	{
		memset(Buffer,0,sizeof(HANDLE_INFO));


		KeStackAttachProcess(EProcess, &as);
		bAttach = TRUE;


		Buffer->Handle = (ULONG_PTR)Handle;
		Buffer->Object = Object;
		if (MmIsAddressValid((PVOID)(Object - 0x18)))
		{
			Buffer->ReferenceCount = *(ULONG_PTR*)((ULONG_PTR)Object - ObjectHeaderSize);
		}
		else
		{
			Buffer->ReferenceCount = 0;
		}


		HsGetHandleObjectName((HANDLE)Handle,Buffer->HandleName);
		HsGetHandleTypeName((HANDLE)Handle, Object, Buffer->ObjectName);

		if (bAttach)
		{
			KeUnstackDetachProcess(&as);
			bAttach = FALSE;
		}

		memcpy(&Handles->Handles[Handles->ulCount],Buffer,sizeof(HANDLE_INFO));
		ExFreePool(Buffer);
		Buffer = NULL;
	}
}




VOID HsGetHandleObjectName(HANDLE hHandle,WCHAR* wzObjectName)
{
	PVOID HandleName = NULL;

	HandleName = ExAllocatePool(PagedPool,0x1000);
	if (HandleName)
	{
		ULONG uRet= 0;
		PETHREAD EThread = NULL;
		CHAR PreMode = 0;

		memset(HandleName, 0, 0x1000);

		EThread = PsGetCurrentThread();
		PreMode = HsChangePreMode(EThread);

		__try
		{
			if (NT_SUCCESS(NtQueryObjectAddress((HANDLE)hHandle, ObjectNameInfo, HandleName, 0x1000, &uRet)))
			{
				POBJECT_NAME_INFORMATION ObjectNameInformation = (POBJECT_NAME_INFORMATION)HandleName;
				if (ObjectNameInformation->Name.Buffer!=NULL)
				{


					if (HsIsUnicodeStringValid(&ObjectNameInformation->Name))
					{
						wcsncpy(wzObjectName,
							ObjectNameInformation->Name.Buffer,ObjectNameInformation->Name.Length);
					}


				}
			}
		}
		__except(1)
		{
			DbgPrint("GetHandleObjectName Catch __Except\r\n");
		}

		HsRecoverPreMode(EThread, PreMode);
		ExFreePool(HandleName);
	}
}


VOID HsGetHandleTypeName(HANDLE hHandle, ULONG_PTR Object,WCHAR* wzTypeName)
{
	PVOID HandleName = NULL;
	BOOLEAN bOk = FALSE;

	HandleName = ExAllocatePool(PagedPool,0x1000);
	if (HandleName)
	{
		ULONG uRet= 0;
		PETHREAD EThread = NULL;
		CHAR PreMode = 0;

		memset(HandleName, 0, 0x1000);

		EThread = PsGetCurrentThread();
		PreMode = HsChangePreMode(EThread);

		__try
		{
			if (NT_SUCCESS(NtQueryObjectAddress((HANDLE)hHandle, ObjectTypeInfo, HandleName, 0x1000, &uRet)))
			{
				POBJECT_NAME_INFORMATION ObjectNameInformation = (POBJECT_NAME_INFORMATION)HandleName;
				if (ObjectNameInformation->Name.Buffer!=NULL)
				{


					if (HsIsUnicodeStringValid(&ObjectNameInformation->Name))
					{
						wcsncpy(wzTypeName, ObjectNameInformation->Name.Buffer,ObjectNameInformation->Name.Length);
					}		

				}
			}
		}
		__except(1)
		{
			DbgPrint("GetHandleObjectName Catch __Except\r\n");
		}

		HsRecoverPreMode(EThread, PreMode);
		ExFreePool(HandleName);



	}
}




VOID HsInitHandleVariable()
{
	switch(WinVersion)
	{
	case WINDOWS_XP:
		{
			PreviousModeOffsetOf_KTHREAD = 0x140;
			ObjectHeaderSize = 0x18;
			ObjectTypeOffsetOf_Object_Header = 0x8;
			ObjectTableOffsetOf_EPROCESS = 0x0c4;
			break;
		}

	case WINDOWS_7:
		{
			PreviousModeOffsetOf_KTHREAD = 0x1f6;
			ObjectTableOffsetOf_EPROCESS = 0x200;
			ObjectHeaderSize = 0x30;
			break;
		}
	}
}