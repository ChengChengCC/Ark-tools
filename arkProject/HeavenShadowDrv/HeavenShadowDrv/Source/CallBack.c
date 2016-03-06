#include "CallBack.h"



//////////////////////////////////////////////////////////////////////////
extern     PDEVICE_OBJECT g_DeviceObject;
extern     PDRIVER_OBJECT g_DriverObject;
//////////////////////////////////////////////////////////////////////////

extern  PVOID        Ntoskrnl_KLDR_DATA_TABLE_ENTRY;
extern  ULONG_PTR    SYSTEM_ADDRESS_START;
extern  WIN_VERSION  WinVersion;

pfnPsSetLoadImageNotifyRoutine PsSetLoadImageNotifyRoutineAddress = NULL;
pfnCmUnRegisterCallback CmUnRegisterCallbackAddress = NULL;
ULONG_PTR      PspLoadImageNotifyRoutineAddress = NULL;
ULONG_PTR      CallBackList = 0;  
pfnKeRegisterBugCheckReasonCallback KeRegisterBugCheckReasonCallbackAddress = NULL;
ULONG_PTR KeBugCheckReasonCallbackListHead = NULL;
pfnKeRegisterBugCheckCallback KeRegisterBugCheckCallbackAddress = NULL;
ULONG_PTR KeBugCheckCallbackListHead = NULL;
pfnIoRegisterShutdownNotification   IoRegisterShutdownNotificationAddress = NULL;
ULONG_PTR IopNotifyShutdownQueueHead = NULL;
pfnPsSetCreateThreadNotifyRoutine   PsSetCreateThreadNotifyRoutineAddress = NULL;
ULONG_PTR PspCreateThreadNotifyRoutineAddress = NULL;


NTSTATUS HsEnumCallBackList(int InputBuffer, PVOID OutputBuffer)
{
	NTSTATUS Status = STATUS_UNSUCCESSFUL;

	Status = GetAllCallbackNotify(OutputBuffer);

// 	switch(InputBuffer)
// 	{
// 	case NotifyLoadImage:
// 		{
// 			Status = HsGetCallbackLoadImage(OutputBuffer);
// 		}
// 	case NotifyCreateProcess:
// 		{
// 
// 		}
// 	case NotifyCreateThread:
// 		{
// 
// 		}
// 	case NotifyShutdown:
// 		{
// 
// 		}
// 	default:
// 		{
// 			Status = STATUS_UNSUCCESSFUL;
// 		}
// 	}

	return Status;
}





NTSTATUS GetAllCallbackNotify(PVOID OutBuffer)
{
	NTSTATUS Status = STATUS_SUCCESS;
	BOOLEAN  bRet = FALSE;
	PGET_CALLBACK GetCallback = (PGET_CALLBACK)OutBuffer;

	DbgPrint("GetAllCallbackNotify\r\n");


	bRet = GetLoadImageCallbackNotify(GetCallback);
	bRet = GetRegisterCallbackNotify(GetCallback);
	bRet = GetBugCheckCallbackNotify(GetCallback);
	bRet = GetBugCheckReasonCallbackNotify(GetCallback);
	bRet = GetShutDownCallbackNotify(GetCallback);
	bRet = GetCreateThreadCallbackNotify(GetCallback);


	if (GetCallback->ulRetCnt > GetCallback->ulCnt)
	{
		DbgPrint("STATUS_BUFFER_TOO_SMALL\r\n");
		Status = STATUS_BUFFER_TOO_SMALL;
	}

	return Status;
}




NTSTATUS RemoveCallbackNotify(PVOID InBuffer)
{	
	NTSTATUS Status = STATUS_SUCCESS;
	PREMOVE_CALLBACK Temp = (PREMOVE_CALLBACK)InBuffer;

	ULONG_PTR CallbackAddress  = Temp->CallbackAddress;
	CALLBACK_TYPE CallBackType = Temp->NotifyType;

	if (!CallbackAddress ||
		!MmIsAddressValid((PVOID)CallbackAddress))
	{
		return STATUS_UNSUCCESSFUL;
	}

	DbgPrint("CallBackType: %d\r\n",CallBackType);
	switch(CallBackType)
	{
	case NotifyLoadImage:
		{
			DbgPrint("Remove NotifyLoadImage\r\n");
			Status = PsRemoveLoadImageNotifyRoutine((PLOAD_IMAGE_NOTIFY_ROUTINE)CallbackAddress);
			break;
		}
	case NotifyCmCallBack:
		{
			LARGE_INTEGER Cookie;
			ULONG_PTR Note = Temp->Note;
			Cookie.QuadPart = 0;

			DbgPrint("Remove NotifyCmCallBack\r\n");

			if (WinVersion == WINDOWS_XP)
			{
				Cookie = XpGetRegisterCallbackCookie(Note);
			}

			if (WinVersion==WINDOWS_7)
			{
				Cookie.QuadPart = Note;
			}

			if (Cookie.LowPart == 0 && Cookie.HighPart == 0)
			{
				return STATUS_UNSUCCESSFUL;
			}

			Status = CmUnRegisterCallback(Cookie);

			break;
		}
	case NotifyKeBugCheckReason:
		{
			PREMOVE_CALLBACK Temp = (PREMOVE_CALLBACK)InBuffer;

			ULONG_PTR Note = Temp->Note;


			if (Note!=NULL&&MmIsAddressValid((PVOID)Note))
			{
				KeDeregisterBugCheckReasonCallback((PKBUGCHECK_REASON_CALLBACK_RECORD)Note);
			}

			break;
		}
	case NotifyShutdown:
		{
			LARGE_INTEGER Cookie;

			PREMOVE_CALLBACK Temp = (PREMOVE_CALLBACK)InBuffer;

			ULONG_PTR Note = Temp->Note;


			if (Note!=NULL&&MmIsAddressValid((PVOID)Note))
			{
				IoUnregisterShutdownNotification((PDEVICE_OBJECT)Note);
			}

			break;
		}
	case NotifyCreateThread:
		{
			NTSTATUS Status = STATUS_SUCCESS;
			PREMOVE_CALLBACK Temp = (PREMOVE_CALLBACK)InBuffer;

			ULONG_PTR CallbackAddress = Temp->CallbackAddress;

			if (!CallbackAddress ||
				!MmIsAddressValid((PVOID)CallbackAddress)||!PsRemoveCreateThreadNotifyRoutine)
			{
				return STATUS_UNSUCCESSFUL;
			}

			Status = PsRemoveCreateThreadNotifyRoutine((PCREATE_THREAD_NOTIFY_ROUTINE)CallbackAddress);

			break;
		}
	default:
		{
			Status = STATUS_UNSUCCESSFUL;
		}
	}

	return Status;
}


