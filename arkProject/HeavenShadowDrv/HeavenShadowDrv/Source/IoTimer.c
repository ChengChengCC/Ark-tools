#include "IoTimer.h"
#include "Module.h"

extern ULONG_PTR		SYSTEM_ADDRESS_START;
extern PDRIVER_OBJECT	g_DriverObject;
extern WIN_VERSION		WinVersion;

pfnIoStartTimer  IoStartTimerAddress = NULL;
pfnIoStopTimer   IoStopTimerAddress = NULL;


NTSTATUS HsEnumIOTimer(PVOID OutBuffer)
{
	NTSTATUS Status = STATUS_SUCCESS;
	PLIST_ENTRY IopTimerQueueHead = 0;
	PIO_TIMER_INFOR  IoTimerInfor = (PIO_TIMER_INFOR)OutBuffer;

	KIRQL OldIrql;


	IopTimerQueueHead = (PLIST_ENTRY)GetIopTimerQueueHead();
	OldIrql = KeRaiseIrqlToDpcLevel();

	if (IopTimerQueueHead && MmIsAddressValid((PVOID)IopTimerQueueHead))
	{
		PLIST_ENTRY CurrentEntry = IopTimerQueueHead->Flink;

		while (MmIsAddressValid(CurrentEntry) && CurrentEntry != (PLIST_ENTRY)IopTimerQueueHead)
		{
			PIO_TIMER Timer = CONTAINING_RECORD(CurrentEntry,IO_TIMER,TimerList);

			if (Timer && MmIsAddressValid(Timer))
			{

				if (IoTimerInfor->ulCnt > IoTimerInfor->ulRetCnt)
				{
					IoTimerInfor->IoTimer[IoTimerInfor->ulRetCnt].TimerObject = (ULONG_PTR)Timer;

					DbgPrint("%p\r\n",Timer);
					IoTimerInfor->IoTimer[IoTimerInfor->ulRetCnt].TimerEntry = (ULONG_PTR)CurrentEntry;
					IoTimerInfor->IoTimer[IoTimerInfor->ulRetCnt].DeviceObject = (ULONG_PTR)Timer->DeviceObject;
					IoTimerInfor->IoTimer[IoTimerInfor->ulRetCnt].TimeDispatch = (ULONG_PTR)Timer->TimerRoutine;
					IoTimerInfor->IoTimer[IoTimerInfor->ulRetCnt].Status = (ULONG_PTR)Timer->TimerFlag;
				}

				IoTimerInfor->ulRetCnt++;
			}

			CurrentEntry = CurrentEntry->Flink;
		}
	}

	KeLowerIrql(OldIrql);

	return Status;
}




