// SystemDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "HeavenShadow.h"
#include "SystemDlg.h"
#include "afxdialogex.h"

#include "SysThread.h"
#include "IoTimerFunc.h"
#include "CallbackFunc.h"
#include "DpcTimerFunc.h"
#include "FilterDriverFunc.h"

#include "HeavenShadowDlg.h"

enum HS_SYSK_LIST
{
	HS_SYSK_CALLBACK,
	HS_SYSK_IOTIMER,
	HS_SYSK_DPCTIMER,
	HS_SYSK_FILTERDRIVER,
	HS_SYSK_SYSTHREAD,
};

extern int dpix;
extern int dpiy;

extern BOOL bIsChecking;

BOOL bNowSystemSel = 255;


// CSystemDlg 对话框

IMPLEMENT_DYNAMIC(CSystemDlg, CDialog)

CSystemDlg::CSystemDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CSystemDlg::IDD, pParent)
{
	m_wParent = pParent;
}

CSystemDlg::~CSystemDlg()
{
}

void CSystemDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_SYSTEM, m_ListSystem);
	DDX_Control(pDX, IDC_LIST_SYSTEM_CTRL, m_ListSystemCtrl);
}


BEGIN_MESSAGE_MAP(CSystemDlg, CDialog)
	ON_WM_SHOWWINDOW()
	ON_WM_PAINT()
	ON_LBN_SELCHANGE(IDC_LIST_SYSTEM, &CSystemDlg::OnSelchangeListSystem)
	ON_NOTIFY(NM_RCLICK, IDC_LIST_SYSTEM_CTRL, &CSystemDlg::OnRclickListSystemCtrl)
	ON_COMMAND(ID_MENU_SYSK_IOTIMER_REFRESH, &CSystemDlg::OnMenuSyskIoTimerRefresh)
	ON_COMMAND(ID_MENU_SYSK_IOTIMER_OPER, &CSystemDlg::OnMenuSyskOperiotimer)
	ON_COMMAND(ID_MENU_SYSK_CALLBACK_REFRESH, &CSystemDlg::OnMenuSyskCallbackRefresh)
	ON_COMMAND(ID_MENU_SYSK_CALLBACK_REMOVE, &CSystemDlg::OnMenuSyskCallbackRemove)
	ON_COMMAND(ID_MENU_SYSK_IOTIMER_REMOVE, &CSystemDlg::OnMenuSyskRemoveiotimer)
	ON_COMMAND(ID_MENU_SYSK_DPCTIMER_REFRESH, &CSystemDlg::OnMenuSyskDpctimerRefresh)
	ON_COMMAND(ID_MENU_SYSK_DPCTIMER_REMOVE, &CSystemDlg::OnMenuSyskDpctimerRemove)
	ON_COMMAND(ID_MENU_SYSK_FILTER_REFRESH, &CSystemDlg::OnMenuSyskFilterRefresh)
	ON_COMMAND(ID_MENU_SYSK_FILTER_REMOVEFILTER, &CSystemDlg::OnMenuSyskFilterRemovefilter)
END_MESSAGE_MAP()


// CSystemDlg 消息处理程序


BOOL CSystemDlg::PreTranslateMessage(MSG* pMsg)
{
	// TODO: 在此添加专用代码和/或调用基类
	if (pMsg->message==WM_KEYDOWN && (pMsg->wParam==VK_RETURN ||pMsg->wParam==VK_ESCAPE))
	{
		return TRUE;
	}
	return CDialog::PreTranslateMessage(pMsg);
}


BOOL CSystemDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  在此添加额外的初始化

	InitSystemList();

	UINT uIconSize = 20;

	uIconSize *= (UINT)(dpix/96.0);

	m_TreeSystemImageList.Create(1, uIconSize, ILC_COLOR32 | ILC_MASK, 2, 2);

	ListView_SetImageList(m_ListSystemCtrl.m_hWnd, m_TreeSystemImageList.GetSafeHandle(), LVSIL_SMALL);

	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}


