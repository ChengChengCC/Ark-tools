// ProcessDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "HeavenShadow.h"
#include "ProcessDlg.h"
#include "afxdialogex.h"
#include "ProcessFunc.h"

#include "HeavenShadowDlg.h"
#include "Common.h"

#include "ProcessViewDlg.h"
#include "InjectFunc.h"

#include <afxdb.h>             //注意这里的文件声明
#include <odbcinst.h>


CWnd* g_process = NULL;


extern HANDLE g_hDevice;
extern BOOL bIsChecking;

extern enum HS_ENUM_PROCVIEW_TYPE;


int ResizeX = 0;
int ResizeY = 0;

extern int dpix;
extern int dpiy;

//////////////////////////////////////////////////////////////////////////
//排序

UINT sort_column;		//记录点击的列
BOOL method = TRUE;		//记录比较方法

//////////////////////////////////////////////////////////////////////////


// CProcessDlg 对话框

IMPLEMENT_DYNAMIC(CProcessDlg, CDialog)

CProcessDlg::CProcessDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CProcessDlg::IDD, pParent)
{
	m_wParent = pParent;
}

CProcessDlg::~CProcessDlg()
{
}

void CProcessDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_PROCESSLIST, m_ProcessList);
}


BEGIN_MESSAGE_MAP(CProcessDlg, CDialog)
	ON_WM_PAINT()
	ON_WM_SHOWWINDOW()
	ON_MESSAGE(HS_PROCESSDIG_SEND_INSERT,HsProcessDlgSendInsert)
	ON_NOTIFY(LVN_COLUMNCLICK, IDC_LIST_PROCESSLIST, &CProcessDlg::OnColumnclickListProcesslist)
	ON_NOTIFY(NM_RCLICK, IDC_LIST_PROCESSLIST, &CProcessDlg::OnRclickListProcesslist)
	ON_COMMAND(ID_MENU_PROCESS_REFRESH, &CProcessDlg::OnMenuProcessRefresh)
	ON_COMMAND(ID_MENU_PROCESS_KILLPROCESS, &CProcessDlg::OnMenuProcessKillprocess)
	ON_COMMAND(ID_MENU_PROCESS_PROCESSTHREAD, &CProcessDlg::OnMenuProcessProcessthread)
	ON_COMMAND(ID_MENU_PROCESS_COPYINFO, &CProcessDlg::OnMenuProcessCopyinfo)
	ON_COMMAND(ID_MENU_PROCESS_ATTRIBUTE, &CProcessDlg::OnMenuProcessAttribute)
	ON_COMMAND(ID_MENU_PROCESS_LOCATIONFILE, &CProcessDlg::OnMenuProcessLocationfile)
	ON_COMMAND(ID_MENU_PROCESS_EXPORTTXT, &CProcessDlg::OnMenuProcessExporttxt)
	ON_COMMAND(ID_MENU_PROCESS_EXPORTEXCEL, &CProcessDlg::OnMenuProcessExportexcel)
	ON_COMMAND(ID_MENU_PROCESS_PROCESSPRIVILEGE, &CProcessDlg::OnMenuProcessProcessprivilege)
	ON_COMMAND(ID_MENU_PROCESS_INJECTDLL, &CProcessDlg::OnMenuProcessInjectdll)
	ON_COMMAND(ID_MENU_PROCESS_KILLMUST, &CProcessDlg::OnMenuProcessKillmust)
	ON_COMMAND(ID_MENU_PROCESS_DETAIL, &CProcessDlg::OnMenuProcessDetail)
	ON_COMMAND(ID_MENU_PROCESS_PROCESSHANDLE, &CProcessDlg::OnMenuProcessProcesshandle)
	ON_COMMAND(ID_MENU_PROCESS_MEMORY, &CProcessDlg::OnMenuProcessMemory)
	ON_COMMAND(ID_MENU_PROCESS_NEWRUN, &CProcessDlg::OnMenuProcessNewrun)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST_PROCESSLIST, &CProcessDlg::OnDblclkListProcesslist)
	ON_COMMAND(ID_MENU_PROCESS_PROCESSWINDOW, &CProcessDlg::OnMenuProcessProcesswindow)
	ON_COMMAND(ID_MENU_PROCESS_SUSPEND, &CProcessDlg::OnMenuProcessSuspend)
	ON_COMMAND(ID_MENU_PROCESS_RECOVERY, &CProcessDlg::OnMenuProcessRecovery)
	ON_COMMAND(ID_MENU_PROCESS_PROCESSMODULE, &CProcessDlg::OnMenuProcessProcessmodule)
	ON_WM_SIZE()
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_LIST_PROCESSLIST, &CProcessDlg::OnNMCustomdrawListProcesslist)
END_MESSAGE_MAP()


