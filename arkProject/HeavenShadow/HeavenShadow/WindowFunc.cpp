#include "stdafx.h"
#include "WindowFunc.h"
#include "Common.h"
#include "resource.h"

#include <vector>

using namespace std;

vector<WND_INFO> m_vectorWnds;

extern ULONG_PTR g_ulProcessId;
extern HANDLE g_hDevice;

COLUMNSTRUCT g_Column_Window[] = 
{
	{	L"窗口句柄",			80	},
	{	L"窗口标题",			140	},
	{	L"窗口类名",			140	},
	{	L"窗口可见性",		90	},
	{	L"线程ID",			70	},
	{	L"进程ID",			70	}
};

UINT g_Column_Window_Count  = 6;	  //进程列表列数

extern int dpix;
extern int dpiy;


void HsInitWindowList(CMyList *m_ListCtrl)
{

	while(m_ListCtrl->DeleteColumn(0));
	m_ListCtrl->DeleteAllItems();

	m_ListCtrl->SetExtendedStyle(LVS_EX_FULLROWSELECT|LVS_EX_GRIDLINES|LVS_EX_HEADERDRAGDROP);

	UINT i;
	for (i = 0;i<g_Column_Window_Count;i++)
	{
		m_ListCtrl->InsertColumn(i, g_Column_Window[i].szTitle,LVCFMT_LEFT,(int)(g_Column_Window[i].nWidth*(dpix/96.0)));
	}
}



VOID HsQueryProcessWindow(CMyList *m_ListCtrl)
{
	m_ListCtrl->DeleteAllItems();

	BOOL bRet = FALSE;
	ULONG ulReturnSize = 0;
	m_vectorWnds.clear();

	PALL_WNDS WndInfo = NULL;
	ULONG ulCount = 1000;

	if (g_ulProcessId <= 4)
	{
		return;
	}

	do 
	{
		ULONG ulSize = sizeof(ALL_WNDS) + ulCount * sizeof(WND_INFO);

		if (WndInfo)
		{
			free(WndInfo);
			WndInfo = NULL;
		}

		WndInfo = (PALL_WNDS)malloc(ulSize);
		if (!WndInfo)
		{
			break;
		}

		memset(WndInfo,0,ulSize);


		bRet = DeviceIoControl(g_hDevice,
			HS_IOCTL(HS_IOCTL_PROC_PROCESSWINDOW),
			&g_ulProcessId,
			sizeof(ULONG),
			WndInfo,
			ulSize,
			&ulReturnSize,
			NULL);

		ulCount = WndInfo->nCnt + 100;

	} while (bRet == FALSE && GetLastError() == ERROR_INSUFFICIENT_BUFFER);

	if (bRet && WndInfo->nCnt > 0)
	{
		for (ULONG i = 0;i<WndInfo->nCnt; i++)
		{
			m_vectorWnds.push_back(WndInfo->WndInfo[i]);
		}
	}

	if (WndInfo)
	{
		free(WndInfo);
		WndInfo = NULL;
	}



	if (m_vectorWnds.empty())
	{
		return;
	}


	//////////////////////////////////////////////////////////////////////////

	for (vector <WND_INFO>::iterator Iter = m_vectorWnds.begin( ); 
		Iter != m_vectorWnds.end( ); 
		Iter++ )
	{
		HsAddWndItem(*Iter,FALSE,m_ListCtrl);
	}
}


void HsAddWndItem(WND_INFO WndInfor,BOOL bAll,CMyList *m_ListCtrl)
{


	WCHAR szClassName[MAX_PATH] = {0};
	WCHAR szWindowsText[MAX_PATH] = {0};

	CString  strhWnd;
	CString  strTid;
	CString  strPid;
	CString  strVisable;

	::GetClassName(WndInfor.hWnd,  szClassName, MAX_PATH);
	::GetWindowText(WndInfor.hWnd, szWindowsText, MAX_PATH);


	if (::IsWindowVisible(WndInfor.hWnd))
	{
		strVisable = L"可见";
	}
	else
	{
		strVisable = L"-";
	}


	if (bAll==TRUE)
	{

	}
	else
	{
		if (WndInfor.uPid!=g_ulProcessId)
		{
			return;
		}
	}





	strhWnd.Format(L"0x%08X",WndInfor.hWnd);
	strTid.Format(L"%d",WndInfor.uTid);
	strPid.Format(L"%d",WndInfor.uPid);
	int n = m_ListCtrl->GetItemCount();
	int j = m_ListCtrl->InsertItem(n,strhWnd);
	m_ListCtrl->SetItemText(j, 1, szWindowsText);
	m_ListCtrl->SetItemText(j, 2, szClassName);
	m_ListCtrl->SetItemText(j, 3, strVisable);
	m_ListCtrl->SetItemText(j, 4, strTid);
	m_ListCtrl->SetItemText(j, 5, strPid);

	m_ListCtrl->SetItemData(j,j);


}



//权限界面弹出菜单
VOID HsProcessWindowPopupMenu(CMyList *m_ListCtrl, CWnd* parent)
{
// 	CMenu	popup;
// 	popup.LoadMenu(IDR_MENU_PROCESS_THREAD);		//加载菜单资源
// 	CMenu*	pM = popup.GetSubMenu(0);				//获得菜单的子项
// 	CPoint	p;
// 	GetCursorPos(&p);
// 	int	count = pM->GetMenuItemCount();
// 	if (m_ListCtrl->GetSelectedCount() == 0)		//如果没有选中
// 	{ 
// 		for (int i = 0;i<count;i++)
// 		{
// 			pM->EnableMenuItem(i, MF_BYPOSITION | MF_DISABLED | MF_GRAYED); //菜单全部变灰
// 		}
// 	}
// 
// 	pM->TrackPopupMenu(TPM_LEFTALIGN, p.x, p.y, parent);
}