void CSystemDlg::OnShowWindow(BOOL bShow, UINT nStatus)
{
	CDialog::OnShowWindow(bShow, nStatus);

	// TODO: 在此处添加消息处理程序代码

	if (bShow == TRUE)
	{
		((CHeavenShadowDlg*)m_wParent)->m_bNowWindow = HS_DIALOG_SYSTEM;

		((CHeavenShadowDlg*)m_wParent)->m_btnSys.EnableWindow(FALSE);

		HsSendStatusDetail(L"显示操作系统内核相关信息。");
		HsSendStatusTip(L"内核");

		m_ListSystem.SetCurSel(0);

		bNowSystemSel = 255;

		OnSelchangeListSystem();

		m_ListSystemCtrl.SetFocus();
	}
}


void CSystemDlg::OnPaint()
{
	CPaintDC dc(this); // device context for painting
	// TODO: 在此处添加消息处理程序代码
	// 不为绘图消息调用 CDialog::OnPaint()


	CRect   rect;
	GetClientRect(rect);
	dc.FillSolidRect(rect,RGB(255,255,255));

	CRect SystemListRect;
	CRect SystemCtrlRect;

	//m_ListKernel.GetClientRect(KernelListRect);
	m_ListSystem.GetWindowRect(&SystemListRect);
	ClientToScreen(&rect);
	SystemListRect.left -= rect.left;
	SystemListRect.right -= rect.left;
	SystemListRect.top -= rect.top;
	SystemListRect.bottom -= rect.top;
	SystemListRect.bottom = rect.Height() - 2;
	//m_ListKernel.MoveWindow(&KernelListRect);

	CPoint startPoint;
	startPoint.x = (LONG)(SystemListRect.right)+2;
	startPoint.y = -1;
	CPoint endPoint;
	endPoint.x = (LONG)(SystemListRect.right)+2;
	endPoint.y = rect.Height()+2;

	SystemCtrlRect.left = startPoint.x+1;
	SystemCtrlRect.right = rect.Width();
	SystemCtrlRect.top = 0;
	SystemCtrlRect.bottom = rect.Height();
	m_ListSystemCtrl.MoveWindow(SystemCtrlRect);

	// 	CPoint startPoint;
	// 	startPoint.x = (UINT)(98*(dpix/96.0))+2;
	// 	startPoint.y = -1;
	// 	CPoint endPoint;
	// 	endPoint.x = (UINT)(98*(dpix/96.0))+2;
	// 	endPoint.y = rect.Height()+2;


	COLORREF m_Color(RGB(190,190,190));

	CClientDC aDC(this);			//CClientDC的构造函数需要一个参数，这个参数是指向绘图窗口的指针，我们用this指针就可以了
	CPen pen(PS_SOLID,1,m_Color);	//建立一个画笔类对象，构造时设置画笔属性
	aDC.SelectObject(&pen);
	aDC.MoveTo(startPoint);
	aDC.LineTo(endPoint);
}




void CSystemDlg::InitSystemList(void)
{
	m_ListSystem.AddString(L"系统回调");
	m_ListSystem.InsertString(HS_SYSK_IOTIMER,L"IOPTimer");
	m_ListSystem.InsertString(HS_SYSK_DPCTIMER,L"DPCTimer");
	m_ListSystem.InsertString(HS_SYSK_FILTERDRIVER,L"过滤驱动");
	m_ListSystem.InsertString(HS_SYSK_SYSTHREAD,L"内核线程");
	
	m_ListSystem.SetItemHeight(-1,(UINT)(16*(dpiy/96.0)));
}

