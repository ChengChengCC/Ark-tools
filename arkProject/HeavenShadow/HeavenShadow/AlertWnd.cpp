// AlertWnd.cpp : 实现文件
//

#include "stdafx.h"
#include "HeavenShadow.h"
#include "AlertWnd.h"
#include "afxdialogex.h"


#include <Mmsystem.h>


// CAlertWnd 对话框

IMPLEMENT_DYNAMIC(CAlertWnd, CDialog)

CAlertWnd::CAlertWnd(CWnd* pParent /*=NULL*/)
	: CDialog(CAlertWnd::IDD, pParent)
{
	m_ulCount = 20;
	m_TimerTip = _T("");
}

CAlertWnd::~CAlertWnd()
{
}

void CAlertWnd::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_STATIC_TIMERTIP, m_TimerTip);
	DDX_Control(pDX, IDC_STATIC_BTN_ALLOW, m_btnAllow);
	DDX_Control(pDX, IDC_STATIC_BTN_PREVENT, m_btnPrevent);
	DDX_Control(pDX, IDC_STATIC_TIMERTIP, m_TextTimer);
	DDX_Control(pDX, IDC_STATIC_ALERT_CONTEXT, m_TextContext);
	DDX_Control(pDX, IDC_STATIC_ALERT_TITLE, m_TextTitle);
}


BEGIN_MESSAGE_MAP(CAlertWnd, CDialog)
	ON_WM_CLOSE()
	ON_WM_PAINT()
	ON_WM_SHOWWINDOW()
	ON_WM_CTLCOLOR()
//	ON_STN_CLICKED(IDC_STATIC_ALERTBTNEXIT, &CAlertWnd::OnStnClickedStaticAlertbtnexit)
	ON_WM_TIMER()
	ON_STN_CLICKED(IDC_STATIC_BTN_PREVENT, &CAlertWnd::OnStnClickedStaticBtnPrevent)
	ON_STN_CLICKED(IDC_STATIC_BTN_ALLOW, &CAlertWnd::OnStnClickedStaticBtnAllow)
	ON_WM_SYSCOMMAND()
	ON_WM_SIZE()
END_MESSAGE_MAP()


// CAlertWnd 消息处理程序






BOOL CAlertWnd::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  在此添加额外的初始化

	ModifyStyleEx(WS_EX_APPWINDOW,WS_EX_TOOLWINDOW);//设置扩展工具窗模式。阻止任务栏显示图标

	m_ulCount = 20;

	m_TimerTip.Format(L"将执行阻止操作： %2d 秒     ",m_ulCount);

	UpdateData(FALSE);

	//获得桌面大小
	CRect rectWorkArea;
	SystemParametersInfoW(SPI_GETWORKAREA,0,&rectWorkArea,SPIF_SENDCHANGE);

	//获得对话框大小
	CRect rectDlg;
	GetWindowRect(&rectDlg);
	int nW = rectDlg.Width();
	int nH = rectDlg.Height();

	//将窗口设置到右下脚
	::SetWindowPos(this->m_hWnd,HWND_BOTTOM,
		rectWorkArea.right-nW-6,rectWorkArea.bottom-nH,
		nW,nH,
		SWP_NOZORDER);

	//动画显示
	//AnimateWindow(150,AW_CENTER|AW_ACTIVATE|AW_VER_NEGATIVE);
	//AnimateWindow(200,AW_SLIDE|AW_ACTIVATE|AW_VER_NEGATIVE);//
	//AnimateWindow(150,AW_BLEND|AW_ACTIVATE|AW_VER_NEGATIVE);

	


	CloseHandle(CreateThread(NULL,0, 
		(LPTHREAD_START_ROUTINE)HsPlayAlertSound,NULL, 0,NULL));


	SetTimer(3333,1000,NULL);





	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}


BOOL CAlertWnd::PreTranslateMessage(MSG* pMsg)
{
	// TODO: 在此添加专用代码和/或调用基类

	if(pMsg->message==WM_KEYDOWN && (pMsg->wParam==VK_RETURN ||pMsg->wParam==VK_ESCAPE))
	{
		return TRUE;
	}

	return CDialog::PreTranslateMessage(pMsg);
}


void CAlertWnd::OnPaint()
{
	CPaintDC dc(this); // device context for painting
	// TODO: 在此处添加消息处理程序代码
	// 不为绘图消息调用 CDialog::OnPaint()

	CRect   rect;
	GetClientRect(rect);
	dc.FillSolidRect(rect,RGB(0,116,179));
}


