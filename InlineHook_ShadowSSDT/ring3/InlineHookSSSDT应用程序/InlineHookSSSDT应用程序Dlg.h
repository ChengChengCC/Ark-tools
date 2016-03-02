
// InlineHookSSSDT应用程序Dlg.h : 头文件
//

#pragma once

#include <WinIoCtl.h>


#define INITILIZE  20
#define  HOOK  10
#define  UNHOOK 30


#define IOCTL_GET_SSSDTSERVERICE	CTL_CODE(FILE_DEVICE_UNKNOWN, 0x801, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_INLINEHOOK_SSSDT \
	CTL_CODE(FILE_DEVICE_UNKNOWN,0x830,METHOD_BUFFERED,FILE_ANY_ACCESS)

#define IOCTL_INLINEUNHOOK_SSSDT \
	CTL_CODE(FILE_DEVICE_UNKNOWN,0x831,METHOD_BUFFERED,FILE_ANY_ACCESS)
// CInlineHookSSSDT应用程序Dlg 对话框
class CInlineHookSSSDT应用程序Dlg : public CDialogEx
{
// 构造
public:
	BOOL m_bHook;
	BOOL CInlineHookSSSDT应用程序Dlg::SendIoControlCode(ULONG_PTR ulControlCode);
	CInlineHookSSSDT应用程序Dlg(CWnd* pParent = NULL);	// 标准构造函数
	HANDLE CInlineHookSSSDT应用程序Dlg::OpenDevice(LPCTSTR wzLinkPath)
	{
		HANDLE hDevice = CreateFile(wzLinkPath,
			GENERIC_READ | GENERIC_WRITE,
			FILE_SHARE_READ | FILE_SHARE_WRITE,
			NULL,
			OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL,
			NULL);

		if (hDevice == INVALID_HANDLE_VALUE)
		{

		}

		return hDevice;

	}
// 对话框数据
	enum { IDD = IDD_INLINEHOOKSSSDT_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedButtonProtect();
	afx_msg void OnBnClickedButtonUnprotect();
	afx_msg void OnClose();
};
