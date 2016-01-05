
// RegisterManagerDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "RegisterManager.h"
#include "RegisterManagerDlg.h"
#include "afxdialogex.h"
#include "KeyDlg.h"
#include "RegFindDlg.h"
#include "RegModifyDlg.h"
#include "RegHexEditDlg.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// 用于应用程序“关于”菜单项的 CAboutDlg 对话框
HANDLE g_hDevice = NULL;
CWnd*  g_Father = NULL;

HANDLE
OpenDevice(LPCTSTR lpDevicePath)
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


// CRegisterManagerDlg 对话框




CRegisterManagerDlg::CRegisterManagerDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CRegisterManagerDlg::IDD, pParent)
	, m_strComboText(_T(""))
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	m_wzhKeyCurrentUser = NULL;
	m_RegPathList.clear();
	m_bmRefresh.LoadBitmap(IDB_REFRESH);   //加载图标
	m_bmDelete.LoadBitmap(IDB_DELETE);
	m_bmCopy.LoadBitmap(IDB_COPY);
	m_bmExport.LoadBitmap(IDB_EXPORT);
	m_bmLookfor.LoadBitmap(IDB_LOOKFOR);

	m_FindRegDlg = NULL;
}

CRegisterManagerDlg::~CRegisterManagerDlg()
{
	if (m_wzhKeyCurrentUser)
	{
		free(m_wzhKeyCurrentUser);
		m_wzhKeyCurrentUser = NULL;
	}

	m_RegPathList.clear();
}

void CRegisterManagerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_TREE, m_Tree);
	DDX_Control(pDX, IDC_LIST, m_List);
	DDX_Control(pDX, IDC_COMBO, m_ComboBox);
	DDX_CBString(pDX, IDC_COMBO, m_strComboText);
}

BEGIN_MESSAGE_MAP(CRegisterManagerDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_NOTIFY(TVN_ITEMEXPANDING, IDC_TREE, &CRegisterManagerDlg::OnTvnItemexpandingTree)
	ON_NOTIFY(TVN_SELCHANGED, IDC_TREE, &CRegisterManagerDlg::OnTvnSelchangedTree)
	ON_BN_CLICKED(IDC_GOTO, &CRegisterManagerDlg::OnBnClickedGoto)
	ON_CBN_DROPDOWN(IDC_COMBO, &CRegisterManagerDlg::OnCbnDropdownCombo)
	ON_COMMAND(ID_REG_REFRESH, &CRegisterManagerDlg::OnRegRefresh)
	ON_NOTIFY(NM_RCLICK, IDC_TREE, &CRegisterManagerDlg::OnNMRClickTree)
	ON_COMMAND(ID_REG_NEW_KEY, &CRegisterManagerDlg::OnRegNewKey)
	ON_COMMAND(ID_STRING_VALUE, &CRegisterManagerDlg::OnStringValue)
	ON_COMMAND(ID_BINARY_VALUE, &CRegisterManagerDlg::OnBinaryValue)
	ON_COMMAND(ID_DWORD_VALUE, &CRegisterManagerDlg::OnDwordValue)
	ON_COMMAND(ID_MUI_STRING_VALUE, &CRegisterManagerDlg::OnMuiStringValue)
	ON_COMMAND(ID_EXPAND_STRING_VALUE, &CRegisterManagerDlg::OnExpandStringValue)
	ON_COMMAND(ID_REG_DELETE, &CRegisterManagerDlg::OnRegDelete)
	ON_COMMAND(ID_REG_RENAME, &CRegisterManagerDlg::OnRegRename)
	ON_COMMAND(ID_QWORD_VALUE, &CRegisterManagerDlg::OnQwordValue)
	ON_COMMAND(ID_REG_COPY_KEY_NAME, &CRegisterManagerDlg::OnRegCopyKeyName)
	ON_COMMAND(ID_REG_COPY_FULL_KEY_NAME, &CRegisterManagerDlg::OnRegCopyFullKeyName)
	ON_COMMAND(ID_REG_ADD_TO_QUICK_ADDRESS, &CRegisterManagerDlg::OnRegAddToQuickAddress)
	ON_COMMAND(ID_REG_INPORT, &CRegisterManagerDlg::OnRegInport)
	ON_COMMAND(ID_REG_EXPORT, &CRegisterManagerDlg::OnRegExport)
	ON_COMMAND(ID_REG_LOOK_FOR, &CRegisterManagerDlg::OnRegLookFor)
	ON_NOTIFY(NM_RCLICK, IDC_LIST, &CRegisterManagerDlg::OnNMRClickList)
	ON_COMMAND(ID_REG_LIST_REFRESH, &CRegisterManagerDlg::OnRegListRefresh)
	ON_COMMAND(ID_REG_LIST_MODIFY, &CRegisterManagerDlg::OnRegListModify)
	ON_COMMAND(ID_REG_LIST_DELETE, &CRegisterManagerDlg::OnRegListDelete)
	ON_COMMAND(ID_REG_LIST_RENAME, &CRegisterManagerDlg::OnRegListRename)
	ON_COMMAND(ID_REG_LIST_COPY_VALUE, &CRegisterManagerDlg::OnRegListCopyValue)
	ON_COMMAND(ID_REG_LIST_COPY_VALUE_DATA, &CRegisterManagerDlg::OnRegListCopyValueData)
END_MESSAGE_MAP()


// CRegisterManagerDlg 消息处理程序

BOOL CRegisterManagerDlg::OnInitDialog()
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



	g_Father = this;
	

	InitControlTree();   //初始化树控件


	//初始化List控件
	HICON hIcon[10];
	hIcon[0] = AfxGetApp()->LoadIcon (IDI_REG_SZ);
	hIcon[1] = AfxGetApp()->LoadIcon (IDI_DWORD);
	m_ImageList.Create(16, 16, ILC_COLOR16 | ILC_MASK, 2, 2); 
	for(int n = 0; n < 2; n++)
	{
		m_ImageList.Add(hIcon[n]);
		m_List.SetImageList(&m_ImageList, LVSIL_SMALL);
	}

	m_List.SetExtendedStyle(LVS_EX_FULLROWSELECT);
	m_List.InsertColumn(0, L"名称", LVCFMT_LEFT, 200);
	m_List.InsertColumn(1, L"类型", LVCFMT_LEFT, 130);
	m_List.InsertColumn(2, L"数据", LVCFMT_LEFT, 590);

	InitRegistry();
	InitRegPathList();    //Combo 上的快速定位项
	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CRegisterManagerDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CRegisterManagerDlg::OnPaint()
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
HCURSOR CRegisterManagerDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}




BOOL CRegisterManagerDlg::InitControlTree()
{

	//导入Icon资源
	int iCount = 3;
	HICON hIcon[4];
	hIcon[0] = AfxGetApp()->LoadIcon (IDI_COMPUTER);
	hIcon[1] = AfxGetApp()->LoadIcon (IDI_CLOSE_DIRECTORY);
	hIcon[2] = AfxGetApp()->LoadIcon (IDI_OPEN_DIRECTORY);


	//定义图片链表
	m_TreeImageList.Create(16, 16, ILC_COLOR16 | ILC_MASK, 2, 2); 
	for(int n = 0; n < iCount; n++)
	{
		m_TreeImageList.Add(hIcon[n]);
		m_Tree.SetImageList(&m_TreeImageList, LVSIL_NORMAL);
	}

	DWORD dwStyle = GetWindowLong(m_Tree.m_hWnd, GWL_STYLE);
	dwStyle |= TVS_HASBUTTONS | TVS_HASLINES | TVS_LINESATROOT;
	::SetWindowLong (m_Tree.m_hWnd, GWL_STYLE, dwStyle);

	HTREEITEM Computer = m_Tree.InsertItem(L"我的电脑", 0, 0);
	HTREEITEM hTreeItem0 = m_Tree.InsertItem(L"HKEY_CLASSES_ROOT", 1, 2, Computer, TVI_LAST);
	HTREEITEM hTreeItem1 = m_Tree.InsertItem(L"HKEY_CURRENT_USER", 1, 2, Computer, TVI_LAST);
	HTREEITEM hTreeItem2 = m_Tree.InsertItem(L"HKEY_LOCAL_MACHINE", 1, 2, Computer, TVI_LAST);
	HTREEITEM hTreeItem3 = m_Tree.InsertItem(L"HKEY_USERS", 1, 2, Computer, TVI_LAST);
	HTREEITEM hTreeItem4 = m_Tree.InsertItem(L"HKEY_CURRENT_CONFIG", 1, 2, Computer, TVI_LAST);

	m_Tree.Expand(Computer, TVE_EXPAND);

	return TRUE;
}


VOID CRegisterManagerDlg::InitRegistry()
{
	

	m_List.DeleteAllItems();
	m_strComboText = L"";
	UpdateData( FALSE );

	HTREEITEM hRootItem = m_Tree.GetRootItem();

	if (hRootItem != NULL)
	{
		HTREEITEM hChild = m_Tree.GetChildItem(hRootItem);
		while(hChild != NULL)
		{
			m_Tree.Expand(hChild, TVE_COLLAPSE);

			CString strKey = m_Tree.GetItemText(hChild);

			if (!strKey.CompareNoCase(L"HKEY_CLASSES_ROOT"))
			{
				EnumSubKeys(CLASSES_ROOT, hChild, TRUE);
			}
			else if (!strKey.CompareNoCase(L"HKEY_CURRENT_USER"))
			{
				EnumCurrentUserSubKeys(hChild);  //这个比较特殊
			}
			else if (!strKey.CompareNoCase(L"HKEY_LOCAL_MACHINE"))
			{
				EnumSubKeys(LOCAL_MACHINE, hChild);
			}
			else if (!strKey.CompareNoCase(L"HKEY_USERS"))
			{
				EnumSubKeys(USERS, hChild);
			}
			else if (!strKey.CompareNoCase(L"HKEY_CURRENT_CONFIG"))
			{
				EnumSubKeys(CURRENT_CONFIGL, hChild, TRUE);
			}
			else
			{
				break;
			}

			hChild = m_Tree.GetNextSiblingItem(hChild);
		}
	}

	m_Tree.Invalidate(TRUE);
}



VOID CRegisterManagerDlg::EnumSubKeys(CString strKey, HTREEITEM hItem, BOOL bSubSubKey/* = FALSE */)
{
	if ( !strKey.IsEmpty() && hItem != NULL )
	{
		DeleteSubTree(hItem);   //屏蔽看效果

		UNICODE_STRING uniKey;

		if (InitUnicodeString(&uniKey, strKey.GetBuffer()))
		{
			HANDLE hKey;
			OBJECT_ATTRIBUTES oa;

			InitializeObjectAttributes(&oa, &uniKey, OBJ_CASE_INSENSITIVE, 0, NULL);

			if (OpenKey(&hKey,KEY_ALL_ACCESS, &oa))
			{
			/*	WCHAR  wzBuffer[256] = {0};
				wsprintf(wzBuffer,L"0x%p",hKey);
				::MessageBox(NULL,wzBuffer,L"Data",MB_OK);*/
				
				for (ULONG i = 0;;i++)
				{
					ULONG nRetLen = 0;
					BOOL bRet = EnumerateKey(hKey,i,KeyBasicInformation,NULL,0,&nRetLen);

					if (!bRet && GetLastError() == ERROR_NO_MORE_ITEMS) 
					{
						break;
					}
					else if (!bRet && GetLastError() == ERROR_INSUFFICIENT_BUFFER) // STATUS_BUFFER_TOO_SMALL
					{


						//MessageBox(L"1",L"1");
						PKEY_BASIC_INFORMATION Buffer = (PKEY_BASIC_INFORMATION)malloc(nRetLen + 0x100);
						if (Buffer)
						{
							memset(Buffer, 0, nRetLen + 0x100);
							bRet = EnumerateKey(hKey, i, KeyBasicInformation,Buffer, nRetLen + 0x100, &nRetLen);

							if (bRet)
							{
								m_Tree.InsertItem(Buffer->Name, 1, 2, hItem, TVI_LAST);
							}

							free(Buffer);

							// 如果是枚举子项的子项，那么只要插入一个就可以了，
							// 因为前面已经有了一个+号了，主要为了速度优化。
							if (bRet && bSubSubKey)
							{
								break;
							}
						}
					}
				}

				CloseHandle(hKey);
			}

			FreeUnicodeString(&uniKey);
		}
	}
}


VOID CRegisterManagerDlg::DeleteSubTree(HTREEITEM hTreeItem)
{
	if(hTreeItem == NULL)   
	{
		return;   
	}

	if(m_Tree.ItemHasChildren(hTreeItem))
	{
		HTREEITEM hNext, hChild = m_Tree.GetChildItem(hTreeItem);
		while(hChild != NULL)
		{
			hNext = m_Tree.GetNextSiblingItem(hChild);
			m_Tree.DeleteItem(hChild);
			hChild = hNext; 
		}
	}
}



