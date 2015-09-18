
// DpcTimerDlg.h : 头文件
//

#pragma once
#include "afxcmn.h"
#include <iostream>
#include <vector>
#include <WinIoCtl.h>
using namespace std;



typedef struct _DRIVER_INFO_
{
	ULONG_PTR LodeOrder;
	ULONG_PTR Base;
	ULONG_PTR Size;
	ULONG_PTR DriverObject;
	ULONG_PTR DirverStartAddress;
	WCHAR wzDriverPath[MAX_PATH];
	WCHAR wzKeyName[MAX_PATH];
}DRIVER_INFO, *PDRIVER_INFO;

typedef struct _ALL_DRIVERS_
{
	ULONG_PTR ulCount;
	DRIVER_INFO Drivers[1];
}ALL_DRIVERS, *PALL_DRIVERS;


typedef struct _REMOVE_DPCTIMER
{
	ULONG_PTR     TimerObject;
}REMOVE_DPCTIMER,*PREMOVE_DPCTIMER;

typedef struct _DPC_TIMER_
{
	ULONG_PTR TimerObject;
	ULONG_PTR Period;			// 周期
	ULONG_PTR TimeDispatch;
	ULONG_PTR Dpc;
}DPC_TIMER, *PDPC_TIMER;

typedef struct _DPC_TIMER_INFOR_
{
	ULONG ulCnt;
	ULONG ulRetCnt;
	DPC_TIMER DpcTimer[1];
}DPC_TIMER_INFOR, *PDPC_TIMER_INFOR;

// CDpcTimerDlg 对话框
class CDpcTimerDlg : public CDialogEx
{
// 构造
public:
	CDpcTimerDlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
	enum { IDD = IDD_DPCTIMER_DIALOG };

	vector<DRIVER_INFO> m_DriverList;
	vector<DPC_TIMER>   m_DPCVector;
	ULONG m_ulDPCCount;
	~CDpcTimerDlg()
	{
		m_DPCVector.clear();
		m_DriverList.clear();
	}
	HANDLE OpenDevice(LPCTSTR lpDevicePath);
	void GetDPC();
	void InsertDPCItem();
	BOOL EnumDriver();
	CString GetDriverPath(ULONG_PTR Address);
	void FixDriverPath(PDRIVER_INFO DriverInfor);
	void OnRemove();
	VOID InitList();

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
	CListCtrl m_List;
	afx_msg void OnBnClickedButton();
	afx_msg void OnNMRClickList(NMHDR *pNMHDR, LRESULT *pResult);
	CString m_strDPCTimer;
};
