#include "DpcTimer.h"
#include "Module.h"

extern ULONG_PTR		SYSTEM_ADDRESS_START;
extern PDRIVER_OBJECT	g_DriverObject;
extern WIN_VERSION		WinVersion;



NTSTATUS HsEnumDPCTimer(PVOID OutBuffer)
{
	NTSTATUS Status = STATUS_UNSUCCESSFUL;
	PDPC_TIMER_INFOR DpcTimerInfor = (PDPC_TIMER_INFOR)OutBuffer;

	switch(WinVersion)
	{
	case WINDOWS_XP:
		{
			ULONG KiTimerTableListHead = 0;

			KiTimerTableListHead = GetKiTimerTableListHeadXP();

			GetDpcTimerInformationWinXP((PLIST_ENTRY)KiTimerTableListHead,DpcTimerInfor);
			break;
		}

	case WINDOWS_7:
		{
			GetDpcTimerInformationWin7(DpcTimerInfor);
			break;
		}
	}


	if (DpcTimerInfor->ulCnt >= DpcTimerInfor->ulRetCnt)
	{
		Status = STATUS_SUCCESS;
	}

	return Status;
}



ULONG GetKiTimerTableListHeadXP()
{
	ULONG KiTimerTableListHead = 0;
	ULONG KeUpdateSystemTime = 0;
	UCHAR Temp = 0;

	KeUpdateSystemTime = (ULONG)HsGetFunctionAddressByName(L"KeUpdateSystemTime");

	DbgPrint("KeUpdateSystemTime: 0X%08x\n", KeUpdateSystemTime);

	if (KeUpdateSystemTime)
	{
		ULONG Start = KeUpdateSystemTime;
		ULONG End = KeUpdateSystemTime + PAGE_SIZE;

		for (;Start<End; Start++)
		{
			/*

			kd> u KeUpdateSystemTime l 50
			nt!KeUpdateSystemTime:
			804e35d8 b90000dfff      mov     ecx,0FFDF0000h
			804e35dd 8b7908          mov     edi,dword ptr [ecx+8]
			804e35e0 8b710c          mov     esi,dword ptr [ecx+0Ch]
			804e35e3 03f8            add     edi,eax
			804e35e5 83d600          adc     esi,0
			804e35e8 897110          mov     dword ptr [ecx+10h],esi
			804e35eb 897908          mov     dword ptr [ecx+8],edi
			804e35ee 89710c          mov     dword ptr [ecx+0Ch],esi
			804e35f1 290514b05580    sub     dword ptr [nt!KiTickOffset (8055b014)],eax
			804e35f7 a100b05580      mov     eax,dword ptr [nt!KeTickCount (8055b000)]
			804e35fc 8bd8            mov     ebx,eax
			804e35fe 0f8f84000000    jg      nt!KeUpdateSystemTime+0xb0 (804e3688)
			804e3604 bb0000dfff      mov     ebx,0FFDF0000h
			804e3609 8b4b14          mov     ecx,dword ptr [ebx+14h]
			804e360c 8b5318          mov     edx,dword ptr [ebx+18h]
			804e360f 030d10b05580    add     ecx,dword ptr [nt!KeTimeAdjustment (8055b010)]
			804e3615 83d200          adc     edx,0
			804e3618 89531c          mov     dword ptr [ebx+1Ch],edx
			804e361b 894b14          mov     dword ptr [ebx+14h],ecx
			804e361e 895318          mov     dword ptr [ebx+18h],edx
			804e3621 8bd8            mov     ebx,eax
			804e3623 8bc8            mov     ecx,eax
			804e3625 8b1504b05580    mov     edx,dword ptr [nt!KeTickCount+0x4 (8055b004)]
			804e362b 83c101          add     ecx,1
			804e362e 83d200          adc     edx,0
			804e3631 891508b05580    mov     dword ptr [nt!KeTickCount+0x8 (8055b008)],edx
			804e3637 890d00b05580    mov     dword ptr [nt!KeTickCount (8055b000)],ecx
			804e363d 891504b05580    mov     dword ptr [nt!KeTickCount+0x4 (8055b004)],edx
			804e3643 50              push    eax
			804e3644 a10000dfff      mov     eax,dword ptr ds:[FFDF0000h]
			804e3649 83c001          add     eax,1
			804e364c 7306            jae     nt!KeUpdateSystemTime+0x7c (804e3654)
			804e364e ff059c005680    inc     dword ptr [nt!ExpTickCountAdjustmentCount (8056009c)]
			804e3654 a198005680      mov     eax,dword ptr [nt!ExpTickCountAdjustment (80560098)]
			804e3659 0faf059c005680  imul    eax,dword ptr [nt!ExpTickCountAdjustmentCount (8056009c)]
			804e3660 03c1            add     eax,ecx
			804e3662 a30000dfff      mov     dword ptr ds:[FFDF0000h],eax
			804e3667 58              pop     eax
			804e3668 25ff000000      and     eax,0FFh
			804e366d 8d0cc5a0355680  lea     ecx,nt!KiTimerTableListHead (805635a0)[eax*8]

			*/

			Temp = *(PCHAR)Start;
			if (Temp == 0x8d)
			{
				KiTimerTableListHead = *(PULONG)(Start + 3);
				if (KiTimerTableListHead && MmIsAddressValid((PVOID)KiTimerTableListHead))
				{
					break;
				}
			}
		}
	}

	return KiTimerTableListHead;
}



