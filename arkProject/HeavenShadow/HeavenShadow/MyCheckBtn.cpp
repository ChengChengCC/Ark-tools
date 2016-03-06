// MyCheckBtn.cpp : 实现文件
//

#include "stdafx.h"
#include "HeavenShadow.h"
#include "MyCheckBtn.h"


// CMyCheckBtn

IMPLEMENT_DYNAMIC(CMyCheckBtn, CStatic)

CMyCheckBtn::CMyCheckBtn()
{

}

CMyCheckBtn::~CMyCheckBtn()
{
}


BEGIN_MESSAGE_MAP(CMyCheckBtn, CStatic)
	ON_WM_MOUSEMOVE()
	ON_WM_MOUSELEAVE()
	ON_WM_MOUSEHOVER()
	ON_WM_SETCURSOR()
END_MESSAGE_MAP()



// CMyCheckBtn 消息处理程序




void CMyCheckBtn::OnMouseMove(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	TRACKMOUSEEVENT csTME;
	csTME.cbSize = sizeof (csTME);
	csTME.dwFlags = TME_LEAVE|TME_HOVER;
	csTME.hwndTrack = m_hWnd;		// 指定要 追踪 的窗口 
	csTME.dwHoverTime = 50;			// 鼠标在按钮上停留超过 50ms ，才认为状态为 HOVER
	::_TrackMouseEvent (&csTME);	// 开启 Windows 的 WM_MOUSELEAVE ， WM_MOUSEHOVER 事件支持
	CStatic::OnMouseMove(nFlags, point);
}


void CMyCheckBtn::OnMouseLeave()
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	HINSTANCE hIns = AfxFindResourceHandle(MAKEINTRESOURCE(IDB_BITMAP_HOMECHECK_BTN),RT_GROUP_ICON);

	HBITMAP   hBmp = ::LoadBitmap(hIns, MAKEINTRESOURCE(IDB_BITMAP_HOMECHECK_BTN));

	SetBitmap(hBmp);

	CStatic::OnMouseLeave();
}


void CMyCheckBtn::OnMouseHover(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	HINSTANCE hIns = AfxFindResourceHandle(MAKEINTRESOURCE(IDB_BITMAP_HOMECHECK_BTN_HOVER),RT_GROUP_ICON);

	HBITMAP   hBmp = ::LoadBitmap(hIns, MAKEINTRESOURCE(IDB_BITMAP_HOMECHECK_BTN_HOVER));

	SetBitmap(hBmp);

	CStatic::OnMouseHover(nFlags, point);
}


BOOL CMyCheckBtn::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	// 使用自己的鼠标绘制函数。
	::SetCursor(::LoadCursorW(NULL,IDC_HAND));

	return TRUE;
}
