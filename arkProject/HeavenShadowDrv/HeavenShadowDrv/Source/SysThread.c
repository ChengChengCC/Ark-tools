#include "SysThread.h"
#include "System.h"


extern PDRIVER_OBJECT	g_DriverObject;
extern WIN_VERSION		WinVersion;
extern ULONG_PTR		SYSTEM_ADDRESS_START;
extern ULONG_PTR		ObjectHeaderSize;
extern ULONG_PTR		ObjectTypeOffsetOf_Object_Header;
extern ULONG_PTR		ObjectTableOffsetOf_EPROCESS;
extern ULONG_PTR		TebOffset;
extern ULONG_PTR		PriorityOffset;
extern ULONG_PTR		ContextSwitchesOffset;
extern ULONG_PTR		StateOffset;
extern ULONG_PTR		Win32StartAddressOffset;
extern ULONG_PTR		StartAddressOffset;
extern ULONG_PTR		SameThreadApcFlags;
extern PVOID			Ntoskrnl_KLDR_DATA_TABLE_ENTRY;

extern PEPROCESS		SystemEProcess;

ULONG_PTR		ulOffset = 0;
ULONG_PTR		ulImageNameOffset = 0;
ULONG_PTR		ulPspCidTable = 0;






NTSTATUS HsEnumSysThread(PVOID OutBuffer, ULONG_PTR OutSize)
{
	NTSTATUS Status;
	PETHREAD EThread;
	CHAR PreMode;

	ULONG_PTR ulCnt = (OutSize - sizeof(ALL_THREADS)) / sizeof(THREAD_INFO);

	//////////////////////////////////////////////////////////////////////////
	
	EThread = PsGetCurrentThread();
	PreMode = HsChangePreMode(EThread);
	
	HsSetGolbalMemberSysThread();
	ulPspCidTable = HsGetPspCidTableValue();
	if ( ulPspCidTable == 0 )  
	{  
		return STATUS_UNSUCCESSFUL;
	}  

	HsRecoverPreMode(EThread, PreMode);

	//////////////////////////////////////////////////////////////////////////

	ScanHandleTableToFindThread(SystemEProcess, (PALL_THREADS)OutBuffer, ulCnt);
	if (ulCnt >= ((PALL_THREADS)OutBuffer)->nCnt)
	{
		Status = STATUS_SUCCESS;
	}
	else
	{
		Status = STATUS_BUFFER_TOO_SMALL;
	}

	return Status;
}



