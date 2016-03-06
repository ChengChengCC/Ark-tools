#pragma once

#include "Common.h"
#include "afxcmn.h"

#include "MyList.h"


// CProcessDlg 对话框

class CProcessDlg : public CDialog
{
	DECLARE_DYNAMIC(CProcessDlg)

public:
	CProcessDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CProcessDlg();

// 对话框数据
	enum { IDD = IDD_DIALOG_PROCESS };


	//////////////////////////////////////////////////////////////////////////
	void HsLoadProcessList(void);
	void HsQuaryProcessThread(ULONG_PTR ProcessId);
	void AddProcessFileIcon(WCHAR* ProcessPath);
	LRESULT HsProcessDlgSendInsert(WPARAM wParam, LPARAM lParam);

	CWnd* m_wParent;
	CImageList m_TreeImageList;

	CListCtrl m_ProcessList;

	void CProcessDlg::HsInitList(void);

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持


	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedButtonTest();
	afx_msg void OnPaint();
	virtual BOOL OnInitDialog();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnColumnclickListProcesslist(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnRclickListProcesslist(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnMenuProcessRefresh();
	afx_msg void OnMenuProcessKillprocess();
	afx_msg void OnMenuProcessProcessthread();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnMenuProcessCopyinfo();
	afx_msg void OnMenuProcessAttribute();
	afx_msg void OnMenuProcessLocationfile();
	afx_msg void OnMenuProcessExporttxt();
	afx_msg void OnMenuProcessExportexcel();
	afx_msg void OnMenuProcessProcessprivilege();
	afx_msg void OnMenuProcessInjectdll();
	afx_msg void OnMenuProcessKillmust();
	void HsOpenProcessViewDlg(int nViewType);
	afx_msg void OnMenuProcessDetail();

	afx_msg void OnMenuProcessProcesshandle();
	afx_msg void OnMenuProcessMemory();
	afx_msg void OnMenuProcessNewrun();
	afx_msg void OnDblclkListProcesslist(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnMenuProcessProcesswindow();
	afx_msg void OnMenuProcessSuspend();
	afx_msg void OnMenuProcessRecovery();
	afx_msg void OnMenuProcessProcessmodule();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnNMCustomdrawListProcesslist(NMHDR *pNMHDR, LRESULT *pResult);
};
