#pragma once
#include "afxwin.h"


// CServiceDlg 对话框

class CServiceDlg : public CDialog
{
	DECLARE_DYNAMIC(CServiceDlg)

public:
	CServiceDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CServiceDlg();

// 对话框数据
	enum { IDD = IDD_DIALOG_SERVICE };


	CWnd* m_wParent;



protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnPaint();
	CButton m_demoBtn;
	virtual BOOL OnInitDialog();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnBnClickedButtonTest2();
};