// CProcessDlg 消息处理程序


void CProcessDlg::OnBnClickedButtonTest()
{
	// TODO: 在此添加控件通知处理程序代码
	MessageBox(L"进程对话框",L"进程对话框");
}


void CProcessDlg::OnPaint()
{
	CPaintDC dc(this); // device context for painting
	// TODO: 在此处添加消息处理程序代码
	// 不为绘图消息调用 CDialog::OnPaint()

	CRect   rect;
	GetClientRect(rect);
	dc.FillSolidRect(rect,RGB(255,255,255));
}


BOOL CProcessDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  在此添加额外的初始化

	g_process = this;

	UINT uIconSize = 20;

	uIconSize *= (UINT)(dpix/96.0);

	m_TreeImageList.Create(uIconSize, uIconSize, ILC_COLOR32 | ILC_MASK, 2, 2);
	
	ListView_SetImageList(m_ProcessList.m_hWnd, m_TreeImageList.GetSafeHandle(), LVSIL_SMALL); 
	
	//m_ProcessList.SetImageList(&m_TreeImageList, LVSIL_SMALL);

	HsInitList();

	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常: OCX 属性页应返回 FALSE
}



void CProcessDlg::HsInitList(void)
{
	HsInitProcessList((CMyList*)&m_ProcessList);
	
	//m_ProcessList.SetSelectedColumn(1);
}

void CProcessDlg::OnShowWindow(BOOL bShow, UINT nStatus)
{
	CDialog::OnShowWindow(bShow, nStatus);

	// TODO: 在此处添加消息处理程序代码

	//////////////////////////////////////////////////////////////////////////
	
	if (bShow == TRUE)
	{
		m_ProcessList.MoveWindow(0,0,ResizeX,ResizeY);

		((CHeavenShadowDlg*)m_wParent)->m_bNowWindow = HS_DIALOG_PROCESS;

		((CHeavenShadowDlg*)m_wParent)->m_btnProc.EnableWindow(FALSE);

		HsLoadProcessList();

		HsSendStatusTip(L"进程");

		m_ProcessList.SetFocus();
	}
}



void CProcessDlg::OnColumnclickListProcesslist(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码

	int nItemCount;
	
	nItemCount = m_ProcessList.GetItemCount();

	sort_column=pNMLV->iSubItem;//点击的列

	m_ProcessList.SetSelectedColumn(sort_column);

	for(int i=0;i<nItemCount;i++) m_ProcessList.SetItemData(i,i);//每行的比较关键字，此处为列序号（点击的列号），可以设置为其他 比较函数的第一二个参数

	m_ProcessList.SortItems(HsProcessListCompareProc,(DWORD_PTR)&m_ProcessList);//排序 第二个参数是比较函数的第三个参数

	if (method)
	{
		method = FALSE;
	}
	else
	{
		method = TRUE;
	}

	for (int i = 0; i < nItemCount; i++)
	{
		if (_wcsnicmp(
			m_ProcessList.GetItemText(i,HS_PROCESS_COLUMN_COMPANY),
			L"Microsoft Corporation",
			wcslen(L"Microsoft Corporation"))==0
			)
		{
			m_ProcessList.SetItemData(i,1);
		}
	}

	*pResult = 0;
}


void CProcessDlg::HsLoadProcessList(void) //加载列表
{

	if (bIsChecking == FALSE)
	{
		bIsChecking = TRUE;

		while(m_TreeImageList.Remove(0));

		m_ProcessList.DeleteAllItems();

		m_ProcessList.SetSelectedColumn(-1);

		HsSendStatusDetail(L"进程列表正在加载。");
		
		CloseHandle(
 			CreateThread(NULL,0, 
 			(LPTHREAD_START_ROUTINE)HsQueryProcessFunction,&m_ProcessList, 0,NULL)
			);
	}
}