BOOL CRegisterManagerDlg::InitUnicodeString(PUNICODE_STRING uniString, WCHAR *wzString)
{
	BOOL bRet = FALSE;
	if (!wzString)
	{
		uniString->Buffer = NULL;
		uniString->Length = 0;
		uniString->MaximumLength = 2;
		bRet = TRUE;
	}
	else
	{
		ULONG nLen = wcslen(wzString);
		if (uniString && nLen > 0)
		{
			PWCHAR Buffer = (PWCHAR)malloc((nLen + 1) * sizeof(WCHAR));
			if (Buffer)
			{
				memset(Buffer, 0, (nLen + 1) * sizeof(WCHAR));
				wcscpy_s(Buffer, nLen + 1, wzString);
				uniString->Buffer = Buffer;
				uniString->Length = (USHORT)(nLen * sizeof(WCHAR));
				uniString->MaximumLength = (USHORT)((nLen + 1) * sizeof(WCHAR));
				bRet = TRUE;
			}
		}
	}

	return bRet;
}
VOID CRegisterManagerDlg::FreeUnicodeString(UNICODE_STRING *uniString)
{
	if (uniString && uniString->Buffer && uniString->Length > 0)
	{
		uniString->Length = 0;
		uniString->MaximumLength = 0;
		free(uniString->Buffer);
		uniString->Buffer = NULL;
	}
}

BOOL CRegisterManagerDlg::OpenKey(OUT PHANDLE  KeyHandle,
	IN ACCESS_MASK  DesiredAccess,
	IN POBJECT_ATTRIBUTES  ObjectAttributes
	)
{

	DWORD dwReturnSize = 0;
	DWORD dwRet = 0;

	struct{
		ACCESS_MASK DesiredAccess;
		POBJECT_ATTRIBUTES ObjectAttributes;
	}Open;



	memset(&Open, 0, sizeof(Open));

	g_hDevice = OpenDevice(L"\\\\.\\RegisterManagerLink");


	if (g_hDevice==(HANDLE)-1)
	{
		return FALSE;
	}
	
	Open.DesiredAccess = DesiredAccess;
	Open.ObjectAttributes = ObjectAttributes;
	
	
	dwRet = DeviceIoControl(g_hDevice,CTL_OPEN_KEY,
		&Open,
		sizeof(Open),
		KeyHandle,
		sizeof(KeyHandle),
		&dwReturnSize,
		NULL);


	if (dwRet==0)
	{

		CloseHandle(g_hDevice);
		return FALSE;
	}

	CloseHandle(g_hDevice);

	return TRUE;
}



BOOL CRegisterManagerDlg::EnumerateKey(IN HANDLE  KeyHandle,
	IN ULONG  ulIndex,
	IN ULONG  ulKeyInformationClass,
	OUT PVOID  KeyInformation,
	IN ULONG   ulLength,
	OUT PULONG ResultLength
	)
{

	DWORD dwReturnSize = 0;
	DWORD dwRet = 0;

	struct{
		HANDLE hKey;
		ULONG Index;
		ULONG InformationClass;
		ULONG Length;
	}Enum;


	memset(&Enum, 0, sizeof(Enum));

	g_hDevice = OpenDevice(L"\\\\.\\RegisterManagerLink");


	if (g_hDevice==(HANDLE)-1)
	{
		return FALSE;
	}


	Enum.hKey = KeyHandle;
	Enum.Index = ulIndex;
	Enum.InformationClass = ulKeyInformationClass;
	Enum.Length = ulLength;


	typedef struct _ENUM_VALUE_
	{
		PULONG RetLength;
		PVOID  ValueInfor;
	}ENUM_VALUE, *PENUM_VALUE;


	ENUM_VALUE EnumValue;
	memset(&EnumValue, 0, sizeof(ENUM_VALUE));
	EnumValue.RetLength = ResultLength;
	EnumValue.ValueInfor = KeyInformation;

	dwRet = DeviceIoControl(g_hDevice,CTL_ENUM_KEY,
		&Enum,
		sizeof(Enum),
		&EnumValue,
		sizeof(ENUM_VALUE),
		&dwReturnSize,
		NULL);


	if (dwRet==0)
	{

		CloseHandle(g_hDevice);
		return FALSE;
	}

	CloseHandle(g_hDevice);

	return TRUE;
}


void CRegisterManagerDlg::EnumCurrentUserSubKeys(HTREEITEM hChild)
{
	DeleteSubTree(hChild);

	if (!m_wzhKeyCurrentUser)
	{
		m_wzhKeyCurrentUser = (WCHAR*)malloc(1024 * sizeof(WCHAR));
		if (m_wzhKeyCurrentUser)
		{
			memset(m_wzhKeyCurrentUser, 0, 1024 * sizeof(WCHAR));

			if (!GetCurrentUserKeyPath(m_wzhKeyCurrentUser))
			{
				wcscpy_s(m_wzhKeyCurrentUser, 1024, L"\\Registry\\User\\.Default");
			}
		}
	}

	if (m_wzhKeyCurrentUser && wcslen(m_wzhKeyCurrentUser) > 0)
	{
		EnumSubKeys(m_wzhKeyCurrentUser, hChild, TRUE);
	}
}



BOOL CRegisterManagerDlg::GetCurrentUserKeyPath(OUT WCHAR *wzCurrentUserPath)
{
	if (!wzCurrentUserPath)
	{
		return FALSE;
	}

	HANDLE hToken;
	UCHAR szBuffer[256] = {0};
	PSID_AND_ATTRIBUTES SidBuffer;
	ULONG Length;
	BOOL Status = FALSE;

	Status = OpenThreadToken(GetCurrentThread(),
		TOKEN_QUERY,
		TRUE,
		&hToken);

	if (!Status)
	{
		Status = OpenProcessToken(GetCurrentProcess(),
			TOKEN_QUERY,
			&hToken);

		if (!Status) 
		{
			return Status;
		}
	}

	SidBuffer = (PSID_AND_ATTRIBUTES)szBuffer;
	Status = GetTokenInformation(hToken,
		TokenUser,
		(PVOID)SidBuffer,
		sizeof(szBuffer),
		&Length);

	CloseHandle(hToken);
	if (!Status)
	{
		return Status;
	}

	LPTSTR StringSid;
	Status = ConvertSidToStringSid(SidBuffer[0].Sid, &StringSid);
	if (!Status) return Status;

	WCHAR wzUser[] =  L"\\Registry\\User\\";

	Length = wcslen(StringSid) * sizeof(WCHAR) + sizeof(wzUser);

	WCHAR *wzPath = (WCHAR *)malloc(Length);
	if (!wzPath)
	{
		LocalFree((HLOCAL)StringSid);
		return FALSE;
	}

	memset(wzPath, 0, Length);
	wcscpy_s(wzPath, Length / sizeof(WCHAR),wzUser);
	wcscat_s(wzPath, Length / sizeof(WCHAR), StringSid);

	LocalFree((HLOCAL)StringSid);

	wcscpy_s(wzCurrentUserPath, 1024, wzPath);
	free(wzPath);

	return TRUE;
}

void CRegisterManagerDlg::OnTvnItemexpandingTree(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMTREEVIEW pNMTreeView = reinterpret_cast<LPNMTREEVIEW>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码
	HTREEITEM hItem = pNMTreeView->itemNew.hItem;
	BOOL nRet = FALSE;
	if (!nRet)
	{
		// 当是折叠状态的时候才插入子节点
		if(!(TVIS_EXPANDED & m_Tree.GetItemState(hItem, TVIS_EXPANDED)) &&
			m_Tree.GetRootItem() != hItem &&
			hItem != NULL)
		{
			DeleteSubTree(hItem);

			HTREEITEM hItemTemp = hItem;
			CString strKeyPath, strKeyPathTemp;
			while (1)
			{
				CString strText = m_Tree.GetItemText(hItemTemp);

				if (!strText.CompareNoCase(L"HKEY_CLASSES_ROOT"))
				{
					strKeyPath = CLASSES_ROOT;
					break;
				}
				else if (!strText.CompareNoCase(L"HKEY_CURRENT_USER"))
				{
					strKeyPath = m_wzhKeyCurrentUser;
					break;
				}
				else if (!strText.CompareNoCase(L"HKEY_LOCAL_MACHINE"))
				{
					strKeyPath = LOCAL_MACHINE;
					break;
				}
				else if (!strText.CompareNoCase(L"HKEY_USERS"))
				{
					strKeyPath = USERS;
					break;
				}
				else if (!strText.CompareNoCase(L"HKEY_CURRENT_CONFIG"))
				{
					strKeyPath = CURRENT_CONFIGL;
					break;
				}
				else
				{
					strKeyPathTemp = strText + L"\\" + strKeyPathTemp;
				}

				hItemTemp = m_Tree.GetParentItem(hItemTemp);
			}

			strKeyPath = strKeyPath + L"\\" + strKeyPathTemp;
			EnumSubKeys(strKeyPath, hItem);
			EnumSubSubKeys(strKeyPath, hItem);
		}
	}


	*pResult = 0;
}


void CRegisterManagerDlg::EnumSubSubKeys(CString strKey, HTREEITEM hItem)
{
	if (!strKey.IsEmpty() && hItem != NULL)
	{
		HTREEITEM hChild = m_Tree.GetChildItem(hItem);
		while(hChild != NULL)
		{
			CString strKeyPath = strKey + L"\\" + m_Tree.GetItemText(hChild);
			EnumSubKeys(strKeyPath, hChild, TRUE);  // bSubSubKey 为true,优化枚举速度
			hChild = m_Tree.GetNextSiblingItem(hChild);
		}
	}
}


void CRegisterManagerDlg::OnTvnSelchangedTree(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMTREEVIEW pNMTreeView = reinterpret_cast<LPNMTREEVIEW>(pNMHDR);
	


	HTREEITEM hItem = pNMTreeView->itemNew.hItem;
	CString strKeyShowPath;

	if(m_Tree.GetRootItem() != hItem &&
		hItem != NULL)
	{
		BOOL nRet = FALSE;
		if (!nRet)
		{
			HTREEITEM hItemTemp = hItem;
			CString strKeyPath, strKeyPathTemp;
			while (1)
			{
				CString strText = m_Tree.GetItemText(hItemTemp);

				if (!strText.CompareNoCase(L"HKEY_CLASSES_ROOT"))
				{
					strKeyPath = CLASSES_ROOT;
					strKeyShowPath = L"HKEY_CLASSES_ROOT";
					break;
				}
				else if (!strText.CompareNoCase(L"HKEY_CURRENT_USER"))
				{
					strText = m_wzhKeyCurrentUser;
					strKeyShowPath = L"HKEY_CURRENT_USER";
					break;
				}
				else if (!strText.CompareNoCase(L"HKEY_LOCAL_MACHINE"))
				{
					strKeyPath = LOCAL_MACHINE;
					strKeyShowPath = L"HKEY_LOCAL_MACHINE";
					break;
				}
				else if (!strText.CompareNoCase(L"HKEY_USERS"))
				{
					strKeyPath = USERS;
					strKeyShowPath = L"HKEY_USERS";
					break;
				}
				else if (!strText.CompareNoCase(L"HKEY_CURRENT_CONFIG"))
				{
					strKeyPath = CURRENT_CONFIGL;
					strKeyShowPath = L"HKEY_CURRENT_CONFIG";
					break;
				}
				else
				{
					strKeyPathTemp = strText + L"\\" + strKeyPathTemp;
				}

				hItemTemp = m_Tree.GetParentItem(hItemTemp);
			}

			strKeyPath = strKeyPath + L"\\" + strKeyPathTemp;
			strKeyShowPath = strKeyShowPath + L"\\" + strKeyPathTemp;
			EnumValues(strKeyPath);   //List 控件显示数据
		}
	}

	strKeyShowPath.TrimRight('\\');
	m_strComboText = strKeyShowPath;
	UpdateData(FALSE);

	*pResult = 0;
}