BOOLEAN GetLoadImageCallbackNotify(PGET_CALLBACK GetCallback)
{

	PsSetLoadImageNotifyRoutineAddress = 
		(pfnPsSetLoadImageNotifyRoutine)HsGetFunctionAddressByName(L"PsSetLoadImageNotifyRoutine");
	DbgPrint("%p\r\n",PsSetLoadImageNotifyRoutineAddress);
	PspLoadImageNotifyRoutineAddress = 
		FindPspLoadImageNotifyRoutine((ULONG_PTR)PsSetLoadImageNotifyRoutineAddress);
	DbgPrint("%p\r\n",PspLoadImageNotifyRoutineAddress);


	if (!PspLoadImageNotifyRoutineAddress)
	{
		DbgPrint("PspLoadImageNotifyRoutineAddress NULL\r\n");
		return FALSE;
	}

	else
	{
		ULONG i = 0;

		switch(WinVersion)
		{
		case WINDOWS_7:
			{
				SYSTEM_ADDRESS_START = 0x80000000000;

				for ( i = 0; i < 64; i++ )
				{
					ULONG64 NotifyItem = 0, CallBackAddress = 0;

					DbgPrint("i = %d\r\n",i);

					if (!MmIsAddressValid( (PVOID)(PspLoadImageNotifyRoutineAddress + i * sizeof(ULONG64))) )
					{
						break;
					}

					NotifyItem = *(PULONG64)(PspLoadImageNotifyRoutineAddress + i * sizeof(ULONG64));

					DbgPrint("NotifyItem: %p\r\n",NotifyItem);

					if (!(NotifyItem > SYSTEM_ADDRESS_START && MmIsAddressValid((PVOID)(NotifyItem & 0xfffffffffffffff8))) )
					{
						break;
					}

					CallBackAddress = *((PULONG64)(NotifyItem & 0xfffffffffffffff8));

					DbgPrint("CallBackAddress: %p\r\n",CallBackAddress);

					if (CallBackAddress && MmIsAddressValid((PVOID)CallBackAddress))
					{
						if (GetCallback->ulCnt > GetCallback->ulRetCnt)
						{
							GetCallback->Callbacks[GetCallback->ulRetCnt].Type = NotifyLoadImage;
							GetCallback->Callbacks[GetCallback->ulRetCnt].CallbackAddress = CallBackAddress;
							GetCallback->Callbacks[GetCallback->ulRetCnt].Note = NotifyItem;

							DbgPrint("GetCallback->ulCnt > GetCallback->ulRetCnt\r\n");
						}

						GetCallback->ulRetCnt++;
					}

				}
				break;
			}

		case WINDOWS_XP:
			{
				SYSTEM_ADDRESS_START = 0x80000000;
				for ( i = 0; i < 64; i++ )
				{
					ULONG32 NotifyItem = 0, CallBackAddress = 0;

					if (!MmIsAddressValid( (PVOID)(PspLoadImageNotifyRoutineAddress + i * sizeof(ULONG32))) )
					{
						break;
					}

					NotifyItem = *(PULONG32)(PspLoadImageNotifyRoutineAddress + i * sizeof(ULONG32));
					if (!(NotifyItem > SYSTEM_ADDRESS_START && MmIsAddressValid((PVOID)(NotifyItem & 0xFFFFFFF8 + sizeof(ULONG32)))) )
					{
						break;
					}

					CallBackAddress = *(PULONG32)(NotifyItem & 0xFFFFFFF8 + sizeof(ULONG));

					if (CallBackAddress && MmIsAddressValid((PVOID)CallBackAddress))
					{
						if (GetCallback->ulCnt > GetCallback->ulRetCnt)
						{
							GetCallback->Callbacks[GetCallback->ulRetCnt].Type = NotifyLoadImage;
							GetCallback->Callbacks[GetCallback->ulRetCnt].CallbackAddress = CallBackAddress;
							GetCallback->Callbacks[GetCallback->ulRetCnt].Note = NotifyItem;
						}

						GetCallback->ulRetCnt++;
					}
				}
				break;
			}
		}
	}

	return TRUE;
}


BOOLEAN GetRegisterCallbackNotify(PGET_CALLBACK GetCallback)
{
	CmUnRegisterCallbackAddress = 
		(pfnCmUnRegisterCallback)HsGetFunctionAddressByName(L"CmUnRegisterCallback");
	DbgPrint("%p\r\n",CmUnRegisterCallbackAddress);
	CallBackList = 
		CmpCallBackVector((ULONG_PTR)CmUnRegisterCallbackAddress);
	DbgPrint("%p\r\n",CallBackList);

	//DbgPrint("%02x\r\n",*(char*)((char*)CmUnRegisterCallbackAddress+0xe));

	if (!CallBackList)
	{
		return FALSE;
	}

	else
	{

		ULONG i = 0;
		switch(WinVersion)
		{
		case WINDOWS_7:   //这是个List
			{
				PCM_NOTIFY_ENTRY	Notify = NULL;
				PLIST_ENTRY			NotifyList = (LIST_ENTRY*)(*(ULONG_PTR*)CallBackList);
				SYSTEM_ADDRESS_START = 0x80000000000;  

				do
				{		
					Notify = (CM_NOTIFY_ENTRY *)NotifyList;
					if (MmIsAddressValid(Notify))
					{
						if (MmIsAddressValid((PVOID)(Notify->Function)) && Notify->Function > SYSTEM_ADDRESS_START)
						{
							if (GetCallback->ulCnt > GetCallback->ulRetCnt)
							{
								GetCallback->Callbacks[GetCallback->ulRetCnt].Type = NotifyCmCallBack;
								GetCallback->Callbacks[GetCallback->ulRetCnt].CallbackAddress = Notify->Function;
								GetCallback->Callbacks[GetCallback->ulRetCnt].Note = Notify->Cookie.QuadPart;
							}

							GetCallback->ulRetCnt++;
						}
					}
					NotifyList = NotifyList->Flink;
				}while ( NotifyList != ((LIST_ENTRY*)(*(ULONG_PTR*)CallBackList)) );
				break;
			}

		case WINDOWS_XP:
			{

				SYSTEM_ADDRESS_START = 0x80000000;
				for ( i = 0; i < 64; i++ )
				{
					ULONG32 NotifyItem = 0, CallBackAddress = 0;
					ULONG   Temp = 0;

					if (!MmIsAddressValid( (PVOID)(CallBackList + i * sizeof(ULONG32))) )
					{
						break;
					}


					NotifyItem = *(PULONG32)(CallBackList + i * sizeof(ULONG32));
					if (!(NotifyItem > SYSTEM_ADDRESS_START && MmIsAddressValid((PVOID)(NotifyItem & 0xFFFFFFF8 + sizeof(ULONG32)))) )
					{
						break;
					}

					CallBackAddress = *(PULONG32)(NotifyItem & 0xFFFFFFF8 + sizeof(ULONG));
					if (CallBackAddress && MmIsAddressValid((PVOID)CallBackAddress))
					{
						if (GetCallback->ulCnt > GetCallback->ulRetCnt)
						{

							GetCallback->Callbacks[GetCallback->ulRetCnt].Type = NotifyCmCallBack;
							GetCallback->Callbacks[GetCallback->ulRetCnt].CallbackAddress = CallBackAddress;
							GetCallback->Callbacks[GetCallback->ulRetCnt].Note = NotifyItem;

						}

						GetCallback->ulRetCnt++;
					}
				}
				break;
			}
		}

	}

}


BOOLEAN GetBugCheckReasonCallbackNotify(PGET_CALLBACK GetCallback)
{

	KeRegisterBugCheckReasonCallbackAddress = 
		(pfnKeRegisterBugCheckReasonCallback)HsGetFunctionAddressByName(L"KeRegisterBugCheckReasonCallback");
	DbgPrint("%p\r\n",KeRegisterBugCheckReasonCallbackAddress);
	KeBugCheckReasonCallbackListHead = 
		FindKeBugCheckReasonCallbackListHeadNotifyRoutine((ULONG_PTR)KeRegisterBugCheckReasonCallbackAddress);
	DbgPrint("%p\r\n",KeBugCheckReasonCallbackListHead);


	if (!KeBugCheckReasonCallbackListHead)
	{
		return FALSE;
	}

	else
	{

		PLIST_ENTRY Entry = NULL;
		ULONG_PTR   Dispatch = 0;

		switch(WinVersion)
		{
		case WINDOWS_7:
			{
				Entry = ((PLIST_ENTRY)KeBugCheckReasonCallbackListHead)->Flink;
				do
				{
					Dispatch = *(ULONG64*)((ULONG64)Entry+sizeof(LIST_ENTRY));
					if(Dispatch&&MmIsAddressValid((PVOID)Dispatch))
					{
						if (GetCallback->ulCnt > GetCallback->ulRetCnt)
						{
							GetCallback->Callbacks[GetCallback->ulRetCnt].Type = NotifyKeBugCheckReason;
							GetCallback->Callbacks[GetCallback->ulRetCnt].CallbackAddress = Dispatch;
							GetCallback->Callbacks[GetCallback->ulRetCnt].Note = (PVOID)Entry;
						}

						GetCallback->ulRetCnt++;

					}
					Entry = Entry->Flink;

				}
				while(Entry != (PLIST_ENTRY)KeBugCheckReasonCallbackListHead);

				break;
			}

		case WINDOWS_XP:
			{
				//在Wrk中搜索 _KBUGCHECK_CALLBACK_RECORD 结构
				Entry = ((PLIST_ENTRY)KeBugCheckReasonCallbackListHead)->Flink;
				do
				{
					Dispatch = *(ULONG32*)((ULONG32)Entry+sizeof(LIST_ENTRY));
					if(Dispatch&&MmIsAddressValid((PVOID)Dispatch))
					{
						if (GetCallback->ulCnt > GetCallback->ulRetCnt)
						{
							GetCallback->Callbacks[GetCallback->ulRetCnt].Type = NotifyKeBugCheckReason;
							GetCallback->Callbacks[GetCallback->ulRetCnt].CallbackAddress = Dispatch;
							GetCallback->Callbacks[GetCallback->ulRetCnt].Note = (PVOID)Entry;;
						}

						GetCallback->ulRetCnt++;

					}
					Entry = Entry->Flink;

				}
				while(Entry != (PLIST_ENTRY)KeBugCheckReasonCallbackListHead);
				break;
			}
		}

	}

}


