// ToolsDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "HeavenShadow.h"
#include "ToolsDlg.h"
#include "afxdialogex.h"

#include "HeavenShadowDlg.h"

// CToolsDlg 对话框

IMPLEMENT_DYNAMIC(CToolsDlg, CDialog)

CToolsDlg::CToolsDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CToolsDlg::IDD, pParent)
{
	m_wParent = pParent;
}

CToolsDlg::~CToolsDlg()
{
}

void CToolsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CToolsDlg, CDialog)
	ON_WM_PAINT()
	ON_WM_SHOWWINDOW()
END_MESSAGE_MAP()


// CToolsDlg 消息处理程序


BOOL CToolsDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  在此添加额外的初始化

	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}


BOOL CToolsDlg::PreTranslateMessage(MSG* pMsg)
{
	// TODO: 在此添加专用代码和/或调用基类
	if (pMsg->message==WM_KEYDOWN && (pMsg->wParam==VK_RETURN ||pMsg->wParam==VK_ESCAPE))
	{
		return TRUE;
	}
	return CDialog::PreTranslateMessage(pMsg);
}


void CToolsDlg::OnPaint()
{
	CPaintDC dc(this); // device context for painting
	// TODO: 在此处添加消息处理程序代码
	// 不为绘图消息调用 CDialog::OnPaint()

	CRect   rect;
	GetClientRect(rect);
	dc.FillSolidRect(rect,RGB(255,255,255));
}


void CToolsDlg::OnShowWindow(BOOL bShow, UINT nStatus)
{
	CDialog::OnShowWindow(bShow, nStatus);

	if (bShow == TRUE)
	{
		((CHeavenShadowDlg*)m_wParent)->m_bNowWindow = HS_DIALOG_TOOLS;

		((CHeavenShadowDlg*)m_wParent)->m_btnTool.EnableWindow(FALSE);

		HsSendStatusDetail(L"系统工具集合。");
		HsSendStatusTip(L"工具箱");
	}
	// TODO: 在此处添加消息处理程序代码
}