void CRegisterManagerDlg::EnumValues(CString strKey)
{
	if (strKey.IsEmpty())
	{
		return;
	}

	UNICODE_STRING uniKey;

	if (InitUnicodeString(&uniKey, strKey.GetBuffer()))
	{
		HANDLE hKey;
		OBJECT_ATTRIBUTES oa;

		InitializeObjectAttributes(&oa, &uniKey, OBJ_CASE_INSENSITIVE, 0, NULL);
		if (OpenKey(&hKey, KEY_ALL_ACCESS, &oa))
		{
			m_List.DeleteAllItems();
			BOOL bDefault = FALSE;

			for (ULONG i = 0; ; i++)
			{
				ULONG nRetLen = 0;
				BOOL bRet = EnumerateValueKey(hKey,i,KeyValueFullInformation,NULL,0,&nRetLen);
				if (!bRet && GetLastError() == ERROR_NO_MORE_ITEMS) 
				{
					//没有了
					break;
				}
				else if (!bRet && GetLastError() == ERROR_INSUFFICIENT_BUFFER) 
				{
					PKEY_VALUE_FULL_INFORMATION Buffer = (PKEY_VALUE_FULL_INFORMATION)malloc(nRetLen + 0x100);
					if (Buffer)
					{
						memset(Buffer, 0, nRetLen + 0x100);
						bRet = EnumerateValueKey(hKey, i, KeyValueFullInformation, Buffer, nRetLen + 0x100, &nRetLen);

						if (bRet)
						{
							CString strName, strType, strData;
							WCHAR wzTempName[1024] = {0};

							strType = GetKeyType(Buffer->Type);
							strData = GetKeyData(Buffer->Type, (WCHAR*)((PBYTE)Buffer + Buffer->DataOffset), Buffer->DataLength);

							// NameLength为0
							if (Buffer->NameLength == 0)
							{
								strName = L"默认";
								bDefault = TRUE;
							}
							else
							{
								wcsncpy_s(wzTempName, 1024, Buffer->Name, Buffer->NameLength / sizeof(WCHAR));
								strName = wzTempName;
							}

							int nImage = 1;
							if (Buffer->Type == REG_SZ ||
								Buffer->Type == REG_EXPAND_SZ ||
								Buffer->Type == REG_MULTI_SZ)
							{
								nImage = 0;
							}

							int n = m_List.InsertItem(m_List.GetItemCount(),strName, nImage);

							if (strType == L"REG_QWORD")
							{
								Sleep(1);
							}
							m_List.SetItemText(n, 1, strType);
							m_List.SetItemText(n, 2, strData);

							if (Buffer->NameLength == 0)
							{
								m_List.SetItemData(n, 1);
							}
						}

						free(Buffer);
					}
				}
			}

			// 添加一个默认的键，内容为空
			if (!bDefault)
			{
				int n = m_List.InsertItem(0,L"默认", 0);
				m_List.SetItemText(n, 1, L"REG_SZ");
				m_List.SetItemText(n, 2, L"数值未设置");
			
				m_List.SetItemData(n, 1);
			}

			CloseHandle(hKey);
		}

			
		FreeUnicodeString(&uniKey);
	}
}


BOOL CRegisterManagerDlg::EnumerateValueKey(IN HANDLE  KeyHandle,
	IN ULONG  ulIndex,
	IN ULONG  ulKeyValueInformationClass,
	OUT PVOID  KeyValueInformation,
	IN ULONG  ulLength,
	OUT PULONG  ulResultLength
	)
{

	DWORD dwReturnSize = 0;
	DWORD dwRet = 0;

	struct{
		HANDLE hKey;
		ULONG Index;
		ULONG InformationClass;
		ULONG Length;
	}Enum;


	memset(&Enum, 0, sizeof(Enum));

	g_hDevice = OpenDevice(L"\\\\.\\RegisterManagerLink");


	if (g_hDevice==(HANDLE)-1)
	{
		return FALSE;
	}


	Enum.hKey = KeyHandle;
	Enum.Index = ulIndex;
	Enum.InformationClass = ulKeyValueInformationClass;
	Enum.Length = ulLength;

	typedef struct _ENUM_VALUE_
	{
		PULONG RetLength;
		PVOID  ValueInfor;
	}ENUM_VALUE, *PENUM_VALUE;


	ENUM_VALUE EnumValue;
	memset(&EnumValue, 0, sizeof(ENUM_VALUE));
	EnumValue.RetLength = ulResultLength;
	EnumValue.ValueInfor = KeyValueInformation;

	dwRet = DeviceIoControl(g_hDevice,CTL_ENUM_KEY_VALUE,
		&Enum,
		sizeof(Enum),
		&EnumValue,
		sizeof(ENUM_VALUE),
		&dwReturnSize,
		NULL);


	if (dwRet==0)
	{

		CloseHandle(g_hDevice);
		return FALSE;
	}

	CloseHandle(g_hDevice);

	return TRUE;

}




CString CRegisterManagerDlg::GetKeyType(ULONG ulType)
{
	CString strRet;

	switch (ulType)
	{
	case REG_NONE:
		strRet = L"REG_NONE";
		break;

	case REG_SZ:
		strRet = L"REG_SZ";
		break;

	case REG_EXPAND_SZ:
		strRet = L"REG_EXPAND_SZ";
		break;

	case REG_BINARY:
		strRet = L"REG_BINARY";
		break;

	case REG_DWORD:
		strRet = L"REG_DWORD";
		break;

	case REG_DWORD_BIG_ENDIAN:
		strRet = L"REG_DWORD_BIG_ENDIAN";
		break;

	case REG_LINK:
		strRet = L"REG_LINK";
		break;

	case REG_MULTI_SZ:
		strRet = L"REG_MULTI_SZ";
		break;

	case REG_RESOURCE_LIST:
		strRet = L"REG_RESOURCE_LIST";
		break;

	case REG_FULL_RESOURCE_DESCRIPTOR:
		strRet = L"REG_FULL_RESOURCE_DESCRIPTOR";
		break;

	case REG_RESOURCE_REQUIREMENTS_LIST:
		strRet = L"REG_RESOURCE_REQUIREMENTS_LIST";
		break;

	case REG_QWORD:
		strRet = L"REG_QWORD";
		break;

	default:
		strRet = L"Unknow";
	}

	return strRet;
}

CString CRegisterManagerDlg::GetKeyData(ULONG ulType, WCHAR* wzData, ULONG ulDataLength)
{
	CString strRet;

	if (!ulDataLength)
	{
		return strRet;
	}

	switch (ulType)
	{
	case REG_SZ:
	case REG_EXPAND_SZ:
		strRet = wzData;
		break;

	case REG_LINK:
	case REG_NONE:
	case REG_RESOURCE_REQUIREMENTS_LIST:
	case REG_FULL_RESOURCE_DESCRIPTOR:
	case REG_RESOURCE_LIST:
	case REG_BINARY:
		{
			for (ULONG i = 0; i<ulDataLength; i++)
			{
				CString strTemp;
				strTemp.Format(L"%02x ", *((PBYTE)wzData + i));
				strRet += strTemp;
			}
		}
		break;

	case REG_DWORD:
		strRet.Format(L"0x%08X (%d)", *(PULONG)wzData, *(PULONG)wzData);
		break;

	case REG_DWORD_BIG_ENDIAN:
		{
			BYTE Value[4] = {0};
			Value[0] = *((PBYTE)wzData + 3);
			Value[1] = *((PBYTE)wzData + 2);
			Value[2] = *((PBYTE)wzData + 1);
			Value[3] = *((PBYTE)wzData + 0);
			strRet.Format(L"0x%08X (%d)", *(PULONG)Value, *(PULONG)Value);
		}
		break;
	case REG_MULTI_SZ:
		{
			DWORD len = 0;
			while (wcslen(wzData + len))
			{
				strRet += (wzData + len);
				strRet += L" ";
				len += wcslen(wzData + len) + 1;
			}
		}
		break;

	case REG_QWORD:
		{
			QWORD qwValue = *((PQWORD)wzData);

			strRet.Format(L"0x%08X (%ld)", qwValue, qwValue);
		}

		break;

	default:
		strRet = L"Unknow";
	}

	return strRet;
}

void CRegisterManagerDlg::OnCbnDropdownCombo()
{
	m_ComboBox.ResetContent();
	InitializeComboBox();
}



VOID CRegisterManagerDlg::InitializeComboBox()
{
	

	list <CString> RegPathList;
	GetAllRegPath(RegPathList);
	m_nComboBoxCnt = 0;

	for (list <CString>::iterator ir = RegPathList.begin(); 
		ir != RegPathList.end();
		ir++)
	{
		m_ComboBox.InsertString(m_nComboBoxCnt, *ir);
		m_nComboBoxCnt++;
	}
}



void CRegisterManagerDlg::GetAllRegPath(std::list <CString> &RegPathList)
{
	RegPathList.clear();

	for (std::list <CString>::iterator itor = m_RegPathList.begin();
		itor != m_RegPathList.end();
		itor++)
	{
		RegPathList.push_back(*itor);
	}
}


void CRegisterManagerDlg::InitRegPathList()
{
	m_RegPathList.push_back(L"HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run");
	m_RegPathList.push_back(L"HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Windows");
	m_RegPathList.push_back(L"HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\ShellExecuteHooks");
	m_RegPathList.push_back(L"HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Shell Extensions\\Approved");
	m_RegPathList.push_back(L"HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\ShellServiceObjectDelayLoad");
	m_RegPathList.push_back(L"HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon");
	m_RegPathList.push_back(L"HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Internet Explorer");
	m_RegPathList.push_back(L"HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\IniFileMapping");
	m_RegPathList.push_back(L"HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Image File Execution Options");
	m_RegPathList.push_back(L"HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Control\\Session Manager");
	m_RegPathList.push_back(L"HKEY_LOCAL_MACHINE\\SOFTWARE\\Policies\\Microsoft\\Windows\\IPSec\\Policy\\Local");
	m_RegPathList.push_back(L"HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Desktop\\NameSpace");
}

void CRegisterManagerDlg::OnBnClickedGoto()
{
	UpdateData(TRUE);
	m_ComboBox.ResetContent();
	m_ComboBox.SetWindowText(m_strComboText);


	if (m_strComboText.IsEmpty())
	{
		return;
	}


	CString strKeyPath = ComboString2KeyPath();
	if (strKeyPath.IsEmpty())
	{
		return;
	}

	CString strHead = m_strComboText.Left(m_strComboText.Find(L"\\"));
	if (strHead.IsEmpty())
	{
		strHead = m_strComboText;
	}

	if (strHead.IsEmpty())
	{
		return;
	}

	HTREEITEM hRootItem = m_Tree.GetRootItem();
	if (hRootItem == NULL)
	{
	
		return;
	}

	HTREEITEM hChild = m_Tree.GetChildItem(hRootItem);
	CString strRoot;

	WCHAR wzhKeyRoot[] = L"HKEY_CLASSES_ROOT";
	WCHAR wzhKCR[] = L"HKCR";

	WCHAR wzhKeyCurrentUser[] = L"HKEY_CURRENT_USER";
	WCHAR wzhKCU[] = L"HKCU";

	WCHAR wzhKeyLocalMachine[] = L"HKEY_LOCAL_MACHINE";
	WCHAR wzhKLM[] = L"HKLM";

	WCHAR wzhKeyUsers[] = L"HKEY_USERS";
	WCHAR wzhKU[] = L"HKU";;

	WCHAR wzhKeyCurrentConfig[] = L"HKEY_CURRENT_CONFIG";
	WCHAR wzhKCC[] = L"HKCC";;

	while(hChild != NULL)
	{
		CString strKey = m_Tree.GetItemText(hChild);

		if ((!strHead.CompareNoCase(wzhKeyRoot) || !strHead.CompareNoCase(wzhKCR)) &&
			!strKey.CompareNoCase(wzhKeyRoot))
		{
			strRoot = CLASSES_ROOT;
			break;
		}
		else if ((!strHead.CompareNoCase(wzhKeyCurrentUser) || !strHead.CompareNoCase(wzhKCU)) &&
			!strKey.CompareNoCase(wzhKeyCurrentUser))
		{
			strRoot = m_wzhKeyCurrentUser;
			break;
		}
		else if ((!strHead.CompareNoCase(wzhKeyLocalMachine) || !strHead.CompareNoCase(wzhKLM)) &&
			!strKey.CompareNoCase(wzhKeyLocalMachine))
		{
			strRoot = LOCAL_MACHINE;
			break;
		}
		else if ((!strHead.CompareNoCase(wzhKeyUsers) || !strHead.CompareNoCase(wzhKU)) &&
			!strKey.CompareNoCase(wzhKeyUsers))
		{
			strRoot = USERS;
			break;
		}
		else if ((!strHead.CompareNoCase(wzhKeyCurrentConfig) || !strHead.CompareNoCase(wzhKCC)) &&
			!strKey.CompareNoCase(wzhKeyCurrentConfig))
		{
			strRoot = CURRENT_CONFIGL;
			break;
		}

		hChild = m_Tree.GetNextSiblingItem(hChild);
	}

	if (hChild == NULL || strRoot.IsEmpty())
	{
		return;
	}

	// 首先展开次根键，类似HKEY_LOCAL_MACHINE，HKEY_USERS
	m_Tree.Expand(hChild, TVE_COLLAPSE);
	EnumSubKeys(strRoot, hChild);
	m_Tree.Expand(hChild, TVE_EXPAND);

	if (m_strComboText.Find(L"\\") != -1)
	{
		CString strKeyEnd = strKeyPath.GetBuffer() + strRoot.GetLength();
		strKeyPath.ReleaseBuffer();

		if (strKeyEnd.IsEmpty())
		{
		
			return;
		}

		strKeyEnd.TrimLeft('\\');

		if (strKeyEnd.IsEmpty())
		{
		
			return;
		}

		CString strTemp;
		BOOL bQuiet = FALSE;

		do 
		{
			strTemp = strKeyEnd.Left(strKeyEnd.Find('\\'));

			if (strTemp.IsEmpty())
			{
				strTemp = strKeyEnd;
				bQuiet = TRUE;
			}

			hChild = GetTreeSubItemByName(hChild, strTemp);
			if (hChild == NULL)
			{
				CString strMsgBox; 

				strMsgBox = L"该项不存在";
				strMsgBox += L" \'" + m_strComboText + L"\'"; 
				MessageBox(strMsgBox, L"Registry", MB_OK | MB_ICONINFORMATION);
				break;
			}

			strRoot += L"\\" + strTemp;

			if (bQuiet)
			{
				EnumValues(strRoot);
				m_Tree.Expand(hChild, TVE_EXPAND);
				m_Tree.Select(hChild, TVGN_FIRSTVISIBLE);
				m_Tree.SelectItem(hChild);
				m_Tree.SetItemState(hChild, TVIS_DROPHILITED | TVIS_BOLD, TVIS_DROPHILITED | TVIS_BOLD);
				//m_hChild = hChild;
			}
			else
			{
				EnumSubKeys(strRoot, hChild);
				m_Tree.Expand(hChild, TVE_EXPAND);
			}

			strKeyEnd = strKeyEnd.Right(strKeyEnd.GetLength() - strKeyEnd.Find('\\') - 1);

		} while (!bQuiet);
	}
}




