// ProcessViewDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "HeavenShadow.h"
#include "ProcessViewDlg.h"
#include "afxdialogex.h"

#include "ThreadFunc.h"
#include "PrivilegeFunc.h"
#include "DetailFunc.h"
#include "HandleFunc.h"
#include "WindowFunc.h"
#include "MemoryFunc.h"
#include "PModuleFunc.h"

#include "Common.h"




ULONG_PTR g_ulProcessId = 0;

extern int dpix;
extern int dpiy;


UINT nowType = 999;


// CProcessViewDlg 对话框

IMPLEMENT_DYNAMIC(CProcessViewDlg, CDialog)

CProcessViewDlg::CProcessViewDlg(ULONG_PTR ulViewType, PHSPROCESSINFO piProcInfo, CWnd* pParent /*=NULL*/)
	: CDialog(CProcessViewDlg::IDD, pParent)
{
	m_wParent = pParent;
	m_piProcInfo = piProcInfo;
	m_ulViewType = ulViewType;

	g_ulProcessId = m_piProcInfo->Pid;
}

CProcessViewDlg::~CProcessViewDlg()
{
}

void CProcessViewDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_PROCESS_VIEW, m_viewList);
	DDX_Control(pDX, IDC_LIST_PROCESSVIEWTYPE, m_typeList);
	DDX_Control(pDX, IDC_STATIC_VIEWICON, m_ViewIcon);
}


BEGIN_MESSAGE_MAP(CProcessViewDlg, CDialog)
	ON_NOTIFY(NM_RCLICK, IDC_LIST_PROCESS_VIEW, &CProcessViewDlg::OnRclickListProcessView)
	ON_COMMAND(ID_MENU_PROCESSPRIVILEGE_REFRESH, &CProcessViewDlg::OnMenuProcessprivilegeRefresh)
	ON_COMMAND(ID_MENU_PROCESSPRIVILEGE_ENABLE, &CProcessViewDlg::OnMenuProcessprivilegeEnable)
	ON_COMMAND(ID_MENU_PROCESSPRIVILEGE_DISABLE, &CProcessViewDlg::OnMenuProcessprivilegeDisable)
	ON_WM_PAINT()
	ON_LBN_SELCHANGE(IDC_LIST_PROCESSVIEWTYPE, &CProcessViewDlg::OnSelchangeListProcessviewtype)
	ON_WM_CTLCOLOR()
	ON_COMMAND(ID_MENU_PROCESSTHREAD_REFRESH, &CProcessViewDlg::OnMenuProcessthreadRefresh)
	ON_COMMAND(ID_MENU_PROCESSTHREAD_CLOSETHREAD, &CProcessViewDlg::OnMenuProcessthreadClosethread)
END_MESSAGE_MAP()


// CProcessViewDlg 消息处理程序


BOOL CProcessViewDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  在此添加额外的初始化

	nowType = 999;

	SHFILEINFO shInfo;
	memset(&shInfo, 0, sizeof(shInfo));
	SHGetFileInfo(m_piProcInfo->Path, FILE_ATTRIBUTE_NORMAL, 
		&shInfo, sizeof(SHFILEINFO), SHGFI_ICON | SHGFI_USEFILEATTRIBUTES);

	m_hIcon = shInfo.hIcon;

	SetIcon(m_hIcon,TRUE);
	SetIcon(m_hIcon,FALSE);


	m_ViewIcon.SetIcon(m_hIcon);

	m_typeList.InsertString(HS_PROCVIEW_TYPE_DETAIL,L"进程映像");
	m_typeList.InsertString(HS_PROCVIEW_TYPE_THREAD,L"进程线程");
	m_typeList.InsertString(HS_PROCVIEW_TYPE_PRIVILEGE,L"进程权限");
	m_typeList.InsertString(HS_PROCVIEW_TYPE_HANDLE,L"进程句柄");
	m_typeList.InsertString(HS_PROCVIEW_TYPE_WINDOW,L"进程窗口");
	m_typeList.InsertString(HS_PROCVIEW_TYPE_MODULE,L"进程模块");
	m_typeList.InsertString(HS_PROCVIEW_TYPE_MEMORY,L"进程内存");

	m_typeList.SetItemHeight(-1,(UINT)(16*(dpiy/96.0)));

	m_typeList.SetCurSel((int)m_ulViewType);

	OnSelchangeListProcessviewtype();

	UINT uIconSize = 20;

	uIconSize *= (UINT)(dpix/96.0);

	m_TreeKernelImageList.Create(1, uIconSize, ILC_COLOR32 | ILC_MASK, 2, 2);

	ListView_SetImageList(m_viewList.m_hWnd, m_TreeKernelImageList.GetSafeHandle(), LVSIL_SMALL);


	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}


