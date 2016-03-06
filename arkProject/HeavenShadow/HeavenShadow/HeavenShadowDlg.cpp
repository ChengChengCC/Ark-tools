
// HeavenShadowDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "HeavenShadow.h"
#include "HeavenShadowDlg.h"
#include "afxdialogex.h"

#include "WzdSplash.h"

extern HANDLE g_hDevice;

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CWnd* g_wParent  = NULL;
BOOL bIsChecking = FALSE; //当前的检查状态


extern CWnd* g_process;

int dpix = 0;
int dpiy = 0;

enum
{
	UM_ICONNOTIFY= WM_USER+0x100
};


// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 对话框数据
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CHeavenShadowDlg 对话框




CHeavenShadowDlg::CHeavenShadowDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CHeavenShadowDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIconW(IDR_MAINFRAME);

	m_processDlg = NULL;
	m_moduleDlg = NULL;
	m_serviceDlg = NULL;
	m_fileDlg = NULL;
	m_aboutDlg = NULL;
	m_systemDlg = NULL;
	m_settingDlg = NULL;
	m_toolsDlg = NULL;
	m_kernelDlg = NULL;


	m_bHideWnd = TRUE;

}

CHeavenShadowDlg::~CHeavenShadowDlg()
{
}

void CHeavenShadowDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_TAB_MAIN, m_mainTab);
	DDX_Control(pDX, IDC_STATIC_PROCESS, m_btnProc);
	DDX_Control(pDX, IDC_STATIC_MODULE, m_btnModu);
	DDX_Control(pDX, IDC_STATIC_SERVICE, m_btnServ);
	DDX_Control(pDX, IDC_STATIC_LOGO, m_btnAbou);
	DDX_Control(pDX, IDC_STATIC_FILE, m_btnFile);
	DDX_Control(pDX, IDC_STATIC_SYSTEM, m_btnSys);
	DDX_Control(pDX, IDC_STATIC_SETTING, m_btnSet);
	DDX_Control(pDX, IDC_STATIC_TOOLS, m_btnTool);
	DDX_Control(pDX, IDC_STATIC_MANMENUBTN, m_btnMenu);
	DDX_Control(pDX, IDC_STATIC_KERNEL, m_btnKrnl);
}