CString CRegisterManagerDlg::ComboString2KeyPath()
{
	CString strHead = m_strComboText.Left(m_strComboText.Find('\\'));
	CString strRight = m_strComboText.Right(m_strComboText.GetLength() - m_strComboText.Find('\\'));
	CString strKeyPath;

	if (strHead.IsEmpty())
	{
		strHead = strRight;
		strRight.Empty();
	}

	WCHAR wzhKeyRoot[] = L"HKEY_CLASSES_ROOT";
	WCHAR wzhKCR[] = L"HKCR";

	WCHAR wzhKeyCurrentUser[] = L"HKEY_CURRENT_USER";
	WCHAR wzhKCU[] = L"HKCU";

	WCHAR wzhKeyLocalMachine[] = L"HKEY_LOCAL_MACHINE";
	WCHAR wzhKLM[] = L"HKLM";

	WCHAR wzhKeyUsers[] = L"HKEY_USERS";
	WCHAR wzhKU[] = L"HKU";;

	WCHAR wzhKeyCurrentConfig[] = L"HKEY_CURRENT_CONFIG";
	WCHAR wzhKCC[] = L"HKCC";;

	if (!strHead.CompareNoCase(wzhKeyRoot) || !strHead.CompareNoCase(wzhKCR))
	{
		strKeyPath = CLASSES_ROOT;
	}
	else if (!strHead.CompareNoCase(wzhKeyCurrentUser) || !strHead.CompareNoCase(wzhKCU))
	{
		strKeyPath = m_wzhKeyCurrentUser;
	}
	else if (!strHead.CompareNoCase(wzhKeyLocalMachine) || !strHead.CompareNoCase(wzhKLM))
	{
		strKeyPath = LOCAL_MACHINE;
	}
	else if (!strHead.CompareNoCase(wzhKeyUsers) || !strHead.CompareNoCase(wzhKU))
	{
		strKeyPath = USERS;
	}
	else if (!strHead.CompareNoCase(wzhKeyCurrentConfig) || !strHead.CompareNoCase(wzhKCC))
	{
		strKeyPath = CURRENT_CONFIGL;
	}

	strKeyPath += strRight;
	strKeyPath.TrimRight('\\');

	return strKeyPath;
}



HTREEITEM CRegisterManagerDlg::GetTreeSubItemByName(HTREEITEM hPatentItem, CString strName)
{
	HTREEITEM hChild = m_Tree.GetChildItem(hPatentItem);
	HTREEITEM hRetItem = NULL;

	while(hChild != NULL)
	{
		CString strKey = m_Tree.GetItemText(hChild);

		if (!strKey.CompareNoCase(strName))
		{
			hRetItem = hChild;
			break;
		}
		else if (!strKey.CompareNoCase(strName))
		{
			hRetItem = hChild;
			break;
		}
		else if (!strKey.CompareNoCase(strName))
		{
			hRetItem = hChild;
			break;
		}
		else if (!strKey.CompareNoCase(strName))
		{
			hRetItem = hChild;
			break;
		}
		else if (!strKey.CompareNoCase(strName))
		{
			hRetItem = hChild;
			break;
		}

		hChild = m_Tree.GetNextSiblingItem(hChild);
	}

	return hRetItem;
}


BOOL CRegisterManagerDlg::PreTranslateMessage(MSG* pMsg)
{
	if(pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_ESCAPE) return TRUE; 

	if(pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_RETURN) 
	{
		OnBnClickedGoto();
		return TRUE;
	}


	return CDialogEx::PreTranslateMessage(pMsg);
}



void CRegisterManagerDlg::OnNMRClickTree(NMHDR *pNMHDR, LRESULT *pResult)
{

	CPoint Point;

	GetCursorPos(&Point);
	m_Tree.ScreenToClient(&Point);

	m_RightClickTreeItem = m_Tree.HitTest(Point, NULL);
	CString strItemText;

	if ( m_RightClickTreeItem != NULL )
	{ 
		strItemText = m_Tree.GetItemText(m_RightClickTreeItem);
		m_Tree.SelectItem(m_RightClickTreeItem);
	}


	if (m_RightClickTreeItem == NULL ||
		!strItemText.CompareNoCase(L"我的电脑"))
	{

	}

	else
	{

		CMenu ExMenu;
		ExMenu.CreatePopupMenu();
		ExMenu.AppendMenu(MF_STRING, ID_STRING_VALUE,L"字符串值");
		ExMenu.AppendMenu(MF_STRING, ID_BINARY_VALUE,L"二进制值");
		ExMenu.AppendMenu(MF_STRING, ID_DWORD_VALUE, L"DWORD值");
		ExMenu.AppendMenu(MF_STRING, ID_QWORD_VALUE, L"QWORD值");
		ExMenu.AppendMenu(MF_STRING, ID_MUI_STRING_VALUE,L"多字符串值");
		ExMenu.AppendMenu(MF_STRING, ID_EXPAND_STRING_VALUE,L"可扩充字符串值");



	    CMenu Menu;
		Menu.CreatePopupMenu();
		Menu.AppendMenu(MF_STRING, ID_REG_REFRESH,L"刷新");
		Menu.AppendMenu(MF_SEPARATOR);
		Menu.AppendMenu(MF_STRING, ID_REG_LOOK_FOR,L"查找");
		Menu.AppendMenu(MF_SEPARATOR);
		Menu.AppendMenu(MF_STRING, ID_REG_NEW_KEY,L"新建项");
		Menu.AppendMenu(MF_POPUP, (UINT)ExMenu.m_hMenu,L"新建值");
		Menu.AppendMenu(MF_SEPARATOR);
		Menu.AppendMenu(MF_STRING, ID_REG_DELETE,L"删除");
		Menu.AppendMenu(MF_STRING, ID_REG_RENAME, L"重命名");

		Menu.AppendMenu(MF_SEPARATOR);
		Menu.AppendMenu(MF_STRING, ID_REG_INPORT,L"导入");
		Menu.AppendMenu(MF_STRING, ID_REG_EXPORT,L"导出");
		Menu.AppendMenu(MF_SEPARATOR);
	
		Menu.AppendMenu(MF_STRING, ID_REG_COPY_KEY_NAME,L"拷贝键名");
		Menu.AppendMenu(MF_STRING, ID_REG_COPY_FULL_KEY_NAME,L"拷贝键完整路径");
		Menu.AppendMenu(MF_SEPARATOR);
		Menu.AppendMenu(MF_STRING, ID_REG_ADD_TO_QUICK_ADDRESS,L"添加到快速定位");
		Menu.AppendMenu(MF_SEPARATOR);
		int x = GetSystemMetrics(SM_CXMENUCHECK);
		int y = GetSystemMetrics(SM_CYMENUCHECK);


		if (x >= 15 && y >= 15)
		{
			Menu.SetMenuItemBitmaps(ID_REG_REFRESH, MF_BYCOMMAND, &m_bmRefresh, &m_bmRefresh);
			Menu.SetMenuItemBitmaps(ID_REG_LOOK_FOR, MF_BYCOMMAND, &m_bmLookfor, &m_bmLookfor);
			Menu.SetMenuItemBitmaps(ID_REG_DELETE, MF_BYCOMMAND, &m_bmDelete, &m_bmDelete);

			Menu.SetMenuItemBitmaps(ID_REG_COPY_KEY_NAME, MF_BYCOMMAND, &m_bmCopy, &m_bmCopy);
			Menu.SetMenuItemBitmaps(ID_REG_COPY_FULL_KEY_NAME, MF_BYCOMMAND, &m_bmCopy, &m_bmCopy);


			Menu.SetMenuItemBitmaps(ID_REG_INPORT, MF_BYCOMMAND, &m_bmExport, &m_bmExport);
			Menu.SetMenuItemBitmaps(ID_REG_EXPORT, MF_BYCOMMAND, &m_bmExport, &m_bmExport);
		}


		if (!strItemText.CompareNoCase(L"HKEY_CLASSES_ROOT") ||
			!strItemText.CompareNoCase(L"HKEY_CURRENT_USER") ||
			!strItemText.CompareNoCase(L"HKEY_LOCAL_MACHINE") ||
			!strItemText.CompareNoCase(L"HKEY_USERS") ||
			!strItemText.CompareNoCase(L"HKEY_CURRENT_CONFIG") )
		{
			Menu.EnableMenuItem(ID_REG_DELETE, MF_BYCOMMAND | MF_GRAYED | MF_DISABLED);
			Menu.EnableMenuItem(ID_REG_RENAME, MF_BYCOMMAND | MF_GRAYED | MF_DISABLED);
		}

		CPoint Point;
		GetCursorPos(&Point);
		Menu.TrackPopupMenu(TPM_RIGHTBUTTON, Point.x, Point.y, this);
		Menu.DestroyMenu();
		ExMenu.DestroyMenu();
	}



	*pResult = 0;
}


void CRegisterManagerDlg::OnRegRefresh()
{
	CString  strItemText;

	if ( m_RightClickTreeItem != NULL )
	{ 
		strItemText = m_Tree.GetItemText(m_RightClickTreeItem);
	}

	// 如果是computer这个根键，那么就直接初始化注册表
	if (m_RightClickTreeItem == NULL ||
		!strItemText.CompareNoCase(L"我的电脑"))
	{
		InitRegistry();
	}
	else
	{
		// 当是打开状态才刷新
		if(m_RightClickTreeItem != NULL && 
			(TVIS_EXPANDED & m_Tree.GetItemState(m_RightClickTreeItem, TVIS_EXPANDED)))
		{
			m_Tree.Expand(m_RightClickTreeItem, TVE_COLLAPSE);
			DeleteSubTree(m_RightClickTreeItem);

			HTREEITEM hItemTemp = m_RightClickTreeItem;
			CString strKeyPath, strKeyPathTemp;

			while (1)
			{
				CString strText = m_Tree.GetItemText(hItemTemp);

				if (!strText.CompareNoCase(L"HKEY_CLASSES_ROOT"))
				{
					strKeyPath = CLASSES_ROOT;
					break;
				}
				else if (!strText.CompareNoCase(L"HKEY_CURRENT_USER"))
				{
					strKeyPath = m_wzhKeyCurrentUser;
					break;
				}
				else if (!strText.CompareNoCase(L"HKEY_LOCAL_MACHINE"))
				{
					strKeyPath = LOCAL_MACHINE;
					break;
				}
				else if (!strText.CompareNoCase(L"HKEY_USERS"))
				{
					strKeyPath = USERS;
					break;
				}
				else if (!strText.CompareNoCase(L"HKEY_CURRENT_CONFIG"))
				{
					strKeyPath = CURRENT_CONFIGL;
					break;
				}
				else
				{
					strKeyPathTemp = strText + L"\\" + strKeyPathTemp;
				}

				hItemTemp = m_Tree.GetParentItem(hItemTemp);
			}

			strKeyPath = strKeyPath + L"\\" + strKeyPathTemp;
			EnumSubKeys(strKeyPath, m_RightClickTreeItem);
			EnumSubSubKeys(strKeyPath, m_RightClickTreeItem);

			m_Tree.Expand(m_RightClickTreeItem, TVE_EXPAND);
		}
	}
}