void CProcessViewDlg::OnRclickListProcessView(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码

	switch (m_ulViewType)
	{
	case HS_PROCVIEW_TYPE_PRIVILEGE:
		{
			HsProcessPrivilegePopupMenu(&m_viewList,this);
			break;
		}
	case HS_PROCVIEW_TYPE_THREAD:
		{
			HsProcessThreadPopupMenu(&m_viewList,this);
			break;
		}
	case HS_PROCVIEW_TYPE_WINDOW:
		{
			HsProcessWindowPopupMenu(&m_viewList,this);
			break;
		}
	}

	*pResult = 0;
}


void CProcessViewDlg::OnMenuProcessprivilegeRefresh()
{
	// TODO: 在此添加命令处理程序代码

 	CloseHandle(CreateThread(NULL, 0, 
 		(LPTHREAD_START_ROUTINE)HsQueryProcessPrivilege,&m_viewList, 0, NULL));

	//HsQueryProcessPrivilege(&m_viewList);
}


void CProcessViewDlg::OnMenuProcessprivilegeEnable()
{
	// TODO: 在此添加命令处理程序代码

	POSITION pos = m_viewList.GetFirstSelectedItemPosition();

	while (pos)
	{
		int nItem = m_viewList.GetNextSelectedItem(pos);

		WCHAR PrivilegeName[60] = {0};
		
		StringCchCopyW(PrivilegeName,m_viewList.GetItemText(nItem,0).GetLength()+1,m_viewList.GetItemText(nItem,0).GetBuffer());

		BOOL bFeedBack = HsAdjustPrivilege(m_piProcInfo->Pid,PrivilegeName,TRUE);

		if (bFeedBack == TRUE)
		{
			m_viewList.SetItemText(nItem,2,L"Enabled");
		}
	}
}


void CProcessViewDlg::OnMenuProcessprivilegeDisable()
{
	// TODO: 在此添加命令处理程序代码

	POSITION pos = m_viewList.GetFirstSelectedItemPosition();

	

	while (pos)
	{
		int nItem = m_viewList.GetNextSelectedItem(pos);

		WCHAR PrivilegeName[60];

		StringCchCopyW(PrivilegeName,m_viewList.GetItemText(nItem,0).GetLength()+1,m_viewList.GetItemText(nItem,0).GetBuffer());

		BOOL bFeedBack = HsAdjustPrivilege(m_piProcInfo->Pid,PrivilegeName,FALSE);

		if (bFeedBack == TRUE)
		{
			m_viewList.SetItemText(nItem,2,L"Disabled");
		}
	}
}