BOOLEAN GetBugCheckCallbackNotify(PGET_CALLBACK GetCallback)
{

	KeRegisterBugCheckCallbackAddress = 
		(pfnKeRegisterBugCheckCallback)HsGetFunctionAddressByName(L"KeRegisterBugCheckCallback");
	DbgPrint("%p\r\n",KeRegisterBugCheckCallbackAddress);
	KeBugCheckCallbackListHead = 
		FindKeBugCheckReasonCallbackListHeadNotifyRoutine((ULONG_PTR)KeRegisterBugCheckCallbackAddress);
	DbgPrint("%p\r\n",KeBugCheckCallbackListHead);


	if (!KeBugCheckCallbackListHead)
	{
		return FALSE;
	}

	else
	{

		PLIST_ENTRY Entry = NULL;
		ULONG_PTR   Dispatch = 0;

		switch(WinVersion)
		{
		case WINDOWS_7:
			{
				Entry = ((PLIST_ENTRY)KeBugCheckCallbackListHead)->Flink;
				do
				{
					Dispatch = *(ULONG64*)((ULONG64)Entry+sizeof(LIST_ENTRY));
					if(Dispatch&&MmIsAddressValid((PVOID)Dispatch))
					{
						if (GetCallback->ulCnt > GetCallback->ulRetCnt)
						{
							GetCallback->Callbacks[GetCallback->ulRetCnt].Type = NotifyKeBugCheck;
							GetCallback->Callbacks[GetCallback->ulRetCnt].CallbackAddress = Dispatch;
							GetCallback->Callbacks[GetCallback->ulRetCnt].Note = (PVOID)Entry;
						}

						GetCallback->ulRetCnt++;

					}
					Entry = Entry->Flink;

				}
				while(Entry != (PLIST_ENTRY)KeBugCheckCallbackListHead);

				break;
			}

		case WINDOWS_XP:
			{
				//在Wrk中搜索 _KBUGCHECK_CALLBACK_RECORD 结构
				Entry = ((PLIST_ENTRY)KeBugCheckCallbackListHead)->Flink;
				do
				{
					Dispatch = *(ULONG32*)((ULONG32)Entry+sizeof(LIST_ENTRY));
					if(Dispatch&&MmIsAddressValid((PVOID)Dispatch))
					{
						if (GetCallback->ulCnt > GetCallback->ulRetCnt)
						{
							GetCallback->Callbacks[GetCallback->ulRetCnt].Type = NotifyKeBugCheck;
							GetCallback->Callbacks[GetCallback->ulRetCnt].CallbackAddress = Dispatch;
							GetCallback->Callbacks[GetCallback->ulRetCnt].Note = (PVOID)Entry;;
						}

						GetCallback->ulRetCnt++;

					}
					Entry = Entry->Flink;

				}
				while(Entry != (PLIST_ENTRY)KeBugCheckCallbackListHead);
				break;
			}
		}

	}

}


BOOLEAN GetShutDownCallbackNotify(PGET_CALLBACK GetCallback)
{

	IoRegisterShutdownNotificationAddress = 
		(pfnIoRegisterShutdownNotification)HsGetFunctionAddressByName(L"IoRegisterShutdownNotification");
	DbgPrint("%p\r\n",IoRegisterShutdownNotificationAddress);
	IopNotifyShutdownQueueHead = 
		FindIopNotifyShutdownQueueHeadNotifyRoutine((ULONG_PTR)IoRegisterShutdownNotificationAddress);
	DbgPrint("%p\r\n",IopNotifyShutdownQueueHead);


	if (!IopNotifyShutdownQueueHead)
	{
		return FALSE;
	}
	else
	{
		ULONG i = 0;

		switch(WinVersion)
		{
		case WINDOWS_7:
			{

				SYSTEM_ADDRESS_START = 0x80000000000;

				if (IopNotifyShutdownQueueHead && MmIsAddressValid((PVOID)IopNotifyShutdownQueueHead))
				{
					PLIST_ENTRY Entry = ((PLIST_ENTRY)IopNotifyShutdownQueueHead)->Flink;
					while (MmIsAddressValid(Entry) && Entry != (PLIST_ENTRY)IopNotifyShutdownQueueHead)
					{
						ULONG64 ValidAddress = (ULONG64)Entry + sizeof(LIST_ENTRY);

						if (ValidAddress && 
							MmIsAddressValid((PVOID)ValidAddress))
						{
							ULONG64 DeviceObject = *(PULONG64)ValidAddress;

							if (DeviceObject && 
								MmIsAddressValid((PVOID)DeviceObject))
							{
								if (GetCallback->ulCnt > GetCallback->ulRetCnt)
								{
									GetCallback->Callbacks[GetCallback->ulRetCnt].Type = NotifyShutdown;
									GetCallback->Callbacks[GetCallback->ulRetCnt].CallbackAddress = GetShutdownDispatch((PDEVICE_OBJECT)DeviceObject);
									GetCallback->Callbacks[GetCallback->ulRetCnt].Note = DeviceObject;
								}

								GetCallback->ulRetCnt++;
							}
						}

						Entry = Entry->Flink;
					}
				}
				break;
			}

		case WINDOWS_XP:
			{

				SYSTEM_ADDRESS_START = 0x80000000;
				if (IopNotifyShutdownQueueHead && MmIsAddressValid((PVOID)IopNotifyShutdownQueueHead))
				{
					PLIST_ENTRY Entry = ((PLIST_ENTRY)IopNotifyShutdownQueueHead)->Flink;
					while (MmIsAddressValid(Entry) && Entry != (PLIST_ENTRY)IopNotifyShutdownQueueHead)
					{
						ULONG ValidAddress = (ULONG)Entry + sizeof(LIST_ENTRY);

						if (ValidAddress && 
							MmIsAddressValid((PVOID)ValidAddress))
						{
							ULONG DeviceObject = *(PULONG)ValidAddress;

							if (DeviceObject && 
								MmIsAddressValid((PVOID)DeviceObject))
							{
								if (GetCallback->ulCnt > GetCallback->ulRetCnt)
								{
									GetCallback->Callbacks[GetCallback->ulRetCnt].Type = NotifyShutdown;
									GetCallback->Callbacks[GetCallback->ulRetCnt].CallbackAddress = GetShutdownDispatch((PDEVICE_OBJECT)DeviceObject);
									GetCallback->Callbacks[GetCallback->ulRetCnt].Note = DeviceObject;
								}

								GetCallback->ulRetCnt++;
							}
						}

						Entry = Entry->Flink;
					}
				}
				break;
			}
		}

	}

}


