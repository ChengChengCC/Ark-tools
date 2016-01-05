#pragma once
#include "afxwin.h"


// CRegHexEditDlg 对话框

class CRegHexEditDlg : public CDialog
{
	DECLARE_DYNAMIC(CRegHexEditDlg)

public:
	CRegHexEditDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CRegHexEditDlg();

// 对话框数据
	enum { IDD = IDD_REG_HEX_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	CEdit m_HexEdit;
	CString m_strValueNameEdit;

	ULONG m_DataLen;
	PBYTE m_Data;
	PBYTE m_RetData;
	ULONG m_RetLen;
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedOk();
	CString m_strHex;
};
