#pragma once
#include "afxwin.h"
#include "afxcmn.h"

// CKernelDlg 对话框

class CKernelDlg : public CDialog
{
	DECLARE_DYNAMIC(CKernelDlg)

public:
	CKernelDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CKernelDlg();

// 对话框数据
	enum { IDD = IDD_DIALOG_KERNEL };

	CWnd* m_wParent;

	CImageList m_TreeKernelImageList;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	void InitKernelList(void);
	CListBox m_ListKernel;
	CListCtrl m_ListKernelCtrl;
	afx_msg void OnSelchangeListKernel();
	afx_msg void OnNMCustomdrawListKernelCtrl(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnMenuSsdtRefresh();
	afx_msg void OnNMRClickListKernelCtrl(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnMenuSsdtResume();
	CListBox m_KernelNameList;
	afx_msg void OnLbnSelchangeListKrnlname();
};