BOOLEAN GetCreateThreadCallbackNotify(PGET_CALLBACK GetCallback)
{
	PsSetCreateThreadNotifyRoutineAddress = 
		(pfnPsSetCreateThreadNotifyRoutine)HsGetFunctionAddressByName(L"PsSetCreateThreadNotifyRoutine");
	DbgPrint("%p\r\n",PsSetCreateThreadNotifyRoutineAddress);
	PspCreateThreadNotifyRoutineAddress = 
		FindPspCreateThreadNotifyRoutine((ULONG_PTR)PsSetCreateThreadNotifyRoutineAddress);
	DbgPrint("%p\r\n",PspCreateThreadNotifyRoutineAddress);


	if (!PspCreateThreadNotifyRoutineAddress)
	{
		return FALSE;
	}

	else
	{

		ULONG i = 0;


		switch(WinVersion)
		{
		case WINDOWS_7:
			{

				SYSTEM_ADDRESS_START = 0x80000000000;

				for ( i = 0; i < 64; i++ )
				{
					ULONG64 NotifyItem = 0, CallBackAddress = 0;

					if (!MmIsAddressValid( (PVOID)(PspCreateThreadNotifyRoutineAddress + i * sizeof(ULONG64))) )
					{
						break;
					}

					NotifyItem = *(PULONG64)(PspCreateThreadNotifyRoutineAddress + i * sizeof(ULONG64));

					if (!(NotifyItem > SYSTEM_ADDRESS_START && MmIsAddressValid((PVOID)(NotifyItem & 0xfffffffffffffff8))) )
					{
						break;
					}

					CallBackAddress = *((PULONG64)(NotifyItem & 0xfffffffffffffff8));

					if (CallBackAddress && MmIsAddressValid((PVOID)CallBackAddress))
					{
						if (GetCallback->ulCnt > GetCallback->ulRetCnt)
						{
							GetCallback->Callbacks[GetCallback->ulRetCnt].Type = NotifyCreateThread;
							GetCallback->Callbacks[GetCallback->ulRetCnt].CallbackAddress = CallBackAddress;
							GetCallback->Callbacks[GetCallback->ulRetCnt].Note = NotifyItem;
						}

						GetCallback->ulRetCnt++;
					}



				}
				break;
			}

		case WINDOWS_XP:
			{

				SYSTEM_ADDRESS_START = 0x80000000;
				for ( i = 0; i < 64; i++ )
				{
					ULONG32 NotifyItem = 0, CallBackAddress = 0;

					if (!MmIsAddressValid( (PVOID)(PspCreateThreadNotifyRoutineAddress + i * sizeof(ULONG32))) )
					{
						break;
					}

					NotifyItem = *(PULONG32)(PspCreateThreadNotifyRoutineAddress + i * sizeof(ULONG32));
					if (!(NotifyItem > SYSTEM_ADDRESS_START && MmIsAddressValid((PVOID)(NotifyItem & 0xFFFFFFF8 + sizeof(ULONG32)))) )
					{
						break;
					}

					CallBackAddress = *(PULONG32)(NotifyItem & 0xFFFFFFF8 + sizeof(ULONG));

					if (CallBackAddress && MmIsAddressValid((PVOID)CallBackAddress))
					{
						if (GetCallback->ulCnt > GetCallback->ulRetCnt)
						{
							GetCallback->Callbacks[GetCallback->ulRetCnt].Type = NotifyCreateThread;
							GetCallback->Callbacks[GetCallback->ulRetCnt].CallbackAddress = CallBackAddress;
							GetCallback->Callbacks[GetCallback->ulRetCnt].Note = NotifyItem;
						}

						GetCallback->ulRetCnt++;
					}
				}
				break;
			}
		}

	}

}




//////////////////////////////////////////////////////////////////////////


ULONG_PTR CmpCallBackVector(ULONG_PTR Address)
{
	ULONG_PTR i = 0;

	/*
	XP:
	kd> u CmUnRegisterCallback l 20
	nt!CmUnRegisterCallback:
	8061ce6f 8bff            mov     edi,edi
	8061ce71 55              push    ebp
	8061ce72 8bec            mov     ebp,esp
	8061ce74 51              push    ecx
	8061ce75 8365fc00        and     dword ptr [ebp-4],0
	8061ce79 53              push    ebx
	8061ce7a 56              push    esi
	8061ce7b 57              push    edi
	8061ce7c bba0135680      mov     ebx,offset nt!CmpCallBackVector (805613a0)
	8061ce81 53              push    ebx

	kd> u 805613a0
	nt!CmpCallBackVector:
	805613a0 0000            add     byte ptr [eax],al
	805613a2 0000            add     byte ptr [eax],al
	805613a4 0000            add     byte ptr [eax],al
	805613a6 0000            add     byte ptr [eax],al
	805613a8 0000            add     byte ptr [eax],al
	805613aa 0000            add     byte ptr [eax],al
	805613ac 0000            add     byte ptr [eax],al
	805613ae 0000            add     byte ptr [eax],al


	Win7

	kd> u CmUnRegisterCallback l 50
	nt!CmUnRegisterCallback:
	fffff800`0430f790 48894c2408      mov     qword ptr [rsp+8],rcx
	fffff800`0430f795 53              push    rbx
	fffff800`0430f796 56              push    rsi
	fffff800`0430f797 57              push    rdi
	fffff800`0430f798 4154            push    r12
	fffff800`0430f79a 4155            push    r13
	fffff800`0430f79c 4156            push    r14
	fffff800`0430f79e 4157            push    r15
	fffff800`0430f7a0 4883ec60        sub     rsp,60h
	fffff800`0430f7a4 41bc0d0000c0    mov     r12d,0C000000Dh
	fffff800`0430f7aa 4489a424b0000000 mov     dword ptr [rsp+0B0h],r12d
	fffff800`0430f7b2 33db            xor     ebx,ebx
	fffff800`0430f7b4 48895c2448      mov     qword ptr [rsp+48h],rbx
	fffff800`0430f7b9 33c0            xor     eax,eax
	fffff800`0430f7bb 4889442450      mov     qword ptr [rsp+50h],rax
	fffff800`0430f7c0 4889442458      mov     qword ptr [rsp+58h],rax
	fffff800`0430f7c5 448d6b01        lea     r13d,[rbx+1]
	fffff800`0430f7c9 458afd          mov     r15b,r13b
	fffff800`0430f7cc 4488ac24a8000000 mov     byte ptr [rsp+0A8h],r13b
	fffff800`0430f7d4 440f20c7        mov     rdi,cr8
	fffff800`0430f7d8 450f22c5        mov     cr8,r13
	fffff800`0430f7dc f00fba351b6adcff00 lock btr dword ptr [nt!CallbackUnregisterLock (fffff800`040d6200)],0
	fffff800`0430f7e5 720c            jb      nt!CmUnRegisterCallback+0x63 (fffff800`0430f7f3)
	fffff800`0430f7e7 488d0d126adcff  lea     rcx,[nt!CallbackUnregisterLock (fffff800`040d6200)]
	fffff800`0430f7ee e85db7b9ff      call    nt!KiAcquireFastMutex (fffff800`03eaaf50)
	fffff800`0430f7f3 65488b042588010000 mov   rax,qword ptr gs:[188h]
	fffff800`0430f7fc 488905056adcff  mov     qword ptr [nt!CallbackUnregisterLock+0x8 (fffff800`040d6208)],rax
	fffff800`0430f803 400fb6c7        movzx   eax,dil
	fffff800`0430f807 8905236adcff    mov     dword ptr [nt!CallbackUnregisterLock+0x30 (fffff800`040d6230)],eax
	fffff800`0430f80d 48c78424b80000009cffffff mov qword ptr [rsp+0B8h],0FFFFFFFFFFFFFF9Ch
	fffff800`0430f819 48895c2420      mov     qword ptr [rsp+20h],rbx
	fffff800`0430f81e 65488b042588010000 mov   rax,qword ptr gs:[188h]
	fffff800`0430f827 4183ceff        or      r14d,0FFFFFFFFh
	fffff800`0430f82b 664401b0c4010000 add     word ptr [rax+1C4h],r14w
	fffff800`0430f833 f0480fba2dab69dcff00 lock bts qword ptr [nt!CallbackListLock (fffff800`040d61e8)],0
	fffff800`0430f83d 730c            jae     nt!CmUnRegisterCallback+0xbb (fffff800`0430f84b)
	fffff800`0430f83f 488d0da269dcff  lea     rcx,[nt!CallbackListLock (fffff800`040d61e8)]
	fffff800`0430f846 e875aabbff      call    nt!ExfAcquirePushLockExclusive (fffff800`03eca2c0)
	fffff800`0430f84b 418af5          mov     sil,r13b
	fffff800`0430f84e 4c8b9424a0000000 mov     r10,qword ptr [rsp+0A0h]
	fffff800`0430f856 4533c0          xor     r8d,r8d
	fffff800`0430f859 488d542420      lea     rdx,[rsp+20h]
	fffff800`0430f85e 488d0d6b69dcff  lea     rcx,[nt!CallbackListHead (fffff800`040d61d0)]      //注意这里的地址   在Win7 中这里是个链表

	kd> u fffff800`040d61d0
	nt!CallbackListHead:
	fffff800`040d61d0 d0610d          shl     byte ptr [rcx+0Dh],1
	fffff800`040d61d3 0400            add     al,0
	fffff800`040d61d5 f8              clc
	fffff800`040d61d6 ff              ???
	fffff800`040d61d7 ffd0            call    rax
	fffff800`040d61d9 61              ???
	fffff800`040d61da 0d0400f8ff      or      eax,0FFF80004h
	fffff800`040d61df ff00            inc     dword ptr [rax]




	*/
	switch(WinVersion)
	{
	case WINDOWS_7:
		{
			for(i=Address;i<Address+0xFF;i++)
			{
				if(*(PUCHAR)i==0x48 && *(PUCHAR)(i+1)==0x8d && *(PUCHAR)(i+2)==0x0d )
				{
					ULONG_PTR j = 0;
					j = i-5;
					if (*(PUCHAR)j==0x48 && *(PUCHAR)(j+1)==0x8d && *(PUCHAR)(j+2)==0x54)
					{
						LONG OffsetAddr = 0;
						
						memcpy(&OffsetAddr,(PVOID)(i+3),4);

						return OffsetAddr+7+i;
					}
				}
					
			}
			break;
		}

	case WINDOWS_XP:
		{
			for(i=Address;i<Address+0xFF;i++)
			{
				if(*(PUCHAR)i==0xbb && *(PUCHAR)(i-1)==0x57)	
				{
					LONG OffsetAddr = 0;
					memcpy(&OffsetAddr,(PUCHAR)(i+1),4);
					return OffsetAddr;
				}
			}
			break;
		}
	}

	

	return 0;
}