BEGIN_MESSAGE_MAP(CHeavenShadowDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()

	ON_MESSAGE(HS_MESSAGE_STATUSDETAIL,HsSetStatusDetail)
	ON_MESSAGE(HS_MESSAGE_STATUSTIP,HsSetStatusTip)

	ON_STN_CLICKED(IDC_STATIC_MODULE, &CHeavenShadowDlg::OnClickedStaticModule)
	ON_STN_CLICKED(IDC_STATIC_PROCESS, &CHeavenShadowDlg::OnClickedStaticProcess)
	ON_STN_CLICKED(IDC_STATIC_SERVICE, &CHeavenShadowDlg::OnClickedStaticService)
	ON_STN_DISABLE(IDC_STATIC_PROCESS, &CHeavenShadowDlg::OnDisableStaticProcess)
	ON_STN_ENABLE(IDC_STATIC_PROCESS, &CHeavenShadowDlg::OnEnableStaticProcess)
	ON_STN_ENABLE(IDC_STATIC_MODULE, &CHeavenShadowDlg::OnEnableStaticModule)
	ON_STN_DISABLE(IDC_STATIC_MODULE, &CHeavenShadowDlg::OnDisableStaticModule)
	ON_STN_ENABLE(IDC_STATIC_SERVICE, &CHeavenShadowDlg::OnEnableStaticService)
	ON_STN_DISABLE(IDC_STATIC_SERVICE, &CHeavenShadowDlg::OnDisableStaticService)
	ON_STN_CLICKED(IDC_STATIC_LOGO, &CHeavenShadowDlg::OnClickedStaticLogo)
	ON_STN_ENABLE(IDC_STATIC_LOGO, &CHeavenShadowDlg::OnEnableStaticLogo)
	ON_STN_DISABLE(IDC_STATIC_LOGO, &CHeavenShadowDlg::OnDisableStaticLogo)
	ON_STN_CLICKED(IDC_STATIC_FILE, &CHeavenShadowDlg::OnClickedStaticFile)
	ON_STN_ENABLE(IDC_STATIC_FILE, &CHeavenShadowDlg::OnEnableStaticFile)
	ON_STN_DISABLE(IDC_STATIC_FILE, &CHeavenShadowDlg::OnDisableStaticFile)
	ON_WM_HELPINFO()
	ON_STN_CLICKED(IDC_STATIC_SYSTEM, &CHeavenShadowDlg::OnClickedStaticSystem)
	ON_STN_ENABLE(IDC_STATIC_SYSTEM, &CHeavenShadowDlg::OnEnableStaticSystem)
	ON_STN_DISABLE(IDC_STATIC_SYSTEM, &CHeavenShadowDlg::OnDisableStaticSystem)
	ON_STN_CLICKED(IDC_STATIC_SETTING, &CHeavenShadowDlg::OnClickedStaticSetting)
	ON_STN_ENABLE(IDC_STATIC_SETTING, &CHeavenShadowDlg::OnEnableStaticSetting)
	ON_STN_DISABLE(IDC_STATIC_SETTING, &CHeavenShadowDlg::OnDisableStaticSetting)
	ON_STN_CLICKED(IDC_STATIC_TOOLS, &CHeavenShadowDlg::OnClickedStaticTools)
	ON_STN_ENABLE(IDC_STATIC_TOOLS, &CHeavenShadowDlg::OnEnableStaticTools)
	ON_STN_DISABLE(IDC_STATIC_TOOLS, &CHeavenShadowDlg::OnDisableStaticTools)

	ON_STN_CLICKED(IDC_STATIC_MANMENUBTN, &CHeavenShadowDlg::OnClickedStaticManmenubtn)
	ON_MESSAGE(UM_ICONNOTIFY, (LRESULT (__thiscall CWnd::* )(WPARAM,LPARAM))OnIconNotify)
	ON_WM_CLOSE()
	ON_COMMAND(ID_MENU_EXIT, &CHeavenShadowDlg::OnMenuExit)
	ON_COMMAND(ID_MENU_SHOWMAIN, &CHeavenShadowDlg::OnMenuShowmain)
	ON_COMMAND(ID_MENU_HIDEMAIN, &CHeavenShadowDlg::OnMenuHidemain)
	ON_WM_SHOWWINDOW()
	ON_STN_CLICKED(IDC_STATIC_KERNEL, &CHeavenShadowDlg::OnClickedStaticKernel)
	ON_STN_ENABLE(IDC_STATIC_KERNEL, &CHeavenShadowDlg::OnEnableStaticKernel)
	ON_STN_DISABLE(IDC_STATIC_KERNEL, &CHeavenShadowDlg::OnDisableStaticKernel)
	ON_WM_CREATE()
END_MESSAGE_MAP()


// CHeavenShadowDlg 消息处理程序

BOOL CHeavenShadowDlg::OnInitDialog()
{
	CDialog::OnInitDialog();


	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
// 		if (!strAboutMenu.IsEmpty())
// 		{
// 			pSysMenu->AppendMenu(MF_SEPARATOR);
// 			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
// 		}
	}

	// 设置此对话框的图标。当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码


	if (g_hDevice == INVALID_HANDLE_VALUE)
	{

	}

	HsInitTray(&m_NotifyIcon);


	CString TempToolTip = L"菜单";
	m_btnMenu.SetToolTipText(TempToolTip);

	//////////////////////////////////////////////////////////////////////////

	g_wParent = this;

	CRect rect;

	GetWindowRect(&rect);

	CPaintDC dc(this);
	dpix = GetDeviceCaps(dc.m_hDC,LOGPIXELSX);
	dpiy = GetDeviceCaps(dc.m_hDC,LOGPIXELSY);
	rect.bottom += (LONG)(1+21*(dpiy/96.0));

	MoveWindow(&rect);

	//////////////////////////////////////////////////////////////////////////
	//添加状态栏
	m_StatusBar = new CStatusBarCtrl;

	m_StatusBar->Create(WS_CHILD|WS_VISIBLE|SBT_OWNERDRAW,CRect(0,0,0,0),this,0);

	int strPartDim[3] = {745,850,0};

	//strPartDim[0] = (int)((double)strPartDim[0]*dpix/96.0);

	GetClientRect(&rect);

	strPartDim[1] = rect.right - (int)(1+21*(dpix/96.0));
	strPartDim[0] = strPartDim[1] - (int)(120*(dpix/96.0));

	m_StatusBar->SetParts(3,strPartDim);

	m_StatusBar->SetText(L"准备就绪。",0,0);
	m_StatusBar->SetText(L"",1,0);


	//////////////////////////////////////////////////////////////////////////
	//加载工具栏

	int leftpos = 5;

	m_btnAbou.MoveWindow(leftpos+(70*0), 0, 70, 94);
	m_btnProc.MoveWindow(leftpos+(70*1), 0, 70, 94);
	m_btnModu.MoveWindow(leftpos+(70*2), 0, 70, 94);
	m_btnSys.MoveWindow(leftpos+(70*3), 0, 70, 94);
	m_btnKrnl.MoveWindow(leftpos+(70*4), 0, 70, 94);
	m_btnServ.MoveWindow(leftpos+(70*5), 0, 70, 94);
	m_btnFile.MoveWindow(leftpos+(70*6), 0, 70, 94);	//m_btnSys
	m_btnTool.MoveWindow(leftpos+(70*7), 0, 70, 94);
	m_btnSet.MoveWindow(leftpos+(70*8), 0, 70, 94);

	CRect menubtnrect;
	m_btnMenu.GetWindowRect(&menubtnrect);
	int menubtnwidth = menubtnrect.Width();
	menubtnrect.left = rect.right - 80;
	menubtnrect.top = rect.top - 20;
	menubtnrect.bottom = menubtnrect.top + menubtnwidth;
	menubtnrect.right = menubtnrect.left + menubtnwidth;

	//////////////////////////////////////////////////////////////////////////
	//初始化子对话框功能类
	
	GetClientRect(&rect);
	rect.top = 94+2;
	rect.bottom -= (LONG)(1+21*(dpiy/96.0));
	m_mainTab.MoveWindow(rect);


	//////////////////////////////////////////////////////////////////////////

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CHeavenShadowDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
// 		CAboutDlg dlgAbout;
// 		dlgAbout.DoModal();
//		//关于对话框

		if (m_bNowWindow != HS_DIALOG_ABOUT)
		{
			OnClickedStaticLogo();
		}
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CHeavenShadowDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CPaintDC dc(this);
		CRect rect;
		GetClientRect(rect);
		dc.FillSolidRect(rect,RGB(255,255,255));

		int ddd = GetDeviceCaps(dc.m_hDC,LOGPIXELSX);

		GetWindowRect(rect);

		CPoint startPoint;
		startPoint.x = -1;
		startPoint.y = 95;
		CPoint endPoint;
		endPoint.x = rect.Width();
		endPoint.y = 95;


		COLORREF m_Color(RGB(55,173,255));

		CClientDC aDC(this); //CClientDC的构造函数需要一个参数，这个参数是指向绘图窗口的指针，我们用this指针就可以了
		CPen pen(PS_SOLID,2,m_Color); ////建立一个画笔类对象，构造时设置画笔属性
		aDC.SelectObject(&pen);
		aDC.MoveTo(startPoint);
		aDC.LineTo(endPoint);

		CDialog::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CHeavenShadowDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CHeavenShadowDlg::HsEnableNowButton()
{
	switch(m_bNowWindow)
	{
	case HS_DIALOG_PROCESS:
		{
			m_btnProc.EnableWindow(TRUE);
			break;
		}
	case HS_DIALOG_MODULE:
		{
			m_btnModu.EnableWindow(TRUE);
			break;
		}
	case HS_DIALOG_KERNEL:
		{
			m_btnKrnl.EnableWindow(TRUE);
			break;
		}
	case HS_DIALOG_SERVICE:
		{
			m_btnServ.EnableWindow(TRUE);
			break;
		}
	case HS_DIALOG_FILE:
		{
			m_btnFile.EnableWindow(TRUE);
			break;
		}
	case HS_DIALOG_SYSTEM:
		{
			m_btnSys.EnableWindow(TRUE);
			break;
		}
	case HS_DIALOG_TOOLS:
		{
			m_btnTool.EnableWindow(TRUE);
			break;
		}
	case HS_DIALOG_SETTING:
		{
			m_btnSet.EnableWindow(TRUE);
			break;
		}
	case HS_DIALOG_ABOUT:
		{
			m_btnAbou.EnableWindow(TRUE);
			break;
		}
	default:
		{

		}
	}
}


//////////////////////////////////////////////////////////////////////////

void CHeavenShadowDlg::HsSelectWindow(UINT CurSel)
{
	switch(CurSel)
	{
	case HS_DIALOG_PROCESS:
		{

			if (m_processDlg == NULL)
			{
				m_processDlg  = new CProcessDlg(this);
				//////////////////////////////////////////////////////////////////////////
				//绑定对话框
				m_processDlg->Create(IDD_DIALOG_PROCESS,GetDlgItem(IDC_TAB_MAIN));
				//////////////////////////////////////////////////////////////////////////
				//移动窗口位置
				CRect rs;
				m_mainTab.GetClientRect(&rs);
				//设置子对话框尺寸并移动到指定位置
				m_processDlg->MoveWindow(&rs);
			}
			if (m_processDlg) m_processDlg->ShowWindow(true);
			if (m_serviceDlg) m_serviceDlg->ShowWindow(false);
			if (m_moduleDlg) m_moduleDlg->ShowWindow(false);
			if (m_aboutDlg) m_aboutDlg->ShowWindow(false);
			if (m_fileDlg) m_fileDlg->ShowWindow(false);
			if (m_systemDlg) m_systemDlg->ShowWindow(false);
			if (m_settingDlg) m_settingDlg->ShowWindow(false);
			if (m_toolsDlg) m_toolsDlg->ShowWindow(false);
			if (m_kernelDlg) m_kernelDlg->ShowWindow(false);
			break;
		}
	case HS_DIALOG_SERVICE:
		{
			if (m_serviceDlg == NULL)
			{
				m_serviceDlg  = new CServiceDlg(this);
				//////////////////////////////////////////////////////////////////////////
				//绑定对话框
				m_serviceDlg->Create(IDD_DIALOG_SERVICE,GetDlgItem(IDC_TAB_MAIN));
				//////////////////////////////////////////////////////////////////////////
				//移动窗口位置
				CRect rs;
				m_mainTab.GetClientRect(&rs);
				//设置子对话框尺寸并移动到指定位置
				m_serviceDlg->MoveWindow(&rs);
			}
			if (m_serviceDlg) m_serviceDlg->ShowWindow(true);
			if (m_processDlg) m_processDlg->ShowWindow(false);
			if (m_moduleDlg) m_moduleDlg->ShowWindow(false);
			if (m_aboutDlg) m_aboutDlg->ShowWindow(false);
			if (m_fileDlg) m_fileDlg->ShowWindow(false);
			if (m_systemDlg) m_systemDlg->ShowWindow(false);
			if (m_settingDlg) m_settingDlg->ShowWindow(false);
			if (m_toolsDlg) m_toolsDlg->ShowWindow(false);
			if (m_kernelDlg) m_kernelDlg->ShowWindow(false);
			break;
		}
	case HS_DIALOG_MODULE:
		{
			if (m_moduleDlg == NULL)
			{
				m_moduleDlg  = new CModuleDlg(this);
				//////////////////////////////////////////////////////////////////////////
				//绑定对话框
				m_moduleDlg->Create(IDD_DIALOG_MODULE,GetDlgItem(IDC_TAB_MAIN));
				//////////////////////////////////////////////////////////////////////////
				//移动窗口位置
				CRect rs;
				m_mainTab.GetClientRect(&rs);
				//设置子对话框尺寸并移动到指定位置
				m_moduleDlg->MoveWindow(&rs);
			}
			if (m_moduleDlg) m_moduleDlg->ShowWindow(true);
			if (m_processDlg) m_processDlg->ShowWindow(false);
			if (m_serviceDlg) m_serviceDlg->ShowWindow(false);
			if (m_aboutDlg) m_aboutDlg->ShowWindow(false);
			if (m_fileDlg) m_fileDlg->ShowWindow(false);
			if (m_systemDlg) m_systemDlg->ShowWindow(false);
			if (m_settingDlg) m_settingDlg->ShowWindow(false);
			if (m_toolsDlg) m_toolsDlg->ShowWindow(false);
			if (m_kernelDlg) m_kernelDlg->ShowWindow(false);
			break;
		}

	case HS_DIALOG_FILE:
		{
			if (m_fileDlg == NULL)
			{
				m_fileDlg  = new CFileDlg(this);
				//////////////////////////////////////////////////////////////////////////
				//绑定对话框
				m_fileDlg->Create(IDD_DIALOG_FILE,GetDlgItem(IDC_TAB_MAIN));
				//////////////////////////////////////////////////////////////////////////
				//移动窗口位置
				CRect rs;
				m_mainTab.GetClientRect(&rs);
				//设置子对话框尺寸并移动到指定位置
				m_fileDlg->MoveWindow(&rs);
			}
			if (m_fileDlg) m_fileDlg->ShowWindow(true);
			if (m_processDlg) m_processDlg->ShowWindow(false);
			if (m_serviceDlg) m_serviceDlg->ShowWindow(false);
			if (m_moduleDlg) m_moduleDlg->ShowWindow(false);
			if (m_aboutDlg) m_aboutDlg->ShowWindow(false);
			if (m_systemDlg) m_systemDlg->ShowWindow(false);
			if (m_settingDlg) m_settingDlg->ShowWindow(false);
			if (m_toolsDlg) m_toolsDlg->ShowWindow(false);
			if (m_kernelDlg) m_kernelDlg->ShowWindow(false);
			break;
		}
	case HS_DIALOG_SYSTEM:
		{

			if (m_systemDlg == NULL)
			{
				m_systemDlg  = new CSystemDlg(this);
				//////////////////////////////////////////////////////////////////////////
				//绑定对话框
				m_systemDlg->Create(IDD_DIALOG_SYSTEM,GetDlgItem(IDC_TAB_MAIN));
				//////////////////////////////////////////////////////////////////////////
				//移动窗口位置
				CRect rs;
				m_mainTab.GetClientRect(&rs);
				//设置子对话框尺寸并移动到指定位置
				m_systemDlg->MoveWindow(&rs);
			}
			if (m_systemDlg) m_systemDlg->ShowWindow(true);
			if (m_processDlg) m_processDlg->ShowWindow(false);
			if (m_serviceDlg) m_serviceDlg->ShowWindow(false);
			if (m_moduleDlg) m_moduleDlg->ShowWindow(false);
			if (m_aboutDlg) m_aboutDlg->ShowWindow(false);
			if (m_fileDlg) m_fileDlg->ShowWindow(false);
			if (m_settingDlg) m_settingDlg->ShowWindow(false);
			if (m_toolsDlg) m_toolsDlg->ShowWindow(false);
			if (m_kernelDlg) m_kernelDlg->ShowWindow(false);
			break;
		}
	case HS_DIALOG_KERNEL:
		{

			if (m_kernelDlg == NULL)
			{
				m_kernelDlg  = new CKernelDlg(this);
				//////////////////////////////////////////////////////////////////////////
				//绑定对话框
				m_kernelDlg->Create(IDD_DIALOG_KERNEL,GetDlgItem(IDC_TAB_MAIN));
				//////////////////////////////////////////////////////////////////////////
				//移动窗口位置
				CRect rs;
				m_mainTab.GetClientRect(&rs);
				//设置子对话框尺寸并移动到指定位置
				m_kernelDlg->MoveWindow(&rs);
			}
			if (m_kernelDlg) m_kernelDlg->ShowWindow(true);
			if (m_toolsDlg) m_toolsDlg->ShowWindow(false);
			if (m_processDlg) m_processDlg->ShowWindow(false);
			if (m_serviceDlg) m_serviceDlg->ShowWindow(false);
			if (m_moduleDlg) m_moduleDlg->ShowWindow(false);
			if (m_aboutDlg) m_aboutDlg->ShowWindow(false);
			if (m_fileDlg) m_fileDlg->ShowWindow(false);
			if (m_systemDlg) m_systemDlg->ShowWindow(false);
			if (m_settingDlg) m_settingDlg->ShowWindow(false);
			break;
		}
	case HS_DIALOG_TOOLS:
		{

			if (m_toolsDlg == NULL)
			{
				m_toolsDlg  = new CToolsDlg(this);
				//////////////////////////////////////////////////////////////////////////
				//绑定对话框
				m_toolsDlg->Create(IDD_DIALOG_TOOLS,GetDlgItem(IDC_TAB_MAIN));
				//////////////////////////////////////////////////////////////////////////
				//移动窗口位置
				CRect rs;
				m_mainTab.GetClientRect(&rs);
				//设置子对话框尺寸并移动到指定位置
				m_toolsDlg->MoveWindow(&rs);
			}
			if (m_toolsDlg) m_toolsDlg->ShowWindow(true);
			if (m_processDlg) m_processDlg->ShowWindow(false);
			if (m_serviceDlg) m_serviceDlg->ShowWindow(false);
			if (m_moduleDlg) m_moduleDlg->ShowWindow(false);
			if (m_aboutDlg) m_aboutDlg->ShowWindow(false);
			if (m_fileDlg) m_fileDlg->ShowWindow(false);
			if (m_systemDlg) m_systemDlg->ShowWindow(false);
			if (m_settingDlg) m_settingDlg->ShowWindow(false);
			if (m_kernelDlg) m_kernelDlg->ShowWindow(false);
			break;
		}
	case HS_DIALOG_SETTING:
		{

			if (m_settingDlg == NULL)
			{
				m_settingDlg  = new CSettingDlg(this);
				//////////////////////////////////////////////////////////////////////////
				//绑定对话框
				m_settingDlg->Create(IDD_DIALOG_SETTING,GetDlgItem(IDC_TAB_MAIN));
				//////////////////////////////////////////////////////////////////////////
				//移动窗口位置
				CRect rs;
				m_mainTab.GetClientRect(&rs);
				//设置子对话框尺寸并移动到指定位置
				m_settingDlg->MoveWindow(&rs);
			}
			if (m_settingDlg) m_settingDlg->ShowWindow(true);
			if (m_systemDlg) m_systemDlg->ShowWindow(false);
			if (m_processDlg) m_processDlg->ShowWindow(false);
			if (m_serviceDlg) m_serviceDlg->ShowWindow(false);
			if (m_moduleDlg) m_moduleDlg->ShowWindow(false);
			if (m_aboutDlg) m_aboutDlg->ShowWindow(false);
			if (m_fileDlg) m_fileDlg->ShowWindow(false);
			if (m_toolsDlg) m_toolsDlg->ShowWindow(false);
			if (m_kernelDlg) m_kernelDlg->ShowWindow(false);
			break;
		}
	case HS_DIALOG_ABOUT:
		{
			if (m_aboutDlg == NULL)
			{
				m_aboutDlg  = new CHsAboutDlg(this);
				//////////////////////////////////////////////////////////////////////////
				//绑定对话框
				m_aboutDlg->Create(IDD_DIALOG_ABOUT,GetDlgItem(IDC_TAB_MAIN));
				//////////////////////////////////////////////////////////////////////////
				//移动窗口位置
				CRect rs;
				m_mainTab.GetClientRect(&rs);
				//设置子对话框尺寸并移动到指定位置
				m_aboutDlg->MoveWindow(&rs);
			}
			if (m_aboutDlg) m_aboutDlg->ShowWindow(true);
			if (m_processDlg) m_processDlg->ShowWindow(false);
			if (m_serviceDlg) m_serviceDlg->ShowWindow(false);
			if (m_moduleDlg) m_moduleDlg->ShowWindow(false);
			if (m_fileDlg) m_fileDlg->ShowWindow(false);
			if (m_systemDlg) m_systemDlg->ShowWindow(false);
			if (m_settingDlg) m_settingDlg->ShowWindow(false);
			if (m_toolsDlg) m_toolsDlg->ShowWindow(false);
			if (m_kernelDlg) m_kernelDlg->ShowWindow(false);
			break;
		}
	}
}

//状态栏消息响应。

LRESULT CHeavenShadowDlg::HsSetStatusDetail(WPARAM wParam, LPARAM lParam)
{

	CString recvstr = (LPCTSTR)lParam;

	m_StatusBar->SetText(recvstr.GetBuffer(),0,0);

	return TRUE;
}

LRESULT CHeavenShadowDlg::HsSetStatusTip(WPARAM wParam, LPARAM lParam)
{
	CString recvstr = (LPCTSTR)lParam;

	m_StatusBar->SetText(recvstr.GetBuffer(),1,0);

	return TRUE;
}




BOOL CHeavenShadowDlg::HsInitChildDialog(void)
{
	HsSelectWindow(HS_DIALOG_ABOUT);

	return TRUE;
}



BOOL CHeavenShadowDlg::PreTranslateMessage(MSG* pMsg)
{
	// TODO: 在此添加专用代码和/或调用基类
	if (pMsg->message==WM_KEYDOWN &&
		(pMsg->wParam==VK_RETURN ||
		pMsg->wParam==VK_ESCAPE))
	{
		return TRUE;
	}
	return CDialog::PreTranslateMessage(pMsg);
}




BOOL CHeavenShadowDlg::OnHelpInfo(HELPINFO* pHelpInfo)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值

	return TRUE;

	//return CDialog::OnHelpInfo(pHelpInfo);
}





