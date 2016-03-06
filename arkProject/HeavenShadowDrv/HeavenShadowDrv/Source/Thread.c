#include "Thread.h"



extern WIN_VERSION  WinVersion;

extern ULONG_PTR    ObjectHeaderSize;
extern ULONG_PTR    ObjectTypeOffsetOf_Object_Header;
extern ULONG_PTR    ObjectTableOffsetOf_EPROCESS;
extern ULONG_PTR    PreviousModeOffsetOf_KTHREAD;

extern ULONG_PTR    ulBuildNumber;

//////////////////////////////////////////////////////////////////////////
// 线程
//////////////////////////////////////////////////////////////////////////

//这是两个关于线程的链表
ULONG_PTR ThreadListHeadOffset = 0;
ULONG_PTR ThreadListEntryOffset = 0;
ULONG_PTR ThreadListHeadOffsetOther = 0;
ULONG_PTR ThreadListEntryOffsetOther =0;
ULONG_PTR ThreadsProcessOffset  = 0;
ULONG_PTR CidOffset = 0;
ULONG_PTR WinOffset = 0;
ULONG_PTR TebOffset = 0;
ULONG_PTR PriorityOffset = 0;
ULONG_PTR ContextSwitchesOffset = 0;
ULONG_PTR StateOffset = 0;
ULONG_PTR SystemRangeStart = 0;
ULONG_PTR Win32StartAddressOffset = 0;
ULONG_PTR StartAddressOffset = 0;
ULONG_PTR SameThreadApcFlags = 0;



NTSTATUS HsEnumProcessThread(PVOID ProcessId)
{
	NTSTATUS status = STATUS_UNSUCCESSFUL;
	PEPROCESS EProcess = NULL;

	PETHREAD            EThread = NULL;
	CHAR                PreMode = 0;

	DbgPrint("PID: %d\r\n",*((ULONG32*)ProcessId));



	status = PsLookupProcessByProcessId((HANDLE)*((ULONG*)ProcessId),&EProcess);

	if (status==STATUS_SUCCESS)
	{
		ObDereferenceObject(EProcess);
	}

	DbgPrint("%d\r\n",status);
	DbgPrint("%x\r\n",EProcess);


	HsEnumThreadByForce(EProcess);


	return STATUS_SUCCESS;
}



VOID
	HsEnumThreadByForce(PEPROCESS EProcess)
{
	NTSTATUS   Status;
	ULONG_PTR  i = 0;
	PEPROCESS  EProcessTemp;
	PETHREAD   EThread = NULL;

	for (i=0;i<100000;i+=4)
	{
		Status = PsLookupThreadByThreadId((HANDLE)i,&EThread);

		if (Status==STATUS_SUCCESS)
		{
			ObDereferenceObject(EThread);

			//通过线程体 获得进程体
			EProcessTemp = IoThreadToProcess(EThread);
			if (EProcessTemp==EProcess)
			{
				DbgPrint("[THREAD]ETHREAD=%p TID=%ld\n",
					EThread, 
					(ULONG)PsGetThreadId(EThread));
			}
		}
	}
}






VOID InitGlobalVariable()
{
	switch(WinVersion)
	{
	case WINDOWS_XP:
		{
			PreviousModeOffsetOf_KTHREAD = 0x140;
			ObjectHeaderSize = 0x18;
			ObjectTypeOffsetOf_Object_Header = 0x8;
			ObjectTableOffsetOf_EPROCESS = 0x0c4;


			Win32StartAddressOffset = 0x228;
			StartAddressOffset = 0x224;
			ThreadListHeadOffset = 0x050;
			ThreadListEntryOffset = 0x1b0;
			ThreadListHeadOffsetOther = 0x190;
			ThreadListEntryOffsetOther = 0x22c;
			ThreadsProcessOffset = 0x220;
			CidOffset  = 0x1ec;
			WinOffset = 4;
			TebOffset = 0x020;
			PriorityOffset = 0x033;
			ContextSwitchesOffset = 0x04c;
			StateOffset = 0x02d;
			SystemRangeStart = 0x80000000;
			SameThreadApcFlags = 0x250;
			break;
		}

	case WINDOWS_7:
		{
			PreviousModeOffsetOf_KTHREAD = 0x1f6;
			ObjectTableOffsetOf_EPROCESS = 0x200;
			ObjectHeaderSize = 0x30;
			SystemRangeStart = 0x80000000000;
			Win32StartAddressOffset = 0x410;	//0x410 0x418
			StartAddressOffset = 0x388;			//0x388 0x390
			ThreadListHeadOffset = 0x030;
			ThreadListEntryOffset = 0x2f8; 
			ThreadListHeadOffsetOther = 0x300;
			ThreadListEntryOffsetOther = 0x420;
			ThreadsProcessOffset = 0x210;
			CidOffset = 0x3b0;
			WinOffset = 8;
			TebOffset = 0x0b8;
			PriorityOffset = 0x07b;
			ContextSwitchesOffset = 0x134;
			StateOffset = 0x164;
			SameThreadApcFlags = 0x450;
			break;
		}
	}
}




