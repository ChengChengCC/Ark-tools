#pragma once
#include "afxwin.h"
#include "MyCheckBtn.h"
#include "MyText2.h"

// CHsAboutDlg 对话框

class CHsAboutDlg : public CDialog
{
	DECLARE_DYNAMIC(CHsAboutDlg)

public:
	CHsAboutDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CHsAboutDlg();

// 对话框数据
	enum { IDD = IDD_DIALOG_ABOUT };

	CWnd* m_wParent;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnPaint();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	CMyText2 m_TextCheckAlt;
	CMyText2 m_TextSysBit;
	CMyText2 m_TextSoftBit;
	CStatic  m_PicCheckAlt;
	CStatic m_PicSysBit;
	CMyCheckBtn m_CheckBtn;
	afx_msg void OnClickedStaticHomecheck();
};
