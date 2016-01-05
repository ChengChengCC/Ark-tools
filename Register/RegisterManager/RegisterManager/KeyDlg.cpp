// KeyDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "RegisterManager.h"
#include "KeyDlg.h"
#include "afxdialogex.h"


// CKeyDlg 对话框

IMPLEMENT_DYNAMIC(CKeyDlg, CDialog)

CKeyDlg::CKeyDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CKeyDlg::IDD, pParent)
	, m_strKeyNameStatic(_T(""))
	, m_strKeyNameEdit(_T(""))
{

}

CKeyDlg::~CKeyDlg()
{
}

void CKeyDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_KEY_NAME, m_strKeyNameStatic);
	DDX_Text(pDX, IDC_EDIT, m_strKeyNameEdit);
}


BEGIN_MESSAGE_MAP(CKeyDlg, CDialog)
END_MESSAGE_MAP()


// CKeyDlg 消息处理程序


BOOL CKeyDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	switch (m_nDlgType)
	{
	case enumRenameKey:
		SetWindowText(L"重命名键");
		m_strKeyNameStatic = L"新键名";
		break;

	case enumRenameValue:
		SetWindowText(L"重命名键值");
		m_strKeyNameStatic = L"新键值";
		break;

	case enumCreateKey:
		SetWindowText(L"创建新键");
		m_strKeyNameStatic = L"新键名";
		break;

	case enumSetValueKey:
		SetWindowText(L"创建新值");
		m_strKeyNameStatic = L"新键值";
		break;
	}

	UpdateData(FALSE);

	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}
