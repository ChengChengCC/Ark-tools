#pragma once
#include "afxcmn.h"


// CModuleDlg 对话框

class CModuleDlg : public CDialog
{
	DECLARE_DYNAMIC(CModuleDlg)

public:
	CModuleDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CModuleDlg();

// 对话框数据
	enum { IDD = IDD_DIALOG_MODULE };

	CWnd* m_wParent;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnPaint();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual BOOL OnInitDialog();
	CListCtrl m_ModuleList;
	afx_msg void OnSize(UINT nType, int cx, int cy);
	void HsLoadModuleList(void);
	afx_msg void OnNMCustomdrawListModulelist(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnLvnColumnclickListModulelist(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnMenuModuleRemovemodule();
	afx_msg void OnRclickListModulelist(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnMenuModuleRefresh();
};


static int CALLBACK HsModuleListCompareProc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);