void CProcessDlg::OnRclickListProcesslist(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码

	CMenu	popup;
	popup.LoadMenu(IDR_MENU_PROCESS);				//加载菜单资源
	CMenu*	pM = popup.GetSubMenu(0);				//获得菜单的子项
	CPoint	p;
	GetCursorPos(&p);
	int	count = pM->GetMenuItemCount();
	if (m_ProcessList.GetSelectedCount() == 0)		//如果没有选中
	{ 
		for (int i = 0;i<count;i++)
		{
			pM->EnableMenuItem(i, MF_BYPOSITION | MF_DISABLED | MF_GRAYED); //菜单全部变灰
		}

		pM->EnableMenuItem(ID_MENU_PROCESS_REFRESH, MF_BYCOMMAND | MF_ENABLED);
	}

	pM->TrackPopupMenu(TPM_LEFTALIGN, p.x, p.y, this);

	*pResult = 0;
}


void CProcessDlg::OnMenuProcessRefresh()
{
	// TODO: 在此添加命令处理程序代码
	HsLoadProcessList();
}





void CProcessDlg::OnMenuProcessKillprocess()
{
	// TODO: 在此添加命令处理程序代码

	if (bIsChecking)
	{
		return;
	}

	if (MessageBox(L"确定要结束该进程吗？",L"天影卫士",MB_YESNO) == IDYES)
	{
		bIsChecking = TRUE;

		POSITION pos = m_ProcessList.GetFirstSelectedItemPosition();

		while (pos)
		{
			int nItem = m_ProcessList.GetNextSelectedItem(pos);

			DWORD_PTR ulPid = _ttoi(m_ProcessList.GetItemText(nItem,HS_PROCESS_COLUMN_PID).GetBuffer());

			if (ulPid == 0 || ulPid == 4)
			{
				MessageBox(L"进程关闭失败。",L"天影卫士",0);
				return;
			}

			HsDebugPrivilege(SE_DEBUG_NAME, TRUE);

			HANDLE hProcess = OpenProcess(PROCESS_TERMINATE | PROCESS_VM_OPERATION, TRUE, (DWORD)ulPid);

			BOOL bIsSuccess = TerminateProcess(hProcess,0);

			HsDebugPrivilege(SE_DEBUG_NAME, FALSE);

			if (bIsSuccess == FALSE)
			{
				MessageBox(L"关闭进程失败。",L"天影卫士",0);
			}
			else
			{
				m_ProcessList.DeleteItem(nItem);
			}

			CloseHandle(hProcess);
		}
		
		bIsChecking = FALSE;
	}

}



// void CProcessDlg::HsQuaryProcessThread(ULONG_PTR ulPid)
// {
// 	DWORD dwReturnSize = 0;
// 	DWORD dwRet = 0;
// 	//发送IO 控制码
// 
// 
// 
// 	dwRet = DeviceIoControl(g_hDevice,HS_IOCTL(HS_IOCTL_PROC_PROCESSTHREAD),
// 		(PVOID)&ulPid,
// 		sizeof(ULONG_PTR),
// 		NULL,
// 		0,
// 		&dwReturnSize,
// 		NULL);
// 
// 	if (dwRet==0)
// 	{
// 		WCHAR *temp = L"与内核层通信异常。";
// 		::SendMessageW(m_wParent->m_hWnd,HS_MESSAGE_STATUSDETAIL,NULL,(LPARAM)temp);
// 
// 		return;
// 	}
// }



BOOL CProcessDlg::PreTranslateMessage(MSG* pMsg)
{
	// TODO: 在此添加专用代码和/或调用基类
	if (pMsg->message==WM_KEYDOWN && (pMsg->wParam==VK_RETURN ||pMsg->wParam==VK_ESCAPE))
	{
		return TRUE;
	}
	if (pMsg->message==WM_KEYDOWN && pMsg->wParam==VK_F5)
	{
		OnMenuProcessRefresh();
	}
	return CDialog::PreTranslateMessage(pMsg);
}


