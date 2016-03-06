// HsAboutDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "HeavenShadow.h"
#include "HsAboutDlg.h"
#include "afxdialogex.h"

#include "HeavenShadowDlg.h"


extern BOOL bDriverIsOK;
extern WIN_VERSION GetWindowsVersion();
extern WIN_VERSION WinVersion;

extern int dpix;
extern int dpiy;



// CHsAboutDlg 对话框

IMPLEMENT_DYNAMIC(CHsAboutDlg, CDialog)

CHsAboutDlg::CHsAboutDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CHsAboutDlg::IDD, pParent)
{
	m_wParent = pParent;
}

CHsAboutDlg::~CHsAboutDlg()
{
}

void CHsAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_TEXT_CHECK_ALT, m_TextCheckAlt);
	DDX_Control(pDX, IDC_STATIC_CHECK_ALT, m_PicCheckAlt);
	DDX_Control(pDX, IDC_STATIC_SYS_BIT, m_PicSysBit);
	DDX_Control(pDX, IDC_TEXT_SYS_BIT, m_TextSysBit);
	DDX_Control(pDX, IDC_TEXT_SOFT_BIT, m_TextSoftBit);
	DDX_Control(pDX, IDC_STATIC_HOMECHECK, m_CheckBtn);
}


BEGIN_MESSAGE_MAP(CHsAboutDlg, CDialog)
	ON_WM_SHOWWINDOW()
	ON_WM_PAINT()
	ON_WM_CTLCOLOR()
	ON_STN_CLICKED(IDC_STATIC_HOMECHECK, &CHsAboutDlg::OnClickedStaticHomecheck)
END_MESSAGE_MAP()


// CHsAboutDlg 消息处理程序


BOOL CHsAboutDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  在此添加额外的初始化

	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}


void CHsAboutDlg::OnShowWindow(BOOL bShow, UINT nStatus)
{
	CDialog::OnShowWindow(bShow, nStatus);

	// TODO: 在此处添加消息处理程序代码


	if (bShow == TRUE)
	{
		((CHeavenShadowDlg*)m_wParent)->m_bNowWindow = HS_DIALOG_ABOUT;

		((CHeavenShadowDlg*)m_wParent)->m_btnAbou.EnableWindow(FALSE);

		HsSendStatusDetail(L"天影卫士安全防护工具。");

		HsSendStatusTip(L"天影卫士");


		if (sizeof(ULONG_PTR) == sizeof(ULONG32))
		{
			m_TextSoftBit.SetWindowTextW(L"天影卫士 32位版。");
		}
		else
		{
			m_TextSoftBit.SetWindowTextW(L"天影卫士 64位版。");
		}

		CString SysBit;

		WinVersion = GetWindowsVersion();

		switch(WinVersion)
		{
		case Windows7:
			{
				SysBit += L"Windows7";
				break;
			}
		case WindowsXP:
			{
				SysBit += L"WindowsXP";
				break;
			}
		default:
			{
				SysBit += L"Other";
			}
		}

		if (HsIs64BitWindows())
		{
			SysBit += L" 64位操作系统。";
		}
		else
		{
			SysBit += L" 32位操作系统。";
		}

		m_TextSysBit.SetWindowTextW(SysBit.GetBuffer());

		if (bDriverIsOK)
		{
			HINSTANCE hIns_proc = AfxFindResourceHandle(MAKEINTRESOURCE(IDB_BITMAP_CHECK_ALT_V),RT_GROUP_ICON);
			HBITMAP   hBmp_proc = ::LoadBitmap(hIns_proc, MAKEINTRESOURCE(IDB_BITMAP_CHECK_ALT_V));
			m_PicCheckAlt.SetBitmap(hBmp_proc);

			m_TextCheckAlt.SetWindowTextW(L"程序与内核层通讯正常。");
		}
		else
		{
			HINSTANCE hIns_proc = AfxFindResourceHandle(MAKEINTRESOURCE(IDB_BITMAP_CHECK_ALT_X),RT_GROUP_ICON);
			HBITMAP   hBmp_proc = ::LoadBitmap(hIns_proc, MAKEINTRESOURCE(IDB_BITMAP_CHECK_ALT_X));
			m_PicCheckAlt.SetBitmap(hBmp_proc);

			m_TextCheckAlt.SetWindowTextW(L"程序与内核层通讯异常。");
		}

	}
}


void CHsAboutDlg::OnPaint()
{
	CPaintDC dc(this); // device context for painting
	// TODO: 在此处添加消息处理程序代码
	// 不为绘图消息调用 CDialog::OnPaint()

	CRect   rect;
	GetClientRect(rect);
	dc.FillSolidRect(rect,RGB(255,255,255));

	ClientToScreen(&rect);

	CRect picrect;
	m_PicSysBit.GetWindowRect(&picrect);
	picrect.left = picrect.left - rect.left;

	CPoint startPoint;
	startPoint.x = (LONG)(picrect.left-(20)*(dpix/96.0));
	startPoint.y = 13;
	CPoint endPoint;
	endPoint.x = (LONG)(picrect.left-(20)*(dpix/96.0));
	endPoint.y = rect.Height()-13;


	COLORREF m_Color(RGB(225,225,255));

	CClientDC aDC(this); //CClientDC的构造函数需要一个参数，这个参数是指向绘图窗口的指针，我们用this指针就可以了
	CPen pen(PS_SOLID,1,m_Color); ////建立一个画笔类对象，构造时设置画笔属性
	aDC.SelectObject(&pen);
	aDC.MoveTo(startPoint);
	aDC.LineTo(endPoint);
}


BOOL CHsAboutDlg::PreTranslateMessage(MSG* pMsg)
{
	// TODO: 在此添加专用代码和/或调用基类
	if (pMsg->message==WM_KEYDOWN && (pMsg->wParam==VK_RETURN ||pMsg->wParam==VK_ESCAPE))
	{
		return TRUE;
	}
	return CDialog::PreTranslateMessage(pMsg);
}


HBRUSH CHsAboutDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);

	// TODO:  在此更改 DC 的任何特性


	// TODO:  如果默认的不是所需画笔，则返回另一个画笔
	return hbr;
}


void CHsAboutDlg::OnClickedStaticHomecheck()
{
	// TODO: 在此添加控件通知处理程序代码

	HINSTANCE hIns_proc = AfxFindResourceHandle(MAKEINTRESOURCE(IDB_BITMAP_HOMECHECK_BTN_CLICK),RT_GROUP_ICON);
	HBITMAP   hBmp_proc = ::LoadBitmap(hIns_proc, MAKEINTRESOURCE(IDB_BITMAP_HOMECHECK_BTN_CLICK));
	m_CheckBtn.SetBitmap(hBmp_proc);

	Sleep(100);
	((CHeavenShadowDlg*)m_wParent)->OnClickedStaticProcess();
}
