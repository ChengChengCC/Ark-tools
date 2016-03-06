
// HeavenShadow.cpp : 定义应用程序的类行为。
//

#include "stdafx.h"
#include "HeavenShadow.h"
#include "HeavenShadowDlg.h"


//////////////////////////////////////////////////////////////////////////
#include "LoadSys.h"

#include "WzdSplash.h"

#define HS_ONLY_ONE_PROCESS L"HS_ONLY_ONE_PROCESS"

extern HANDLE g_hDevice;
BOOL bDriverIsOK = FALSE;
//////////////////////////////////////////////////////////////////////////

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CHeavenShadowApp

BEGIN_MESSAGE_MAP(CHeavenShadowApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()


// CHeavenShadowApp 构造

CHeavenShadowApp::CHeavenShadowApp()
{
	// 支持重新启动管理器
	m_dwRestartManagerSupportFlags = AFX_RESTART_MANAGER_SUPPORT_RESTART;

	// TODO: 在此处添加构造代码，
	// 将所有重要的初始化放置在 InitInstance 中
}


// 唯一的一个 CHeavenShadowApp 对象

CHeavenShadowApp theApp;


// CHeavenShadowApp 初始化

BOOL CHeavenShadowApp::InitInstance()
{
	//////////////////////////////////////////////////////////////////////////


	HANDLE hMutex = ::CreateMutexW(NULL,TRUE,HS_ONLY_ONE_PROCESS);
	if (GetLastError() == ERROR_ALREADY_EXISTS)
	{
		::MessageBox(
			NULL,
			L"您已经运行了天影卫士！",
			L"天影卫士",
			0
			);	//弹出对话框确认不能运行第二个实例。

		CloseHandle(hMutex);
		ExitProcess(0);
	}

	if (HsIs64BitWindows() == TRUE && sizeof(ULONG_PTR) == sizeof(ULONG32))
	{
		::MessageBox(
			NULL,
			L"您在使用 64 位的 Windows 操作系统。运行天影卫士 32 位版\r\n可能会造成不可预料的后果。敬请选择天影卫士 64 位版。",
			L"天影卫士",
			0
			);
		ExitProcess(0);
	}

	//////////////////显示Splash，2010-10-15///////////////////////////////////
	CWzdSplash wndSplash;                 //创建启动窗口类的实例
	wndSplash.Create(IDB_BITMAP_WZDSPLASH);
	wndSplash.CenterWindow();
	wndSplash.UpdateWindow();          //send WM_PAINT


	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////

	// 如果一个运行在 Windows XP 上的应用程序清单指定要
	// 使用 ComCtl32.dll 版本 6 或更高版本来启用可视化方式，
	//则需要 InitCommonControlsEx()。否则，将无法创建窗口。
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// 将它设置为包括所有要在应用程序中使用的
	// 公共控件类。
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinApp::InitInstance();


	AfxEnableControlContainer();

	// 创建 shell 管理器，以防对话框包含
	// 任何 shell 树视图控件或 shell 列表视图控件。
	CShellManager *pShellManager = new CShellManager;

	// 标准初始化
	// 如果未使用这些功能并希望减小
	// 最终可执行文件的大小，则应移除下列
	// 不需要的特定初始化例程
	// 更改用于存储设置的注册表项
	// TODO: 应适当修改该字符串，
	// 例如修改为公司或组织名
	SetRegistryKey(_T("天影卫士"));


	//////////////////////////////////////////////////////////////////////////

	// 	WCHAR *Temp = AfxGetApp()->m_lpCmdLine;
	// 
	// 	BOOL bIsHideWnd = FALSE;
	// 
	// 	if (wcslen(Temp))
	// 	{
	// 		WCHAR* p = Temp;
	// 
	// 		for (int i = 0; i< wcslen(Temp); i++)
	// 		{
	// 			if (_wcsnicmp(p,L"-HideWnd",wcslen(L"-HideWnd"))==0)
	// 			{
	// 				bIsHideWnd = TRUE;
	// 			}
	// 			p++;
	// 		}
	// 	}

	// 	CHeavenShadowDlg* dlg = new CHeavenShadowDlg();
	// 
	// 	m_pMainWnd = dlg;
	// 
	// 	INT_PTR nResponse = dlg->Create(IDD_HEAVENSHADOW_DIALOG);
	// 
	// 	dlg->ShowWindow(SW_HIDE);
	// 
	// 	return FALSE;


	//////////////////////////////////////////////////////////////////////////

	//加载驱动
	//ExtractFile();
	//////////////////////////////////////////////////////////////////////////
	WCHAR wzSysPath[260] = {0};
	WCHAR *p;
	CString SysPath;
	HMODULE module = GetModuleHandle(0);
	GetModuleFileName(module,wzSysPath,sizeof(wzSysPath));
	p = wcsrchr(wzSysPath,L'\\');
	*p = 0;
	SysPath = wzSysPath;
	if (HsIs64BitWindows()) 
		SysPath += L"\\sys\\x64\\HeavenShadowDrv.sys";
	else 
		SysPath += L"\\sys\\x86\\HeavenShadowDrv.sys";

	HsLoadNTDriver(HS_DRIVER_NAME,SysPath.GetBuffer());
	//////////////////////////////////////////////////////////////////////////


	g_hDevice = CreateFileW(HS_LINK_NAME,
		GENERIC_READ | GENERIC_WRITE,
		0,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL );


	if (g_hDevice != INVALID_HANDLE_VALUE)
	{
		ULONG_PTR ulCurrentPid = GetCurrentProcessId();
		ULONG_PTR ulRetCode = 0;
		DWORD dwReturnSize = 0;

		BOOL dwRet = DeviceIoControl(g_hDevice,
			HS_IOCTL(HS_IOCTL_PROC_SENDSELFPID),
			&ulCurrentPid,
			sizeof(ULONG_PTR),
			&ulRetCode,
			sizeof(ULONG_PTR),
			&dwReturnSize,
			NULL);

		if (dwRet && ulRetCode)
		{
			bDriverIsOK = TRUE;
		}
	}

	//////////////////////////////////////////////////////////////////////////

	
	



	CHeavenShadowDlg dlg;
	m_pMainWnd = &dlg;
	wndSplash.DestroyWindow();
	INT_PTR nResponse = dlg.DoModal();


	//////////////////////////////////////////////////////////////////////////

	CloseHandle(hMutex);

	CloseHandle(g_hDevice);

	HsUnloadNTDriver(HS_DRIVER_NAME);	//卸载驱动
	
	//////////////////////////////////////////////////////////////////////////


	if (nResponse == IDOK)
	{
		// TODO: 在此放置处理何时用
		//  “确定”来关闭对话框的代码
	}
	else if (nResponse == IDCANCEL)
	{
		// TODO: 在此放置处理何时用
		//  “取消”来关闭对话框的代码
	}

	// 删除上面创建的 shell 管理器。
	if (pShellManager != NULL)
	{
		delete pShellManager;
	}
 
// 	// 由于对话框已关闭，所以将返回 FALSE 以便退出应用程序，
// 	//  而不是启动应用程序的消息泵。
 	return FALSE;
}