void CRegisterManagerDlg::OnRegNewKey()
{
	CString strKeyPath = String2KeyPath();
	if (strKeyPath.IsEmpty())
	{
		return;
	}

	CKeyDlg CreateKeyDlg;   //这个Dlg 会在新建和重命名中 被创建 所以定义一个DlgType
	CString strNewKey;

	CreateKeyDlg.m_nDlgType = enumCreateKey;
	if (CreateKeyDlg.DoModal() == IDOK)
	{
		strNewKey = CreateKeyDlg.m_strKeyNameEdit;
	}

	if (strNewKey.IsEmpty())
	{
		return;
	}

	CString strKeyPathTemp = strKeyPath + L"\\";
	strKeyPathTemp += strNewKey;

	UNICODE_STRING uniKey;
	if (InitUnicodeString(&uniKey, strKeyPathTemp.GetBuffer()))
	{
		HANDLE hKey;
		OBJECT_ATTRIBUTES oa;
		ULONG Disposition = 0;

		InitializeObjectAttributes(&oa, &uniKey, OBJ_CASE_INSENSITIVE, 0, NULL);

		if (CreateKey(&hKey, KEY_ALL_ACCESS, &oa, 0, NULL, 0, &Disposition))
		{
			if (Disposition == REG_OPENED_EXISTING_KEY)
			{
				MessageBox(L"该键已经存在",L"Shine", MB_ICONERROR | MB_OK);
			}
			else
			{
			
				HTREEITEM hItem = m_Tree.GetSelectedItem();
				m_Tree.InsertItem(strNewKey, 1, 2, hItem, TVI_LAST);
			}

			CloseHandle(hKey);
		}

		FreeUnicodeString(&uniKey);
	}
}




CString CRegisterManagerDlg::String2KeyPath()
{
	CString strHead = m_strComboText.Left(m_strComboText.Find('\\'));
	CString strRight = m_strComboText.Right(m_strComboText.GetLength() - m_strComboText.Find('\\'));
	CString strKeyPath;

	if (strHead.IsEmpty())
	{
		strHead = strRight;
		strRight.Empty();
	}

	if (!strHead.CompareNoCase(L"HKEY_CLASSES_ROOT"))
	{
		strKeyPath = CLASSES_ROOT;
	}
	else if (!strHead.CompareNoCase(L"HKEY_CURRENT_USER"))
	{
		strKeyPath = m_wzhKeyCurrentUser;
	}
	else if (!strHead.CompareNoCase(L"HKEY_LOCAL_MACHINE"))
	{
		strKeyPath = LOCAL_MACHINE;
	}
	else if (!strHead.CompareNoCase(L"HKEY_USERS"))
	{
		strKeyPath = USERS;
	}
	else if (!strHead.CompareNoCase(L"HKEY_CURRENT_CONFIG"))
	{
		strKeyPath = CURRENT_CONFIGL;
	}

	strKeyPath += strRight;

	return strKeyPath;
}



BOOL CRegisterManagerDlg::CreateKey(
	OUT PHANDLE  KeyHandle,
	IN ACCESS_MASK  DesiredAccess,
	IN POBJECT_ATTRIBUTES  ObjectAttributes,
	IN ULONG  ulTitleIndex,
	IN PUNICODE_STRING  uniClass,
	IN ULONG  ulCreateOptions,
	OUT PULONG  Disposition
	)

{

	DWORD dwReturnSize = 0;
	DWORD dwRet = 0;

	struct{
		ACCESS_MASK DesiredAccess;
		POBJECT_ATTRIBUTES ObjectAttributes;
	}Create;

	memset(&Create, 0, sizeof(Create));
	Create.DesiredAccess = DesiredAccess;
	Create.ObjectAttributes = ObjectAttributes;

	

	typedef struct _CREATE_VALUE_
	{
		PHANDLE KeyHandle;
		PULONG  Disposition;
	}CREATE_VALUE, *PCREATE_VALUE;


	CREATE_VALUE CreateValue;
	memset(&CreateValue, 0, sizeof(CREATE_VALUE));
	

	CreateValue.KeyHandle = KeyHandle;
	CreateValue.Disposition = Disposition;

	g_hDevice = OpenDevice(L"\\\\.\\RegisterManagerLink");


	if (g_hDevice==(HANDLE)-1)
	{
		return FALSE;
	}

	

	dwRet = DeviceIoControl(g_hDevice,CTL_CREATE_KEY,
		&Create,
		sizeof(Create),
		&CreateValue,
		sizeof(CREATE_VALUE),
		&dwReturnSize,
		NULL);


	if (dwRet==0)
	{

		CloseHandle(g_hDevice);
		return FALSE;
	}

	CloseHandle(g_hDevice);

	return TRUE;
}


void CRegisterManagerDlg::OnStringValue()
{
	SetValueKey(REG_SZ,NULL,0);
}




void CRegisterManagerDlg::SetValueKey(ULONG ulType, PVOID Data, ULONG ulDataSize)
{
	CString strKeyPath = String2KeyPath();
	if (strKeyPath.IsEmpty())
	{
		return;
	}

	CKeyDlg SetValueKeyDlg;
	CString strValue;

	SetValueKeyDlg.m_nDlgType = enumSetValueKey;
	if (SetValueKeyDlg.DoModal() == IDOK)
	{
		strValue = SetValueKeyDlg.m_strKeyNameEdit;
	}

	if (strValue.IsEmpty())
	{
		return;
	}

	UNICODE_STRING uniKey;
	if (InitUnicodeString(&uniKey, strKeyPath.GetBuffer()))
	{
		HANDLE hKey;
		OBJECT_ATTRIBUTES oa;

		InitializeObjectAttributes(&oa, &uniKey, OBJ_CASE_INSENSITIVE, 0, NULL);

		if (OpenKey(&hKey, KEY_ALL_ACCESS, &oa))
		{
			UNICODE_STRING uniValue;

			if (InitUnicodeString(&uniValue, strValue.GetBuffer()))
			{
				SetValueKey(hKey, &uniValue, 0, ulType, Data, ulDataSize);
				FreeUnicodeString(&uniValue);
			}

			CloseHandle(hKey);
		}

		FreeUnicodeString(&uniKey);
	}

	OnRegListRefresh();   //刷新ControlList
}




BOOL CRegisterManagerDlg::SetValueKey(
	IN HANDLE  KeyHandle,
	IN PUNICODE_STRING  uniValueName,
	IN ULONG  ulTitleIndex,
	IN ULONG  ulType,
	IN PVOID  Data,
	IN ULONG  ulDataSize
	)
{



	DWORD dwReturnSize = 0;
	DWORD dwRet = 0;

	struct{
		HANDLE hKey;
		PUNICODE_STRING ValueName;
		ULONG Type;
		PVOID Data;
		ULONG DataSize;
	}Set;

	memset(&Set, 0, sizeof(Set));

	Set.hKey = KeyHandle;
	Set.ValueName = uniValueName;
	Set.Type = ulType;
	Set.Data = Data;
	Set.DataSize = ulDataSize;

	g_hDevice = OpenDevice(L"\\\\.\\RegisterManagerLink");


	if (g_hDevice==(HANDLE)-1)
	{
		return FALSE;
	}



	dwRet = DeviceIoControl(g_hDevice,CTL_SET_KEY_VALUE,
		&Set,
		sizeof(Set),
		NULL,0,
		&dwReturnSize,
		NULL);


	if (dwRet==0)
	{

		CloseHandle(g_hDevice);
		return FALSE;
	}

	CloseHandle(g_hDevice);

	return TRUE;
}



void CRegisterManagerDlg::OnRegListRefresh()
{
	CString strKeyPath = String2KeyPath();
	EnumValues(strKeyPath);
}

void CRegisterManagerDlg::OnBinaryValue()
{
	SetValueKey(REG_BINARY,NULL,0);
}


void CRegisterManagerDlg::OnDwordValue()
{
	DWORD dwValue = 0;
	SetValueKey(REG_DWORD,&dwValue,sizeof(DWORD));
}


void CRegisterManagerDlg::OnQwordValue()
{
	QWORD dwValue = 0;
	SetValueKey(REG_QWORD,&dwValue,sizeof(QWORD));
}

void CRegisterManagerDlg::OnMuiStringValue()
{
	SetValueKey(REG_MULTI_SZ,NULL,0);
}


void CRegisterManagerDlg::OnExpandStringValue()
{
	SetValueKey(REG_EXPAND_SZ,NULL,0);
}



void CRegisterManagerDlg::OnRegDelete()
{
	if (m_RightClickTreeItem != NULL)
	{
		if (MessageBox(L"确定删除该项吗？",L"Shine", MB_YESNO | MB_ICONQUESTION) != IDYES)
		{
			return;
		}

		HTREEITEM hItemTemp = m_RightClickTreeItem;
		CString strKeyPath, strKeyPathTemp;

		while (1)
		{
			CString strText = m_Tree.GetItemText(hItemTemp);

			if (!strText.CompareNoCase(L"HKEY_CLASSES_ROOT"))
			{
				strKeyPath = CLASSES_ROOT;
				break;
			}
			else if (!strText.CompareNoCase(L"HKEY_CURRENT_USER"))
			{
				strKeyPath = m_wzhKeyCurrentUser;
				break;
			}
			else if (!strText.CompareNoCase(L"HKEY_LOCAL_MACHINE"))
			{
				strKeyPath = LOCAL_MACHINE;
				break;
			}
			else if (!strText.CompareNoCase(L"HKEY_USERS"))
			{
				strKeyPath = USERS;
				break;
			}
			else if (!strText.CompareNoCase(L"HKEY_CURRENT_CONFIG"))
			{
				strKeyPath = CURRENT_CONFIGL;
				break;
			}
			else
			{
				strKeyPathTemp = strText + L"\\" + strKeyPathTemp;
			}

			hItemTemp = m_Tree.GetParentItem(hItemTemp);
		}

		strKeyPath = strKeyPath + L"\\" + strKeyPathTemp;
		strKeyPath.TrimRight('\\');

		m_Tree.Expand(m_RightClickTreeItem, TVE_COLLAPSE);
		DeleteKeyAndSubKeys(strKeyPath);
		UpdateKey(strKeyPath, m_RightClickTreeItem);   //刷新Tree控件
	}
}



void CRegisterManagerDlg::DeleteKeyAndSubKeys(CString strKey)
{
	if (strKey.IsEmpty())
	{
		return;
	}

	UNICODE_STRING uniKey;
	BOOL bQuit = FALSE;
	if (InitUnicodeString(&uniKey, strKey.GetBuffer()))
	{
		strKey.ReleaseBuffer();

		HANDLE hKey;
		OBJECT_ATTRIBUTES oa;

		InitializeObjectAttributes(&oa, &uniKey, OBJ_CASE_INSENSITIVE, 0, NULL);

		if (OpenKey(&hKey, KEY_ALL_ACCESS, &oa))
		{
			ULONG i = 0;
			while (1)
			{
				if (bQuit)
				{
					break;
				}

				ULONG ulRetLen = 0;
				BOOL bRet = EnumerateKey(hKey, i, KeyBasicInformation, NULL, 0, &ulRetLen);
				if (!bRet && GetLastError() == ERROR_NO_MORE_ITEMS) // STATUS_NO_MORE_ENTRIES
				{
					if (!DeleteKey(hKey) )
					{
						
						bQuit = TRUE;
					}

					break;
				}
				else if (!bRet && GetLastError() == ERROR_INSUFFICIENT_BUFFER) // STATUS_BUFFER_TOO_SMALL
				{
					PKEY_BASIC_INFORMATION Buffer = (PKEY_BASIC_INFORMATION)malloc(ulRetLen + 100);

					if (Buffer)
					{
						memset(Buffer, 0, ulRetLen + 100);
						bRet = EnumerateKey(hKey, i, KeyBasicInformation, Buffer, ulRetLen + 100, &ulRetLen);

						if (bRet && Buffer->NameLength)
						{
							WCHAR wzName[1024] = {0};
							wcsncpy_s(wzName, 1024, Buffer->Name, Buffer->NameLength / sizeof(WCHAR));
							CString strKeyTemp = strKey + L"\\" + wzName;
							DeleteKeyAndSubKeys(strKeyTemp);
						}
						else
						{
							break;
						}

						free(Buffer);
					}
				}
			}

			CloseHandle(hKey);
		}

		FreeUnicodeString(&uniKey);
	}
}