void CProcessDlg::OnMenuProcessCopyinfo()
{
	// TODO: 在此添加命令处理程序代码
	POSITION pos = m_ProcessList.GetFirstSelectedItemPosition();

	while(pos)
	{
		int iItem = m_ProcessList.GetNextSelectedItem(pos);

		CStringA(csProcInfo);
		csProcInfo = "映像名称: ";
		csProcInfo += m_ProcessList.GetItemText(iItem,HS_PROCESS_COLUMN_NAME);
		csProcInfo += "  进程ID: ";
		csProcInfo += m_ProcessList.GetItemText(iItem,HS_PROCESS_COLUMN_PID);
		csProcInfo += "  父进程ID: ";
		csProcInfo += m_ProcessList.GetItemText(iItem,HS_PROCESS_COLUMN_PPID);
		csProcInfo += "  映像路径: ";
		csProcInfo += m_ProcessList.GetItemText(iItem,HS_PROCESS_COLUMN_PATH);
		csProcInfo += "  EPROCESS: ";
		csProcInfo += m_ProcessList.GetItemText(iItem,HS_PROCESS_COLUMN_EPROCESS);
		csProcInfo += "  应用层访问: ";
		csProcInfo += m_ProcessList.GetItemText(iItem,HS_PROCESS_COLUMN_USERACCESS);
		csProcInfo += "  文件厂商: ";
		if (m_ProcessList.GetItemText(iItem,HS_PROCESS_COLUMN_COMPANY).GetLength()>0)
		{
			csProcInfo += m_ProcessList.GetItemText(iItem,HS_PROCESS_COLUMN_COMPANY);
		}

		if(::OpenClipboard(NULL))
		{
			HGLOBAL hmem=GlobalAlloc(GHND,csProcInfo.GetLength()+1);
			char *pmem=(char*)GlobalLock(hmem);

			EmptyClipboard();
			memcpy(pmem,csProcInfo.GetBuffer(),csProcInfo.GetLength()+1);
			SetClipboardData(CF_TEXT,hmem);
			CloseClipboard();
			GlobalFree(hmem); 
		}
	}
}



void CProcessDlg::OnMenuProcessAttribute()
{
	// TODO: 在此添加命令处理程序代码

	POSITION pos = m_ProcessList.GetFirstSelectedItemPosition();

	while(pos)
	{
		int iItem = m_ProcessList.GetNextSelectedItem(pos);

		CString csFilePath = m_ProcessList.GetItemText(iItem,HS_PROCESS_COLUMN_PATH);

		HsCheckAttribute(csFilePath);
	}
}


void CProcessDlg::OnMenuProcessLocationfile()
{
	// TODO: 在此添加命令处理程序代码
	POSITION pos = m_ProcessList.GetFirstSelectedItemPosition();

	while(pos)
	{
		int iItem = m_ProcessList.GetNextSelectedItem(pos);

		CString csFilePath = m_ProcessList.GetItemText(iItem,HS_PROCESS_COLUMN_PATH);

		HsLocationExplorer(csFilePath);
	}
}


void CProcessDlg::OnMenuProcessExporttxt()
{
	// TODO: 在此添加命令处理程序代码
	
	if (m_ProcessList.GetItemCount () > 0) 
	{ 
		CString strFile; 

		if (!HsGetDefaultTextFileName(strFile))
			return;

		CFile File;

		TRY
		{
			if (File.Open(strFile, CFile::modeCreate | CFile::modeWrite | CFile::modeNoTruncate, NULL))
			{
				int i = 0;
				LVCOLUMN columnData;
				CString  columnName;
				int      columnNum = 0;
				CString  strCloumn;
				CHAR  szColumn[0x1000] = {0};
				WCHAR wzColumn[0x1000] = {0};

				columnData.mask = LVCF_TEXT;
				columnData.cchTextMax = 100;
				columnData.pszText = columnName.GetBuffer(100);

				for(i = 0;m_ProcessList.GetColumn(i,&columnData); i++)
				{
					strCloumn = strCloumn + columnData.pszText + L"  |  ";
				}

				strCloumn += "\r\n";
				ULONG  nLenTemp = strCloumn.GetLength();
				wcsncpy_s(wzColumn,0x1000,strCloumn.GetBuffer(),nLenTemp);
				strCloumn.ReleaseBuffer();
				WideCharToMultiByte(CP_ACP,0,wzColumn,-1,szColumn,0x1000,NULL,NULL);
				File.Write(szColumn,(UINT)strlen(szColumn));  

				columnName.ReleaseBuffer();
				columnNum = i;

				//上面是取出ShowList 的标题


				for (int nItemIndex = 0; nItemIndex <m_ProcessList.GetItemCount(); nItemIndex++)
				{
					CHAR  szColumn[0x1000]  = {0};
					WCHAR wzColumn[0x1000] = {0};
					CString strItem;

					for( i = 0; i < columnNum; i++)
					{
						strItem = strItem + m_ProcessList.GetItemText(nItemIndex,i) + L"  |  ";
					}

					strItem += "\r\n";
					nLenTemp = strItem.GetLength();
					wcsncpy_s(wzColumn,0x1000,strItem.GetBuffer(),nLenTemp);
					strItem.ReleaseBuffer();
					WideCharToMultiByte(CP_ACP,0,wzColumn,-1,szColumn,0x1000,NULL,NULL);
					File.Write(szColumn, (UINT)strlen(szColumn));  
				}

				File.Close();
			}
		}
		CATCH_ALL(e)
		{
			File.Abort();  
		}
		END_CATCH_ALL


		if ( PathFileExists(strFile))
		{
			ShellExecuteW(NULL, L"open", strFile, NULL, NULL, SW_SHOW);
		}
		else
		{
			::MessageBox(NULL, L"导出到文本文件失败。", L"天影卫士", MB_OK | MB_ICONERROR);
		}
	}
}