ULONG_PTR GetIopTimerQueueHead()
{
	ULONG_PTR i = 0;
	ULONG_PTR IopTimerQueueHead = 0;
	ULONG_PTR IoInitializeTimer = 0;
	long Temp = 0;


	IoInitializeTimer = (ULONG_PTR)HsGetFunctionAddressByName(L"IoInitializeTimer");

	if (IoInitializeTimer)
	{

		switch(WinVersion)
		{
		case WINDOWS_7:
			{
				/*
				kd> u IoInitializeTimer l 50
				nt!IoInitializeTimer:
				fffff800`0427f3b0 48895c2408      mov     qword ptr [rsp+8],rbx
				fffff800`0427f3b5 48896c2410      mov     qword ptr [rsp+10h],rbp
				fffff800`0427f3ba 4889742418      mov     qword ptr [rsp+18h],rsi
				fffff800`0427f3bf 57              push    rdi
				fffff800`0427f3c0 4883ec20        sub     rsp,20h
				fffff800`0427f3c4 488b5928        mov     rbx,qword ptr [rcx+28h]
				fffff800`0427f3c8 498bf0          mov     rsi,r8
				fffff800`0427f3cb 488bea          mov     rbp,rdx
				fffff800`0427f3ce 488bf9          mov     rdi,rcx
				fffff800`0427f3d1 4885db          test    rbx,rbx
				fffff800`0427f3d4 753f            jne     nt!IoInitializeTimer+0x65 (fffff800`0427f415)
				fffff800`0427f3d6 8d5330          lea     edx,[rbx+30h]
				fffff800`0427f3d9 33c9            xor     ecx,ecx
				fffff800`0427f3db 41b8496f5469    mov     r8d,69546F49h
				fffff800`0427f3e1 e8fa2cd3ff      call    nt!ExAllocatePoolWithTag (fffff800`03fb20e0)
				fffff800`0427f3e6 488bd8          mov     rbx,rax
				fffff800`0427f3e9 4885c0          test    rax,rax
				fffff800`0427f3ec 7507            jne     nt!IoInitializeTimer+0x45 (fffff800`0427f3f5)
				fffff800`0427f3ee b89a0000c0      mov     eax,0C000009Ah
				fffff800`0427f3f3 eb41            jmp     nt!IoInitializeTimer+0x86 (fffff800`0427f436)
				fffff800`0427f3f5 33d2            xor     edx,edx
				fffff800`0427f3f7 488bc8          mov     rcx,rax
				fffff800`0427f3fa 448d4230        lea     r8d,[rdx+30h]
				fffff800`0427f3fe e88da4c0ff      call    nt!memset (fffff800`03e89890)
				fffff800`0427f403 41bb09000000    mov     r11d,9
				fffff800`0427f409 48897b28        mov     qword ptr [rbx+28h],rdi
				fffff800`0427f40d 6644891b        mov     word ptr [rbx],r11w
				fffff800`0427f411 48895f28        mov     qword ptr [rdi+28h],rbx
				fffff800`0427f415 488d5308        lea     rdx,[rbx+8]
				fffff800`0427f419 4c8d0520ade3ff  lea     r8,[nt!IopTimerLock (fffff800`040ba140)]
				fffff800`0427f420 488d0dd94ce0ff  lea     rcx,[nt!IopTimerQueueHead (fffff800`04084100)]

				*/

				for(i=IoInitializeTimer; i<IoInitializeTimer+0xff; i++)
				{
					if(*(PUCHAR)i==0x48 && *(PUCHAR)(i+1)==0x8D && *(PUCHAR)(i+2)==0x0D)	
					{
						memcpy(&Temp,(PUCHAR)i+3,4);
						IopTimerQueueHead = (ULONGLONG)Temp + (ULONGLONG)i + 7;

						break;
					}
				}
				break;
			}

		case WINDOWS_XP:
			{
				/*
				kd> u IoInitializeTimer l 20
				nt!IoInitializeTimer:
				805d3044 8bff            mov     edi,edi
				805d3046 55              push    ebp
				805d3047 8bec            mov     ebp,esp
				805d3049 56              push    esi
				805d304a 8b7508          mov     esi,dword ptr [ebp+8]
				805d304d 8b5618          mov     edx,dword ptr [esi+18h]
				805d3050 85d2            test    edx,edx
				805d3052 752d            jne     nt!IoInitializeTimer+0x40 (805d3081)
				805d3054 68496f5469      push    69546F49h
				805d3059 6a18            push    18h
				805d305b 52              push    edx
				805d305c e8a4eff7ff      call    nt!ExAllocatePoolWithTag (80552005)
				805d3061 8bd0            mov     edx,eax
				805d3063 85d2            test    edx,edx
				805d3065 0f84f4c70100    je      nt!IoInitializeTimer+0x23 (805ef85f)
				805d306b 57              push    edi
				805d306c 6a06            push    6
				805d306e 59              pop     ecx
				805d306f 33c0            xor     eax,eax
				805d3071 8bfa            mov     edi,edx
				805d3073 f3ab            rep stos dword ptr es:[edi]
				805d3075 66c7020900      mov     word ptr [edx],9
				805d307a 897214          mov     dword ptr [edx+14h],esi
				805d307d 895618          mov     dword ptr [esi+18h],edx
				805d3080 5f              pop     edi
				805d3081 8b450c          mov     eax,dword ptr [ebp+0Ch]
				805d3084 89420c          mov     dword ptr [edx+0Ch],eax
				805d3087 8b4510          mov     eax,dword ptr [ebp+10h]
				805d308a 894210          mov     dword ptr [edx+10h],eax
				805d308d 6880a65580      push    offset nt!IopTimerLock (8055a680)
				805d3092 83c204          add     edx,4
				805d3095 b9e01d5680      mov     ecx,offset nt!IopTimerQueueHead (80561de0)



				*/

				for(i=IoInitializeTimer;i<IoInitializeTimer+0xFF;i++)
				{
					if(*(PUCHAR)i==0xb9)	
					{
						IopTimerQueueHead = *(PULONG)(i + 1);
						if (IopTimerQueueHead && MmIsAddressValid((PVOID)IopTimerQueueHead))
						{
							break;
						}
					}
				}
				break;
			}
		}


	}
	
	return IopTimerQueueHead;
}