ULONG_PTR HsGetPspCidTableValue()  
{  
	PVOID PsLookupProcessByProcessIdAddress = NULL;  
	ULONG_PTR ulPspCidTableValue = 0;  
	UNICODE_STRING uniFuncName; 
	ULONG  uIndex = 0;
	int    Offset = 0;

	// 获取PsLookupProcessByProcessId的函数地址   
	RtlInitUnicodeString(&uniFuncName, L"PsLookupProcessByProcessId");    //从Ntos导出表中获得函数PsLookupProcessByProcessId地址
	PsLookupProcessByProcessIdAddress = MmGetSystemRoutineAddress(&uniFuncName);  
	if (PsLookupProcessByProcessIdAddress== NULL )  
	{  
		return ulPspCidTableValue;  
	}  
	DbgPrint("PsLookupProcessByProcessId->%08X",PsLookupProcessByProcessIdAddress);  

	switch(WinVersion)
	{
	case WINDOWS_7:
		{
			/*
			kd> u PsLookupProcessByProcessId l 20
			nt!PsLookupProcessByProcessId:
			fffff800`041a61fc 48895c2408      mov     qword ptr [rsp+8],rbx
			fffff800`041a6201 48896c2410      mov     qword ptr [rsp+10h],rbp
			fffff800`041a6206 4889742418      mov     qword ptr [rsp+18h],rsi
			fffff800`041a620b 57              push    rdi
			fffff800`041a620c 4154            push    r12
			fffff800`041a620e 4155            push    r13
			fffff800`041a6210 4883ec20        sub     rsp,20h
			fffff800`041a6214 65488b3c2588010000 mov   rdi,qword ptr gs:[188h]
			fffff800`041a621d 4533e4          xor     r12d,r12d
			fffff800`041a6220 488bea          mov     rbp,rdx
			fffff800`041a6223 66ff8fc4010000  dec     word ptr [rdi+1C4h]
			fffff800`041a622a 498bdc          mov     rbx,r12
			fffff800`041a622d 488bd1          mov     rdx,rcx
			fffff800`041a6230 488b0d9149edff  mov     rcx,qword ptr [nt!PspCidTable (fffff800`0407abc8)]
			fffff800`041a6237 e834480200      call    nt!ExMapHandleToPointer (fffff800`041caa70)
			*/
			for (uIndex=0;uIndex<0x1000;uIndex++ )  
			{  
				if (*((PUCHAR)((ULONG_PTR)PsLookupProcessByProcessIdAddress+ uIndex)) == 0x48 &&  
					*((PUCHAR)((ULONG_PTR)PsLookupProcessByProcessIdAddress+ uIndex + 1) ) == 0x8B &&  
					*((PUCHAR)((ULONG_PTR)PsLookupProcessByProcessIdAddress+ uIndex + 7) ) == 0xE8 )  
				{  
			
					memcpy(&Offset,(PUCHAR)((ULONG_PTR)PsLookupProcessByProcessIdAddress+ uIndex + 3),4);
					ulPspCidTableValue = (ULONG_PTR)PsLookupProcessByProcessIdAddress+uIndex+Offset+7; 

					DbgPrint("Found OK!!\r\n");
					break;  
				}  
			}  
			break;
		}

	case WINDOWS_XP:
		{
			/*
			kd> u PsLookupProcessByProcessId l 20
			nt!PsLookupProcessByProcessId:
			80582687 8bff            mov     edi,edi
			80582689 55              push    ebp
			8058268a 8bec            mov     ebp,esp
			8058268c 53              push    ebx
			8058268d 56              push    esi
			8058268e 64a124010000    mov     eax,dword ptr fs:[00000124h]
			80582694 ff7508          push    dword ptr [ebp+8]
			80582697 8bf0            mov     esi,eax
			80582699 ff8ed4000000    dec     dword ptr [esi+0D4h]
			8058269f ff3560a75680    push    dword ptr [nt!PspCidTable (8056a760)]

			*/
	    	for (uIndex = 0; uIndex < 0x1000; uIndex++ )  
			{  
				if ( *( (PUCHAR)((ULONG_PTR)PsLookupProcessByProcessIdAddress+ uIndex) ) == 0xFF &&  
					*( (PUCHAR)((ULONG_PTR)PsLookupProcessByProcessIdAddress+ uIndex + 1) ) == 0x35 &&  
					*( (PUCHAR)((ULONG_PTR)PsLookupProcessByProcessIdAddress+ uIndex + 6) ) == 0xE8 )  
				{  
					DbgPrint("Found OK!!\r\n");  
					ulPspCidTableValue = *((PULONG)((ULONG)PsLookupProcessByProcessIdAddress+ uIndex + 2) );  
					break;  
				}  
			}  
			break;
		}
	}

	return ulPspCidTableValue;  
}  




VOID ScanHandleTableToFindThread(PEPROCESS EProcess, PALL_THREADS AllThreads, ULONG_PTR ulCnt)
{
	PHANDLE_TABLE   HandleTable = NULL;    // 指向句柄表的指针   
	ULONG_PTR uTableCode = 0;  
	ULONG uFlag = 0;


	HandleTable = (PHANDLE_TABLE)(*(ULONG_PTR*)ulPspCidTable);  

	if (HandleTable && MmIsAddressValid((PVOID)HandleTable))
	{
		uTableCode = (ULONG_PTR)(HandleTable->TableCode) & 0xFFFFFFFFFFFFFFFC;  ;
		if (uTableCode && MmIsAddressValid((PVOID)uTableCode))
		{
			uFlag = (ULONG)(HandleTable->TableCode) & 0x03;    //00  01  10  

			switch (uFlag)
			{
			case 0:
				{
					EnumTable1(uTableCode, EProcess, AllThreads, ulCnt);
					break;
				}
			case 1:
				{
					EnumTable2(uTableCode, EProcess, AllThreads, ulCnt);
					break;
				}


			case 2:
				{
					EnumTable3(uTableCode, EProcess, AllThreads, ulCnt);
					break; 
				}


			default:
				KdPrint(("TableCode error\n"));
			} 			
		}
	}
}



