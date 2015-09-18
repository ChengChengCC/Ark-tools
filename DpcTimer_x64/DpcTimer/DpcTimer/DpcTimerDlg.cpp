
// DpcTimerDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "DpcTimer.h"
#include "DpcTimerDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


#define CTL_GET_DPCTIMER \
	CTL_CODE(FILE_DEVICE_UNKNOWN,0x830,METHOD_NEITHER,FILE_ANY_ACCESS)
#define CTL_GET_DRIVER \
	CTL_CODE(FILE_DEVICE_UNKNOWN,0x831,METHOD_NEITHER,FILE_ANY_ACCESS)
#define CTL_REMOVEDPCTIMER \
	CTL_CODE(FILE_DEVICE_UNKNOWN,0x832,METHOD_NEITHER,FILE_ANY_ACCESS)


typedef struct _COLUMNSTRUCT
{
	WCHAR*	szTitle;
	UINT    nWidth;
}COLUMNSTRUCT;


COLUMNSTRUCT g_Column_DPCTimer[] = 
{
	{	L"定时器对象",	150	},
	{	L"DPC",			150	},
	{	L"触发周期(s)",	80 },
	{	L"函数入口",	150	},
	{	L"所属模块",	420 }

};

UINT g_Column_DCPTimer_Count  = 5;	  //进程列表列数
HANDLE  g_hDevice = NULL;



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


// CDpcTimerDlg 对话框




CDpcTimerDlg::CDpcTimerDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CDpcTimerDlg::IDD, pParent)
	, m_strDPCTimer(_T(""))
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CDpcTimerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST, m_List);
	DDX_Text(pDX, IDC_STATIC_COUNT, m_strDPCTimer);
}

BEGIN_MESSAGE_MAP(CDpcTimerDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON, &CDpcTimerDlg::OnBnClickedButton)
	ON_NOTIFY(NM_RCLICK, IDC_LIST, &CDpcTimerDlg::OnNMRClickList)
	ON_COMMAND(ID_REMOVE,OnRemove)
END_MESSAGE_MAP()


// CDpcTimerDlg 消息处理程序

BOOL CDpcTimerDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

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
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 设置此对话框的图标。当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码

	InitList();
	
	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}



VOID CDpcTimerDlg::InitList()
{
	while(m_List.DeleteColumn(0));
	m_List.DeleteAllItems();

	m_List.SetExtendedStyle(LVS_EX_FULLROWSELECT|LVS_EX_GRIDLINES|LVS_EX_HEADERDRAGDROP);

	UINT i;
	for (i = 0;i<g_Column_DCPTimer_Count;i++)
	{
		m_List.InsertColumn(i, g_Column_DPCTimer[i].szTitle,LVCFMT_LEFT,g_Column_DPCTimer[i].nWidth);
	}
}


void CDpcTimerDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CDpcTimerDlg::OnPaint()
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
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CDpcTimerDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CDpcTimerDlg::OnBnClickedButton()
{
	// TODO: 在此添加控件通知处理程序代码
	g_hDevice = OpenDevice(L"\\\\.\\DPCTimerLink");

	if (g_hDevice==NULL)
	{
		::MessageBox(NULL,L"设备获取失败",NULL,0);
		return;
	}
	EnumDriver();
	GetDPC();


	CloseHandle(g_hDevice);
}