void CHeavenShadowDlg::HsInitTray(PNOTIFYICONDATA nid)
{
	nid->cbSize = sizeof(NOTIFYICONDATA);				//大小赋值
	nid->hWnd = m_hWnd;									//父窗口
	nid->uID = IDR_MAINFRAME;							//icon  ID
	nid->uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;		//托盘所拥有的状态
	nid->uCallbackMessage = UM_ICONNOTIFY;				//回调消息

	HICON hIcon = AfxGetApp()->LoadIconW(IDI_ICON_LITTLE);
	nid->hIcon = hIcon;									//icon 变量

	CString str=L"天影卫士";							//气泡提示
	lstrcpyn(nid->szTip, str, sizeof(nid->szTip) / sizeof(nid->szTip[0]));
	Shell_NotifyIcon(NIM_ADD, nid);						//显示托盘

}

void CHeavenShadowDlg::OnClose()
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值

	//if (m_bHideWnd == TRUE)
	{
		//ShowWindow(SW_HIDE);
	}
	//else
	{
		Shell_NotifyIcon(NIM_DELETE, &m_NotifyIcon);
		CDialog::OnClose();
	}


	//ShowWindow(SW_HIDE);
}


void CHeavenShadowDlg::OnIconNotify(WPARAM wParam, LPARAM lParam)
{
	switch ((UINT)lParam)
	{
	case WM_LBUTTONDOWN:
//	case WM_LBUTTONDBLCLK:
// 		if (!IsWindowVisible())
// 			ShowWindow(SW_SHOWNORMAL);
// 		else
// 			ShowWindow(SW_HIDE);
// 		break;
	case WM_RBUTTONDOWN: 
		CMenu menu;
		menu.LoadMenu(IDR_MENU_ICONNOTIFY);

		if (!IsWindowVisible()) 
			menu.DeleteMenu(ID_MENU_HIDEMAIN,MF_BYCOMMAND);
		else
			menu.DeleteMenu(ID_MENU_SHOWMAIN,MF_BYCOMMAND);

		CPoint point;
		GetCursorPos(&point);
		SetForegroundWindow();   //设置当前窗口
		menu.GetSubMenu(0)->TrackPopupMenu(
			TPM_LEFTBUTTON|TPM_RIGHTBUTTON, 
			point.x, point.y, this, NULL); 
		PostMessage(WM_USER, 0, 0);
		break;
	}
}



