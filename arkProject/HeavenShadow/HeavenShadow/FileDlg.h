#pragma once
#include "afxcmn.h"
#include "afxshelltreectrl.h"

#include "MyList.h"
#include "TrueColorToolBar.h"

// CFileDlg 对话框

class CFileDlg : public CDialog
{
	DECLARE_DYNAMIC(CFileDlg)

public:
	CFileDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CFileDlg();

// 对话框数据
	enum { IDD = IDD_DIALOG_FILE };

	CTrueColorToolBar m_wndToolBar; //工具栏
	CTrueColorToolBar m_wndToolBar_goto; //工具栏

	CWnd* m_wParent;

	

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnPaint();
	virtual BOOL OnInitDialog();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	CListCtrl m_fileList;
	CComboBoxEx m_filePath;
	CTreeCtrl m_dirTree;
	// 初始化文件树和列表

	afx_msg void OnRclickTreeFiledirectory(NMHDR *pNMHDR, LRESULT *pResult);
};



void HsLoadFileTreeList(CFileDlg *cFleDlg);