HANDLE CDpcTimerDlg::OpenDevice(LPCTSTR lpDevicePath)
{
	HANDLE hDevice = CreateFile(lpDevicePath,
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



void CDpcTimerDlg::GetDPC()
{
	ULONG_PTR ulCnt = 100;
	PDPC_TIMER_INFOR TimerInfor = NULL;
	BOOL bRet = FALSE;
	DWORD ulReturnSize = 0;

	m_List.DeleteAllItems();
	m_DPCVector.clear();

	do 
	{
		ULONG_PTR ulSize = sizeof(DPC_TIMER_INFOR) + ulCnt * sizeof(DPC_TIMER);

		if (TimerInfor)
		{
			free(TimerInfor);
			TimerInfor = NULL;
		}

		TimerInfor = (PDPC_TIMER_INFOR)malloc(ulSize);

		if (TimerInfor)
		{
			memset(TimerInfor, 0, ulSize);
			TimerInfor->ulCnt = ulCnt;		
			bRet = DeviceIoControl(g_hDevice,CTL_GET_DPCTIMER,
				NULL,
				0,
				TimerInfor,
				ulSize,
				&ulReturnSize,
				NULL);
		}

		ulCnt =TimerInfor->ulCnt + 10;

	} while (!bRet && TimerInfor->ulRetCnt > TimerInfor->ulCnt);

	if (bRet &&
		TimerInfor->ulCnt >= TimerInfor->ulRetCnt)
	{
		for (ULONG i = 0; i < TimerInfor->ulRetCnt; i++)
		{
			m_DPCVector.push_back(TimerInfor->DpcTimer[i]);
		}
	}

	if (TimerInfor)
	{
		free(TimerInfor);
		TimerInfor = NULL;
	}

	InsertDPCItem();
}




void CDpcTimerDlg::InsertDPCItem()
{
	m_ulDPCCount = 0;
	int i = 0;
	for (vector<DPC_TIMER>::iterator itor = m_DPCVector.begin(); itor != m_DPCVector.end(); itor++)
	{
		CString strTimerObject, strPeriod, strDispatch, strPath, strDpc;

		strTimerObject.Format(L"0x%08p", itor->TimerObject);
		strPeriod.Format(L"%d", itor->Period / 1000);
		strDispatch.Format(L"0x%08p", itor->TimeDispatch);
		strPath = GetDriverPath(itor->TimeDispatch);
		strDpc.Format(L"0x%08p", itor->Dpc);

		int n = m_List.InsertItem(m_List.GetItemCount(),strTimerObject);
		m_List.SetItemText(n, 1, strDpc);
		m_List.SetItemText(n, 2, strPeriod);
		m_List.SetItemText(n, 3, strDispatch);
		m_List.SetItemText(n, 4, strPath);
	
		i++;
		m_ulDPCCount++;
	}


	m_strDPCTimer.Format(L"%d",m_ulDPCCount);

	UpdateData(FALSE);

}




BOOL CDpcTimerDlg::EnumDriver()
{
	ULONG ulReturnSize = 0;
	BOOL bRet = FALSE;


	m_DriverList.clear();

	ULONG ulCount = 1000;
	PALL_DRIVERS Drivers = NULL;

	do 
	{
		ULONG ulSize = 0;
		if (Drivers)
		{
			free(Drivers);
			Drivers = NULL;
		}
		ulSize = sizeof(ALL_DRIVERS) + ulCount * sizeof(DRIVER_INFO);
		Drivers = (PALL_DRIVERS)malloc(ulSize);
		if (!Drivers)
		{
			break;
		}
		memset(Drivers,0,ulSize);
		bRet = DeviceIoControl(g_hDevice,CTL_GET_DRIVER,
			NULL,
			0,
			Drivers,
			ulSize,
			&ulReturnSize,
			NULL);

		ulCount = Drivers->ulCount + 100;
	} while (bRet == FALSE && GetLastError() == ERROR_INSUFFICIENT_BUFFER);

	if (bRet && Drivers->ulCount > 0)
	{
		for (ULONG i = 0;i<Drivers->ulCount; i++)
		{
			FixDriverPath(&Drivers->Drivers[i]);
			m_DriverList.push_back(Drivers->Drivers[i]);
		}
	}

	if (Drivers)
	{
		free(Drivers);
		Drivers = NULL;
	}

	return bRet;
}




CString CDpcTimerDlg::GetDriverPath(ULONG_PTR Address)
{
	CString strPath;

	for (vector<DRIVER_INFO>::iterator itor = m_DriverList.begin(); 
		itor != m_DriverList.end(); 
		itor++)
	{
		ULONG_PTR ulBase = itor->Base;
		ULONG_PTR ulEnd = itor->Base + itor->Size;
		if (Address >= ulBase && Address <= ulEnd)
		{
			strPath = itor->wzDriverPath;
			break;
		}
	}
	return strPath;
}


void CDpcTimerDlg::FixDriverPath(PDRIVER_INFO DriverInfor)
{
	if (!DriverInfor || wcslen(DriverInfor->wzDriverPath) == 0)
	{
		return;
	}

	WCHAR wzWindowsDirectory[MAX_PATH] = {0};
	WCHAR wzDriverDirectory[MAX_PATH] = {0};	
	WCHAR wzDriver[] = L"\\system32\\drivers\\";

	GetWindowsDirectory(wzWindowsDirectory, MAX_PATH - 1);
	wcscpy(wzDriverDirectory, wzWindowsDirectory);
	wcscat(wzDriverDirectory, wzDriver);

	WCHAR* wzOriginPath = DriverInfor->wzDriverPath;
	WCHAR  wzPath[MAX_PATH] = {0};
	WCHAR* wzTemp = wcschr(wzOriginPath, L'\\');

	// 没有目录信息，只有一个驱动名字的，直接拼接Driver目录。
	if (!wzTemp)
	{
		wcscpy(wzPath, wzDriverDirectory);
		wcscat(wzPath, wzOriginPath);
		wcscpy(wzOriginPath, wzPath);
		wzOriginPath[wcslen(wzPath)] = L'\0';
	}
	else
	{
		WCHAR wzUnknow[] = L"\\??\\";
		WCHAR wzSystemRoot[] = L"\\SystemRoot";
		WCHAR wzWindows[] = L"\\Windows";
		WCHAR wzWinnt[] = L"\\Winnt";
		size_t nOrigin = wcslen(wzOriginPath);

		if ( nOrigin >= wcslen(wzUnknow) && !_wcsnicmp(wzOriginPath, wzUnknow, wcslen(wzUnknow)) )
		{
			wcscpy(wzPath, wzOriginPath + wcslen(wzUnknow));
			wcscpy(wzOriginPath, wzPath);
			wzOriginPath[wcslen(wzPath)] = L'\0';
		}
		else if (nOrigin >= wcslen(wzSystemRoot) && !_wcsnicmp(wzOriginPath, wzSystemRoot, wcslen(wzSystemRoot)))
		{
			wcscpy(wzPath, wzWindowsDirectory);
			wcscat(wzPath, wzOriginPath + wcslen(wzSystemRoot));
			wcscpy(wzOriginPath, wzPath);
			wzOriginPath[wcslen(wzPath)] = L'\0';
		}
		else if (nOrigin >= wcslen(wzWindows) && !_wcsnicmp(wzOriginPath, wzWindows, wcslen(wzWindows)))
		{
			wcscpy(wzPath, wzWindowsDirectory);
			wcscat(wzPath, wzOriginPath + wcslen(wzWindows));
			wcscpy(wzOriginPath, wzPath);
			wzOriginPath[wcslen(wzPath)] = L'\0';
		}
		else if (nOrigin >= wcslen(wzWinnt) && !_wcsnicmp(wzOriginPath, wzWinnt, wcslen(wzWinnt)))
		{
			wcscpy(wzPath, wzWindowsDirectory);
			wcscat(wzPath, wzOriginPath + wcslen(wzWinnt));
			wcscpy(wzOriginPath, wzPath);
			wzOriginPath[wcslen(wzPath)] = L'\0';
		}
	}

	// 如果是短文件名
	if (wcschr(wzOriginPath, '~'))
	{
		WCHAR wzLongPath[MAX_PATH] = {0};
		DWORD nRet = GetLongPathName(wzOriginPath, wzLongPath, MAX_PATH);
		if ( !(nRet >= MAX_PATH || nRet == 0) )
		{
			wcscpy(wzOriginPath, wzLongPath);
			wzOriginPath[wcslen(wzLongPath)] = L'\0';
		}
	}
}

void CDpcTimerDlg::OnNMRClickList(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码
	CMenu Menu;
	Menu.CreatePopupMenu();
	Menu.AppendMenu(MF_STRING, ID_REMOVE,L"移除");	
	CPoint Pt;
	GetCursorPos(&Pt);         //得到鼠标位置
	Menu.TrackPopupMenu(TPM_RIGHTBUTTON, Pt.x, Pt.y, this);
	Menu.DestroyMenu();        //销毁资源
	*pResult = 0;
}




void CDpcTimerDlg::OnRemove()
{

	BOOL bRet = FALSE;
	DWORD ulReturnSize = 0;
	int Index = m_List.GetSelectionMark();
	if (Index<0)
	{
		return;
	}
	g_hDevice = OpenDevice(L"\\\\.\\DPCTimerLink");
	if (g_hDevice==NULL)
	{
		::MessageBox(NULL,L"设备获取失败",NULL,0);
		return;
	}

	CString Temp = m_List.GetItemText(Index,0);
	REMOVE_DPCTIMER  RemoveDPCTimer;
	for ( vector <DPC_TIMER>::iterator Iter = m_DPCVector.begin( ); Iter != m_DPCVector.end( ); Iter++ )
	{
		CString strTimerObject;
		strTimerObject.Format(L"0x%08p", Iter->TimerObject);
		if (!strTimerObject.CompareNoCase(Temp))
		{
			RemoveDPCTimer.TimerObject = Iter->TimerObject;
			bRet = DeviceIoControl(g_hDevice,CTL_REMOVEDPCTIMER,
				&RemoveDPCTimer,
				sizeof(REMOVE_DPCTIMER),
				NULL,
				0,
				&ulReturnSize,
				NULL);
			break;
		}	
	}
	m_ulDPCCount--;
	m_List.DeleteItem(Index);
	m_strDPCTimer.Format(L"%d",m_ulDPCCount);
	UpdateData(FALSE);
	CloseHandle(g_hDevice);


}