void CProcessViewDlg::OnPaint()
{
	CPaintDC dc(this); // device context for painting
	// TODO: 在此处添加消息处理程序代码
	// 不为绘图消息调用 CDialog::OnPaint()
 
 	CRect   rect;
 	GetClientRect(&rect);
	dc.FillSolidRect(rect,RGB(255,255,255));

	CRect leftlistrect;
	CRect rightlistrect;

	m_typeList.GetClientRect(leftlistrect);

	CPoint startPoint;
	startPoint.x = (LONG)(leftlistrect.right)+4;
	startPoint.y = -1;
	CPoint endPoint;
	endPoint.x = (LONG)(leftlistrect.right)+4;
	endPoint.y = rect.Height()+2;

	rightlistrect.left = startPoint.x+1;
	rightlistrect.right = rect.Width();
	rightlistrect.top = 0;
	rightlistrect.bottom = rect.Height();
	m_viewList.MoveWindow(rightlistrect);



	COLORREF m_Color(RGB(190,190,190));

	CClientDC aDC(this); //CClientDC的构造函数需要一个参数，这个参数是指向绘图窗口的指针，我们用this指针就可以了
	CPen pen(PS_SOLID,1,m_Color); ////建立一个画笔类对象，构造时设置画笔属性
	aDC.SelectObject(&pen);
	aDC.MoveTo(startPoint);
	aDC.LineTo(endPoint);

	if (IsIconic())
	{
		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;
		dc.DrawIcon(32,32,m_hIcon);
	}
}


void CProcessViewDlg::OnSelchangeListProcessviewtype()
{
	// TODO: 在此添加控件通知处理程序代码

	int nItem = m_typeList.GetCurSel();

	m_ulViewType = nItem;

	switch(m_ulViewType)
	{
	case HS_PROCVIEW_TYPE_DETAIL:
		{
			if (nowType == HS_PROCVIEW_TYPE_DETAIL)
			{
				return;
			}
			nowType = HS_PROCVIEW_TYPE_DETAIL;

			CString Temp;
			Temp.Format(L"进程映像 - %s", m_piProcInfo->Name);

			SetWindowText(Temp.GetBuffer());

			HsInitProcessDetailList(&m_viewList);

			HsLoadProcessDetailList(m_piProcInfo,&m_viewList);

			break;
		}
	case HS_PROCVIEW_TYPE_THREAD:
		{
			if (nowType == HS_PROCVIEW_TYPE_THREAD)
			{
				return;
			}
			nowType = HS_PROCVIEW_TYPE_THREAD;

			CString Temp;

			Temp.Format(L"进程线程 - %s", m_piProcInfo->Name);

			SetWindowText(Temp.GetBuffer());

			HsInitThreadList(&m_viewList);

			//CloseHandle(CreateThread(NULL, 0,
			//	(LPTHREAD_START_ROUTINE)HsQueryProcessThread,&m_viewList, 0, NULL));

			HsQueryProcessThread(&m_viewList);

			break;
		}
	case HS_PROCVIEW_TYPE_PRIVILEGE:
		{
			if (nowType == HS_PROCVIEW_TYPE_PRIVILEGE)
			{
				return;
			}
			nowType = HS_PROCVIEW_TYPE_PRIVILEGE;

			CString Temp;
			Temp.Format(L"进程权限 - %s", m_piProcInfo->Name);

			SetWindowText(Temp.GetBuffer());

			HsInitPrivilegeList(&m_viewList);

			//CloseHandle(CreateThread(NULL, 0,
			//	(LPTHREAD_START_ROUTINE)HsQueryProcessPrivilege,&m_viewList, 0, NULL));

			HsQueryProcessPrivilege(&m_viewList);

			break;
		}
	case HS_PROCVIEW_TYPE_HANDLE:
		{
			if (nowType == HS_PROCVIEW_TYPE_HANDLE)
			{
				return;
			}
			nowType = HS_PROCVIEW_TYPE_HANDLE;

			CString Temp;
			Temp.Format(L"进程句柄 - %s", m_piProcInfo->Name);

			SetWindowText(Temp.GetBuffer());

			HsInitProcessHandleList(&m_viewList);

			//CloseHandle(CreateThread(NULL, 0,
			//	(LPTHREAD_START_ROUTINE)HsQueryProcessHandle,&m_viewList, 0, NULL));

			HsQueryProcessHandle(&m_viewList);

			break;
		}
	case HS_PROCVIEW_TYPE_WINDOW:
		{
			if (nowType == HS_PROCVIEW_TYPE_WINDOW)
			{
				return;
			}
			nowType = HS_PROCVIEW_TYPE_WINDOW;


			CString Temp;
			Temp.Format(L"进程窗口 - %s", m_piProcInfo->Name);

			SetWindowText(Temp.GetBuffer());

			HsInitWindowList(&m_viewList);

			//CloseHandle(CreateThread(NULL, 0,
			//	(LPTHREAD_START_ROUTINE)HsQueryProcessWindow,&m_viewList, 0, NULL));

			HsQueryProcessWindow(&m_viewList);

			break;
		}
	case HS_PROCVIEW_TYPE_MODULE:
		{
			if (nowType == HS_PROCVIEW_TYPE_MODULE)
			{
				return;
			}
			nowType = HS_PROCVIEW_TYPE_MODULE;

			CString Temp;
			Temp.Format(L"进程模块 - %s", m_piProcInfo->Name);

			SetWindowText(Temp.GetBuffer());

			HsInitPModuleDetailList(&m_viewList);

			HsLoadPModuleDetailList(&m_viewList);

			break;
		}
	case HS_PROCVIEW_TYPE_MEMORY:
		{
			if (nowType == HS_PROCVIEW_TYPE_MEMORY)
			{
				return;
			}
			nowType = HS_PROCVIEW_TYPE_MEMORY;

			CString Temp;
			Temp.Format(L"进程内存 - %s", m_piProcInfo->Name);

			SetWindowText(Temp.GetBuffer());

			HsInitMemoryList(&m_viewList);

			HsQueryProcessMemory(&m_viewList);

			break;
		}
	}

	m_viewList.SetFocus();
}


