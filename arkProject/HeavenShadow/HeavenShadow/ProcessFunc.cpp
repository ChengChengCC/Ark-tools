#include "stdafx.h"
#include "ProcessFunc.h"
#include "Common.h"



#include <afxdb.h>
#include <odbcinst.h>

#include <WinVer.h>

#pragma comment(lib, "version.lib")

extern BOOL bIsChecking;
extern HANDLE g_hDevice;
extern CWnd* g_process;
extern CWnd* g_wParent;



extern UINT sort_column;
extern BOOL method;

extern enum HS_PROCESSDIG_SEND_TYPE;

int nProcCount = 0;


COLUMNSTRUCT g_Column_Process[] = 
{
	{	L"映像名称",			160	},
	{	L"进程ID",			65	},
	{	L"父进程ID",			65	},
	{	L"映像路径",			230	},
	{	L"EPROCESS",		125	},
	{	L"应用层访问",		75	},
	{	L"文件厂商",			122	}
};

UINT g_Column_Process_Count  = 7;	  //进程列表列数

extern int dpix;
extern int dpiy;


//自定函数
//////////////////////////////////////////////////////////////////////////


void HsInitProcessList(CMyList *m_ListCtrl)
{
	m_ListCtrl->SetExtendedStyle(LVS_EX_FULLROWSELECT|LVS_EX_GRIDLINES|LVS_EX_HEADERDRAGDROP);

	UINT i;
	for (i = 0;i<g_Column_Process_Count;i++)
	{
		if (i == HS_PROCESS_COLUMN_USERACCESS)
		{
			m_ListCtrl->InsertColumn(i, g_Column_Process[i].szTitle,LVCFMT_CENTER,(int)(g_Column_Process[i].nWidth*(dpix/96.0)));
		}
		else
		{
			m_ListCtrl->InsertColumn(i, g_Column_Process[i].szTitle,LVCFMT_LEFT,(int)(g_Column_Process[i].nWidth*(dpix/96.0)));
		}
	}

}



ULONG_PTR
HsQuerySystemProcessCount(CMyList *m_ListCtrl)
{

	ULONG_PTR dwReturnSize = 0;
	ULONG_PTR dwRet = 0;

	ULONG_PTR ulProcessCount = 0;

	dwRet = DeviceIoControl(g_hDevice,
		HS_IOCTL(HS_IOCTL_PROC_PROCESSCOUNT),
		NULL,
		0,
		&ulProcessCount,
		sizeof(ULONG_PTR),
		(LPDWORD)&dwReturnSize,
		NULL);

	//发送IO 控制码

	if (dwRet==0)
	{
		//cout<<"Send IoCode Error"<<endl;
	}

	return ulProcessCount;
}





DWORD WINAPI HsQueryProcessFunction(CMyList *m_ListCtrl)
{
	bIsChecking = TRUE;

	HsQueryProcessList(m_ListCtrl);

	bIsChecking = FALSE;
	return 0;
}