void CSystemDlg::OnSelchangeListSystem()
{
	// TODO: 在此添加控件通知处理程序代码

	int nCurSel = m_ListSystem.GetCurSel();

	switch(nCurSel)
	{
	case HS_SYSK_CALLBACK:
		{
			if (bIsChecking == TRUE || bNowSystemSel == HS_SYSK_CALLBACK)	//
			{
				m_ListSystem.SetCurSel(bNowSystemSel);
				m_ListSystemCtrl.SetFocus();
				return;
			}

			bNowSystemSel = nCurSel;

			HsInitCallBackList(&m_ListSystemCtrl);

			CloseHandle(
				CreateThread(NULL,0, 
				(LPTHREAD_START_ROUTINE)HsLoadCallBackList,&m_ListSystemCtrl, 0,NULL)
				);
			//HsLoadCallBackList(&m_ListSystemCtrl);

			break;
		}
	case HS_SYSK_IOTIMER:
		{
			if (bIsChecking == TRUE || bNowSystemSel == HS_SYSK_IOTIMER)	//
			{
				m_ListSystem.SetCurSel(bNowSystemSel);
				m_ListSystemCtrl.SetFocus();
				return;
			}

			bNowSystemSel = nCurSel;

			HsInitIOTimerList(&m_ListSystemCtrl);

			CloseHandle(
				CreateThread(NULL,0, 
				(LPTHREAD_START_ROUTINE)HsLoadIOTimerList,&m_ListSystemCtrl, 0,NULL)
				);
			//HsLoadIOTimerList(&m_ListSystemCtrl);

			break;
		}
	case HS_SYSK_DPCTIMER:
		{
			if (bIsChecking == TRUE || bNowSystemSel == HS_SYSK_DPCTIMER)	//
			{
				m_ListSystem.SetCurSel(bNowSystemSel);
				m_ListSystemCtrl.SetFocus();
				return;
			}

			bNowSystemSel = nCurSel;

			HsInitDPCTimerList(&m_ListSystemCtrl);

			CloseHandle(
				CreateThread(NULL,0, 
				(LPTHREAD_START_ROUTINE)HsLoadDPCTimerList,&m_ListSystemCtrl, 0,NULL)
				);
			//HsLoadDPCTimerList(&m_ListSystemCtrl);

			break;
		}
	case HS_SYSK_FILTERDRIVER:
		{
			if (bIsChecking == TRUE || bNowSystemSel == HS_SYSK_FILTERDRIVER)	//
			{
				m_ListSystem.SetCurSel(bNowSystemSel);
				m_ListSystemCtrl.SetFocus();
				return;
			}

			bNowSystemSel = nCurSel;

			HsInitFilterDriverList(&m_ListSystemCtrl);

			CloseHandle(
				CreateThread(NULL,0, 
				(LPTHREAD_START_ROUTINE)HsLoadFilterDriverList,&m_ListSystemCtrl, 0,NULL)
				);
			//HsLoadFilterDriverList(&m_ListSystemCtrl);

			break;
		}
	case HS_SYSK_SYSTHREAD:
		{
			if (bIsChecking == TRUE || bNowSystemSel == HS_SYSK_SYSTHREAD)	//
			{
				m_ListSystem.SetCurSel(bNowSystemSel);
				m_ListSystemCtrl.SetFocus();
				return;
			}

			bNowSystemSel = nCurSel;

			HsInitSysThreadList(&m_ListSystemCtrl);

			CloseHandle(
				CreateThread(NULL,0, 
				(LPTHREAD_START_ROUTINE)HsLoadSysThreadList,&m_ListSystemCtrl, 0,NULL)
				);
			//HsLoadSysThreadList(&m_ListSystemCtrl);

			break;
		}
	default:
		{

		}
	}

	m_ListSystemCtrl.SetFocus();
}


