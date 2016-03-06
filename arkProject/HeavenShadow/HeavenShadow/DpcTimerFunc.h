#pragma once
#include "stdafx.h"



typedef struct _REMOVE_DPCTIMER
{
	ULONG_PTR     TimerObject;
}REMOVE_DPCTIMER,*PREMOVE_DPCTIMER;

typedef struct _DPC_TIMER_
{
	ULONG_PTR TimerObject;
	ULONG_PTR Period;			// ÖÜÆÚ
	ULONG_PTR TimeDispatch;
	ULONG_PTR Dpc;
}DPC_TIMER, *PDPC_TIMER;

typedef struct _DPC_TIMER_INFOR_
{
	ULONG ulCnt;
	ULONG ulRetCnt;
	DPC_TIMER DpcTimer[1];
}DPC_TIMER_INFOR, *PDPC_TIMER_INFOR;


VOID HsInitDPCTimerList(CListCtrl *m_ListCtrl);

VOID HsLoadDPCTimerList(CListCtrl *m_ListCtrl);

VOID HsQueryDPCTimerList(CListCtrl *m_ListCtrl);

BOOL HsGetDPCTimerList();

VOID HsInsertDPCTimerItem(CListCtrl* m_ListCtrl);

VOID HsRemoveDPCTimerItem(CListCtrl* m_ListCtrl);