void HsQueryProcessList(CMyList *m_ListCtrl)
{
	DWORD dwReturnSize = 0;
	DWORD dwRet = 0;

	//发送IO 控制码
	//PVOID hProcessList = NULL;

	int ulItem = 0;	//进程计数

	ULONG_PTR ulPid = 0;

	ulItem = 0;


	//ULONG_PTR ulProcessCount = HsQuerySystemProcessCount(m_ListCtrl);

	while(1)
	{
		TRY 
		{
			HSPROCESSINFO hsProcItem = {0};


			dwRet = DeviceIoControl(g_hDevice,
				HS_IOCTL(HS_IOCTL_PROC_PROCESSLIST),
				&ulPid,
				sizeof(ULONG_PTR),
				&hsProcItem,
				sizeof(HSPROCESSINFO),
				&dwReturnSize,
				NULL);

			if (dwReturnSize==0)
			{
				break;
			}
			else
			{
				//发送状态栏消息响应
				CString csStatusMsg;
				csStatusMsg.Format(L"进程列表正在加载。 进程数：%d",ulItem);

				HsSendStatusDetail(csStatusMsg.GetBuffer());
			}

			if (hsProcItem.Eprocess == 0||hsProcItem.Eprocess == NULL) 
			{
				break;
			}

			if ((_wcsnicmp(hsProcItem.Path,L"UnKnow",wcslen(L"UnKnow"))==0&&hsProcItem.Pid>4))
			{
				goto CONTINUE;
			}

			if (hsProcItem.Pid == 0)
			{
				StringCchCopyW(hsProcItem.Name,wcslen(L"System Idle Process")+1,L"System Idle Process");
				StringCchCopyW(hsProcItem.Path,wcslen(L"System Idle Process")+1,L"System Idle Process");
				hsProcItem.UserAccess = FALSE;
				hsProcItem.PPid = 0xffffffff;
				StringCchCopyW(hsProcItem.CompanyName,wcslen(L" ")+1,L" ");

			}
			else if (hsProcItem.Pid == 4)	//System进程
			{
				StringCchCopyW(hsProcItem.Name,wcslen(L"System")+1,L"System");

				WCHAR wzTempDir[260] = {0};
				CString Temp;
				GetEnvironmentVariableW(L"windir",wzTempDir,MAX_PATH);

				Temp = wzTempDir;
				Temp += L"\\System32\\ntoskrnl.exe";


				StringCchCopyW(hsProcItem.Path,Temp.GetLength()+1,Temp.GetBuffer());
				hsProcItem.UserAccess = FALSE;
				hsProcItem.PPid = 0xffffffff;
				StringCchCopyW(hsProcItem.CompanyName,
					HsGetFileCompanyName(hsProcItem.Path).GetLength()+1,
					HsGetFileCompanyName(hsProcItem.Path).GetBuffer());
			}
			else
			{
				WCHAR* szImageFileName = NULL;
				szImageFileName = wcsrchr(hsProcItem.Path,'\\');
				szImageFileName++;

				StringCchCopyW(hsProcItem.Name,wcslen(szImageFileName)+1,szImageFileName);

				hsProcItem.PPid = hsProcItem.PPid;

				if (HsR3CanOpenProcess((DWORD)hsProcItem.Pid)==TRUE)
				{
					hsProcItem.UserAccess = TRUE;
				}
				else
				{
					hsProcItem.UserAccess = FALSE;
				}
				
				CString Path, CompanyName;
				Path = hsProcItem.Path;
				CompanyName = HsGetFileCompanyName(Path);
				if (CompanyName.GetLength() == 0)
				{
					CompanyName = L" ";
				}

				StringCchCopyW(hsProcItem.CompanyName,CompanyName.GetLength()+1,CompanyName.GetBuffer());

				if (HsIs64BitWindows())
				{
					if (HsIs32BitFile(hsProcItem.Path) == TRUE)
					{
						CString Name;
						Name = hsProcItem.Name;
						Name += L" *32";
						StringCchCopyW(hsProcItem.Name,Name.GetLength()+1,Name.GetBuffer());
					}
				}
			}

			hsProcItem.Pid = hsProcItem.Pid;

			hsProcItem.Eprocess = hsProcItem.Eprocess;

			//////////////////////////////////////////////////////////////////////////
			// 开始插入列表

			PHSPROCESSINFO pHsProcessInfo = &hsProcItem;

			::SendMessageW(g_process->m_hWnd,HS_PROCESSDIG_SEND_INSERT,sizeof(HSPROCESSINFO),(LPARAM)&pHsProcessInfo);

			//插入列表结束
			//////////////////////////////////////////////////////////////////////////

CONTINUE:


			ulPid = hsProcItem.Pid + 4;

			ulItem = ulItem + 1;

		}
		CATCH (CMemoryException, e)
		{
			
		}
		END_CATCH
		
	}
	

	bIsChecking = FALSE;

	CString csStatusMsg;
	csStatusMsg.Format(L"进程列表加载完成。 进程数：%d",ulItem);

	HsSendStatusDetail(csStatusMsg.GetBuffer());

	nProcCount = (int)ulItem;
}