void CProcessDlg::OnMenuProcessExportexcel()
{
// 	// TODO: 在此添加命令处理程序代码
// 
// 	BOOL bOk = FALSE;
// 
// 	CString strEnd = L"枚举完毕";
// 
// 	if (m_ProcessList.GetItemCount () > 0) 
// 	{ 
// 		CDatabase DataBase;
// 		CString   strDriver;
// 		CString   strExcelFile; 
// 		CString   strSql;
// 		CString   TableName = L"HeavenShadow";
// 		strDriver = HsGetExcelDriver();
// 		if (strDriver.IsEmpty())
// 		{
// 			::MessageBox(NULL, L"没有安装Excel!\n请先安装Excel软件才能使用导出功能!", L"导出", MB_OK | MB_ICONERROR);
// 			return;
// 		}
//
// 		if (!HsGetDefaultExcelFileName(strExcelFile))
// 		{
// 			return;
// 		}
// 
// 		strSql.Format(L"DRIVER={%s};DSN='';FIRSTROWHASNAMES=1;READONLY=FALSE;CREATE_DB=\"%s\";DBQ=%s", strDriver, strExcelFile, strExcelFile);
// 
// 		if(DataBase.OpenEx(strSql, CDatabase::noOdbcDialog))
// 		{
// 			int i;
// 			LVCOLUMN columnData;
// 			CString columnName;
// 			int columnNum = 0;
// 			CString strH;
// 			CString strV;
// 
// 			strSql = L"";
// 			strH = L"";
// 			columnData.mask = LVCF_TEXT;
// 			columnData.cchTextMax = 100;
// 			columnData.pszText = columnName.GetBuffer(100);
// 
// 			for(i = 0; m_ProcessList.GetColumn(i, &columnData); i++)
// 			{
// 				if (i != 0)
// 				{
// 					strSql = strSql + L", " ;
// 					strH = strH + L", " ;
// 				}
// 
// 				strSql = strSql + L" " + L"[" + columnData.pszText + L"]" + L" TEXT";
// 				strH = strH + L" " + L"[" + columnData.pszText + L"]" + L" ";
// 			}
// 
// 			columnName.ReleaseBuffer();
// 			columnNum = i;
// 
// 			strSql = L"CREATE TABLE " +TableName + L" ( " + strSql +   L" ) ";
// 			DataBase.ExecuteSQL(strSql);
// 
// 			for (int nItemIndex = 0; nItemIndex < m_ProcessList.GetItemCount(); nItemIndex++)
// 			{
// 				strV = L"";
// 				for( i = 0; i < columnNum; i++)
// 				{
// 					if (i != 0)
// 					{
// 						strV = strV + L", " ;
// 					}
// 
// 					strV = strV + L" '" + m_ProcessList.GetItemText(nItemIndex, i) + L"' ";
// 				}
// 
// 				strSql = L"INSERT INTO "+ TableName + L" ("+ strH + L")" + L" VALUES("+ strV + L")";
// 				DataBase.ExecuteSQL(strSql);
// 			}
// 
// 			strV = L"";
// 			for( i = 0; i < columnNum; i++)
// 			{
// 				if (i != 0)
// 				{
// 					strV = strV + L", " ;
// 				}
// 
// 				strV = strV + L" '" + L" " + L"' ";
// 			}
// 
// 			strSql = L"INSERT INTO "+ TableName + L" ("+ strH + L")" + L" VALUES("+ strV + L")";
// 			DataBase.ExecuteSQL(strSql);
// 
// 			strV = L" ";
// 			strV = strV + L" '" + strEnd + L"' ";
// 
// 			for( i = 1; i < columnNum; i++)
// 			{
// 				strV = strV + L", " ;
// 				strV = strV + L" '" + L" " + L"' ";
// 			}
// 
// 			strSql = L"INSERT INTO "+ TableName + L" ("+ strH + L")" + L" VALUES("+ strV + L")";
// 			DataBase.ExecuteSQL(strSql);
// 
// 			bOk = TRUE;
// 		}      
// 
// 		DataBase.Close();
// 
// 		if ( bOk && PathFileExists(strExcelFile) )
// 		{
// 			ShellExecuteW(NULL,L"open",strExcelFile,NULL,NULL,SW_SHOW);
// 		}
// 		else
// 		{
// 			::MessageBox(NULL,L"导出到Excel文件失败。",L"天影卫士",MB_OK | MB_ICONERROR);
// 		}
// 	}
}