BOOL CRegisterManagerDlg::DeleteKey(IN HANDLE KeyHandle)
{
	DWORD dwReturnSize = 0;
	DWORD dwRet = 0;
	struct   
	{
		HANDLE  hKey;
	}Delete;
	
	memset(&Delete, 0, sizeof(Delete));
	Delete.hKey = KeyHandle;



	g_hDevice = OpenDevice(L"\\\\.\\RegisterManagerLink");


	if (g_hDevice==(HANDLE)-1)
	{
		return FALSE;
	}



	dwRet = DeviceIoControl(g_hDevice,CTL_DELETE_KEY,
		&Delete,
		sizeof(Delete),
		NULL,0,
		&dwReturnSize,
		NULL);


	if (dwRet==0)
	{

		CloseHandle(g_hDevice);
		return FALSE;
	}

	CloseHandle(g_hDevice);

	return TRUE;
}



void CRegisterManagerDlg::UpdateKey(CString strKey, HTREEITEM hItem)
{
	if ( !strKey.IsEmpty() && hItem != NULL )
	{
		DeleteSubTree(hItem);

		UNICODE_STRING uniKey;

		if (InitUnicodeString(&uniKey, strKey.GetBuffer()))
		{
			HANDLE hKey;
			OBJECT_ATTRIBUTES oa;

			InitializeObjectAttributes(&oa, &uniKey, OBJ_CASE_INSENSITIVE, 0, NULL);

			if (OpenKey(&hKey, KEY_ALL_ACCESS, &oa))
			{
				for (ULONG i = 0; ; i++)
				{
					ULONG ulRetLen = 0;
					BOOL bRet = EnumerateKey(hKey, i, KeyBasicInformation, NULL, 0, &ulRetLen);

					if (!bRet && GetLastError() == ERROR_NO_MORE_ITEMS) 
					{
						break;
					}
					else if (!bRet && GetLastError() == ERROR_INSUFFICIENT_BUFFER)
					{
						PKEY_BASIC_INFORMATION Buffer = (PKEY_BASIC_INFORMATION)malloc(ulRetLen + 0x100);
						if (Buffer)
						{
							memset(Buffer, 0, ulRetLen + 0x100);
							bRet = EnumerateKey(hKey, i, KeyBasicInformation, Buffer, ulRetLen + 0x100, &ulRetLen);

							if (bRet)
							{
								m_Tree.InsertItem(Buffer->Name, 1, 2, hItem, TVI_LAST);
							}

							free(Buffer);
						}
					}
				}

				CloseHandle(hKey);
			}
			else
			{
				m_Tree.DeleteItem(hItem);
			}

			FreeUnicodeString(&uniKey);
		}
	}
}

void CRegisterManagerDlg::OnRegRename()
{
	CString strKeyPath = String2KeyPath();
	if (strKeyPath.IsEmpty())
	{
		return;
	}

	CString strKeyName = m_Tree.GetItemText(m_RightClickTreeItem);
	CKeyDlg RenameKeyDlg;
	RenameKeyDlg.m_nDlgType = enumRenameKey;
	RenameKeyDlg.m_strKeyNameEdit = strKeyName;
	if (RenameKeyDlg.DoModal() == IDOK)
	{
		CString strNewKeyName = RenameKeyDlg.m_strKeyNameEdit;
		if (strNewKeyName.CompareNoCase(strKeyName))   //如果两个名字不相等
		{
			UNICODE_STRING uniKey;
			if (strNewKeyName.GetBuffer()!=NULL)
			{
				HANDLE hKey;
				OBJECT_ATTRIBUTES oa;
				HTREEITEM hItemTemp = m_RightClickTreeItem;
				CString strKeyPath, strKeyPathTemp;
				while (1)
				{
					CString strText = m_Tree.GetItemText(hItemTemp);

					if (!strText.CompareNoCase(L"HKEY_CLASSES_ROOT"))
					{
						strKeyPath = CLASSES_ROOT;
						break;
					}
					else if (!strText.CompareNoCase(L"HKEY_CURRENT_USER"))
					{
						strKeyPath = m_wzhKeyCurrentUser;
						break;
					}
					else if (!strText.CompareNoCase(L"HKEY_LOCAL_MACHINE"))
					{
						strKeyPath = LOCAL_MACHINE;
						break;
					}
					else if (!strText.CompareNoCase(L"HKEY_USERS"))
					{
						strKeyPath = USERS;
						break;
					}
					else if (!strText.CompareNoCase(L"HKEY_CURRENT_CONFIG"))
					{
						strKeyPath = CURRENT_CONFIGL;
						break;
					}
					else
					{
						strKeyPathTemp = strText + L"\\" + strKeyPathTemp;
					}

					hItemTemp = m_Tree.GetParentItem(hItemTemp);
				}

				strKeyPath = strKeyPath + L"\\" + strKeyPathTemp;

				InitUnicodeString(&uniKey, strKeyPath.GetBuffer());

				InitializeObjectAttributes(&oa, &uniKey, OBJ_CASE_INSENSITIVE, 0, NULL);

				if (OpenKey(&hKey, KEY_ALL_ACCESS, &oa))
				{
					UNICODE_STRING uniNewKey;
					if (InitUnicodeString(&uniNewKey,strNewKeyName.GetBuffer()))
					{
						if (RenameKey(hKey, &uniNewKey))
						{
							m_Tree.SetItemText(m_RightClickTreeItem, strNewKeyName);  //在树控件上修改成新的名字


							//修正ComboBox数据
							CString strTemp = m_strComboText.Left(m_strComboText.ReverseFind('\\'));

							strTemp+=L"\\";

							strTemp+=strNewKeyName;


							m_ComboBox.SetWindowText(strTemp);


						}

						FreeUnicodeString(&uniNewKey);
					}

					CloseHandle(hKey);
				}

				FreeUnicodeString(&uniKey);
			}
		}
	}
}


BOOL CRegisterManagerDlg::RenameKey(
	IN HANDLE  KeyHandle,
	IN PUNICODE_STRING  uniNewName
	)
{
	DWORD dwReturnSize = 0;
	DWORD dwRet = 0;

	struct
	{
		HANDLE hKey;
		PUNICODE_STRING  uniNewName;
	}Rename;

	memset(&Rename, 0, sizeof(Rename));
	
	Rename.hKey = KeyHandle;
	Rename.uniNewName = uniNewName;

	g_hDevice = OpenDevice(L"\\\\.\\RegisterManagerLink");


	if (g_hDevice==(HANDLE)-1)
	{
		return FALSE;
	}



	dwRet = DeviceIoControl(g_hDevice,CTL_RENAME_KEY,
		&Rename,
		sizeof(Rename),
		NULL,0,
		&dwReturnSize,
		NULL);


	if (dwRet==0)
	{

		CloseHandle(g_hDevice);
		return FALSE;
	}

	CloseHandle(g_hDevice);

	return TRUE;
}



void CRegisterManagerDlg::OnRegCopyKeyName()
{
	UpdateData(TRUE);
	CString strKeyName = m_strComboText.Right(m_strComboText.GetLength() - m_strComboText.ReverseFind('\\') - 1);
	if (strKeyName.IsEmpty())
	{
		strKeyName = m_strComboText;
	}

	SetStringToClipboard(strKeyName);
}


void CRegisterManagerDlg::OnRegCopyFullKeyName()
{
	UpdateData(TRUE);
	SetStringToClipboard(m_strComboText);
}



BOOL CRegisterManagerDlg::SetStringToClipboard(CString strImageName)
{
	if (strImageName.IsEmpty())
	{
		return TRUE;
	}

	BOOL bRet = FALSE;

	if( OpenClipboard() )
	{
		HGLOBAL hClipBuffer = 0;
		WCHAR* wzBuffer = NULL;

		EmptyClipboard();
		hClipBuffer = LocalAlloc(GMEM_ZEROINIT, (strImageName.GetLength() + 1) * sizeof(WCHAR));
		if (hClipBuffer)
		{
			wzBuffer = (WCHAR*)GlobalLock(hClipBuffer);
			if (wzBuffer)
			{
				wcsncpy_s(wzBuffer, strImageName.GetLength() + 1, strImageName.GetBuffer(), strImageName.GetLength());
				strImageName.ReleaseBuffer();
				SetClipboardData(CF_UNICODETEXT, hClipBuffer);
				GlobalUnlock(hClipBuffer);

				bRet = TRUE;
			}
		}

		CloseClipboard();
	}

	return bRet;
}


void CRegisterManagerDlg::OnRegAddToQuickAddress()
{
	UpdateData(TRUE);
	if (!m_strComboText.IsEmpty())
	{
		m_RegPathList.push_back(m_strComboText);
		m_nComboBoxCnt++;

		UpdateData(FALSE);
	}
}


void CRegisterManagerDlg::OnRegInport()
{
	CFileDialog FileDlg(TRUE);			
	FileDlg.m_ofn.lpstrTitle = L"Open Register File";
	FileDlg.m_ofn.lpstrFilter = L"Registration Files(*.reg)\0*.reg\0\0";

	if (IDOK == FileDlg.DoModal())
	{
		CString strPath = FileDlg.GetPathName();
		ShellImportRegister(strPath);

		MessageBox(L"导入成功",L"Shine", MB_OK | MB_ICONINFORMATION);

		OnRegRefresh();
	}	
}


VOID CRegisterManagerDlg::ShellImportRegister(
	LPCTSTR ImportFile)    //导入的注册表文件
{
	CString strItem(ImportFile);
	CString strParameters;
	strParameters = L"/s \"" + strItem + L"\"";
	ShellExecute(NULL, L"open", L"regedit.exe",
		strParameters, NULL, SW_HIDE);

}

void CRegisterManagerDlg::OnRegExport()
{
	CString strItemText;

	if ( m_RightClickTreeItem != NULL )
	{ 
		strItemText = m_Tree.GetItemText(m_RightClickTreeItem);

		if (strItemText.CompareNoCase(L"我的电脑"))
		{
			HTREEITEM hItemTemp = m_RightClickTreeItem;
			CString strKeyPath, strKeyPathTemp;

			while (1)
			{
				CString strText = m_Tree.GetItemText(hItemTemp);

				if (!strText.CompareNoCase(L"HKEY_CLASSES_ROOT"))
				{
					strKeyPath = L"HKEY_CLASSES_ROOT";
					break;
				}
				else if (!strText.CompareNoCase(L"HKEY_CURRENT_USER"))
				{
					strKeyPath = m_wzhKeyCurrentUser;
					break;
				}
				else if (!strText.CompareNoCase(L"HKEY_LOCAL_MACHINE"))
				{
					strKeyPath = L"HKEY_LOCAL_MACHINE";
					break;
				}
				else if (!strText.CompareNoCase(L"HKEY_USERS"))
				{
					strKeyPath = L"HKEY_USERS";
					break;
				}
				else if (!strText.CompareNoCase(L"HKEY_CURRENT_CONFIG"))
				{
					strKeyPath = L"HKEY_CURRENT_CONFIG";
					break;
				}
				else
				{
					strKeyPathTemp = strText + L"\\" + strKeyPathTemp;
				}

				hItemTemp = m_Tree.GetParentItem(hItemTemp);
			}

			strKeyPath = strKeyPath + L"\\" + strKeyPathTemp;
			strKeyPath.TrimRight('\\');

			CString strFileName = strKeyPath.Right(strKeyPath.GetLength() - strKeyPath.ReverseFind(L'\\') - 1);
			CFileDialog FileDlg( FALSE, 0, strFileName, 0, L"Register Files (*.reg)|*.reg;|All Files (*.*)|*.*||", 0 );
			if (IDOK == FileDlg.DoModal())
			{
				CString strFilePath = FileDlg.GetFileName();
				CString strExtern = strFilePath.Right(4);

			

				if (strExtern.CompareNoCase(L".reg"))
				{
					strFilePath += L".reg";
				}

				if ( !PathFileExists(strFilePath) ||
					(PathFileExists(strFilePath) && 
					MessageBox(L"文件已经存在",L"Shine", MB_YESNO | MB_ICONQUESTION) == IDYES &&
					DeleteFile(strFilePath)))
				{
					ShellExportRegister(strKeyPath, strFilePath);

					MessageBox(L"导出成功",L"Shine", MB_OK | MB_ICONINFORMATION);
				}
			}
		}
	}
}


VOID CRegisterManagerDlg::ShellExportRegister(
	CString strItem,		
	CString strFileName)	
{
	CString strParameters = L"/e \"" + strFileName + L"\" \"" + strItem + L"\"";
	ShellExecute(0, L"open", L"regedit.exe", strParameters, NULL, SW_SHOWNORMAL);
}


void CRegisterManagerDlg::OnRegLookFor()
{
	m_FindRegDlg = new CRegFindDlg();
	m_FindRegDlg->m_strSearchInKey = m_strComboText;
	m_FindRegDlg->Create(IDD_FIND_REG_DIALOG);
	m_FindRegDlg->ShowWindow(SW_SHOWNORMAL);
}