NTSTATUS HsOperIOTimer(PVOID InBuffer)
{
	NTSTATUS Status = STATUS_UNSUCCESSFUL;
	PCOMMUNICATE_IO_TIMER IoTimerInfor = (PCOMMUNICATE_IO_TIMER)InBuffer;
	PDEVICE_OBJECT DeviceObject = (PDEVICE_OBJECT)IoTimerInfor->DeviceObject;

	if (!DeviceObject ||
		!MmIsAddressValid(DeviceObject))
	{
		return Status;
	}

	if (IoTimerInfor->bStart)
	{
		IoStartTimerAddress = (pfnIoStartTimer)HsGetFunctionAddressByName(L"IoStartTimer");
		if (IoStartTimerAddress)
		{
			IoStartTimerAddress(DeviceObject);
			Status = STATUS_SUCCESS;
		}
	}
	else 
	{
		IoStopTimerAddress = (pfnIoStopTimer)HsGetFunctionAddressByName(L"IoStopTimer");
		IoStopTimerAddress(DeviceObject);
		Status = STATUS_SUCCESS;
	}

	return Status;
}






NTSTATUS RemoveIOTimer(PVOID InBuffer)
{
	NTSTATUS Status = STATUS_UNSUCCESSFUL;
	PLIST_ENTRY IopTimerQueueHead = 0;
	KIRQL OldIrql;
	PCOMMUNICATE_IO_TIMER IoTimerInfor = (PCOMMUNICATE_IO_TIMER)InBuffer;
	PLIST_ENTRY  TimerEntry = (PLIST_ENTRY)IoTimerInfor->TimerEntry;



	IopTimerQueueHead = (PLIST_ENTRY)GetIopTimerQueueHead();

	if (!TimerEntry ||
		!MmIsAddressValid(TimerEntry))
	{
		return Status;
	}



	OldIrql = KeRaiseIrqlToDpcLevel();

	if (IopTimerQueueHead && MmIsAddressValid((PVOID)IopTimerQueueHead))
	{
		PLIST_ENTRY Temp = IopTimerQueueHead->Flink;

		while (MmIsAddressValid(Temp) && Temp != IopTimerQueueHead)
		{
			if (Temp == TimerEntry)
			{
				PIO_TIMER Timer = CONTAINING_RECORD(Temp,IO_TIMER, TimerList);
				if (Timer && MmIsAddressValid(Timer))
				{
					RemoveEntryList(Temp);
					ExFreePoolWithTag(Timer,0);
					Status = STATUS_SUCCESS;
				}

				break;
			}

			Temp = Temp->Flink;
		}
	}

	KeLowerIrql(OldIrql);

	return Status;
}