NTSTATUS GetDpcTimerInformationWinXP(PLIST_ENTRY KiTimerTableListHead, PDPC_TIMER_INFOR DpcTimerInfor)
{
	NTSTATUS Status = STATUS_UNSUCCESSFUL;
	ULONG TIMER_TABLE_SIZE = 0x100;     //查看Wrk代码是0x200 但 写成0x200 会蓝屏



	if (KiTimerTableListHead &&
		DpcTimerInfor &&
		TIMER_TABLE_SIZE && 
		MmIsAddressValid((PVOID)KiTimerTableListHead))
	{
		ULONG i = 0, n = 0;
		KIRQL OldIrql = KeRaiseIrqlToDpcLevel();

		for (i = 0; i < TIMER_TABLE_SIZE; i++)
		{
			PLIST_ENTRY NextEntry = (&KiTimerTableListHead[i])->Flink;

			while (MmIsAddressValid(NextEntry) && &KiTimerTableListHead[i] != NextEntry)
			{
				PKTIMER Timer = CONTAINING_RECORD(NextEntry, KTIMER, TimerListEntry);

				if (Timer && 
					MmIsAddressValid(Timer) &&
					MmIsAddressValid(Timer->Dpc) &&
					MmIsAddressValid(Timer->Dpc->DeferredRoutine))
				{
					if (DpcTimerInfor->ulCnt > DpcTimerInfor->ulRetCnt)
					{
						PKDPC Dpc = Timer->Dpc;
						PVOID TimerDispatch = Dpc->DeferredRoutine;


						DpcTimerInfor->DpcTimer[n].Dpc = (ULONG)Dpc;
						DpcTimerInfor->DpcTimer[n].Period = Timer->Period;
						DpcTimerInfor->DpcTimer[n].TimeDispatch = (ULONG)TimerDispatch;
						DpcTimerInfor->DpcTimer[n].TimerObject = (ULONG)Timer;

						n++;
					}

					DpcTimerInfor->ulRetCnt++;
				}

				NextEntry = NextEntry->Flink;
			}
		}

		KeLowerIrql(OldIrql);
	}

	return Status;
}