void CRegisterManagerDlg::JmpToReg(CString strKey, CString strData)
{
	if (strKey.IsEmpty())
	{
		return;
	}

	JmpToRegCommon(strKey, strData);
}


void CRegisterManagerDlg::JmpToRegCommon(CString strKey, CString strData)
{
	HTREEITEM hRootItem = m_Tree.GetRootItem();
	if (hRootItem == NULL)
	{
		return;
	}

	HTREEITEM hChild = m_Tree.GetChildItem(hRootItem);
	CString strRoot;

	while(hChild != NULL)
	{
		CString strKeyTemp = m_Tree.GetItemText(hChild);

		if (!_wcsnicmp(strKey.GetBuffer(), L"HKEY_CLASSES_ROOT", wcslen(L"HKEY_CLASSES_ROOT")) &&
			!strKeyTemp.CompareNoCase(L"HKEY_CLASSES_ROOT"))
		{
			strRoot = CLASSES_ROOT;
			break;
		}
		else if (!_wcsnicmp(strKey.GetBuffer(), L"HKEY_CURRENT_USER", wcslen(L"HKEY_CURRENT_USER")) &&
			!strKeyTemp.CompareNoCase(L"HKEY_CURRENT_USER"))
		{
			strRoot = m_wzhKeyCurrentUser;
			break;
		}
		else if (!_wcsnicmp(strKey.GetBuffer(), L"HKEY_LOCAL_MACHINE", wcslen(L"HKEY_LOCAL_MACHINE")) &&
			!strKeyTemp.CompareNoCase(L"HKEY_LOCAL_MACHINE"))
		{
			strRoot = LOCAL_MACHINE;
			break;
		}
		else if (!_wcsnicmp(strKey.GetBuffer(), L"HKEY_USERS", wcslen(L"HKEY_USERS")) &&
			!strKeyTemp.CompareNoCase(L"HKEY_USERS"))
		{
			strRoot = USERS;
			break;
		}
		else if (!_wcsnicmp(strKey.GetBuffer(), L"HKEY_CURRENT_CONFIG", wcslen(L"HKEY_CURRENT_CONFIG")) &&
			!strKeyTemp.CompareNoCase(L"HKEY_CURRENT_CONFIG"))
		{
			strRoot = CURRENT_CONFIGL;
			break;
		}

		hChild = m_Tree.GetNextSiblingItem(hChild);
	}

	if (hChild == NULL || strRoot.IsEmpty())
	{
		return;
	}

	m_Tree.Expand(hChild, TVE_COLLAPSE);
	EnumSubKeys(strRoot, hChild);
	m_Tree.Expand(hChild, TVE_EXPAND);

	if (strKey.Find(L"\\") != -1)
	{
		CString strKeyEnd = strKey.Right(strKey.GetLength() - strKey.Find('\\') - 1);

		if (strKeyEnd.IsEmpty())
		{
			return;
		}

		strKeyEnd.TrimLeft('\\');

		if (strKeyEnd.IsEmpty())
		{
			return;
		}

		CString strTemp;
		BOOL bQuiet = FALSE;

		do 
		{
			strTemp = strKeyEnd.Left(strKeyEnd.Find('\\'));

			if (strTemp.IsEmpty())
			{
				strTemp = strKeyEnd;
				bQuiet = TRUE;
			}

			hChild = GetTreeSubItemByName(hChild, strTemp);
			if (hChild == NULL)
			{
				CString strMsgBox; 

				strMsgBox = L"不存在";
				strMsgBox += L" \'" + strKey + L"\'"; 
				MessageBox(strMsgBox, L"Shine", MB_OK | MB_ICONINFORMATION);
				break;
			}

			strRoot += L"\\" + strTemp;

			if (bQuiet)
			{
				EnumValues(strRoot);
				m_Tree.Expand(hChild, TVE_EXPAND);
				m_Tree.Select(hChild, TVGN_FIRSTVISIBLE);
				m_Tree.SelectItem(hChild);
				m_Tree.SetItemState(hChild, TVIS_DROPHILITED | TVIS_BOLD, TVIS_DROPHILITED | TVIS_BOLD);
				
				if (!strData.IsEmpty())
				{
					DWORD dwCnt = m_List.GetItemCount();
					for (DWORD i = 0; i < dwCnt; i++)
					{
						if (!(m_List.GetItemText(i, 0)).CompareNoCase(strData))
						{
							m_List.EnsureVisible(i, false);
							m_List.SetItemState(i, LVIS_FOCUSED | LVIS_SELECTED,LVIS_FOCUSED | LVIS_SELECTED);
							m_List.SetFocus();
							break;
						}
					}
				}	

				break;
			}
			else
			{
				EnumSubKeys(strRoot, hChild);
				m_Tree.Expand(hChild, TVE_EXPAND);
			}

			strKeyEnd = strKeyEnd.Right(strKeyEnd.GetLength() - strKeyEnd.Find('\\') - 1);

		} while (!bQuiet);
	}
}

void CRegisterManagerDlg::OnNMRClickList(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	
	if (!m_strComboText.IsEmpty())
	{
	

		CMenu Menu;
		Menu.CreatePopupMenu();
		Menu.AppendMenu(MF_STRING, ID_REG_LIST_REFRESH,L"刷新");
	
		Menu.AppendMenu(MF_SEPARATOR);
		Menu.AppendMenu(MF_STRING, ID_REG_LIST_MODIFY,L"修改");
		Menu.AppendMenu(MF_STRING, ID_REG_LIST_DELETE, L"删除");
		Menu.AppendMenu(MF_STRING, ID_REG_LIST_RENAME, L"重命名");
		Menu.AppendMenu(MF_SEPARATOR);
		Menu.AppendMenu(MF_STRING, ID_REG_LIST_COPY_VALUE, L"复制值名称");
		Menu.AppendMenu(MF_STRING, ID_REG_LIST_COPY_VALUE_DATA,L"复制值数据");

		Menu.AppendMenu(MF_SEPARATOR);


		int x = GetSystemMetrics(SM_CXMENUCHECK);
		int y = GetSystemMetrics(SM_CYMENUCHECK);
		if (x >= 15 && y >= 15)
		{
			Menu.SetMenuItemBitmaps(ID_REG_LIST_REFRESH, MF_BYCOMMAND, &m_bmRefresh, &m_bmRefresh);
			Menu.SetMenuItemBitmaps(ID_REG_LIST_DELETE, MF_BYCOMMAND, &m_bmDelete, &m_bmDelete);
			Menu.SetMenuItemBitmaps(ID_REG_LIST_COPY_VALUE, MF_BYCOMMAND, &m_bmCopy, &m_bmCopy);
			Menu.SetMenuItemBitmaps(ID_REG_LIST_COPY_VALUE_DATA, MF_BYCOMMAND, &m_bmCopy, &m_bmCopy);
		}

		if (m_List.GetSelectedCount() == 0)
		{
			Menu.EnableMenuItem(ID_REG_LIST_MODIFY, MF_BYCOMMAND | MF_GRAYED | MF_DISABLED);
			Menu.EnableMenuItem(ID_REG_LIST_DELETE, MF_BYCOMMAND | MF_GRAYED | MF_DISABLED);
			Menu.EnableMenuItem(ID_REG_LIST_RENAME, MF_BYCOMMAND | MF_GRAYED | MF_DISABLED);
			Menu.EnableMenuItem(ID_REG_LIST_COPY_VALUE, MF_BYCOMMAND | MF_GRAYED | MF_DISABLED);
			Menu.EnableMenuItem(ID_REG_LIST_COPY_VALUE_DATA, MF_BYCOMMAND | MF_GRAYED | MF_DISABLED);
		}
		else if (m_List.GetSelectedCount() == 1)
		{
			int nItem = m_List.GetSelectionMark();
			if (nItem != -1)
			{
				if (m_List.GetItemData(nItem))
				{
					Menu.EnableMenuItem(ID_REG_LIST_RENAME, MF_BYCOMMAND | MF_GRAYED | MF_DISABLED);

					if (!(m_List.GetItemText(nItem, 2)).CompareNoCase(L"(数值未设置)"))
					{
						Menu.EnableMenuItem(ID_REG_LIST_DELETE, MF_BYCOMMAND | MF_GRAYED | MF_DISABLED);
					}
				}
			}
		}

		CPoint Point;
		GetCursorPos(&Point);
		Menu.TrackPopupMenu(TPM_RIGHTBUTTON, Point.x, Point.y, this);
		Menu.DestroyMenu();
		
	}

	*pResult = 0;
}


void CRegisterManagerDlg::OnRegListModify()
{
	int nItem = m_List.GetSelectionMark();
	if (nItem != -1)
	{
		CString strType = m_List.GetItemText(nItem, 1);

		if (strType.Find(L"SZ") != -1)
		{
			CString strValue = m_List.GetItemText(nItem, 0);
			CString strData = m_List.GetItemText(nItem, 2);

			if (m_List.GetItemData(nItem) && !strData.CompareNoCase(L"(数值未设置)"))
			{
				strData = L"";
			}

			CRegModifyDlg RegModifyDlg;
			RegModifyDlg.m_strValueDataEdit = strData;
			RegModifyDlg.m_strValueNameEdit = strValue;

			if (RegModifyDlg.DoModal() == IDOK)
			{
				CString strDataNew = RegModifyDlg.m_strValueDataEdit;
				if (m_List.GetItemData(nItem))
				{
					strValue = L"";
				}

				ModifyValue(strValue,GetValueType(m_List.GetItemText(nItem, 1)),strDataNew.GetBuffer(), strDataNew.GetLength() * sizeof(WCHAR));
				OnRegListRefresh();
			}
		}
		else if (!strType.CompareNoCase(L"REG_DWORD"))
		{
			CString strValue = m_List.GetItemText(nItem, 0);
			CString strData = m_List.GetItemText(nItem, 2);

			CString strShowData;
			strShowData.Format(L"%ld", HexStringToLong(strData));

			CRegModifyDlg RegModifyDlg;
			RegModifyDlg.m_strValueDataEdit = strShowData;
			RegModifyDlg.m_strValueNameEdit = strValue;

			if (RegModifyDlg.DoModal() == IDOK)
			{
				CString strDataNew = RegModifyDlg.m_strValueDataEdit;
				DWORD dwData = 0;

			
			
				swscanf_s(strDataNew.GetBuffer(0), L"%x", &dwData);
			
			

				ModifyValue(strValue, GetValueType(m_List.GetItemText(nItem, 1)), &dwData, sizeof(DWORD));
				OnRegListRefresh();
			}
		}

		else if (!strType.CompareNoCase(L"REG_QWORD"))
		{
			CString strValue = m_List.GetItemText(nItem, 0);
			CString strData = m_List.GetItemText(nItem, 2);

			CString strShowData;
			strShowData.Format(L"%ld", HexStringToLong(strData));

			CRegModifyDlg RegModifyDlg;
			RegModifyDlg.m_strValueDataEdit = strShowData;
			RegModifyDlg.m_strValueNameEdit = strValue;

			if (RegModifyDlg.DoModal() == IDOK)
			{
				CString strDataNew = RegModifyDlg.m_strValueDataEdit;
				LONGLONG dwData = 0;



				swscanf_s(strDataNew.GetBuffer(0), L"%p", &dwData);



				ModifyValue(strValue, GetValueType(m_List.GetItemText(nItem, 1)), &dwData, sizeof(LONGLONG));
				OnRegListRefresh();
			}
		}



		else if (!strType.CompareNoCase(L"REG_BINARY") ||
			!strType.CompareNoCase(L"REG_RESOURCE_REQUIREMENTS_LIST") ||
			!strType.CompareNoCase(L"REG_FULL_RESOURCE_DESCRIPTOR") ||
			!strType.CompareNoCase(L"REG_RESOURCE_LIST"))
		{
			CString strValue = m_List.GetItemText(nItem, 0);
			CString strData = m_List.GetItemText(nItem, 2);
			ULONG ulLen = strData.GetLength() + strData.GetLength() % 2 + 10;
			PBYTE Data = (PBYTE)malloc(ulLen);
			ULONG ulDataLen = 0;

			if (Data)
			{
				memset(Data, 0, ulLen);
				int n = strData.Find(' ');
				while (n != -1)
				{
					swscanf_s(strData, L"%2x", &Data[ulDataLen++]);
					strData = strData.Right(strData.GetLength() - n - 1);
					n = strData.Find(' ');
				}

				CRegHexEditDlg EditDlg;
				EditDlg.m_Data = Data;
				EditDlg.m_DataLen = ulDataLen;
				EditDlg.m_strValueNameEdit = strValue;

				if (EditDlg.DoModal() == IDOK)
				{
					PBYTE RetData = EditDlg.m_RetData;
					ULONG RetLen = EditDlg.m_RetLen;
					if (RetData && RetLen)
					{
						ModifyValue(strValue, GetValueType(m_List.GetItemText(nItem, 1)),RetData, RetLen);
						OnRegListRefresh();
					}

					if (RetData)
					{
						free(RetData);
					}
				}

				if (Data)
				{
					free(Data);
					Data = NULL;
				}
			}
		}
	}
}

