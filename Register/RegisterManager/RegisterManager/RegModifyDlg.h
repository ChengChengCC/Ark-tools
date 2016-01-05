#pragma once
#include "afxwin.h"


// CRegModifyDlg 对话框

class CRegModifyDlg : public CDialog
{
	DECLARE_DYNAMIC(CRegModifyDlg)

public:
	CRegModifyDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CRegModifyDlg();

// 对话框数据
	enum { IDD = IDD_REG_MODIFY_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	CString m_strValueNameStatic;

	CString m_strValueNameEdit;
	CString m_strValueDataStatic;
	CString m_strValueDataEdit;
};
