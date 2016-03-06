#pragma once 


#if DBG
#define dprintf DbgPrint
#else
#define dprintf
#endif

#include <ntifs.h>
#include "common.h"


typedef enum _CALLBACK_TYPE_
{
	NotifyCreateProcess,
	NotifyCreateThread,
	NotifyLoadImage,
	NotifyShutdown,
	NotifyCmCallBack,
	NotifyKeBugCheckReason,
	NotifyKeBugCheck
}CALLBACK_TYPE;

typedef struct _REMOVE_CALLBACK
{
	CALLBACK_TYPE NotifyType;
	ULONG_PTR     CallbackAddress;
	ULONG_PTR     Note;
}REMOVE_CALLBACK,*PREMOVE_CALLBACK;

typedef struct _CM_NOTIFY_ENTRY
{
	LIST_ENTRY		ListEntryHead;
	ULONG			UnKnown1;
	ULONG			UnKnown2;
	LARGE_INTEGER	Cookie;
	ULONG64			Context; 
	ULONG64			Function;
}CM_NOTIFY_ENTRY, *PCM_NOTIFY_ENTRY;

typedef struct _CALLBACK_INFO_
{
	CALLBACK_TYPE Type;
	ULONG_PTR     CallbackAddress;
	ULONG_PTR     Note;
}CALLBACK_INFO, *PCALLBACK_INFO;

typedef struct _GET_CALLBACK_
{
	ULONG_PTR ulCnt;
	ULONG_PTR ulRetCnt;
	CALLBACK_INFO Callbacks[1];
}GET_CALLBACK, *PGET_CALLBACK;


typedef
	NTSTATUS (*pfnPsSetLoadImageNotifyRoutine)(PVOID NotifyRoutine);

typedef
	NTSTATUS (*pfnCmUnRegisterCallback)(LARGE_INTEGER  Cookie);

typedef
	NTKERNELAPI
	BOOLEAN
	(*pfnKeRegisterBugCheckReasonCallback)(
	PKBUGCHECK_REASON_CALLBACK_RECORD CallbackRecord,
	PKBUGCHECK_REASON_CALLBACK_ROUTINE CallbackRoutine,
	KBUGCHECK_CALLBACK_REASON Reason,
	PUCHAR Component);

typedef
	NTKERNELAPI
	NTSTATUS
	(*pfnKeRegisterBugCheckCallback)(
	PKBUGCHECK_CALLBACK_RECORD CallbackRecord,
	PKBUGCHECK_CALLBACK_ROUTINE CallbackRoutine,
	PVOID Buffer,
	ULONG Length,
	PUCHAR Component
	);

typedef
	NTKERNELAPI
	NTSTATUS
	(*pfnIoRegisterShutdownNotification)(
	IN PDEVICE_OBJECT DeviceObject);

typedef
	NTKERNELAPI
	NTSTATUS
	(*pfnPsSetCreateThreadNotifyRoutine)(
	PCREATE_THREAD_NOTIFY_ROUTINE NotifyRoutine);




//////////////////////////////////////////////////////////////////////////

NTSTATUS HsEnumCallBackList(int InputBuffer, PVOID OutputBuffer);

ULONG_PTR FindPspLoadImageNotifyRoutine(ULONG_PTR Address);

ULONG_PTR FindKeBugCheckReasonCallbackListHeadNotifyRoutine(ULONG_PTR Address);

ULONG_PTR FindIopNotifyShutdownQueueHeadNotifyRoutine(ULONG_PTR Address);

ULONG_PTR FindPspCreateThreadNotifyRoutine(ULONG_PTR Address);

BOOLEAN GetLoadImageCallbackNotify(PGET_CALLBACK GetCallback);

BOOLEAN GetRegisterCallbackNotify(PGET_CALLBACK GetCallback);

BOOLEAN GetBugCheckReasonCallbackNotify(PGET_CALLBACK GetCallback);

BOOLEAN GetBugCheckCallbackNotify(PGET_CALLBACK GetCallback);

BOOLEAN GetShutDownCallbackNotify(PGET_CALLBACK GetCallback);

BOOLEAN GetCreateThreadCallbackNotify(PGET_CALLBACK GetCallback);

ULONG_PTR CmpCallBackVector(ULONG_PTR Address);

NTSTATUS GetAllCallbackNotify(PVOID OutBuffer);

NTSTATUS RemoveCallbackNotify(PVOID InBuffer);

LARGE_INTEGER XpGetRegisterCallbackCookie(ULONG Address);

ULONG_PTR GetShutdownDispatch(PDEVICE_OBJECT DeviceObject);