LARGE_INTEGER XpGetRegisterCallbackCookie(ULONG Address)
{

	LARGE_INTEGER Cookie;
	ULONG Temp = 0;
	ULONG Item = 0;

	Cookie.QuadPart = 0;

	if (Address && MmIsAddressValid((PVOID)Address))
	{
		Item = Address & 0xFFFFFFF8;


		if (MmIsAddressValid((PVOID)Item) &&
			MmIsAddressValid((PVOID)(Item + 8)))
		{
			Temp = *(PULONG)(Item + 8);


			if (MmIsAddressValid((PVOID)Temp))
			{
				Cookie.LowPart = *(PULONG)Temp;
				Cookie.HighPart = *(PULONG)(Temp + sizeof(ULONG));
			}
		}
	}

	return Cookie;
}


ULONG_PTR FindPspLoadImageNotifyRoutine(ULONG_PTR Address)
{
	ULONG_PTR i = 0;

	/*
	XP:
	kd> u PsSetLoadImageNotifyRoutine l 20
	nt!PsSetLoadImageNotifyRoutine:
	805d0f90 8bff            mov     edi,edi
	805d0f92 55              push    ebp
	805d0f93 8bec            mov     ebp,esp
	805d0f95 53              push    ebx
	805d0f96 57              push    edi
	805d0f97 33ff            xor     edi,edi
	805d0f99 57              push    edi
	805d0f9a ff7508          push    dword ptr [ebp+8]
	805d0f9d e8faca0300      call    nt!ExAllocateCallBack (8060da9c)
	805d0fa2 8bd8            mov     ebx,eax
	805d0fa4 3bdf            cmp     ebx,edi
	805d0fa6 7507            jne     nt!PsSetLoadImageNotifyRoutine+0x1f (805d0faf)
	805d0fa8 b89a0000c0      mov     eax,0C000009Ah
	805d0fad eb2a            jmp     nt!PsSetLoadImageNotifyRoutine+0x49 (805d0fd9)
	805d0faf 56              push    esi
	805d0fb0 bee0495680      mov     esi,offset nt!PspLoadImageNotifyRoutine (805649e0)
	805d0fb5 6a00            push    0
	805d0fb7 53              push    ebx
	805d0fb8 56              push    esi
	805d0fb9 e80ecb0300      call    nt!ExCompareExchangeCallBack (8060dacc)
	805d0fbe 84c0            test    al,al
	805d0fc0 751d            jne     nt!PsSetLoadImageNotifyRoutine+0x4f (805d0fdf)
	805d0fc2 83c704          add     edi,4
	805d0fc5 83c604          add     esi,4
	805d0fc8 83ff20          cmp     edi,20h
	805d0fcb 72e8            jb      nt!PsSetLoadImageNotifyRoutine+0x25 (805d0fb5)
	805d0fcd 53              push    ebx
	805d0fce e83f1a0000      call    nt!RtlpFreeAtom (805d2a12)
	805d0fd3 b89a0000c0      mov     eax,0C000009Ah
	805d0fd8 5e              pop     esi
	805d0fd9 5f              pop     edi
	805d0fda 5b              pop     ebx
	kd> u 805649e0
	nt!PspLoadImageNotifyRoutine:
	805649e0 0000            add     byte ptr [eax],al
	805649e2 0000            add     byte ptr [eax],al
	805649e4 0000            add     byte ptr [eax],al
	805649e6 0000            add     byte ptr [eax],al
	805649e8 0000            add     byte ptr [eax],al
	805649ea 0000            add     byte ptr [eax],al
	805649ec 0000            add     byte ptr [eax],al
	805649ee 0000            add     byte ptr [eax],al

	Win7

	kd> u PsSetLoadImageNotifyRoutine l 20
	nt!PsSetLoadImageNotifyRoutine:
	fffff800`0429db60 48895c2408      mov     qword ptr [rsp+8],rbx
	fffff800`0429db65 57              push    rdi
	fffff800`0429db66 4883ec20        sub     rsp,20h
	fffff800`0429db6a 33d2            xor     edx,edx
	fffff800`0429db6c e8efaffeff      call    nt!ExAllocateCallBack (fffff800`04288b60)
	fffff800`0429db71 488bf8          mov     rdi,rax
	fffff800`0429db74 4885c0          test    rax,rax
	fffff800`0429db77 7507            jne     nt!PsSetLoadImageNotifyRoutine+0x20 (fffff800`0429db80)
	fffff800`0429db79 b89a0000c0      mov     eax,0C000009Ah
	fffff800`0429db7e eb4a            jmp     nt!PsSetLoadImageNotifyRoutine+0x6a (fffff800`0429dbca)
	fffff800`0429db80 33db            xor     ebx,ebx
	fffff800`0429db82 488d0d7799d9ff  lea     rcx,[nt!PspLoadImageNotifyRoutine (fffff800`04037500)]


	kd> u fffff800`04037500
	nt!PspLoadImageNotifyRoutine:
	fffff800`04037500 ff6014          jmp     qword ptr [rax+14h]
	fffff800`04037503 00a0f8ffff00    add     byte ptr [rax+0FFFFF8h],ah
	fffff800`04037509 0000            add     byte ptr [rax],al
	fffff800`0403750b 0000            add     byte ptr [rax],al
	fffff800`0403750d 0000            add     byte ptr [rax],al
	fffff800`0403750f 0000            add     byte ptr [rax],al
	fffff800`04037511 0000            add     byte ptr [rax],al
	fffff800`04037513 0000            add     byte ptr [rax],al



	*/
	switch(WinVersion)
	{
	case WINDOWS_7:
		{
			for(i=Address;i<Address+0xFF;i++)
			{
				if(*(PUCHAR)i==0x48 && *(PUCHAR)(i+1)==0x8d && *(PUCHAR)(i+2)==0x0d)	//lea rcx,xxxx
				{
					LONG OffsetAddr = 0;
					memcpy(&OffsetAddr,(PUCHAR)(i+3),4);
					return OffsetAddr+7+i;
				}
			}
			break;
		}

	case WINDOWS_XP:
		{
// 			UCHAR TempChar = *(UCHAR*)((UCHAR*)IoRegisterShutdownNotificationAddress+0xe);
// 
// 			DbgPrint("%x\r\n",TempChar);

			for(i=Address;i<Address+0xFF;i++)
			{
				if(*(PUCHAR)i==0x56 && *(PUCHAR)(i+1)==0xbe)	//lea rcx,xxxx
				{
					LONG OffsetAddr = 0;
					memcpy(&OffsetAddr,(PUCHAR)(i+2),4);
					return OffsetAddr;
				}
			}
			break;
		}
	}

	

	return 0;
}