NTSTATUS EnumProcessThread(PVOID InBuffer, ULONG InSize, PVOID OutBuffer, ULONG_PTR OutSize)
{
	NTSTATUS Status = STATUS_UNSUCCESSFUL, PsStatus = STATUS_UNSUCCESSFUL;

	PEPROCESS EProcess = NULL;
	ULONG_PTR ulPid = 0, ulCount = 0;

	ULONG_PTR  i = 0;
	PETHREAD   EThread = NULL;
	PEPROCESS  EProcessTemp;


	InitGlobalVariable();

	if (!InBuffer								||
		!OutBuffer								||
		InSize != sizeof(ULONG)					||
		OutSize < sizeof(ALL_THREADS))
	{
		return STATUS_INVALID_PARAMETER;
	}

	ulCount = (OutSize - sizeof(ALL_THREADS)) / sizeof(THREAD_INFO);

	ulPid = *(ULONG*)InBuffer;


	if (ulPid != 0)
	{
		PsStatus = PsLookupProcessByProcessId((HANDLE)ulPid, &EProcess);
		if (!NT_SUCCESS(PsStatus))
		{
			return STATUS_UNSUCCESSFUL;
		}
	}

	if (HsIsRealProcess(EProcess))
	{
		EnumThreads(EProcess, (PALL_THREADS)OutBuffer,ulCount);
		if (ulCount >= ((PALL_THREADS)OutBuffer)->nCnt)
		{
			Status = STATUS_SUCCESS;
		}
		else
		{
			Status = STATUS_BUFFER_TOO_SMALL;
		}
	}

	if (NT_SUCCESS(PsStatus))
	{
		ObfDereferenceObject(EProcess);
	}

	return Status;
}


VOID EnumThreads(PEPROCESS EProcess, PALL_THREADS ProcessThreads, ULONG_PTR ulCount)
{
	if (!EProcess || !ProcessThreads)
	{
		return;
	}

	EnumProcessThreadByList(EProcess,ProcessThreads,ulCount);
}



VOID EnumProcessThreadByList(PEPROCESS EProcess, PALL_THREADS ProcessThreads, ULONG_PTR ulCount)
{
// 	NTSTATUS Status = STATUS_UNSUCCESSFUL;
// 	ULONG_PTR  i = 0;
// 	PEPROCESS  EProcessTemp;
// 	PETHREAD   EThread = NULL;
// 	if (EProcess && MmIsAddressValid(EProcess))
// 	{
// 		for (i=0;i<100000;i+=4)
// 		{
// 			Status = PsLookupThreadByThreadId((HANDLE)i,&EThread);
// 
// 			if (Status==STATUS_SUCCESS)
// 			{
// 				ObDereferenceObject(EThread);
// 
// 				//通过线程体 获得进程体
// 				EProcessTemp = IoThreadToProcess(EThread);
// 				if (EProcessTemp==EProcess)
// 				{
// 
// 					DbgPrint("[THREAD]ETHREAD=%p TID=%ld\n",
// 						EThread, 
// 						(ULONG)PsGetThreadId(EThread));
// 
// 					InsertThread(EThread, EProcess, ProcessThreads, ulCount);
// 
// 				}
// 			}
// 		}
// 	}
	

	if (EProcess && MmIsAddressValid(EProcess))
	{
		PLIST_ENTRY  ListEntry = (PLIST_ENTRY)((ULONG_PTR)EProcess + ThreadListHeadOffset);
		if (ListEntry && MmIsAddressValid(ListEntry) && MmIsAddressValid(ListEntry->Flink))
		{
			KIRQL     OldIrql = KeRaiseIrqlToDpcLevel();
			ULONG_PTR nMaxCnt = PAGE_SIZE * 2;
			PLIST_ENTRY Temp = ListEntry->Flink;
			while (MmIsAddressValid(Temp) && Temp != ListEntry && nMaxCnt--)
			{
				PETHREAD EThread = (PETHREAD)((ULONG_PTR)Temp - ThreadListEntryOffset);
				InsertThread(EThread, EProcess, ProcessThreads, ulCount);
				Temp = Temp->Flink;
			}

			KeLowerIrql(OldIrql);
		}

		ListEntry = (PLIST_ENTRY)((ULONG)EProcess + ThreadListHeadOffsetOther);
		if (ListEntry && MmIsAddressValid(ListEntry) && MmIsAddressValid(ListEntry->Flink))
		{
			KIRQL     OldIrql = KeRaiseIrqlToDpcLevel();
			ULONG_PTR nMaxCnt = PAGE_SIZE * 2;
			PLIST_ENTRY Temp = ListEntry->Flink;
			while (MmIsAddressValid(Temp) && Temp != ListEntry && nMaxCnt--)
			{
				PETHREAD EThread = (PETHREAD)((ULONG_PTR)Temp - ThreadListEntryOffsetOther);
				InsertThread(EThread,EProcess, ProcessThreads,ulCount);
				Temp = Temp->Flink;
			}

			KeLowerIrql(OldIrql);
		}
	}
}




