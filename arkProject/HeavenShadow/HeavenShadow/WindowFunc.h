#pragma once
#include "stdafx.h"

#include "MyList.h"


typedef struct _WND_INFO_
{
	HWND  hWnd;
	ULONG uPid;
	ULONG uTid;
}WND_INFO, *PWND_INFO;

typedef struct _ALL_WNDS_
{
	ULONG nCnt;
	WND_INFO WndInfo[1];
}ALL_WNDS, *PALL_WNDS;



void HsInitWindowList(CMyList *m_ListCtrl);

VOID HsQueryProcessWindow(CMyList *m_ListCtrl);

void HsAddWndItem(WND_INFO WndInfor,BOOL bAll,CMyList *m_ListCtrl);


//权限界面弹出菜单
VOID HsProcessWindowPopupMenu(CMyList *m_ListCtrl, CWnd* parent);