ULONG_PTR FindKeBugCheckReasonCallbackListHeadNotifyRoutine(ULONG_PTR Address)
{
	ULONG_PTR i = 0;

	/*
	XP:
	kd> u KeRegisterBugCheckReasonCallback l 20
	nt!KeRegisterBugCheckReasonCallback:
	8050f0c1 8bff            mov     edi,edi
	8050f0c3 55              push    ebp
	8050f0c4 8bec            mov     ebp,esp
	8050f0c6 51              push    ecx
	8050f0c7 53              push    ebx
	8050f0c8 57              push    edi
	8050f0c9 b11f            mov     cl,1Fh
	8050f0cb ff152c904d80    call    dword ptr [nt!_imp_KfRaiseIrql (804d902c)]
	8050f0d1 bf6c355680      mov     edi,offset nt!KeBugCheckCallbackLock (8056356c)
	8050f0d6 8bcf            mov     ecx,edi
	8050f0d8 8845ff          mov     byte ptr [ebp-1],al
	8050f0db e86843fdff      call    nt!KiAcquireSpinLock (804e3448)
	8050f0e0 8b4508          mov     eax,dword ptr [ebp+8]
	8050f0e3 32db            xor     bl,bl
	8050f0e5 385818          cmp     byte ptr [eax+18h],bl
	8050f0e8 7538            jne     nt!KeRegisterBugCheckReasonCallback+0x61 (8050f122)
	8050f0ea 8b4d0c          mov     ecx,dword ptr [ebp+0Ch]
	8050f0ed 8b5510          mov     edx,dword ptr [ebp+10h]
	8050f0f0 894808          mov     dword ptr [eax+8],ecx
	8050f0f3 56              push    esi
	8050f0f4 8b7514          mov     esi,dword ptr [ebp+14h]
	8050f0f7 03ca            add     ecx,edx
	8050f0f9 03ce            add     ecx,esi
	8050f0fb 89700c          mov     dword ptr [eax+0Ch],esi
	8050f0fe 895014          mov     dword ptr [eax+14h],edx
	8050f101 894810          mov     dword ptr [eax+10h],ecx
	8050f104 c6401801        mov     byte ptr [eax+18h],1
	8050f108 8b0d70355680    mov     ecx,dword ptr [nt!KeBugCheckReasonCallbackListHead (80563570)]



	kd> u 80563570
	nt!KeBugCheckReasonCallbackListHead:
	80563570 809afdba602bbb  sbb     byte ptr [edx+2B60BAFDh],0BBh
	80563577 ba38ca6589      mov     edx,8965CA38h
	8056357c c02071          shl     byte ptr [eax],71h
	8056357f 800000          add     byte ptr [eax],0
	80563582 0000            add     byte ptr [eax],al
	80563584 0000            add     byte ptr [eax],al
	80563586 0000            add     byte ptr [eax],al


	Win7

	kd> u KeRegisterBugCheckReasonCallback l 50
	nt!KeRegisterBugCheckReasonCallback:
	fffff800`03f3d390 48895c2418      mov     qword ptr [rsp+18h],rbx
	fffff800`03f3d395 4c894c2420      mov     qword ptr [rsp+20h],r9
	fffff800`03f3d39a 4889542410      mov     qword ptr [rsp+10h],rdx
	fffff800`03f3d39f 55              push    rbp
	fffff800`03f3d3a0 56              push    rsi
	fffff800`03f3d3a1 57              push    rdi
	fffff800`03f3d3a2 4154            push    r12
	fffff800`03f3d3a4 4155            push    r13
	fffff800`03f3d3a6 4156            push    r14
	fffff800`03f3d3a8 4157            push    r15
	fffff800`03f3d3aa 4883ec30        sub     rsp,30h
	fffff800`03f3d3ae bf01000000      mov     edi,1
	fffff800`03f3d3b3 4963e8          movsxd  rbp,r8d
	fffff800`03f3d3b6 488bd9          mov     rbx,rcx
	fffff800`03f3d3b9 448aef          mov     r13b,dil
	fffff800`03f3d3bc 440f20c0        mov     rax,cr8
	fffff800`03f3d3c0 4889442470      mov     qword ptr [rsp+70h],rax
	fffff800`03f3d3c5 8d470e          lea     eax,[rdi+0Eh]
	fffff800`03f3d3c8 440f22c0        mov     cr8,rax
	fffff800`03f3d3cc 65488b342520000000 mov   rsi,qword ptr gs:[20h]
	fffff800`03f3d3d5 4533c9          xor     r9d,r9d
	fffff800`03f3d3d8 0fba25a4120e0010 bt      dword ptr [nt!PerfGlobalGroupMask+0x4 (fffff800`0401e684)],10h
	fffff800`03f3d3e0 7318            jae     nt!KeRegisterBugCheckReasonCallback+0x6a (fffff800`03f3d3fa)
	fffff800`03f3d3e2 448ae7          mov     r12b,dil
	fffff800`03f3d3e5 0f31            rdtsc
	fffff800`03f3d3e7 448bbe00470000  mov     r15d,dword ptr [rsi+4700h]
	fffff800`03f3d3ee 48c1e220        shl     rdx,20h
	fffff800`03f3d3f2 480bc2          or      rax,rdx
	fffff800`03f3d3f5 4c8bf0          mov     r14,rax
	fffff800`03f3d3f8 eb0d            jmp     nt!KeRegisterBugCheckReasonCallback+0x77 (fffff800`03f3d407)
	fffff800`03f3d3fa 4c8b742470      mov     r14,qword ptr [rsp+70h]
	fffff800`03f3d3ff 448b7c2470      mov     r15d,dword ptr [rsp+70h]
	fffff800`03f3d404 4532e4          xor     r12b,r12b
	fffff800`03f3d407 01be004b0000    add     dword ptr [rsi+4B00h],edi
	fffff800`03f3d40d f0480fba2d3927150000 lock bts qword ptr [nt!KeBugCheckCallbackLock (fffff800`0408fb50)],0
	fffff800`03f3d417 731d            jae     nt!KeRegisterBugCheckReasonCallback+0xa6 (fffff800`03f3d436)
	fffff800`03f3d419 488d0d30271500  lea     rcx,[nt!KeBugCheckCallbackLock (fffff800`0408fb50)]
	fffff800`03f3d420 e8ab1ef4ff      call    nt!KxWaitForSpinLockAndAcquire (fffff800`03e7f2d0)
	fffff800`03f3d425 01be044b0000    add     dword ptr [rsi+4B04h],edi
	fffff800`03f3d42b 0186084b0000    add     dword ptr [rsi+4B08h],eax
	fffff800`03f3d431 448bc8          mov     r9d,eax
	fffff800`03f3d434 eb03            jmp     nt!KeRegisterBugCheckReasonCallback+0xa9 (fffff800`03f3d439)
	fffff800`03f3d436 0faee8          lfence
	fffff800`03f3d439 4584e4          test    r12b,r12b
	fffff800`03f3d43c 7428            je      nt!KeRegisterBugCheckReasonCallback+0xd6 (fffff800`03f3d466)
	fffff800`03f3d43e 0f31            rdtsc
	fffff800`03f3d440 48c1e220        shl     rdx,20h
	fffff800`03f3d444 488d0d05271500  lea     rcx,[nt!KeBugCheckCallbackLock (fffff800`0408fb50)]
	fffff800`03f3d44b c644242800      mov     byte ptr [rsp+28h],0
	fffff800`03f3d450 480bc2          or      rax,rdx
	fffff800`03f3d453 44897c2420      mov     dword ptr [rsp+20h],r15d
	fffff800`03f3d458 448bc0          mov     r8d,eax
	fffff800`03f3d45b 488bd0          mov     rdx,rax
	fffff800`03f3d45e 452bc6          sub     r8d,r14d
	fffff800`03f3d461 e86a66feff      call    nt!PerfLogSpinLockAcquire (fffff800`03f23ad0)
	fffff800`03f3d466 807b2c00        cmp     byte ptr [rbx+2Ch],0
	fffff800`03f3d46a 7570            jne     nt!KeRegisterBugCheckReasonCallback+0x14c (fffff800`03f3d4dc)
	fffff800`03f3d46c 488b542478      mov     rdx,qword ptr [rsp+78h]
	fffff800`03f3d471 488b8c2488000000 mov     rcx,qword ptr [rsp+88h]
	fffff800`03f3d479 896b28          mov     dword ptr [rbx+28h],ebp
	fffff800`03f3d47c 488d042a        lea     rax,[rdx+rbp]
	fffff800`03f3d480 48895310        mov     qword ptr [rbx+10h],rdx
	fffff800`03f3d484 48894b18        mov     qword ptr [rbx+18h],rcx
	fffff800`03f3d488 4803c1          add     rax,rcx
	fffff800`03f3d48b 40887b2c        mov     byte ptr [rbx+2Ch],dil
	fffff800`03f3d48f 48894320        mov     qword ptr [rbx+20h],rax
	fffff800`03f3d493 83fd04          cmp     ebp,4
	fffff800`03f3d496 7422            je      nt!KeRegisterBugCheckReasonCallback+0x12a (fffff800`03f3d4ba)
	fffff800`03f3d498 488b05d1261500  mov     rax,qword ptr [nt!KeBugCheckReasonCallbackListHead (fffff800`0408fb70)]
	fffff800`03f3d49f 488d0dca261500  lea     rcx,[nt!KeBugCheckReasonCallbackListHead (fffff800`0408fb70)]

	kd> u fffff800`0408fb70
	nt!KeBugCheckReasonCallbackListHead:
	fffff800`0408fb70 58              pop     rax
	fffff800`0408fb71 52              push    rdx
	fffff800`0408fb72 de18            ficomp  word ptr [rax]
	fffff800`0408fb74 80faff          cmp     dl,0FFh
	fffff800`0408fb77 ffe0            jmp     rax
	fffff800`0408fb79 011d0180f8ff    add     dword ptr [nt!KeNodeBlock+0x180 (fffff800`04017b80)],ebx
	fffff800`0408fb7f ffa0ce651980    jmp     qword ptr [rax-7FE69A32h]
	fffff800`0408fb85 fa              cli

	*/
	switch(WinVersion)
	{
	case WINDOWS_7:
		{
			for(i=Address;i<Address+0xFFF;i++)
			{
				if(*(PUCHAR)i==0x48 && *(PUCHAR)(i+1)==0x8b && *(PUCHAR)(i+2)==0x05)	
				{
					LONG OffsetAddr = 0;
					memcpy(&OffsetAddr,(PUCHAR)(i+3),4);
					return OffsetAddr+7+i;
				}
			}
			break;
		}

	case WINDOWS_XP:
		{
			for(i=Address;i<Address+0xFF;i++)
			{
				if(*(PUCHAR)i==0x8b && *(PUCHAR)(i+1)==0x0d)	
				{
					LONG OffsetAddr = 0;
					memcpy(&OffsetAddr,(PUCHAR)(i+2),4);
					return OffsetAddr;
				}
			}
			break;
		}
	}

	

	return 0;
}