VOID InsertThread(PETHREAD EThread, PEPROCESS EProcess, PALL_THREADS ProcessThreads, ULONG ulCount)
{

	if (EThread && EProcess && MmIsAddressValid((PVOID)EThread))
	{ 

		PEPROCESS Temp = NULL;

		if (IoThreadToProcess)
		{
			Temp = IoThreadToProcess(EThread);
		}
		else
		{
			Temp = (PEPROCESS)(*(PULONG_PTR)(ULONG_PTR)EThread + ThreadsProcessOffset);
		}

		if (EProcess == Temp &&
			!IsThreadInList(EThread, ProcessThreads,ulCount) && 
			NT_SUCCESS(ObReferenceObjectByPointer(EThread, 0, NULL, KernelMode)))
		{
			ULONG_PTR nCurCnt = ProcessThreads->nCnt;
			if (ulCount > nCurCnt)
			{
				if (PsGetThreadId)
				{
					ProcessThreads->Threads[nCurCnt].Tid = (ULONG_PTR)PsGetThreadId(EThread);
				}
				else
				{
					ProcessThreads->Threads[nCurCnt].Tid = *(PULONG_PTR)((ULONG_PTR)EThread + CidOffset + WinOffset);
				}

				ProcessThreads->Threads[nCurCnt].Thread = (ULONG_PTR)EThread;
				ProcessThreads->Threads[nCurCnt].Win32StartAddress = GetThreadStartAddress(EThread);
				ProcessThreads->Threads[nCurCnt].Teb = *(PULONG_PTR)((ULONG_PTR)EThread + TebOffset);
				ProcessThreads->Threads[nCurCnt].Priority = *(PUCHAR)((ULONG_PTR)EThread + PriorityOffset);
				ProcessThreads->Threads[nCurCnt].ContextSwitches = *(PULONG)((ULONG_PTR)EThread + ContextSwitchesOffset);
				ProcessThreads->Threads[nCurCnt].State = *(PUCHAR)((ULONG_PTR)EThread + StateOffset);
			}

			ProcessThreads->nCnt++;
			ObfDereferenceObject(EThread);
		}
	} 
}



BOOLEAN IsThreadInList(PETHREAD EThread, PALL_THREADS ProcessThreads, ULONG ulCount)
{
	BOOLEAN bRet = FALSE;
	ULONG_PTR Temp = ulCount > ProcessThreads->nCnt ? ProcessThreads->nCnt : ulCount;
	ULONG_PTR i = 0;

	if (!EThread || !ProcessThreads)
	{
		return TRUE;
	}

	for (i = 0; i < Temp; i++)
	{
		if (ProcessThreads->Threads[i].Thread == (ULONG_PTR)EThread)
		{
			bRet = TRUE;
			break;
		}
	}

	return bRet; 
}




