// ServiceDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "HeavenShadow.h"
#include "ServiceDlg.h"
#include "afxdialogex.h"
#include "HeavenShadowDlg.h"
#include "Common.h"

#include "AlertWnd.h"


// CServiceDlg 对话框

IMPLEMENT_DYNAMIC(CServiceDlg, CDialog)

CServiceDlg::CServiceDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CServiceDlg::IDD, pParent)
{
	m_wParent = pParent;
}

CServiceDlg::~CServiceDlg()
{
}

void CServiceDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_BUTTON_TEST2, m_demoBtn);
}


BEGIN_MESSAGE_MAP(CServiceDlg, CDialog)
	ON_WM_PAINT()
	ON_WM_SHOWWINDOW()
	ON_BN_CLICKED(IDC_BUTTON_TEST2, &CServiceDlg::OnBnClickedButtonTest2)
END_MESSAGE_MAP()


// CServiceDlg 消息处理程序



void CServiceDlg::OnPaint()
{
	CPaintDC dc(this); // device context for painting
	// TODO: 在此处添加消息处理程序代码
	// 不为绘图消息调用 CDialog::OnPaint()

	CRect   rect;
	GetClientRect(rect);
	dc.FillSolidRect(rect,RGB(255,255,255));
}


BOOL CServiceDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  在此添加额外的初始化

	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}


void CServiceDlg::OnShowWindow(BOOL bShow, UINT nStatus)
{
	CDialog::OnShowWindow(bShow, nStatus);

	// TODO: 在此处添加消息处理程序代码

	if (bShow == TRUE)
	{
		((CHeavenShadowDlg*)m_wParent)->m_bNowWindow = HS_DIALOG_SERVICE;

		((CHeavenShadowDlg*)m_wParent)->m_btnServ.EnableWindow(FALSE);

		HsSendStatusDetail(L"系统服务正在加载。");
		HsSendStatusTip(L"服务");

	}

	
}


BOOL CServiceDlg::PreTranslateMessage(MSG* pMsg)
{
	// TODO: 在此添加专用代码和/或调用基类
	if (pMsg->message==WM_KEYDOWN && (pMsg->wParam==VK_RETURN ||pMsg->wParam==VK_ESCAPE))
	{
		return TRUE;
	}
	return CDialog::PreTranslateMessage(pMsg);
}


void CServiceDlg::OnBnClickedButtonTest2()
{
	// TODO: 在此添加控件通知处理程序代码

	CAlertWnd *dlg = new CAlertWnd(this);

	dlg->Create(IDD_DIALOG_ALERT,GetDesktopWindow());

	dlg->ShowWindow(SW_SHOW);
}