void CAlertWnd::OnShowWindow(BOOL bShow, UINT nStatus)
{
	CDialog::OnShowWindow(bShow, nStatus);

	// TODO: 在此处添加消息处理程序代码

	if (bShow == TRUE)
	{
	}
}

void CAlertWnd::OnClose()
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值

	//动画显示
	//AnimateWindow(150,AW_CENTER|AW_HIDE|AW_VER_POSITIVE);
	//AnimateWindow(200,AW_SLIDE|AW_HIDE|AW_VER_POSITIVE);//AW_SLIDE|
	//AnimateWindow(150,AW_BLEND|AW_HIDE|AW_VER_POSITIVE);

	CDialog::OnClose();
}


HBRUSH CAlertWnd::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);

	// TODO:  在此更改 DC 的任何特性

	if ( nCtlColor == CTLCOLOR_BTN)
	{
		pDC->SetBkMode(TRANSPARENT);
		return (HBRUSH)::GetStockObject(NULL_BRUSH);

	}

// 	if ( nCtlColor == CTLCOLOR_STATIC|| nCtlColor == CTLCOLOR_BTN|| nCtlColor == CTLCOLOR_MAX )
// 	{
// 		//pDC->SetBkColor(RGB(232,80,80));
// 
// 		pDC->SetBkMode(TRANSPARENT);
// 
// 		pDC->SetTextColor(RGB(255,255,255));
// 
// 		// TODO:  如果默认的不是所需画笔，则返回另一个画笔
// 		return (HBRUSH)::GetStockObject(NULL_BRUSH);
// 	}

	return hbr;
}


void CAlertWnd::OnOK()
{
	// TODO: 在此添加专用代码和/或调用基类

	CDialog::OnOK();
}


void CAlertWnd::OnCancel()
{
	// TODO: 在此添加专用代码和/或调用基类

	CDialog::OnCancel();
}


//void CAlertWnd::OnStnClickedStaticAlertbtnexit()
//{
//	// TODO: 在此添加控件通知处理程序代码
//	SendMessage(WM_CLOSE);
//}


void CAlertWnd::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值

	switch(nIDEvent)
	{
	case 3333:
		{
			if (m_ulCount == 0)
			{
				KillTimer(3333);
				SendMessage(WM_CLOSE);
			}
			m_ulCount--;

			
			m_TimerTip.Format(L"将执行阻止操作： %2d 秒",m_ulCount);

			UpdateData(FALSE);

			SendMessage(WM_PAINT);
		}
	}

	CDialog::OnTimer(nIDEvent);
}


void HsPlayAlertSound(void)
{
	::PlaySoundW(MAKEINTRESOURCE(IDR_WAVE_SYSTEM_ALERT), ::GetModuleHandle(NULL), SND_RESOURCE | SND_SYNC);

}


void CAlertWnd::OnStnClickedStaticBtnPrevent()
{
	// TODO: 在此添加控件通知处理程序代码
	HINSTANCE hIns = AfxFindResourceHandle(MAKEINTRESOURCE(IDB_BITMAP_BTN_PREVENT_C),RT_GROUP_ICON);

	HBITMAP   hBmp = ::LoadBitmap(hIns, MAKEINTRESOURCE(IDB_BITMAP_BTN_PREVENT_C));

	m_btnPrevent.SetBitmap(hBmp);

	Sleep(100);

	SendMessage(WM_CLOSE);
}


void CAlertWnd::OnStnClickedStaticBtnAllow()
{
	// TODO: 在此添加控件通知处理程序代码
	HINSTANCE hIns = AfxFindResourceHandle(MAKEINTRESOURCE(IDB_BITMAP_BTN_ALLOW_C),RT_GROUP_ICON);

	HBITMAP   hBmp = ::LoadBitmap(hIns, MAKEINTRESOURCE(IDB_BITMAP_BTN_ALLOW_C));

	m_btnAllow.SetBitmap(hBmp);

	Sleep(100);

	SendMessage(WM_CLOSE);
}


void CAlertWnd::OnSysCommand(UINT nID, LPARAM lParam)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值

	CDialog::OnSysCommand(nID, lParam);
}


void CAlertWnd::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);

	// TODO: 在此处添加消息处理程序代码

	if (SIZE_MINIMIZED==nType)
	{
		ShowWindow(TRUE);
	}
}