NTSTATUS EnumTable1(ULONG_PTR uTableCode, PEPROCESS EProcess, PALL_THREADS AllThreads, ULONG_PTR ulCnt)
{


	PVOID  Object = NULL;
	PHANDLE_TABLE_ENTRY HandleTableEntry = NULL;  
	ULONG uIndex = 0;


	HandleTableEntry = (PHANDLE_TABLE_ENTRY)((ULONG_PTR)(*(ULONG_PTR*)uTableCode) + ulOffset);  

	for (uIndex = 0;uIndex<0x200; uIndex++ )  
	{  
		if (MmIsAddressValid((PVOID)&(HandleTableEntry->NextFreeTableEntry)))
		{
			if (HandleTableEntry->NextFreeTableEntry==0)
			{
				if (HandleTableEntry->Object != NULL )  
				{  

					if (MmIsAddressValid(HandleTableEntry->Object))
					{

						Object = (PVOID)(((ULONG_PTR)HandleTableEntry->Object)  & 0xFFFFFFFFFFFFFFF8);  
						InsertThread((PETHREAD)Object,EProcess,AllThreads, ulCnt);

					}

				}
			}

		}

		HandleTableEntry++;  

	}  
}



NTSTATUS EnumTable2(ULONG_PTR uTableCode, PEPROCESS EProcess, PALL_THREADS AllThreads, ULONG_PTR ulCnt)
{
	do   
	{  
		EnumTable1(uTableCode,EProcess,AllThreads,ulCnt);  
		uTableCode += sizeof(ULONG_PTR);  
	} while (*(PULONG_PTR)uTableCode != 0&&MmIsAddressValid((PVOID)*(PULONG_PTR)uTableCode));  

	return STATUS_SUCCESS;
}



NTSTATUS EnumTable3(ULONG_PTR uTableCode, PEPROCESS EProcess, PALL_THREADS AllThreads, ULONG_PTR ulCnt)
{
	do   
	{  
		EnumTable2(uTableCode,EProcess,AllThreads,ulCnt);  
		uTableCode += sizeof(ULONG_PTR);  
	} while (*(PULONG_PTR)uTableCode != 0);  

	return STATUS_SUCCESS;  
}


//////////////////////////////////////////////////////////////////////////

VOID HsSetGolbalMemberSysThread()
{
	switch(WinVersion)
	{
	case WINDOWS_XP:
		{
			ulOffset = 0x8;
			ulImageNameOffset = 0x174;
			ObjectTypeOffsetOf_Object_Header = 0x8;
			ObjectTableOffsetOf_EPROCESS = 0x0c4;
			ObjectHeaderSize = 0x18;
			SYSTEM_ADDRESS_START = 0x80000000;

			TebOffset = 0x020;
			PriorityOffset = 0x033;
			ContextSwitchesOffset = 0x04c;
			StateOffset = 0x02d;

			Win32StartAddressOffset = 0x228;
			StartAddressOffset = 0x224;
			SameThreadApcFlags = 0x250;
			break;
		}
	case WINDOWS_7:
		{
			ulOffset = 0x10;
			ulImageNameOffset = 0x2e0;
			ObjectTableOffsetOf_EPROCESS = 0x200;
			ObjectHeaderSize = 0x30;
			SYSTEM_ADDRESS_START = 0x80000000000;
			TebOffset = 0x0b8;
			PriorityOffset = 0x07b;
			ContextSwitchesOffset = 0x134;
			StateOffset = 0x164;

			Win32StartAddressOffset = 0x410;
			StartAddressOffset = 0x388;
			SameThreadApcFlags = 0x450;
			break;
		}
	}
}