ULONG_PTR GetThreadStartAddress(PETHREAD EThread)
{


	ULONG_PTR ulStartAddress = 0;

	if (!EThread ||
		!MmIsAddressValid(EThread))
	{
		return ulStartAddress;
	}

	__try
	{
		ulStartAddress = *(PULONG_PTR)((ULONG_PTR)EThread + StartAddressOffset);

		if ( ulBuildNumber < 6000 )
		{
			if (ulStartAddress < (ULONG_PTR)SystemRangeStart)
			{
				ULONG_PTR Win32StartAddress = *(PULONG_PTR)((ULONG_PTR)EThread + Win32StartAddressOffset);
				if ( Win32StartAddress )
				{

					ulStartAddress = Win32StartAddress;

				}
			}
		}
		else
		{
			if (*(ULONG_PTR*)((ULONG_PTR)EThread + SameThreadApcFlags) & 2 )
			{
				ulStartAddress = *(ULONG_PTR*)((ULONG_PTR)EThread + Win32StartAddressOffset);
			}
			else
			{
				if (*(ULONG_PTR*)((ULONG_PTR)EThread + StartAddressOffset))
				{
					ulStartAddress = *(ULONG_PTR*)((ULONG_PTR)EThread + StartAddressOffset);
				}
			}

			if (ulStartAddress <= 0xf)
			{
				Win32StartAddressOffset = 0x418;	//0x410 0x418
				StartAddressOffset = 0x390;			//0x388 0x390
			}

			if (*(ULONG_PTR*)((ULONG_PTR)EThread + SameThreadApcFlags) & 2 )
			{
				ulStartAddress = *(ULONG_PTR*)((ULONG_PTR)EThread + Win32StartAddressOffset);
			}
			else
			{
				if (*(ULONG_PTR*)((ULONG_PTR)EThread + StartAddressOffset))
				{
					ulStartAddress = *(ULONG_PTR*)((ULONG_PTR)EThread + StartAddressOffset);
				}
			}
		}
	}
	__except(1)
	{}

	return ulStartAddress;
}