ULONG_PTR FindIopNotifyShutdownQueueHeadNotifyRoutine(ULONG_PTR Address)
{
	ULONG_PTR i = 0;

	/*
	XP:
	kd> u IoRegisterShutdownNotification l 20
	nt!IoRegisterShutdownNotification:
	805b6b58 8bff            mov     edi,edi
	805b6b5a 55              push    ebp
	805b6b5b 8bec            mov     ebp,esp
	805b6b5d 57              push    edi
	805b6b5e 68496f5368      push    68536F49h
	805b6b63 6a0c            push    0Ch
	805b6b65 6a00            push    0
	805b6b67 e899b4f9ff      call    nt!ExAllocatePoolWithTag (80552005)
	805b6b6c 8bf8            mov     edi,eax
	805b6b6e 85ff            test    edi,edi
	805b6b70 0f84748d0300    je      nt!IoRegisterShutdownNotification+0x1a (805ef8ea)
	805b6b76 56              push    esi
	805b6b77 8b7508          mov     esi,dword ptr [ebp+8]
	805b6b7a 8bce            mov     ecx,esi
	805b6b7c 897708          mov     dword ptr [edi+8],esi
	805b6b7f e8df44f2ff      call    nt!ObfReferenceObject (804db063)
	805b6b84 8bd7            mov     edx,edi
	805b6b86 b9601e5680      mov     ecx,offset nt!IopNotifyShutdownQueueHead (80561e60)
	805b6b8b e83c5ff5ff      call    nt!IopInterlockedInsertHeadList (8050cacc)


	kd> u IoRegisterShutdownNotification l 20
	nt!IoRegisterShutdownNotification:
	8056ab64 8bff            mov     edi,edi
	8056ab66 55              push    ebp
	8056ab67 8bec            mov     ebp,esp
	8056ab69 57              push    edi
	8056ab6a 68496f5368      push    68536F49h
	8056ab6f 6a0c            push    0Ch
	8056ab71 6a00            push    0
	8056ab73 e808b3fdff      call    nt!ExAllocatePoolWithTag (80545e80)
	8056ab78 8bf8            mov     edi,eax
	8056ab7a 85ff            test    edi,edi
	8056ab7c 7507            jne     nt!IoRegisterShutdownNotification+0x21 (8056ab85)
	8056ab7e b89a0000c0      mov     eax,0C000009Ah
	8056ab83 eb21            jmp     nt!IoRegisterShutdownNotification+0x42 (8056aba6)
	8056ab85 56              push    esi
	8056ab86 8b7508          mov     esi,dword ptr [ebp+8]
	8056ab89 8bce            mov     ecx,esi
	8056ab8b 897708          mov     dword ptr [edi+8],esi
	8056ab8e e8bd8dfbff      call    nt!ObfReferenceObject (80523950)
	8056ab93 8bd7            mov     edx,edi
	8056ab95 b9e0285580      mov     ecx,offset nt!IopNotifyShutdownQueueHead (805528e0)
	8056ab9a e8d59bf8ff      call    nt!IopInterlockedInsertHeadList (804f4774)


	kd> dd 80561e60
	80561e60  89616ba8 897d8150 80561e68 80561e68
	80561e70  80561e70 80561e70 897d223c 897d223c
	80561e80  8958052c 8973b7b4 897041fc 897d35d4
	80561e90  895844d4 8981a5d4 00000000 00000000
	80561ea0  80561ba0 80561ee0 89806340 00000000
	80561eb0  00000000 00000000 00000000 00000000
	80561ec0  00000000 00000000 00000000 00000000
	80561ed0  00000000 00000000 00000000 00000000


	Win7

	kd> u IoRegisterShutdownNotification l 20
	nt!IoRegisterShutdownNotification:
	fffff800`0428bf20 48895c2408      mov     qword ptr [rsp+8],rbx
	fffff800`0428bf25 57              push    rdi
	fffff800`0428bf26 4883ec20        sub     rsp,20h
	fffff800`0428bf2a 488bd9          mov     rbx,rcx
	fffff800`0428bf2d ba18000000      mov     edx,18h
	fffff800`0428bf32 41b8496f5368    mov     r8d,68536F49h
	fffff800`0428bf38 33c9            xor     ecx,ecx
	fffff800`0428bf3a e8a111d3ff      call    nt!ExAllocatePoolWithTag (fffff800`03fbd0e0)
	fffff800`0428bf3f 488bf8          mov     rdi,rax
	fffff800`0428bf42 4885c0          test    rax,rax
	fffff800`0428bf45 7507            jne     nt!IoRegisterShutdownNotification+0x2e (fffff800`0428bf4e)
	fffff800`0428bf47 b89a0000c0      mov     eax,0C000009Ah
	fffff800`0428bf4c eb22            jmp     nt!IoRegisterShutdownNotification+0x50 (fffff800`0428bf70)
	fffff800`0428bf4e 488bcb          mov     rcx,rbx
	fffff800`0428bf51 48895810        mov     qword ptr [rax+10h],rbx
	fffff800`0428bf55 e8962fc1ff      call    nt!ObfReferenceObject (fffff800`03e9eef0)
	fffff800`0428bf5a 488d0d8f32e0ff  lea     rcx,[nt!IopNotifyShutdownQueueHead (fffff800`0408f1f0)]
	fffff800`0428bf61 488bd7          mov     rdx,rdi

	kd> dq fffff800`0408f1f0
	fffff800`0408f1f0  fffffa80`197d9170 fffffa80`18e1b280
	fffff800`0408f200  fffff800`0408f200 fffff800`0408f200
	fffff800`0408f210  fffff800`0408f210 fffff800`0408f210
	fffff800`0408f220  fffffa80`18d46e90 fffffa80`18d46e90
	fffff800`0408f230  fffffa80`191f0680 fffffa80`191f0680
	fffff800`0408f240  fffffa80`191067d0 fffffa80`18d460b0
	fffff800`0408f250  fffffa80`191077d0 fffffa80`18d452a0
	fffff800`0408f260  fffff800`0408ecc0 fffff800`0408f360

	*/
	switch(WinVersion)
	{
	case WINDOWS_7:
		{
			for(i=Address;i<Address+0xFF;i++)
			{
				if(*(PUCHAR)i==0x48 && *(PUCHAR)(i+1)==0x8d && *(PUCHAR)(i+2)==0x0d)
				{
					LONG OffsetAddr = 0;
					memcpy(&OffsetAddr,(PUCHAR)(i+3),4);
					return OffsetAddr+7+i;
				}
			}
			break;
		}

	case WINDOWS_XP:
		{
			for(i=Address;i<Address+0xFF;i++)
			{
				if(*(PUCHAR)i==0xb9 && *(PUCHAR)(i-1)==0xd7)
				{
					LONG OffsetAddr = 0;
					memcpy(&OffsetAddr,(PUCHAR)(i+1),4);
					return OffsetAddr;
				}
			}
			break;
		}
	}

	return 0;
}