CString HsGetFileCompanyName(CString strPath)
{
	CString strCompanyName = 0;;

	if (strPath.IsEmpty())
	{
		return NULL;
	}

	if (!strPath.CompareNoCase(L"Idle") || !strPath.CompareNoCase(L"System"))
	{
		return NULL;
	}

	struct LANGANDCODEPAGE {
		WORD wLanguage;
		WORD wCodePage;
	} *lpTranslate;

	LPWSTR lpstrFilename = (LPWSTR)(LPCWSTR)strPath;
	DWORD dwHandle = 0;
	DWORD dwVerInfoSize = GetFileVersionInfoSize(lpstrFilename, &dwHandle);

	if (dwVerInfoSize)
	{
		LPVOID Buffer = malloc(sizeof(char)*dwVerInfoSize);

		if (Buffer)
		{
			if (GetFileVersionInfo(lpstrFilename, dwHandle, dwVerInfoSize, Buffer))
			{
				UINT cbTranslate = 0;

				if ( VerQueryValue(Buffer, L"\\VarFileInfo\\Translation", (LPVOID*) &lpTranslate, &cbTranslate))
				{                
					LPCWSTR lpwszBlock = 0;          
					UINT    cbSizeBuf  = 0;
					WCHAR   wzSubBlock[MAX_PATH] = {0};

					if ((cbTranslate/sizeof(struct LANGANDCODEPAGE)) > 0)   
					{
						StringCchPrintf(wzSubBlock, sizeof(wzSubBlock)/sizeof(WCHAR), 
							L"\\StringFileInfo\\%04x%04x\\CompanyName", lpTranslate[0].wLanguage, lpTranslate[0].wCodePage); 
					}

					if ( VerQueryValue(Buffer, wzSubBlock, (LPVOID*)&lpwszBlock, &cbSizeBuf))
					{
						WCHAR wzCompanyName[MAX_PATH] = {0};

						StringCchCopy(wzCompanyName, MAX_PATH/sizeof(WCHAR), (LPCWSTR)lpwszBlock);   //将系统中内存的数据拷贝到我们自己内存当中
						strCompanyName = wzCompanyName;
					}
				}
			}

			free(Buffer);
		}
	}

	return strCompanyName;
}



BOOL __stdcall HsIs32BitFile(const WCHAR * pwszFullPath)
{
	FILE * peFile = NULL;
	_wfopen_s(&peFile, pwszFullPath, L"rb");
	if (peFile == NULL)
	{
		fclose(peFile);
		return -1;
	}

	IMAGE_DOS_HEADER imageDosHeader;
	fread(&imageDosHeader, sizeof(IMAGE_DOS_HEADER), 1, peFile);
	if (imageDosHeader.e_magic != IMAGE_DOS_SIGNATURE)
	{
		fclose(peFile);
		return FALSE;
	}

	IMAGE_NT_HEADERS imageNtHeaders;
	fseek(peFile, imageDosHeader.e_lfanew, SEEK_SET);
	fread(&imageNtHeaders, sizeof(IMAGE_NT_HEADERS), 1, peFile);
	fclose(peFile);
	if (imageNtHeaders.Signature != IMAGE_NT_SIGNATURE)
	{
		return FALSE;
	}

	if (imageNtHeaders.FileHeader.Machine == IMAGE_FILE_MACHINE_I386)
	{
		return TRUE;
	}
// 	if (imageNtHeaders.FileHeader.Machine == IMAGE_FILE_MACHINE_IA64 ||
// 		imageNtHeaders.FileHeader.Machine == IMAGE_FILE_MACHINE_AMD64)
// 	{
// 		return FALSE;	//64BIT
// 	}

	return FALSE;
}



BOOL HsR3CanOpenProcess(DWORD dwPid)
{
	BOOL bOpen = TRUE;

	HsDebugPrivilege(SE_DEBUG_NAME, TRUE);

	HANDLE hProcess = OpenProcess(PROCESS_TERMINATE | PROCESS_VM_OPERATION, TRUE, dwPid);

	HsDebugPrivilege(SE_DEBUG_NAME, FALSE);
	if (hProcess)
	{
		CloseHandle(hProcess);
	}
	else
	{
		bOpen = FALSE;
	}

	return bOpen;
}