void CSystemDlg::OnRclickListSystemCtrl(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码

	CMenu	popup;
	CPoint	p;

	switch(bNowSystemSel)
	{
	case HS_SYSK_IOTIMER:
		{
			popup.LoadMenu(IDR_MENU_SYSK_IOTIMER);			//加载菜单资源
			CMenu*	pM = popup.GetSubMenu(0);				//获得菜单的子项

			GetCursorPos(&p);
			int	count = pM->GetMenuItemCount();
			if (m_ListSystemCtrl.GetSelectedCount() == 0)		//如果没有选中
			{ 
				for (int i = 1;i<count;i++)
				{
					pM->EnableMenuItem(i, MF_BYPOSITION | MF_DISABLED | MF_GRAYED); //菜单全部变灰
				}
			}

			POSITION pos = m_ListSystemCtrl.GetFirstSelectedItemPosition();

			while (pos)
			{
				int nItem = m_ListSystemCtrl.GetNextSelectedItem(pos);

				if (_wcsnicmp(L"运行",m_ListSystemCtrl.GetItemText(nItem,2),wcslen(L"运行"))==0)
				{
					pM->ModifyMenuW(ID_MENU_SYSK_IOTIMER_OPER,MF_BYCOMMAND,ID_MENU_SYSK_IOTIMER_OPER,L"停止(&S)");
				}
				else if (_wcsnicmp(L"停止",m_ListSystemCtrl.GetItemText(nItem,2),wcslen(L"停止"))==0)
				{
					pM->ModifyMenuW(ID_MENU_SYSK_IOTIMER_OPER,MF_BYCOMMAND,ID_MENU_SYSK_IOTIMER_OPER,L"运行(&S)");
				}
			}

			pM->TrackPopupMenu(TPM_LEFTALIGN, p.x, p.y, this);
			break;
		}
	case HS_SYSK_CALLBACK:
		{
			popup.LoadMenu(IDR_MENU_SYSK_CALLBACK);			//加载菜单资源
			CMenu*	pM = popup.GetSubMenu(0);				//获得菜单的子项

			GetCursorPos(&p);
			int	count = pM->GetMenuItemCount();
			if (m_ListSystemCtrl.GetSelectedCount() == 0)		//如果没有选中
			{ 
				for (int i = 1;i<count;i++)
				{
					pM->EnableMenuItem(i, MF_BYPOSITION | MF_DISABLED | MF_GRAYED); //菜单全部变灰
				}
			}

			pM->TrackPopupMenu(TPM_LEFTALIGN, p.x, p.y, this);
			break;
		}
	case HS_SYSK_DPCTIMER:
		{
			popup.LoadMenu(IDR_MENU_SYSK_DPCTIMER);			//加载菜单资源
			CMenu*	pM = popup.GetSubMenu(0);				//获得菜单的子项

			GetCursorPos(&p);
			int	count = pM->GetMenuItemCount();
			if (m_ListSystemCtrl.GetSelectedCount() == 0)		//如果没有选中
			{ 
				for (int i = 1;i<count;i++)
				{
					pM->EnableMenuItem(i, MF_BYPOSITION | MF_DISABLED | MF_GRAYED); //菜单全部变灰
				}
			}

			pM->TrackPopupMenu(TPM_LEFTALIGN, p.x, p.y, this);
			break;
		}
	case HS_SYSK_FILTERDRIVER:
		{
			popup.LoadMenu(IDR_MENU_SYSK_FILTERDRIVER);		//加载菜单资源
			CMenu*	pM = popup.GetSubMenu(0);				//获得菜单的子项

			GetCursorPos(&p);
			int	count = pM->GetMenuItemCount();
			if (m_ListSystemCtrl.GetSelectedCount() == 0)		//如果没有选中
			{ 
				for (int i = 1;i<count;i++)
				{
					pM->EnableMenuItem(i, MF_BYPOSITION | MF_DISABLED | MF_GRAYED); //菜单全部变灰
				}
			}

			pM->TrackPopupMenu(TPM_LEFTALIGN, p.x, p.y, this);
			break;
		}
	default:
		{

		}
	}


	*pResult = 0;
}


void CSystemDlg::OnMenuSyskIoTimerRefresh()
{
	// TODO: 在此添加命令处理程序代码
	m_ListSystem.SetCurSel(HS_SYSK_IOTIMER);
	bNowSystemSel = 255;
	OnSelchangeListSystem();
}