NTSTATUS
EnumProcessThreadModule(ULONG ulProcessID,PVOID OutBuffer,ULONG_PTR ulOutSize)
{
	NTSTATUS Status = STATUS_UNSUCCESSFUL;
	PEPROCESS EProcess = NULL;

	ULONG ulCount = (ulOutSize - sizeof(ALL_MODULES)) / sizeof(MODULE_INFO);

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


NTSTATUS EnumDllModuleByPeb( PEPROCESS EProcess, PALL_MODULES AllModules, ULONG_PTR ulCount)
{

	BOOLEAN bAttach = FALSE;
	KAPC_STATE ApcState;
	NTSTATUS Status = STATUS_UNSUCCESSFUL;
	ULONG_PTR LdrInPebOffset = 0;



	KeStackAttachProcess(EProcess, &ApcState);
	bAttach = TRUE;


	__try
	{
		ULONG_PTR Peb = 0;
		Peb = (ULONG_PTR)PsGetProcessPeb(EProcess);

		if ((ULONG)Peb > 0)
		{



			switch(WinVersion)
			{
			case WINDOWS_7:
				{
					PPEB_LDR_DATA64 LdrEntry = NULL;
					LdrInPebOffset = 0x18;


					LdrEntry = (PPEB_LDR_DATA64)(*(PULONG_PTR)((Peb + (ULONG_PTR)LdrInPebOffset)));
					ProbeForRead((PVOID)LdrEntry,8,8);

					if ((ULONG_PTR)LdrEntry>0)
					{
						WalkerModuleList64((PLIST_ENTRY64)&(LdrEntry->InLoadOrderModuleList), 1, AllModules, ulCount);
						WalkerModuleList64((PLIST_ENTRY64)&(LdrEntry->InMemoryOrderModuleList), 2, AllModules, ulCount);
						WalkerModuleList64((PLIST_ENTRY64)&(LdrEntry->InInitializationOrderModuleList), 3, AllModules, ulCount);

						Status = STATUS_SUCCESS;
					}

					break;
				}

			case WINDOWS_XP:
				{


					PPEB_LDR_DATA32 LdrEntry = NULL;
					LdrInPebOffset = 0x00c;


					LdrEntry = (PPEB_LDR_DATA32)(*(PULONG_PTR)((Peb + (ULONG_PTR)LdrInPebOffset)));
					ProbeForRead((PVOID)LdrEntry,4,4);

					if ((ULONG_PTR)LdrEntry>0)
					{
						WalkerModuleList32((PLIST_ENTRY32)&(LdrEntry->InLoadOrderModuleList), 1, AllModules, ulCount);
						WalkerModuleList32((PLIST_ENTRY32)&(LdrEntry->InMemoryOrderModuleList), 2, AllModules, ulCount);
						WalkerModuleList32((PLIST_ENTRY32)&(LdrEntry->InInitializationOrderModuleList), 3, AllModules, ulCount);

						Status = STATUS_SUCCESS;
					}

					break;
				}
			}



		}
	}
	__except(1)
	{
		DbgPrint("EnumDllModuleByPeb Catch __Except\r\n");
		Status = STATUS_UNSUCCESSFUL;
	}

	if (bAttach)
	{
		KeUnstackDetachProcess(&ApcState);
		bAttach = FALSE;
	}

	return Status;
}



VOID WalkerModuleList32(PLIST_ENTRY32 ListEntry, ULONG nType, PALL_MODULES AllModules, ULONG ulCount)
{
	PLIST_ENTRY32 Entry = NULL;
	PLDR_DATA_TABLE_ENTRY LdrEntry = NULL;
	Entry = (PLIST_ENTRY32)ListEntry->Flink;

	while ((ULONG_PTR)Entry > 0 && Entry != ListEntry)
	{


		switch (nType)
		{
		case 1:
			LdrEntry = CONTAINING_RECORD(Entry, LDR_DATA_TABLE_ENTRY, InLoadOrderLinks);
			break;

		case 2:
			LdrEntry = CONTAINING_RECORD(Entry, LDR_DATA_TABLE_ENTRY, InMemoryOrderLinks);
			break;

		case 3:
			LdrEntry = CONTAINING_RECORD(Entry, LDR_DATA_TABLE_ENTRY, InInitializationOrderLinks);
			break;
		}

		if ((ULONG)LdrEntry > 0)
		{
			__try
			{
				ProbeForRead(LdrEntry, sizeof(LDR_DATA_TABLE_ENTRY), 1);

				if (!IsModuleInList((ULONG_PTR)LdrEntry->DllBase, (ULONG_PTR)(LdrEntry->SizeOfImage), AllModules, ulCount))
				{
					if (ulCount > AllModules->ulCount)
					{


						AllModules->Modules[AllModules->ulCount].Base = (ULONG_PTR)LdrEntry->DllBase;
						AllModules->Modules[AllModules->ulCount].Size = LdrEntry->SizeOfImage;

						wcsncpy(AllModules->Modules[AllModules->ulCount].Path, LdrEntry->FullDllName.Buffer,LdrEntry->FullDllName.Length);
					}

					AllModules->ulCount++;
				}
			}
			__except(1)
			{
				DbgPrint("WalkerModuleList __Except(1)\r\n");
			}
		}

		Entry = (PLIST_ENTRY32)Entry->Flink;
	}
}



VOID WalkerModuleList64(PLIST_ENTRY64 ListEntry, ULONG nType, PALL_MODULES AllModules, ULONG ulCount)
{
	PLIST_ENTRY64 Entry = NULL;
	PLDR_DATA_TABLE_ENTRY64 LdrEntry = NULL;
	Entry = (PLIST_ENTRY64)ListEntry->Flink;

	while ((ULONG_PTR)Entry > 0 && Entry != ListEntry)
	{


		switch (nType)
		{
		case 1:
			LdrEntry = CONTAINING_RECORD(Entry, LDR_DATA_TABLE_ENTRY64, InLoadOrderLinks);
			break;

		case 2:
			LdrEntry = CONTAINING_RECORD(Entry, LDR_DATA_TABLE_ENTRY64, InMemoryOrderLinks);
			break;

		case 3:
			LdrEntry = CONTAINING_RECORD(Entry, LDR_DATA_TABLE_ENTRY64, InInitializationOrderLinks);
			break;
		}

		if ((ULONG)LdrEntry > 0)
		{
			__try
			{
				ProbeForRead(LdrEntry, sizeof(LDR_DATA_TABLE_ENTRY64), 8);

				if (!IsModuleInList((ULONG_PTR)LdrEntry->DllBase, (ULONG_PTR)(LdrEntry->SizeOfImage), AllModules, ulCount))
				{
					if (ulCount > AllModules->ulCount)
					{


						AllModules->Modules[AllModules->ulCount].Base = (ULONG_PTR)LdrEntry->DllBase;
						AllModules->Modules[AllModules->ulCount].Size = LdrEntry->SizeOfImage;

						wcsncpy(AllModules->Modules[AllModules->ulCount].Path, LdrEntry->FullDllName.Buffer,LdrEntry->FullDllName.Length);
					}

					AllModules->ulCount++;
				}
			}
			__except(1)
			{
				DbgPrint("WalkerModuleList __Except(1)\r\n");
			}
		}

		Entry = (PLIST_ENTRY64)Entry->Flink;
	}
}




BOOLEAN IsModuleInList(ULONG_PTR Base, ULONG_PTR Size, PALL_MODULES AllModules, ULONG_PTR ulCount)
{
	BOOLEAN bIn = FALSE;
	ULONG i = 0;
	ULONG ulTempCount = AllModules->ulCount > ulCount ? ulCount : AllModules->ulCount;

	for (i = 0; i < ulTempCount; i++)
	{
		if (Base == AllModules->Modules[i].Base && Size == AllModules->Modules[i].Size)
		{
			bIn = TRUE;
			break;
		}
	}

	return bIn;
}



