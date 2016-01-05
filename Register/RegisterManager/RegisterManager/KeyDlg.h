#pragma once


// CKeyDlg 对话框


typedef enum DLG_TYPE
{
	enumRenameKey,
	enumCreateKey,
	enumSetValueKey,
	enumRenameValue,
};
class CKeyDlg : public CDialog
{
	DECLARE_DYNAMIC(CKeyDlg)

public:
	CKeyDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CKeyDlg();

	DLG_TYPE m_nDlgType;
	
	
// 对话框数据
	enum { IDD = IDD_KEY_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	CString m_strKeyNameStatic;
	CString m_strKeyNameEdit;
	virtual BOOL OnInitDialog();
};