ULONG CRegisterManagerDlg::GetValueType(CString strType)
{
	ULONG ulRet = 0;
	if (strType.IsEmpty())
	{
		return 0;
	}

	if (!strType.CompareNoCase(L"REG_NONE"))
	{
		ulRet = REG_NONE;
	}
	else if (!strType.CompareNoCase(L"REG_SZ"))
	{
		ulRet = REG_SZ;
	}
	else if (!strType.CompareNoCase(L"REG_EXPAND_SZ"))
	{
		ulRet = REG_EXPAND_SZ;
	}
	else if (!strType.CompareNoCase(L"REG_BINARY"))
	{
		ulRet = REG_BINARY;
	}
	else if (!strType.CompareNoCase(L"REG_DWORD"))
	{
		ulRet = REG_DWORD;
	}
	else if (!strType.CompareNoCase(L"REG_DWORD_BIG_ENDIAN"))
	{
		ulRet = REG_DWORD_BIG_ENDIAN;
	}
	else if (!strType.CompareNoCase(L"REG_LINK"))
	{
		ulRet = REG_LINK;
	}
	else if (!strType.CompareNoCase(L"REG_MULTI_SZ"))
	{
		ulRet = REG_MULTI_SZ;
	}
	else if (!strType.CompareNoCase(L"REG_RESOURCE_LIST"))
	{
		ulRet = REG_RESOURCE_LIST;
	}
	else if (!strType.CompareNoCase(L"REG_FULL_RESOURCE_DESCRIPTOR"))
	{
		ulRet = REG_FULL_RESOURCE_DESCRIPTOR;
	}
	else if (!strType.CompareNoCase(L"REG_RESOURCE_REQUIREMENTS_LIST"))
	{
		ulRet = REG_RESOURCE_REQUIREMENTS_LIST;
	}
	else if (!strType.CompareNoCase(L"REG_QWORD"))
	{
		ulRet = REG_QWORD;
	}
	else if (!strType.CompareNoCase(L"Unknow"))
	{
		ulRet = 0;
	}

	return ulRet;
}



void CRegisterManagerDlg::ModifyValue(CString strValue, ULONG ulType, PVOID Data, ULONG ulDataSize)
{
	CString strKeyPath = String2KeyPath();

	if (!strKeyPath.IsEmpty())
	{
		UNICODE_STRING uniKey;
		if (InitUnicodeString(&uniKey, strKeyPath.GetBuffer()))
		{
			HANDLE hKey;
			OBJECT_ATTRIBUTES oa;

			InitializeObjectAttributes(&oa, &uniKey, OBJ_CASE_INSENSITIVE, 0, NULL);

			if (OpenKey(&hKey, KEY_ALL_ACCESS, &oa))
			{
				if (strValue.IsEmpty())
				{
					UNICODE_STRING uniValue;
					uniValue.Buffer = NULL;
					uniValue.Length = 0;
					uniValue.MaximumLength = 2;

					if (SetValueKey(hKey, &uniValue, 0, ulType,Data, ulDataSize))
					{
					
					}
				}
				else 
				{
					UNICODE_STRING uniValue;
					if (InitUnicodeString(&uniValue, strValue.GetBuffer()))
					{
						SetValueKey(hKey, &uniValue, 0, ulType, Data, ulDataSize);
						FreeUnicodeString(&uniValue);
					}
				}

				CloseHandle(hKey);
			}

			FreeUnicodeString(&uniKey);
		}
	}
}


void CRegisterManagerDlg::OnRegListDelete()
{
	int nItem = m_List.GetSelectionMark();
	if (nItem != -1)
	{
		CString strValue = m_List.GetItemText(nItem, 0);
		CString strKeyPath = String2KeyPath();

		if (!strKeyPath.IsEmpty())
		{
			if (MessageBox(L"确定删除吗？",L"Shine", MB_ICONQUESTION | MB_YESNO) != IDYES)
			{
				return;
			}

			UNICODE_STRING uniKey;
			BOOL bOk = FALSE;
			if (InitUnicodeString(&uniKey, strKeyPath.GetBuffer()))
			{
				HANDLE hKey;
				OBJECT_ATTRIBUTES oa;

				InitializeObjectAttributes(&oa, &uniKey, OBJ_CASE_INSENSITIVE, 0, NULL);

				if (OpenKey(&hKey, KEY_ALL_ACCESS, &oa))
				{
					UNICODE_STRING uniValue;

					if (m_List.GetItemData(nItem))
					{
						uniValue.Buffer = NULL;
						uniValue.Length = 0;
						uniValue.MaximumLength = 2;

						bOk = DeleteValueKey(hKey, &uniValue);
					}
					else
					{
						if (InitUnicodeString(&uniValue, strValue.GetBuffer()))
						{
							bOk = DeleteValueKey(hKey, &uniValue);
							FreeUnicodeString(&uniValue);
							
						}
					}

					CloseHandle(hKey);
				}

				FreeUnicodeString(&uniKey);
			}

			if (bOk)
			{
				OnRegListRefresh();
			}
		}
	}
}

ULONG CRegisterManagerDlg::HexStringToLong(CString strHex)
{
	ULONG ulRet = 0;

	if (!strHex.IsEmpty())
	{
		swscanf_s(strHex.GetBuffer(wcslen(L"0x")), L"%x", &ulRet);
		strHex.ReleaseBuffer(wcslen(L"0x"));
	}

	return ulRet;
}


BOOL CRegisterManagerDlg::DeleteValueKey(
	IN HANDLE  KeyHandle,
	IN PUNICODE_STRING  uniValueName
	)
{
	DWORD dwReturnSize = 0;
	DWORD dwRet = 0;


	struct
	{
		HANDLE hKey;
		PUNICODE_STRING  uniValueName;
	}DeleteValueKey;


	memset(&DeleteValueKey, 0, sizeof(DeleteValueKey));
	
	DeleteValueKey.hKey = KeyHandle;
	DeleteValueKey.uniValueName = uniValueName;

	g_hDevice = OpenDevice(L"\\\\.\\RegisterManagerLink");


	if (g_hDevice==(HANDLE)-1)
	{
		return FALSE;
	}



	dwRet = DeviceIoControl(g_hDevice,CTL_DELETE_KEY_VALUE,
		&DeleteValueKey,
		sizeof(DeleteValueKey),
		NULL,0,
		&dwReturnSize,
		NULL);


	if (dwRet==0)
	{

		CloseHandle(g_hDevice);
		return FALSE;
	}

	CloseHandle(g_hDevice);

	return TRUE;
}


void CRegisterManagerDlg::OnRegListRename()
{
	CString strKeyPath = String2KeyPath();
	if (strKeyPath.IsEmpty())
	{
		return;
	}

	int nItem = m_List.GetSelectionMark();
	if (nItem != -1)
	{
		CKeyDlg RenameValueDlg;
		RenameValueDlg.m_nDlgType = enumRenameValue;
		CString strValueNameOld = m_List.GetItemText(nItem, 0);
		RenameValueDlg.m_strKeyNameEdit = strValueNameOld;
		CString strNewValueName;
		if (RenameValueDlg.DoModal() == IDOK)
		{
			strNewValueName = RenameValueDlg.m_strKeyNameEdit;
		}

		if (strNewValueName.IsEmpty())
		{
			return;
		}

		if (strNewValueName.CompareNoCase(strValueNameOld))
		{
			//获得原先键值的各种属性
			PKEY_VALUE_FULL_INFORMATION ValueInfor = (PKEY_VALUE_FULL_INFORMATION)GetValueInfo(strKeyPath, strValueNameOld);
			if (ValueInfor)
			{
				//重建一个新的
				CreateValueKey(strNewValueName, ValueInfor->Type, 
					(PVOID)((PBYTE)ValueInfor + ValueInfor->DataOffset), ValueInfor->DataLength);

				// 删除旧的键值
				UNICODE_STRING uniKey;
				if (InitUnicodeString(&uniKey, strKeyPath.GetBuffer()))
				{
					HANDLE hKey;
					OBJECT_ATTRIBUTES oa;

					InitializeObjectAttributes(&oa, &uniKey, OBJ_CASE_INSENSITIVE, 0, NULL);

					if (OpenKey(&hKey, KEY_ALL_ACCESS, &oa))
					{
						UNICODE_STRING uniValue;

						if (InitUnicodeString(&uniValue, strValueNameOld.GetBuffer()))
						{
							DeleteValueKey(hKey, &uniValue);
							FreeUnicodeString(&uniValue);
							m_List.DeleteItem(nItem);
						}

						CloseHandle(hKey);
					}

					FreeUnicodeString(&uniKey);
				}

				// 释放资源
				free(ValueInfor);
				ValueInfor = NULL;
			}
		}
	}	
}



PVOID CRegisterManagerDlg::GetValueInfo(CString strKey, CString strValue)
{
	PVOID Ret = NULL;

	if (strKey.IsEmpty() || strValue.IsEmpty())
	{
		return NULL;
	}

	UNICODE_STRING uniKey;

	if (InitUnicodeString(&uniKey,strKey.GetBuffer()))
	{
		HANDLE hKey;
		OBJECT_ATTRIBUTES oa;

		InitializeObjectAttributes(&oa, &uniKey, OBJ_CASE_INSENSITIVE, 0, NULL);
		if (OpenKey(&hKey, KEY_ALL_ACCESS, &oa))
		{
			for (ULONG i = 0; ; i++)
			{
				ULONG ulRetLen = 0;
				BOOL bRet = EnumerateValueKey(hKey, i, KeyValueFullInformation, NULL, 0, &ulRetLen);
				if (!bRet && GetLastError() == ERROR_NO_MORE_ITEMS) 
				{
					break;
				}
				else if (!bRet && GetLastError() == ERROR_INSUFFICIENT_BUFFER) // STATUS_BUFFER_TOO_SMALL
				{
					PKEY_VALUE_FULL_INFORMATION Buffer = (PKEY_VALUE_FULL_INFORMATION)malloc(ulRetLen + 0x100);
					if (Buffer)
					{
						memset(Buffer, 0, ulRetLen + 0x100);
						bRet = EnumerateValueKey(hKey, i, KeyValueFullInformation, Buffer, ulRetLen + 0x100, &ulRetLen);

						if (bRet)
						{
							WCHAR wzTempName[1024] = {0};
							wcsncpy_s(wzTempName, 1024, Buffer->Name,Buffer->NameLength / sizeof(WCHAR));

							if (!strValue.CompareNoCase(wzTempName))
							{
								Ret = (PVOID)Buffer;
								break;
							}
						}

						free(Buffer);
					}
				}
			}

			CloseHandle(hKey);
		}

		FreeUnicodeString(&uniKey);
	}

	return Ret;
}




void CRegisterManagerDlg::CreateValueKey(CString strValue, ULONG ulType, PVOID Data, ULONG ulDataSize)
{
	CString strKeyPath = String2KeyPath();
	if (strKeyPath.IsEmpty() || strValue.IsEmpty())
	{
		return;
	}

	UNICODE_STRING uniKey;
	if (InitUnicodeString(&uniKey, strKeyPath.GetBuffer()))
	{
		HANDLE hKey;
		OBJECT_ATTRIBUTES oa;

		InitializeObjectAttributes(&oa, &uniKey, OBJ_CASE_INSENSITIVE, 0, NULL);

		if (OpenKey(&hKey, KEY_ALL_ACCESS, &oa))
		{
			UNICODE_STRING uniValue;

			if (InitUnicodeString(&uniValue, strValue.GetBuffer()))
			{
				SetValueKey(hKey, &uniValue, 0, ulType, Data, ulDataSize);
				FreeUnicodeString(&uniValue);
			}

			CloseHandle(hKey);
		}

		FreeUnicodeString(&uniKey);
	}

	OnRegListRefresh();
}


void CRegisterManagerDlg::OnRegListCopyValue()
{
	int nItem = m_List.GetSelectionMark();
	if (nItem != -1)
	{
		CString strValue = m_List.GetItemText(nItem, 0);
		SetStringToClipboard(strValue);
	}
}


void CRegisterManagerDlg::OnRegListCopyValueData()
{
	int nItem = m_List.GetSelectionMark();
	if (nItem != -1)
	{
		CString strData = m_List.GetItemText(nItem, 2);
		SetStringToClipboard(strData);
	}
}
