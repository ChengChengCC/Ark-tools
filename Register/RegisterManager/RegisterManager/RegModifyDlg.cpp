// RegModifyDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "RegisterManager.h"
#include "RegModifyDlg.h"
#include "afxdialogex.h"


// CRegModifyDlg 对话框

IMPLEMENT_DYNAMIC(CRegModifyDlg, CDialog)

CRegModifyDlg::CRegModifyDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CRegModifyDlg::IDD, pParent)
	, m_strValueNameStatic(_T(""))
	, m_strValueNameEdit(_T(""))
	, m_strValueDataStatic(_T(""))
	, m_strValueDataEdit(_T(""))
{

}

CRegModifyDlg::~CRegModifyDlg()
{
}

void CRegModifyDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_VALUE_NAME_STATIC, m_strValueNameStatic);
	DDX_Text(pDX, IDC_EDIT_VALUE_NAME, m_strValueNameEdit);
	DDX_Text(pDX, IDC_VALUE_DATA_STATIC, m_strValueDataStatic);
	DDX_Text(pDX, IDC_EDIT_VALUE_DATA, m_strValueDataEdit);
}


BEGIN_MESSAGE_MAP(CRegModifyDlg, CDialog)
END_MESSAGE_MAP()


// CRegModifyDlg 消息处理程序