void CSystemDlg::OnMenuSyskOperiotimer()
{
	// TODO: 在此添加命令处理程序代码
	if (bIsChecking)
	{
		return;
	}

	bIsChecking = TRUE;

	CloseHandle(
		CreateThread(NULL,0, 
		(LPTHREAD_START_ROUTINE)HsOperIOTimer,&m_ListSystemCtrl, 0,NULL)
		);
	//HsOperIOTimer(&m_ListSystemCtrl);
}


void CSystemDlg::OnMenuSyskCallbackRefresh()
{
	// TODO: 在此添加命令处理程序代码
	m_ListSystem.SetCurSel(HS_SYSK_CALLBACK);
	bNowSystemSel = 255;
	OnSelchangeListSystem();
}


void CSystemDlg::OnMenuSyskCallbackRemove()
{
	// TODO: 在此添加命令处理程序代码
	if (bIsChecking)
	{
		return;
	}

	bIsChecking = TRUE;

	CloseHandle(
		CreateThread(NULL,0, 
		(LPTHREAD_START_ROUTINE)HsRemoveCallBackItem,&m_ListSystemCtrl, 0,NULL)
		);
	//HsRemoveCallBackItem(&m_ListSystemCtrl);
}


void CSystemDlg::OnMenuSyskRemoveiotimer()
{
	// TODO: 在此添加命令处理程序代码

	BOOL bRet = MessageBox(L"删除IOTimer操作可能会造成系统异常或崩溃，\r\n请在确认后继续。",L"天影卫士",MB_ICONWARNING | MB_OKCANCEL);

	if (bRet == IDCANCEL)
	{
		return;
	}

	if (bIsChecking == TRUE)
	{
		return;
	}

	bIsChecking = TRUE;

	CloseHandle(
		CreateThread(NULL,0, 
		(LPTHREAD_START_ROUTINE)HsRemoveIOTimerItem,&m_ListSystemCtrl, 0,NULL)
		);
	//HsRemoveIOTimerItem(&m_ListSystemCtrl);
}


void CSystemDlg::OnMenuSyskDpctimerRefresh()
{
	// TODO: 在此添加命令处理程序代码
	m_ListSystem.SetCurSel(HS_SYSK_DPCTIMER);
	bNowSystemSel = 255;
	OnSelchangeListSystem();
}


void CSystemDlg::OnMenuSyskDpctimerRemove()
{
	// TODO: 在此添加命令处理程序代码
	BOOL bRet = MessageBox(L"删除DPCTimer操作可能会造成系统异常或崩溃，\r\n请在确认后继续。",L"天影卫士",MB_ICONWARNING | MB_OKCANCEL);

	if (bRet == IDCANCEL)
	{
		return;
	}

	if (bIsChecking == TRUE)
	{
		return;
	}

	bIsChecking = TRUE;

	CloseHandle(
		CreateThread(NULL,0, 
		(LPTHREAD_START_ROUTINE)HsRemoveDPCTimerItem,&m_ListSystemCtrl, 0,NULL)
		);
	//HsRemoveDPCTimerItem(&m_ListSystemCtrl);
}


void CSystemDlg::OnMenuSyskFilterRefresh()
{
	// TODO: 在此添加命令处理程序代码
	m_ListSystem.SetCurSel(HS_SYSK_FILTERDRIVER);
	bNowSystemSel = 255;
	OnSelchangeListSystem();
}


void CSystemDlg::OnMenuSyskFilterRemovefilter()
{
	// TODO: 在此添加命令处理程序代码
	BOOL bRet = MessageBox(L"卸载过滤驱动操作可能会造成系统异常或崩溃，\r\n请在确认后继续。",L"天影卫士",MB_ICONWARNING | MB_OKCANCEL);

	if (bRet == IDCANCEL)
	{
		return;
	}

	if (bIsChecking == TRUE)
	{
		return;
	}

	bIsChecking = TRUE;

	CloseHandle(
		CreateThread(NULL,0, 
		(LPTHREAD_START_ROUTINE)HsRemoveFilterDriverItem,&m_ListSystemCtrl, 0,NULL)
		);
	//HsRemoveFilterDriverItem(&m_ListSystemCtrl);
}
