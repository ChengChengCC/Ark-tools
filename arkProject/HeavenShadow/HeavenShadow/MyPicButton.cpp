// MyPicButton.cpp : 实现文件
//

#include "stdafx.h"
#include "HeavenShadow.h"
#include "MyPicButton.h"

#include "Common.h"


extern CWnd* g_wParent;

// CMyPicButton

IMPLEMENT_DYNAMIC(CMyPicButton, CStatic)

CMyPicButton::CMyPicButton()
{
}

CMyPicButton::~CMyPicButton()
{
}


BEGIN_MESSAGE_MAP(CMyPicButton, CStatic)
	ON_WM_SETCURSOR()
END_MESSAGE_MAP()



// CMyPicButton 消息处理程序



BOOL CMyPicButton::PreTranslateMessage(MSG* pMsg)
{
	m_toolTip.RelayEvent(pMsg);

	return CStatic::PreTranslateMessage(pMsg);
}


void CMyPicButton::PreSubclassWindow()
{
	CRect rect;
	GetClientRect(rect);
	m_toolTip.Create(this);
	m_toolTip.AddTool(this, m_szToolTipText, rect, GetDlgCtrlID());
	m_toolTip.Activate(TRUE);
	EnableToolTips(TRUE);

	CStatic::PreSubclassWindow();
}

void CMyPicButton::SetToolTipText(const CString &szTip)
{
	m_szToolTipText = szTip;
	UpdateToolTipText();
	return;
}

void CMyPicButton::UpdateToolTipText()
{
	if (::IsWindow(GetSafeHwnd()))
	{
		m_toolTip.UpdateTipText(m_szToolTipText, this, GetDlgCtrlID());
	}
	return;
}


INT_PTR CMyPicButton::OnToolHitTest(CPoint point, TOOLINFO* pTI) const
{
	pTI->hwnd = m_hWnd;
	pTI->uId = (UINT)m_hWnd;
	pTI->uFlags |= TTF_IDISHWND;
	pTI->lpszText = LPSTR_TEXTCALLBACK;
	return GetDlgCtrlID();

	//return CStatic::OnToolHitTest(point, pTI);
}



BOOL CMyPicButton::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值

	// 使用自己的鼠标绘制函数。
	::SetCursor(::LoadCursorW(NULL,IDC_HAND));

	return TRUE;
}