//排序比较函数
static int CALLBACK HsProcessListCompareProc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{

	//从参数中提取所需比较lc的两行数据

	int row1=(int) lParam1;
	int row2=(int) lParam2;

	CListCtrl*lc=(CListCtrl*)lParamSort;

	CString lp1=lc->GetItemText(row1,sort_column);
	CString lp2=lc->GetItemText(row2,sort_column);


	//比较，对不同的列，不同比较，注意记录前一次排序方向，下一次要相反排序

	if (sort_column == HS_PROCESS_COLUMN_PID ||
		sort_column == HS_PROCESS_COLUMN_PPID)
	{
		// int型比较
		if (method)
			return _ttoi(lp1)-_ttoi(lp2);
		else
			return _ttoi(lp2)-_ttoi(lp1);
	}
	else if (sort_column == HS_PROCESS_COLUMN_EPROCESS)
	{
		if (method)
		{
			ULONG_PTR nlp1 = 0, nlp2 = 0;

			lp1 = lp1.GetBuffer()+2;
			lp2 = lp2.GetBuffer()+2;

			swscanf_s(lp1.GetBuffer(),L"%P",&nlp1);
			swscanf_s(lp2.GetBuffer(),L"%P",&nlp2);
			return (int)(nlp1 - nlp2);
		}
		else
		{
			int nlp1 = 0, nlp2 = 0;
			lp1 = lp1.GetBuffer()+2;
			lp2 = lp2.GetBuffer()+2;
			swscanf_s(lp1.GetBuffer(),L"%X",&nlp1);
			swscanf_s(lp2.GetBuffer(),L"%X",&nlp2);
			return nlp2 - nlp1;
		}
	}
	else
	{
		// 文字型比较
		if(method)
			return lp1.CompareNoCase(lp2);
		else
			return lp2.CompareNoCase(lp1);
	}

	return 0;
}


void CProcessDlg::OnMenuProcessInjectdll()
{
	// TODO: 在此添加命令处理程序代码

	CloseHandle(
		CreateThread(NULL,0, 
		(LPTHREAD_START_ROUTINE)HsRemoteThreadInjectDll,&m_ProcessList, 0,NULL)
		);

}


void CProcessDlg::OnMenuProcessKillmust()
{
	// TODO: 在此添加命令处理程序代码

	if (bIsChecking)
	{
		return;
	}

	if (MessageBox(L"强制关闭进程的操作有风险，请谨慎操作。\r\n确定要结束该进程吗？",L"天影卫士",MB_YESNO) == IDNO)
	{
		return;
	}

// 	CreateThread(NULL,0, 
// 		(LPTHREAD_START_ROUTINE)HsKillProcessByForce,(CMyList*)&m_ProcessList, 0,NULL);

	bIsChecking = TRUE;

	HsKillProcessByForce((CMyList*)&m_ProcessList);

	bIsChecking = FALSE;

}






void CProcessDlg::OnMenuProcessProcessthread()
{
	// TODO: 在此添加命令处理程序代码

	//HsQuaryProcessThread(ulPid);

	HsOpenProcessViewDlg(HS_PROCVIEW_TYPE_THREAD);

}

void CProcessDlg::OnMenuProcessProcessprivilege()
{
	// TODO: 在此添加命令处理程序代码
	HsOpenProcessViewDlg(HS_PROCVIEW_TYPE_PRIVILEGE);
}


void CProcessDlg::OnMenuProcessDetail()
{
	// TODO: 在此添加命令处理程序代码
	HsOpenProcessViewDlg(HS_PROCVIEW_TYPE_DETAIL);
}