NTSTATUS GetDpcTimerInformationWin7(PDPC_TIMER_INFOR DpcTimerInfor)
{
	ULONG CPUNumber = KeNumberProcessors;   //系统变量
	PUCHAR CurrentKPRCBAddress = NULL;             //CPU控制块
	PUCHAR CurrentTimerTableEntry = NULL;
	PLIST_ENTRY CurrentEntry = NULL;
	PLIST_ENTRY NextEntry = NULL;
	PULONG64    KiWaitAlways = NULL;
	PULONG64    KiWaitNever  = NULL;
	int i = 0;
	int j = 0;
	int n = 0;
	PKTIMER Timer;
	for(j=0; j<CPUNumber; j++)
	{
		KeSetSystemAffinityThread(j+1);   //使当前线程运行在第一个处理器上，因为只有第一个处理器的值才有效
		CurrentKPRCBAddress=(PUCHAR)__readmsr(0xC0000101) + 0x20;
		KeRevertToUserAffinityThread();   ////恢复线程运行的处理器
	
		CurrentTimerTableEntry=(PUCHAR)(*(ULONG64*)CurrentKPRCBAddress + 0x2200 + 0x200);
		/*
		kd> dt _Kprcb
		nt!_KPRCB
		+0x000 MxCsr            : Uint4B
		+0x004 LegacyNumber     : UChar
		+0x005 ReservedMustBeZero : UChar
		+0x006 InterruptRequest : UChar
		+0x21ec UnusedPad        : Uint4B
		+0x21f0 PrcbPad50        : [2] Uint8B
		+0x2200 TimerTable       : _KTIMER_TABLE


		kd> dt _KTIMER_TABLE
		nt!_KTIMER_TABLE
		+0x000 TimerExpiry      : [64] Ptr64 _KTIMER
		+0x200 TimerEntries     : [256] _KTIMER_TABLE_ENTRY


		kd> dt _KTIMER_TABLE_ENTRY
		nt!_KTIMER_TABLE_ENTRY
		+0x000 Lock             : Uint8B
		+0x008 Entry            : _LIST_ENTRY
		+0x018 Time             : _ULARGE_INTEGER
		*/

	
		FindKiWaitFunc(&KiWaitNever,&KiWaitAlways);
		for(i=0; i<0x100; i++)
		{
			typedef struct _KTIMER_TABLE_ENTRY
			{
				ULONG64			Lock;
				LIST_ENTRY		Entry;
				ULARGE_INTEGER	Time;
			} KTIMER_TABLE_ENTRY, *PKTIMER_TABLE_ENTRY;
			CurrentEntry = (PLIST_ENTRY)(CurrentTimerTableEntry + sizeof(KTIMER_TABLE_ENTRY) * i + 8);  //这里是个数组  + 8 过Lock
			NextEntry = CurrentEntry->Blink;
			if( MmIsAddressValid(CurrentEntry) && MmIsAddressValid(CurrentEntry) )
			{
				while( NextEntry != CurrentEntry )
				{
					PKDPC RealDpc;

					//获得首地址
					Timer = CONTAINING_RECORD(NextEntry,KTIMER,TimerListEntry);
					/*
					kd> dt _KTIMER
					nt!_KTIMER
					+0x000 Header           : _DISPATCHER_HEADER
					+0x018 DueTime          : _ULARGE_INTEGER
					+0x020 TimerListEntry   : _LIST_ENTRY
					+0x030 Dpc              : Ptr64 _KDPC
					+0x038 Processor        : Uint4B
					+0x03c Period           : Uint4B
					*/
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
	/*
	kd> u kesettimer l 50
	nt!KeSetTimer:
	fffff800`03ef10a8 4883ec38        sub     rsp,38h
	fffff800`03ef10ac 4c89442420      mov     qword ptr [rsp+20h],r8
	fffff800`03ef10b1 4533c9          xor     r9d,r9d
	fffff800`03ef10b4 4533c0          xor     r8d,r8d
	fffff800`03ef10b7 e814000000      call    nt!KiSetTimerEx (fffff800`03ef10d0)
	fffff800`03ef10bc 4883c438        add     rsp,38h
	fffff800`03ef10c0 c3              ret
	fffff800`03ef10c1 90              nop
	fffff800`03ef10c2 90              nop
	fffff800`03ef10c3 90              nop
	fffff800`03ef10c4 90              nop
	fffff800`03ef10c5 90              nop
	fffff800`03ef10c6 90              nop
	fffff800`03ef10c7 90              nop
	nt!KxWaitForLockChainValid:
	fffff800`03ef10c8 90              nop
	fffff800`03ef10c9 90              nop
	fffff800`03ef10ca 90              nop
	fffff800`03ef10cb 90              nop
	fffff800`03ef10cc 90              nop
	fffff800`03ef10cd 90              nop
	fffff800`03ef10ce 90              nop
	fffff800`03ef10cf 90              nop
	nt!KiSetTimerEx:
	fffff800`03ef10d0 48895c2408      mov     qword ptr [rsp+8],rbx
	fffff800`03ef10d5 4889542410      mov     qword ptr [rsp+10h],rdx
	fffff800`03ef10da 55              push    rbp
	fffff800`03ef10db 56              push    rsi
	fffff800`03ef10dc 57              push    rdi
	fffff800`03ef10dd 4154            push    r12
	fffff800`03ef10df 4155            push    r13
	fffff800`03ef10e1 4156            push    r14
	fffff800`03ef10e3 4157            push    r15
	fffff800`03ef10e5 4883ec50        sub     rsp,50h
	fffff800`03ef10e9 488b0518502200  mov     rax,qword ptr [nt!KiWaitNever (fffff800`04116108)]
	fffff800`03ef10f0 488b1de9502200  mov     rbx,qword ptr [nt!KiWaitAlways (fffff800`041161e0)]
    */
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