VOID HsCheckAttribute(CString strPath)
{

	if (!strPath.IsEmpty())
	{
		SHELLEXECUTEINFO ExecInfor ;
		memset(&ExecInfor, 0, sizeof(ExecInfor)) ;
		ExecInfor.fMask = SEE_MASK_INVOKEIDLIST ;
		ExecInfor.cbSize = sizeof(ExecInfor) ;
		ExecInfor.hwnd = NULL ;
		ExecInfor.lpVerb = _T( "properties" );
		ExecInfor.lpFile = strPath ;
		ExecInfor.lpParameters = NULL ;
		ExecInfor.lpDirectory = NULL ;
		ExecInfor.nShow = SW_SHOWNORMAL ;
		ExecInfor.hProcess = NULL ;
		ExecInfor.lpIDList = 0 ;
		ExecInfor.hInstApp = 0 ;
		ShellExecuteEx(&ExecInfor) ;
	}
	else
	{
		MessageBox(NULL,L"文件路径错误", NULL, MB_OK | MB_ICONWARNING);
	}
}



VOID HsLocationExplorer(CString strPath)
{
	if (!strPath.IsEmpty() && PathFileExists(strPath))
	{
		CString strCmd;
		strCmd.Format(L"/select,%s", strPath);
		ShellExecuteW(NULL, L"open", L"explorer.exe", strCmd, NULL, SW_SHOW);
	}
	else
	{
		MessageBox(NULL,L"文件路径错误", NULL, MB_OK | MB_ICONWARNING);
	}
}




BOOL HsGetDefaultTextFileName(CString& strFile)
{
	CString strTime;
	CTime Time = CTime::GetCurrentTime();
	int FileNameDay, FileNameMonth, FileNameYear, FileHour, FileMin, FileSec;

	FileNameDay = Time.GetDay();
	FileNameMonth = Time.GetMonth();
	FileNameYear = Time.GetYear();
	FileHour = Time.GetHour();
	FileMin = Time.GetMinute();
	FileSec = Time.GetSecond();
	strTime.Format(L"%04d%02d%02d%02d%02d%02d", FileNameYear, FileNameMonth, FileNameDay, FileHour, FileMin, FileSec);

	strFile = strTime + L".txt";

	CFileDialog FileDlg(
		FALSE, 
		0, 
		strFile, 
		0, 
		L"文本文件 (*.txt)|*.txt|所有文件 (*.*)|*.*||",
		0
		);

	if (IDOK == FileDlg.DoModal())
	{
		strFile = FileDlg.GetFileName();   //可以获得文件路径 并且修改

		CString strTemp = strFile.Right((int)wcslen(L".txt"));

		if (strTemp.CompareNoCase(L".txt") != 0)
		{
			strFile += L".txt";
		}

		if (!PathFileExists(strFile))
		{
			return TRUE;
		}
		else if	(PathFileExists(strFile) && MessageBox(NULL, L"文件已经存在，是否覆盖？", L"天影卫士", MB_YESNO | MB_ICONQUESTION) == IDYES)
		{
			if(!DeleteFile(strFile))
			{
				MessageBox(NULL, L"覆盖文件失败。", L"天影卫士", MB_OK | MB_ICONERROR);
				return FALSE;
			}

			return TRUE;
		}
	}

	return FALSE;
}



// CString HsGetExcelDriver()
// {
// 	WCHAR wzBuffer[0x1000] = {0};
// 	WORD  cbBufferMax = 0x1000;
// 	WORD  cbBufferOut;
// 	WCHAR *pwzBuffer = wzBuffer;
// 	CString strDriver;
// 
// 	if (!SQLGetInstalledDrivers(wzBuffer, cbBufferMax, &cbBufferOut))
// 	{
// 		return NULL;
// 	}
// 
// 	do
// 	{
// 		if (wcsstr(pwzBuffer, L"Excel") != 0)
// 		{
// 			strDriver = CString(pwzBuffer);
// 			break;
// 		}
// 
// 		pwzBuffer = wcschr(pwzBuffer, L'\0') + 1;
// 	}
// 	while (pwzBuffer[1] != L'\0');
// 
// 	return strDriver;
// }