void CProcessDlg::HsOpenProcessViewDlg(int nViewType)
{
	POSITION pos = m_ProcessList.GetFirstSelectedItemPosition();

	while(pos)
	{
		int nItem = m_ProcessList.GetNextSelectedItem(pos);

		ULONG_PTR ulPid = _ttoi(m_ProcessList.GetItemText(nItem,HS_PROCESS_COLUMN_PID).GetBuffer());
		ULONG_PTR ulPPid = _ttoi(m_ProcessList.GetItemText(nItem,HS_PROCESS_COLUMN_PPID).GetBuffer());

		CString ProcessEProcess = m_ProcessList.GetItemText(nItem,HS_PROCESS_COLUMN_EPROCESS);

		ProcessEProcess = ProcessEProcess.GetBuffer()+2;

		ULONG_PTR ulEProcess = 0;

		swscanf_s(ProcessEProcess.GetBuffer(),L"%P",&ulEProcess);


		HSPROCESSINFO hsProcInfo = {0};

		hsProcInfo.Pid = ulPid;
		hsProcInfo.PPid = ulPPid;
		hsProcInfo.Eprocess = ulEProcess;

		if (_wcsnicmp(m_ProcessList.GetItemText(nItem,HS_PROCESS_COLUMN_COMPANY).GetBuffer(),L"拒绝",wcslen(L"拒绝")) == 0)
		{
			hsProcInfo.UserAccess = FALSE;
		}
		else
		{
			hsProcInfo.UserAccess = TRUE;
		}

		StringCchCopyW(hsProcInfo.CompanyName,
			m_ProcessList.GetItemText(nItem,HS_PROCESS_COLUMN_COMPANY).GetLength()+1,
			m_ProcessList.GetItemText(nItem,HS_PROCESS_COLUMN_COMPANY).GetBuffer());
		StringCchCopyW(hsProcInfo.Name,
			m_ProcessList.GetItemText(nItem,HS_PROCESS_COLUMN_NAME).GetLength()+1,
			m_ProcessList.GetItemText(nItem,HS_PROCESS_COLUMN_NAME).GetBuffer());
		StringCchCopyW(hsProcInfo.Path,
			m_ProcessList.GetItemText(nItem,HS_PROCESS_COLUMN_PATH).GetLength()+1,
			m_ProcessList.GetItemText(nItem,HS_PROCESS_COLUMN_PATH).GetBuffer());

		CProcessViewDlg* dlg = new CProcessViewDlg(nViewType,&hsProcInfo,this);
		dlg->DoModal();
	}
}



LRESULT CProcessDlg::HsProcessDlgSendInsert(WPARAM wParam, LPARAM lParam)
{
	PHSPROCESSINFO* hsProcItem = (PHSPROCESSINFO*)lParam;

	CString Name = NULL;
	CString Pid = NULL;
	CString PPid = NULL;
	CString Path = NULL;
	CString EProcess = NULL;
	CString UserAccess = NULL;
	CString CompanyName = NULL;

	ULONG ulItem = m_ProcessList.GetItemCount();

	WCHAR tempdir[100];
	GetEnvironmentVariableW(L"windir",tempdir,100);


	Name = (*hsProcItem)->Name;
	Pid.Format(L"%d",(*hsProcItem)->Pid);

	if ((*hsProcItem)->PPid == 0xffffffff)
	{
		PPid = L"-";
	}
	else
	{
		PPid.Format(L"%d",(*hsProcItem)->PPid);
	}

	Path = (*hsProcItem)->Path;

	EProcess.Format(L"0x%p",(*hsProcItem)->Eprocess);

	if ((*hsProcItem)->UserAccess == TRUE)
	{
		UserAccess = L"允许";
	}
	else
	{
		UserAccess = L"拒绝";
	}

	CompanyName = (*hsProcItem)->CompanyName;

	AddProcessFileIcon(Path.GetBuffer());

	m_ProcessList.InsertItem(ulItem,Name,ulItem);
	m_ProcessList.SetItemText(ulItem,HS_PROCESS_COLUMN_PID,Pid);
	m_ProcessList.SetItemText(ulItem,HS_PROCESS_COLUMN_PPID,PPid);
	m_ProcessList.SetItemText(ulItem,HS_PROCESS_COLUMN_PATH,Path);
	m_ProcessList.SetItemText(ulItem,HS_PROCESS_COLUMN_EPROCESS,EProcess);
	m_ProcessList.SetItemText(ulItem,HS_PROCESS_COLUMN_USERACCESS,UserAccess);
	m_ProcessList.SetItemText(ulItem,HS_PROCESS_COLUMN_COMPANY,CompanyName);

	if (_wcsnicmp(CompanyName,L"Microsoft Corporation", wcslen(L"Microsoft Corporation"))==0)
	{
		m_ProcessList.SetItemData(ulItem,1);
	}


	CToolTipCtrl ToolTipCtrl;

	ToolTipCtrl.Create(this);
	m_ProcessList.SetToolTips(&ToolTipCtrl);


	return TRUE;
}



