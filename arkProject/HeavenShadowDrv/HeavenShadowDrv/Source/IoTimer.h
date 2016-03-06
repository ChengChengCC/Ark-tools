#pragma once 


#if DBG
#define dprintf DbgPrint
#else
#define dprintf
#endif

#include <ntifs.h>
#include "common.h"

typedef struct _IO_TIMER64
{
	short		Type;
	short		TimerFlag;
	long		Unknown;
	LIST_ENTRY	TimerList;
	PVOID		TimerRoutine;
	PVOID		Context;
	PVOID		DeviceObject;
} IO_TIMER64,*PIO_TIMER64;


typedef struct _IO_TIMER32
{
	short		Type;
	short		TimerFlag;
	LIST_ENTRY	TimerList;
	PVOID		TimerRoutine;
	PVOID		Context;
	PVOID		DeviceObject;
} IO_TIMER32,*PIO_TIMER32;

#ifdef _WIN64
#define PIO_TIMER PIO_TIMER64
#define IO_TIMER  IO_TIMER64
#else
#define PIO_TIMER PIO_TIMER32
#define IO_TIMER  IO_TIMER32
#endif


typedef struct _IO_TIMERS_
{
	ULONG_PTR TimerObject;
	ULONG_PTR DeviceObject;
	ULONG_PTR TimeDispatch;
	ULONG_PTR TimerEntry;
	ULONG     Status;
}IO_TIMERS, *PIO_TIMERS;

typedef struct _IO_TIMER_INFOR_
{
	ULONG ulCnt;
	ULONG ulRetCnt;
	IO_TIMERS IoTimer[1];
}IO_TIMER_INFOR, *PIO_TIMER_INFOR;



typedef struct _PCOMMUNICATE_IO_TIMER_  
{
	PLIST_ENTRY     TimerEntry;
	PDEVICE_OBJECT  DeviceObject;
	BOOLEAN         bStart;
}COMMUNICATE_IO_TIMER,*PCOMMUNICATE_IO_TIMER;
typedef
	NTKERNELAPI
	VOID
	(*pfnIoStartTimer)(
	IN PDEVICE_OBJECT DeviceObject);

typedef
	NTKERNELAPI
	VOID
	(*pfnIoStopTimer)(
	IN PDEVICE_OBJECT DeviceObject);



NTSTATUS HsEnumIOTimer(PVOID OutBuffer);

ULONG_PTR GetIopTimerQueueHead();

NTSTATUS HsOperIOTimer(PVOID InBuffer);

NTSTATUS RemoveIOTimer(PVOID InBuffer);














