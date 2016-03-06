// MyText.cpp : 实现文件
//

#include "stdafx.h"
#include "HeavenShadow.h"
#include "MyText.h"


// CMyText

IMPLEMENT_DYNAMIC(CMyText, CStatic)

CMyText::CMyText()
{

}

CMyText::~CMyText()
{
}


BEGIN_MESSAGE_MAP(CMyText, CStatic)
	ON_WM_CTLCOLOR_REFLECT()
END_MESSAGE_MAP()



// CMyText 消息处理程序



HBRUSH CMyText::CtlColor(CDC* pDC, UINT nCtlColor)
{
	pDC->SetBkMode(TRANSPARENT);
	pDC->SetTextColor(RGB(255,255,255));

	return (HBRUSH)::GetStockObject(NULL_BRUSH);
}


LRESULT CMyText::DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	// TODO: 在此添加专用代码和/或调用基类

	if (message == WM_SETTEXT)
	{
		CRect rect;

		GetWindowRect(&rect);

		CWnd* pParent = GetParent();

		if (pParent)
		{
			pParent->ScreenToClient(&rect);
			pParent->InvalidateRect(&rect);
		}
	}

	return CStatic::DefWindowProc(message, wParam, lParam);
}

