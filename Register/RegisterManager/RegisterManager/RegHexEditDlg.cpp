// RegHexEditDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "RegisterManager.h"
#include "RegHexEditDlg.h"
#include "afxdialogex.h"


// CRegHexEditDlg 对话框

IMPLEMENT_DYNAMIC(CRegHexEditDlg, CDialog)

CRegHexEditDlg::CRegHexEditDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CRegHexEditDlg::IDD, pParent)
	, m_strValueNameEdit(_T(""))
	, m_strHex(_T(""))
{

}

CRegHexEditDlg::~CRegHexEditDlg()
{
}

void CRegHexEditDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_HEXEDIT, m_HexEdit);
	DDX_Text(pDX, IDC_EDIT_VALUE_NAME, m_strValueNameEdit);
	DDX_Text(pDX, IDC_HEXEDIT, m_strHex);
}


BEGIN_MESSAGE_MAP(CRegHexEditDlg, CDialog)
	ON_BN_CLICKED(IDOK, &CRegHexEditDlg::OnBnClickedOk)
END_MESSAGE_MAP()


// CRegHexEditDlg 消息处理程序


BOOL CRegHexEditDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

		
	SetWindowText(L"二进制编辑器");

	ULONG i = 0;
	CString  strShowData;
	for (i=0;i<m_DataLen;i++)
	{
		CString Temp;


		Temp.Format(L"%02X ",(UCHAR)m_Data[i]);

		strShowData += Temp;

	}


	m_HexEdit.SetWindowTextW(strShowData);



	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}


void CRegHexEditDlg::OnBnClickedOk()
{

	UpdateData(TRUE);
	WCHAR* p = m_strHex.GetBuffer();
	m_RetLen = 0;

	int j = 0;
	WCHAR* q = m_strHex.GetBuffer();

	int n = m_strHex.GetLength();
	while (j!=m_strHex.GetLength())
	{
		if (q[j]==' ')
		{
			m_RetLen++;
		}

		j++;
	}
	int i = 0;
	int nTemp = 0;

	m_RetData = (PBYTE)malloc(m_RetLen);
	if (m_RetData)
	{
		memset(m_RetData, 0, m_RetLen);


		while (1)
		{
			if (*p==' ')
			{
				p++;
				continue;
			}

			if (*p=='\r')
			{
				p+=2;
				continue;
			}


			swscanf(p,L"%2x",&nTemp);

			m_RetData[i] = (UCHAR)nTemp;

			i++;

			if (i==m_RetLen)
			{
				break;
			}

			p+=2;
		}
	
	}


	CDialog::OnOK();
}