// BOOL HsGetDefaultExcelFileName(CString& strExcelFile)
// {
// 	CString strTime;
// 	CTime Time = CTime::GetCurrentTime();
// 	int FileNameDay, FileNameMonth, FileNameYear, FileHour, FileMin, FileSec;
// 
// 	FileNameDay = Time.GetDay();
// 	FileNameMonth = Time.GetMonth();
// 	FileNameYear = Time.GetYear();
// 	FileHour = Time.GetHour();
// 	FileMin = Time.GetMinute();
// 	FileSec = Time.GetSecond();
// 	strTime.Format(L"%04d%02d%02d%02d%02d%02d", FileNameDay, FileNameMonth, FileNameYear, FileHour, FileMin, FileSec);
// 
// 	strExcelFile = strTime + L".xls";
// 
// 	CFileDialog FileDlg(
// 		FALSE, 
// 		0, 
// 		strExcelFile, 
// 		0, 
// 		L"Excel Files (*.xls)|*.xls|All Files (*.*)|*.*||",
// 		0
// 		);
// 
// 	if (IDOK == FileDlg.DoModal())
// 	{
// 		strExcelFile = FileDlg.GetFileName();
// 
// 		CString szTemp = strExcelFile.Right((int)wcslen(L".xls"));
// 
// 		if (szTemp.CompareNoCase(L".xls") != 0)
// 		{
// 			strExcelFile += L".xls";
// 		}
// 
// 		if ( !PathFileExists(strExcelFile) )
// 		{
// 			return TRUE;
// 		}
// 		else if	(PathFileExists(strExcelFile) && MessageBox(NULL, L"文件已经存在，是否覆盖？", L"导出", MB_YESNO | MB_ICONQUESTION) == IDYES)
// 		{
// 			if(!DeleteFile(strExcelFile))
// 			{
// 				MessageBox(NULL, L"覆盖文件失败！", L"导出", MB_OK | MB_ICONERROR);
// 				return FALSE;
// 			}
// 
// 			return TRUE;
// 		}
// 	}
// 
// 	return FALSE;
// }






BOOL HsDebugPrivilege(const WCHAR *pName, BOOL bEnable)
{
	BOOL              bResult = TRUE;
	HANDLE            hToken;
	TOKEN_PRIVILEGES  TokenPrivileges;

	if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY | TOKEN_ADJUST_PRIVILEGES, &hToken))
	{
		bResult = FALSE;
		return bResult;
	}
	TokenPrivileges.PrivilegeCount = 1;
	TokenPrivileges.Privileges[0].Attributes = bEnable ? SE_PRIVILEGE_ENABLED : 0;

	LookupPrivilegeValue(NULL, pName, &TokenPrivileges.Privileges[0].Luid);
	AdjustTokenPrivileges(hToken, FALSE, &TokenPrivileges, sizeof(TOKEN_PRIVILEGES), NULL, NULL);
	if (GetLastError() != ERROR_SUCCESS)
	{
		bResult = FALSE;
	}

	CloseHandle(hToken);
	return bResult;	
}







BOOL HsKillProcessByForce(CMyList* m_ListCtrl)
{
	POSITION pos = m_ListCtrl->GetFirstSelectedItemPosition();

	ULONG_PTR ulProcessId = 0;

	int nItem = 0;

	while(pos)
	{
		nItem = m_ListCtrl->GetNextSelectedItem(pos);

		ulProcessId = _ttoi(m_ListCtrl->GetItemText(nItem,HS_PROCESS_COLUMN_PID).GetBuffer());

		if (ulProcessId <= 4)
		{
			return FALSE;
		}

	} 

	if (g_hDevice==NULL)
	{
		return FALSE;
	}

	ULONG dwReturnSize = 0;
	ULONG dwRet = 0;

	BOOL bFeedback = FALSE;

	    
	dwRet = DeviceIoControl(g_hDevice,HS_IOCTL(HS_IOCTL_PROC_KILLPROCESSBYFORCE),
		&ulProcessId,
		sizeof(ULONG_PTR),
		&bFeedback,
		sizeof(BOOL),
		&dwReturnSize,
		NULL);

	//发送IO 控制码

	if (bFeedback = TRUE)
	{
		m_ListCtrl->DeleteItem(nItem);
	}
	else
	{
		MessageBox(NULL,L"关闭进程失败。",L"天影卫士",0);
	}

	bIsChecking = FALSE;

	return bFeedback;

}




