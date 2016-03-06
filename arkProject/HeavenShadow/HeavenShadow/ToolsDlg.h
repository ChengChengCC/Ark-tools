#pragma once


// CToolsDlg 对话框

class CToolsDlg : public CDialog
{
	DECLARE_DYNAMIC(CToolsDlg)

public:
	CToolsDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CToolsDlg();

// 对话框数据
	enum { IDD = IDD_DIALOG_TOOLS };

	CWnd* m_wParent;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnPaint();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
};
