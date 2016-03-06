// MyEdit.cpp : 实现文件
//

#include "stdafx.h"
#include "HeavenShadow.h"
#include "MyEdit.h"


// CMyEdit

IMPLEMENT_DYNAMIC(CMyEdit, CEdit)

CMyEdit::CMyEdit()
{

}

CMyEdit::~CMyEdit()
{
}


BEGIN_MESSAGE_MAP(CMyEdit, CEdit)
	ON_WM_CONTEXTMENU()
END_MESSAGE_MAP()



// CMyEdit 消息处理程序




void CMyEdit::OnContextMenu(CWnd* /*pWnd*/, CPoint /*point*/)
{
	// TODO: 在此处添加消息处理程序代码
}