BOOL CProcessViewDlg::PreTranslateMessage(MSG* pMsg)
{
	// TODO: 在此添加专用代码和/或调用基类
	if (pMsg->message==WM_KEYDOWN && (pMsg->wParam==VK_RETURN ||pMsg->wParam==VK_ESCAPE))
	{
		return TRUE;
	}
	return CDialog::PreTranslateMessage(pMsg);
}


HBRUSH CProcessViewDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);

	// TODO:  在此更改 DC 的任何特性

// 	if ( nCtlColor == CTLCOLOR_STATIC)
// 	{
// 		pDC->SetBkMode(TRANSPARENT);
// 		pDC->SetBkColor(RGB(255,255,255));
// 
// 		return (HBRUSH)::GetStockObject(NULL_BRUSH);
// 
// 	}


// 	if (nCtlColor == CTLCOLOR_LISTBOX)
// 	{
// 		pDC->SetBkMode(TRANSPARENT);
// 		return (HBRUSH)::GetStockObject(NULL_BRUSH);
// 	}

	// TODO:  如果默认的不是所需画笔，则返回另一个画笔
	return hbr;
}


void CProcessViewDlg::OnMenuProcessthreadRefresh()
{
	// TODO: 在此添加命令处理程序代码
	CloseHandle(CreateThread(NULL, 0, 
		(LPTHREAD_START_ROUTINE)HsQueryProcessThread,&m_viewList, 0, NULL));
}


void CProcessViewDlg::OnMenuProcessthreadClosethread()
{
	// TODO: 在此添加命令处理程序代码
	POSITION pos = m_viewList.GetFirstSelectedItemPosition();

	while (pos)
	{
		UINT nItem = m_viewList.GetNextSelectedItem(pos);

		ULONG ulThreadId = _ttoi(m_viewList.GetItemText(nItem,0));

		HANDLE hThread = OpenThread(THREAD_TERMINATE,FALSE,ulThreadId);

		BOOL bRet = TerminateThread(hThread,0);

		if (bRet)
		{
			m_viewList.DeleteItem(nItem);
		}
	}
}