void CHeavenShadowDlg::OnClickedStaticManmenubtn()
{
	// TODO: 在此添加控件通知处理程序代码
	CMenu	popup;

	popup.LoadMenu(IDR_MENU_MAIN);		//加载菜单资源
	CMenu*	pM = popup.GetSubMenu(0);	//获得菜单的子项
	CPoint	p;

	CRect rect;
	m_btnMenu.GetWindowRect(&rect);

	p.SetPoint(rect.left,rect.top+20);

	int	count = pM->GetMenuItemCount();
	
	pM->TrackPopupMenu(TPM_LEFTALIGN, p.x, p.y, this);
	

}





void CHeavenShadowDlg::OnMenuExit()
{
	// TODO: 在此添加命令处理程序代码
	m_bHideWnd = FALSE;
	SendMessage(WM_CLOSE);
}


void CHeavenShadowDlg::OnMenuShowmain()
{
	// TODO: 在此添加命令处理程序代码
	ShowWindow(SW_SHOWNORMAL);
}


void CHeavenShadowDlg::OnMenuHidemain()
{
	// TODO: 在此添加命令处理程序代码
	ShowWindow(SW_HIDE);
}


void CHeavenShadowDlg::OnShowWindow(BOOL bShow, UINT nStatus)
{
	CDialog::OnShowWindow(bShow, nStatus);

	// TODO: 在此处添加消息处理程序代码

	if (bShow)
	{
		if (!bIsChecking && m_bNowWindow != 1)
		{
			HsEnableNowButton();
			HsInitChildDialog();
		}
	}
}





int CHeavenShadowDlg::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDialog::OnCreate(lpCreateStruct) == -1)
		return -1;

	// TODO:  在此添加您专用的创建代码

	return 0;
}
