
// HeavenShadowDlg.h : 头文件
//

#pragma once

#include "afxwin.h"
#include "afxcmn.h"

#include "ProcessDlg.h"
#include "ServiceDlg.h"
#include "ModuleDlg.h"
#include "HsAboutDlg.h"
#include "FileDlg.h"
#include "SystemDlg.h"
#include "SettingDlg.h"
#include "ToolsDlg.h"
#include "KernelDlg.h"

#include "MyPicButton.h"




// CHeavenShadowDlg 对话框
class CHeavenShadowDlg : public CDialog
{
// 构造
public:
	CHeavenShadowDlg(CWnd* pParent = NULL);	// 标准构造函数
	virtual ~CHeavenShadowDlg();


// 对话框数据
	enum { IDD = IDD_HEAVENSHADOW_DIALOG };

	//////////////////////////////////////////////////////////////////////////
	/// 函数
	//////////////////////////////////////////////////////////////////////////
	

	//////////////////////////////////////////////////////////////////////////
	void HsSelectWindow(UINT CurSel);
	LRESULT HsSetStatusDetail(WPARAM wParam, LPARAM lParam);
	LRESULT HsSetStatusTip(WPARAM wParam, LPARAM lParam);
	BOOL HsLoadToolBar(void);
	BOOL HsInitChildDialog(void);
	void HsEnableNowButton(void);

	// 工具条
	CStatusBarCtrl*   m_StatusBar;

	CWnd* m_pHndWnd;


	BOOL m_bHideWnd; //判断是否显示界面

	//任务栏图标
	NOTIFYICONDATA m_NotifyIcon; //系统托盘结构
	void HsInitTray(PNOTIFYICONDATA nid);
	void OnIconNotify(WPARAM wParam, LPARAM lParam);

	//////////////////////////////////////////////////////////////////////////
	BOOL m_bNowWindow;
	//////////////////////////////////////////////////////////////////////////
	//子对话框类成员对象

	CProcessDlg* m_processDlg;
	CServiceDlg* m_serviceDlg;
	CModuleDlg*  m_moduleDlg;
	CHsAboutDlg* m_aboutDlg;
	CFileDlg*    m_fileDlg;
	CSystemDlg*  m_systemDlg;
	CSettingDlg* m_settingDlg;
	CToolsDlg*   m_toolsDlg;
	CKernelDlg*  m_kernelDlg;

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
	CTabCtrl m_mainTab;

	afx_msg void OnClickedStaticModule();
	CMyPicButton m_btnProc;
	CMyPicButton m_btnModu;
	CMyPicButton m_btnServ;
	CMyPicButton m_btnAbou;
	CMyPicButton m_btnFile;
	CMyPicButton m_btnSys;
	CMyPicButton m_btnSet;
	CMyPicButton m_btnTool;
	CMyPicButton m_btnMenu;
	CMyPicButton m_btnKrnl;

	afx_msg void OnClickedStaticProcess();
	afx_msg void OnClickedStaticService();
	afx_msg void OnEnableStaticProcess();
	afx_msg void OnDisableStaticProcess();
	afx_msg void OnEnableStaticModule();
	afx_msg void OnDisableStaticModule();
	afx_msg void OnEnableStaticService();
	afx_msg void OnDisableStaticService();
	afx_msg void OnClickedStaticLogo();
	afx_msg void OnEnableStaticLogo();
	afx_msg void OnDisableStaticLogo();
	
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnClickedStaticFile();
	afx_msg void OnEnableStaticFile();
	afx_msg void OnDisableStaticFile();
	afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
	afx_msg void OnClickedStaticSystem();
	afx_msg void OnEnableStaticSystem();
	afx_msg void OnDisableStaticSystem();
	afx_msg void OnClickedStaticSetting();
	afx_msg void OnEnableStaticSetting();
	afx_msg void OnDisableStaticSetting();
	afx_msg void OnClickedStaticTools();
	afx_msg void OnEnableStaticTools();
	afx_msg void OnDisableStaticTools();
	afx_msg void OnClickedStaticManmenubtn();
	afx_msg void OnClose();
	afx_msg void OnMenuExit();
	afx_msg void OnMenuShowmain();
	afx_msg void OnMenuHidemain();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnClickedStaticKernel();
	afx_msg void OnEnableStaticKernel();
	afx_msg void OnDisableStaticKernel();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
};