ULONG_PTR FindPspCreateThreadNotifyRoutine(ULONG_PTR Address)
{
	ULONG_PTR i = 0;

	/*
	XP:
	kd> u PsSetCreateThreadNotifyRoutine l 20
	nt!PsSetCreateThreadNotifyRoutine:
	8063622d 8bff            mov     edi,edi
	8063622f 55              push    ebp
	80636230 8bec            mov     ebp,esp
	80636232 53              push    ebx
	80636233 57              push    edi
	80636234 33ff            xor     edi,edi
	80636236 57              push    edi
	80636237 ff7508          push    dword ptr [ebp+8]
	8063623a e829760100      call    nt!ExAllocateCallBack (8064d868)
	8063623f 8bd8            mov     ebx,eax
	80636241 3bdf            cmp     ebx,edi
	80636243 7507            jne     nt!PsSetCreateThreadNotifyRoutine+0x1f (8063624c)
	80636245 b89a0000c0      mov     eax,0C000009Ah
	8063624a eb2a            jmp     nt!PsSetCreateThreadNotifyRoutine+0x49 (80636276)
	8063624c 56              push    esi
	8063624d bea0a75680      mov     esi,offset nt!PspCreateThreadNotifyRoutine (8056a7a0)

	kd> u 8056a7a0
	nt!PspCreateThreadNotifyRoutine:
	8056a7a0 0000            add     byte ptr [eax],al
	8056a7a2 0000            add     byte ptr [eax],al
	8056a7a4 0000            add     byte ptr [eax],al
	8056a7a6 0000            add     byte ptr [eax],al
	8056a7a8 0000            add     byte ptr [eax],al
	8056a7aa 0000            add     byte ptr [eax],al
	8056a7ac 0000            add     byte ptr [eax],al
	8056a7ae 0000            add     byte ptr [eax],al


	Win7

	kd>  u PsSetCreateThreadNotifyRoutine l 20
	nt!PsSetCreateThreadNotifyRoutine:
	fffff800`042ddbe0 48895c2408      mov     qword ptr [rsp+8],rbx
	fffff800`042ddbe5 57              push    rdi
	fffff800`042ddbe6 4883ec20        sub     rsp,20h
	fffff800`042ddbea 33d2            xor     edx,edx
	fffff800`042ddbec e86faffeff      call    nt!ExAllocateCallBack (fffff800`042c8b60)
	fffff800`042ddbf1 488bf8          mov     rdi,rax
	fffff800`042ddbf4 4885c0          test    rax,rax
	fffff800`042ddbf7 7507            jne     nt!PsSetCreateThreadNotifyRoutine+0x20 (fffff800`042ddc00)
	fffff800`042ddbf9 b89a0000c0      mov     eax,0C000009Ah
	fffff800`042ddbfe eb4a            jmp     nt!PsSetCreateThreadNotifyRoutine+0x6a (fffff800`042ddc4a)
	fffff800`042ddc00 33db            xor     ebx,ebx
	fffff800`042ddc02 488d0d5799d9ff  lea     rcx,[nt!PspCreateThreadNotifyRoutine (fffff800`04077560)]
	fffff800`042ddc09 4533c0          xor     r8d,r8d

	kd> u fffff800`04077560
	nt!PspCreateThreadNotifyRoutine:
	fffff800`04077560 0000            add     byte ptr [rax],al
	fffff800`04077562 0000            add     byte ptr [rax],al
	fffff800`04077564 0000            add     byte ptr [rax],al
	fffff800`04077566 0000            add     byte ptr [rax],al
	fffff800`04077568 0000            add     byte ptr [rax],al
	fffff800`0407756a 0000            add     byte ptr [rax],al
	fffff800`0407756c 0000            add     byte ptr [rax],al
	fffff800`0407756e 0000            add     byte ptr [rax],al




	*/
	switch(WinVersion)
	{
	case WINDOWS_7:
		{
			for(i=Address;i<Address+0xFF;i++)
			{
				if(*(PUCHAR)i==0x48 && *(PUCHAR)(i+1)==0x8d && *(PUCHAR)(i+2)==0x0d)	
				{
					LONG OffsetAddr = 0;
					memcpy(&OffsetAddr,(PUCHAR)(i+3),4);
					return OffsetAddr+7+i;
				}
			}
			break;
		}

	case WINDOWS_XP:
		{
			for(i=Address;i<Address+0xFF;i++)
			{
				if(*(PUCHAR)i==0xbe && *(PUCHAR)(i+1)==0xa0)	
				{
					LONG OffsetAddr = 0;
					memcpy(&OffsetAddr,(PUCHAR)(i+1),4);
					return OffsetAddr;
				}
			}
			break;
		}
	}

	

	return 0;
}


ULONG_PTR GetShutdownDispatch(PDEVICE_OBJECT DeviceObject)
{
	PDRIVER_OBJECT DriverObject = NULL;
	ULONG_PTR ShutdownDispatch = 0;


	if (DeviceObject && MmIsAddressValid((PVOID)DeviceObject))
	{
		DriverObject = DeviceObject->DriverObject;
		if (DriverObject && MmIsAddressValid((PVOID)DriverObject))
		{
			ShutdownDispatch = (ULONG_PTR)DriverObject->MajorFunction[IRP_MJ_SHUTDOWN];
		}
	}

	return ShutdownDispatch;
}