void CProcessDlg::OnMenuProcessProcesshandle()
{
	// TODO: 在此添加命令处理程序代码
	HsOpenProcessViewDlg(HS_PROCVIEW_TYPE_HANDLE);
}

void CProcessDlg::OnMenuProcessProcesswindow()
{
	// TODO: 在此添加命令处理程序代码
	HsOpenProcessViewDlg(HS_PROCVIEW_TYPE_WINDOW);
}


void CProcessDlg::OnMenuProcessProcessmodule()
{
	// TODO: 在此添加命令处理程序代码
	HsOpenProcessViewDlg(HS_PROCVIEW_TYPE_MODULE);
}

void CProcessDlg::OnMenuProcessMemory()
{
	// TODO: 在此添加命令处理程序代码

	HsOpenProcessViewDlg(HS_PROCVIEW_TYPE_MEMORY);
}


void CProcessDlg::OnMenuProcessNewrun()
{
	// TODO: 在此添加命令处理程序代码

	WCHAR wzFileFilter[] = L"应用程序 (*.exe)\0*.exe\0";
	WCHAR wzFileChoose[] = L"打开文件";


	CFileDialog FileDlg(TRUE);
	FileDlg.m_ofn.lpstrTitle  = wzFileChoose;
	FileDlg.m_ofn.lpstrFilter = wzFileFilter;

	if (IDOK != FileDlg.DoModal())
	{
		return;
	}

	CString ExePath = FileDlg.GetPathName();

	ShellExecuteW(NULL, L"open", ExePath, L"", L"", SW_SHOW);
}



void CProcessDlg::AddProcessFileIcon(WCHAR* ProcessPath)
{
	SHFILEINFO shInfo;
	memset(&shInfo, 0, sizeof(shInfo));
	SHGetFileInfo(ProcessPath, FILE_ATTRIBUTE_NORMAL, 
		&shInfo, sizeof(SHFILEINFO), SHGFI_ICON | SHGFI_USEFILEATTRIBUTES);

	HICON  hIcon = shInfo.hIcon;

	m_TreeImageList.Add(hIcon);
}

void CProcessDlg::OnDblclkListProcesslist(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码

	OnMenuProcessDetail();

	*pResult = 0;
}





void CProcessDlg::OnMenuProcessSuspend()
{
	// TODO: 在此添加命令处理程序代码
}


void CProcessDlg::OnMenuProcessRecovery()
{
	// TODO: 在此添加命令处理程序代码
}




void CProcessDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);

	// TODO: 在此处添加消息处理程序代码

	ResizeX = cx;
	ResizeY = cy;
}


void CProcessDlg::OnNMCustomdrawListProcesslist(NMHDR *pNMHDR, LRESULT *pResult)
{
	NMLVCUSTOMDRAW* pLVCD = reinterpret_cast<NMLVCUSTOMDRAW*>( pNMHDR );
	// TODO: 在此添加控件通知处理程序代码

	*pResult = CDRF_DODEFAULT;

	if ( CDDS_PREPAINT == pLVCD->nmcd.dwDrawStage )
	{
		*pResult = CDRF_NOTIFYITEMDRAW;
	}
	else if ( CDDS_ITEMPREPAINT == pLVCD->nmcd.dwDrawStage )
	{
		*pResult = CDRF_NOTIFYSUBITEMDRAW;
	}
	else if ( (CDDS_ITEMPREPAINT | CDDS_SUBITEM) == pLVCD->nmcd.dwDrawStage )
	{
		COLORREF clrNewTextColor, clrNewBkColor;
		int bHooked = 0;
		int nItem = static_cast<int>( pLVCD->nmcd.dwItemSpec );

		clrNewTextColor = RGB( 0, 0, 0 );
		clrNewBkColor = RGB( 255, 255, 255 );

		bHooked = (int)m_ProcessList.GetItemData(nItem); 		
		if (bHooked == 1)
		{
			clrNewTextColor = RGB( 0, 0, 255 );
		}

		pLVCD->clrText = clrNewTextColor;
		pLVCD->clrTextBk = clrNewBkColor;

		*pResult = CDRF_DODEFAULT;
	}

}
