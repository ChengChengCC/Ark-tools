#pragma once

#include "MyPicButton.h"

#include "MyText.h"
#include "afxwin.h"

// CAlertWnd 对话框

class CAlertWnd : public CDialog
{
	DECLARE_DYNAMIC(CAlertWnd)

public:
	CAlertWnd(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CAlertWnd();

// 对话框数据
	enum { IDD = IDD_DIALOG_ALERT };

	ULONG m_ulCount;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnClose();
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnPaint();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	virtual void OnOK();
	virtual void OnCancel();
//	afx_msg void OnStnClickedStaticAlertbtnexit();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	CString m_TimerTip;
	afx_msg void OnStnClickedStaticBtnPrevent();
	afx_msg void OnStnClickedStaticBtnAllow();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	CMyPicButton m_btnAllow;
	CMyPicButton m_btnPrevent;
	CMyText m_TextTimer;
	CMyText m_TextContext;
	CMyText m_TextTitle;
};


void HsPlayAlertSound(void);