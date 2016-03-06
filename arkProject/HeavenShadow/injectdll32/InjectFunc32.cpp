#include "stdafx.h"

#include "InjectFunc32.h"

WIN_VERSION  GetWindowsVersion();
WIN_VERSION  WinVersion = WinUnknown;

typedef long (__fastcall *pfnRtlAdjustPrivilege64)(ULONG,ULONG,ULONG,PVOID);
typedef long (__fastcall *pfnRtlAdjustPrivilege32)(ULONG,ULONG,ULONG,PVOID);
pfnRtlAdjustPrivilege64 RtlAdjustPrivilege64;
pfnRtlAdjustPrivilege32 RtlAdjustPrivilege32;

static OPENFILENAME ofn ;

DWORD HsRemoteThreadInjectDll32(WCHAR* ProcessInfo)
{
	ULONG_PTR ProcessId = 0;
	WCHAR*    ProcessPath = NULL;
	WCHAR wzFileFilter[] = L"应用程序扩展 (*.dll)\0*.dll\0所有文件 (*.*)\0*.*\0";
	WCHAR wzFileChoose[] = L"打开文件";

	int i;
	int j;

	WCHAR szPid[10] = {0};

	for (i = 0; _wcsnicmp(ProcessInfo+i,L"-d ",wcslen(L"-d "))!=0 && i<=10 ;i++);

	if (i == 10)
	{
		return FALSE;
	}

	WCHAR* PidTemp = ProcessInfo + i + wcslen(L"-d ");

	j = i;

	for ( ; _wcsnicmp(ProcessInfo + j,L" -p ",wcslen(L" -p "))!=0 && j-i<=10 ;j++);

	if (j-i==10)
	{
		return FALSE;
	}

	memcpy(szPid,PidTemp,j-i);

	ProcessId   = _ttoi(szPid);

	ProcessPath = ProcessInfo + j + wcslen(L" -p ");

	static TCHAR szFileName[MAX_PATH], szTitleName[MAX_PATH] ;
	HWND hwnd;
	hwnd=GetForegroundWindow(); //获取前台窗口句柄。本程序中的前台窗口就是控制台窗口。 
	PopFileInitialize (hwnd);   //初始化ofn
	PopFileOpenDlg(hwnd, szFileName, szTitleName);//打开文件对话框

	CString strDllPath = szFileName;

	BOOL bResult = FALSE;

	if (PathFileExists(strDllPath))
	{
		if (HsIs32BitFile(ProcessPath)==TRUE &&
			HsIs32BitFile(strDllPath.GetBuffer())==TRUE)
		{
			bResult = HsInjectDll(TRUE,&strDllPath,ProcessId);
		}
	}

	if (bResult == FALSE)
	{
		::MessageBox(NULL,L"远程线程注入失败。",L"天影卫士",MB_SYSTEMMODAL|MB_OK);

	}
	else
	{
		::MessageBox(NULL,L"远程线程注入成功。",L"天影卫士",MB_SYSTEMMODAL|MB_OK);
	}

	return bResult;
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





//改程序编译成64位可以注入64位 编译成32位可以注入32位

BOOL HsInjectDll(BOOL Is32Bit, CString* strDllPath, ULONG_PTR ProcessID)
{
	BOOL bResult = FALSE;

	if (ProcessID <= 0)
	{
		return FALSE;
	}


	if (PathFileExists(*strDllPath))
	{
		WinVersion = GetWindowsVersion();

		switch(WinVersion)
		{
		case Windows7:   //注意我们这里针对的是64位的Win7
			{

				WCHAR wzPath[MAX_PATH] = {0};
				wcscpy_s(wzPath, strDllPath->GetBuffer());
				strDllPath->ReleaseBuffer();

				HsDebugPrivilege(SE_DEBUG_NAME, TRUE);
				bResult = HsInjectDllByRemoteThreadWin7(wzPath,ProcessID,Is32Bit);
				HsDebugPrivilege(SE_DEBUG_NAME, FALSE);

				break;
			}

		case WindowsXP:  //这里是针对的32位的XP
			{
				WCHAR wzPath[MAX_PATH] = {0};
				wcscpy_s(wzPath, strDllPath->GetBuffer());

				strDllPath->ReleaseBuffer();

				bResult = HsInjectDllByRemoteThreadWinXP(wzPath,ProcessID);

				break;
			}
		}
	}
	return bResult;
}


BOOL HsInjectDllByRemoteThreadWin7(const TCHAR* wzDllFile, ULONG_PTR ProcessId, BOOL Is32Bit)
{
	if (NULL == wzDllFile || 0 == ::_tcslen(wzDllFile) || ProcessId == 0 || -1 == _taccess(wzDllFile, 0))
	{
		return FALSE;
	}
	HANDLE                 hProcess = NULL;
	HANDLE                 hThread  = NULL;
	DWORD                  dwRetVal    = 0;
	LPTHREAD_START_ROUTINE FuncAddress = NULL;
	DWORD  dwSize = 0;
	TCHAR* VirtualAddress = NULL;

#ifdef _UNICODE
	FuncAddress = (PTHREAD_START_ROUTINE)::GetProcAddress(::GetModuleHandle(_T("Kernel32")), "LoadLibraryW");
#else
	FuncAddress = (PTHREAD_START_ROUTINE)::GetProcAddress(::GetModuleHandle(_T("Kernel32")), "LoadLibraryA");
#endif

	if (FuncAddress==NULL)
	{
		return FALSE;
	}

	hProcess = OpenProcess(PROCESS_ALL_ACCESS,FALSE, (DWORD)ProcessId);

	if (NULL == hProcess)
	{
		printf("Open Process Fail\r\n");
		return FALSE;
	}

	// 在目标进程中分配内存空间
	dwSize = (DWORD)::_tcslen(wzDllFile) + 1;
	VirtualAddress = (TCHAR*)::VirtualAllocEx(hProcess, NULL, dwSize * sizeof(TCHAR), MEM_COMMIT, PAGE_READWRITE);  
	if (NULL == VirtualAddress)
	{

		printf("Virtual Process Memory Fail\r\n");
		CloseHandle(hProcess);
		return FALSE;
	}

	// 在目标进程的内存空间中写入所需参数(模块名)
	if (FALSE == ::WriteProcessMemory(hProcess, VirtualAddress, (LPVOID)wzDllFile, dwSize * sizeof(TCHAR), NULL))
	{
		printf("Write Data Fail\r\n");
		VirtualFreeEx(hProcess, VirtualAddress, dwSize, MEM_DECOMMIT);
		CloseHandle(hProcess);
		return FALSE;
	}

	hThread = ::CreateRemoteThread(hProcess, NULL, 0, FuncAddress, VirtualAddress, 0, NULL);
	if (NULL == hThread)
	{
		printf("CreateRemoteThread Fail\r\n");

		VirtualFreeEx(hProcess, VirtualAddress, dwSize, MEM_DECOMMIT);
		CloseHandle(hProcess);
		return FALSE;
	}


	// 等待远程线程结束
	WaitForSingleObject(hThread, INFINITE);
	// 清理
	VirtualFreeEx(hProcess, VirtualAddress, dwSize, MEM_DECOMMIT);
	CloseHandle(hThread);
	CloseHandle(hProcess);

	return TRUE;

}




BOOL HsInjectDllByRemoteThreadWinXP(const TCHAR* wzDllFile, ULONG_PTR ProcessId)
{
	//提高本进程的权限

	if (!HsInjectUpPrivilige())  //提权
	{
		printf("Up Privilige Is Error\n");
		return FALSE;
	}

	CStringA *Dll = new CStringA(wzDllFile);


	//我们就要打开想要打开的进程
	HANDLE hProcess = NULL;
	HANDLE hThread  = NULL;

	hProcess = OpenProcess(PROCESS_ALL_ACCESS,false,(DWORD)ProcessId);  //explorer.exe  hProcess
	if (hProcess==NULL)
	{
		printf("Open Process Is Error\n");

		return FALSE;
	}

	char* szDllName = NULL;

	szDllName = (char*)VirtualAllocEx(hProcess,
		NULL,Dll->GetLength()+1,MEM_COMMIT,PAGE_READWRITE);   

	if (szDllName==NULL)
	{

		CloseHandle(hProcess);
		printf("Apply Memory Is Error\n");

		return FALSE;
	}

	//然后将完整的路径直接写入内存

	if (!WriteProcessMemory(hProcess,szDllName,Dll->GetBuffer(),Dll->GetLength()+1,NULL))
	{

		CloseHandle(hProcess);

		printf("Write Memory Is Error\n");

		return FALSE;
	}

	LPTHREAD_START_ROUTINE StartRoutine = NULL;

	StartRoutine = 
		(LPTHREAD_START_ROUTINE)GetProcAddress(GetModuleHandle(TEXT("Kernel32")),
		"LoadLibraryA");

	if(StartRoutine == NULL)
	{
		printf("Get ProcAddress Error\n");
		return FALSE;
	}

	hThread = CreateRemoteThread(hProcess,NULL,0,StartRoutine,szDllName,0,NULL);

	if (hThread==NULL)
	{
		CloseHandle(hProcess);

		printf("Create Remote Thread Is Error\n");

		return FALSE;
	}

	WaitForSingleObject(hThread, INFINITE);
	CloseHandle(hThread);
	VirtualFreeEx(hProcess,szDllName,0,MEM_RELEASE);
	CloseHandle(hProcess);

	return TRUE;
}


BOOL HsInjectUpPrivilige()	//XP
{

	HANDLE hToken = NULL;

	_TOKEN_PRIVILEGES tp = {0};

	LUID luid={0};


	if (!OpenProcessToken(GetCurrentProcess(),TOKEN_ALL_ACCESS,&hToken))
	{
		printf("OpenProcess Is Error\n");

		return FALSE;
	}

	if (!LookupPrivilegeValue(NULL,SE_DEBUG_NAME,&luid))
	{

		printf("Lookup Is Error\n");

		return FALSE;
	}

	tp.PrivilegeCount = 1;
	tp.Privileges[0].Luid = luid;
	tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;


	if (!AdjustTokenPrivileges(hToken,FALSE,&tp,sizeof(TOKEN_PRIVILEGES),NULL,NULL))
	{
		printf("Adjust Is Error\n");

		return FALSE;
	}

	return TRUE;
}


/*
BOOL HsInjectDllByRemoteThreadWinXP(const TCHAR* wzDllFile, ULONG_PTR ProcessId)
{
// 参数无效
if (NULL == wzDllFile || 0 == ::_tcslen(wzDllFile) || ProcessId == 0 || -1 == _taccess(wzDllFile, 0))
{

return FALSE;
}

HANDLE hProcess = NULL;
HANDLE hThread  = NULL;
DWORD dwSize = 0;
TCHAR* VirtualAddress = NULL;
LPTHREAD_START_ROUTINE FuncAddress = NULL;

// 获取目标进程句柄
hProcess = OpenProcess(PROCESS_CREATE_THREAD | PROCESS_VM_OPERATION | PROCESS_VM_WRITE, FALSE, (DWORD)ProcessId);
if (NULL == hProcess)
{
printf("Open Process Fail\r\n");
return FALSE;
}

// 在目标进程中分配内存空间
dwSize = (DWORD)::_tcslen(wzDllFile) + 1;
VirtualAddress = (TCHAR*)::VirtualAllocEx(hProcess, NULL, dwSize * sizeof(TCHAR), MEM_COMMIT, PAGE_READWRITE);
if (NULL == VirtualAddress)
{

printf("Virtual Process Memory Fail\r\n");
CloseHandle(hProcess);
return FALSE;
}

// 在目标进程的内存空间中写入所需参数(模块名)
if (FALSE == ::WriteProcessMemory(hProcess, VirtualAddress, (LPVOID)wzDllFile, dwSize * sizeof(TCHAR), NULL))
{
printf("Write Data Fail\r\n");
VirtualFreeEx(hProcess, VirtualAddress, dwSize, MEM_DECOMMIT);
CloseHandle(hProcess);
return FALSE;
}

// 从 Kernel32.dll 中获取 LoadLibrary 函数地址
#ifdef _UNICODE
FuncAddress = (PTHREAD_START_ROUTINE)::GetProcAddress(::GetModuleHandle(_T("Kernel32")), "LoadLibraryW");
#else
FuncAddress = (PTHREAD_START_ROUTINE)::GetProcAddress(::GetModuleHandle(_T("Kernel32")), "LoadLibraryA");
#endif

if (NULL == FuncAddress)
{
printf("Get LoadLibrary Fail\r\n");
VirtualFreeEx(hProcess, VirtualAddress, dwSize, MEM_DECOMMIT);
CloseHandle(hProcess);
return false;
}

// 创建远程线程调用 LoadLibrary
hThread = ::CreateRemoteThread(hProcess, NULL, 0, FuncAddress, VirtualAddress, 0, NULL);
if (NULL == hThread)
{
printf("CreateRemoteThread Fail\r\n");

VirtualFreeEx(hProcess, VirtualAddress, dwSize, MEM_DECOMMIT);
CloseHandle(hProcess);
return FALSE;
}

// 等待远程线程结束
WaitForSingleObject(hThread, INFINITE);
// 清理
VirtualFreeEx(hProcess, VirtualAddress, dwSize, MEM_DECOMMIT);
CloseHandle(hThread);
CloseHandle(hProcess);

return TRUE;
}


*/




WIN_VERSION  GetWindowsVersion()
{
	OSVERSIONINFOEX	OsVerInfoEx;
	OsVerInfoEx.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
	GetVersionEx((OSVERSIONINFO *)&OsVerInfoEx); // 注意转换类型

	switch (OsVerInfoEx.dwPlatformId)
	{
	case VER_PLATFORM_WIN32_NT:
		{
			if (OsVerInfoEx.dwMajorVersion <= 4 )
			{
				return WindowsNT;
			}
			if (OsVerInfoEx.dwMajorVersion == 5 && OsVerInfoEx.dwMinorVersion == 0)
			{
				return Windows2000;
			}

			if (OsVerInfoEx.dwMajorVersion == 5 && OsVerInfoEx.dwMinorVersion == 1)
			{
				return WindowsXP;
			}
			if (OsVerInfoEx.dwMajorVersion == 5 && OsVerInfoEx.dwMinorVersion == 2)
			{
				return Windows2003;
			}
			if (OsVerInfoEx.dwMajorVersion == 6 && OsVerInfoEx.dwMinorVersion == 0)
			{
				return WindowsVista;
			}

			if (OsVerInfoEx.dwMajorVersion == 6 && OsVerInfoEx.dwMinorVersion == 1)
			{
				return Windows7;
			}
			if (OsVerInfoEx.dwMajorVersion == 6 && OsVerInfoEx.dwMinorVersion == 2 )
			{
				return Windows8;
			}
			if (OsVerInfoEx.dwMajorVersion == 6 && OsVerInfoEx.dwMinorVersion == 3 )
			{
				return Windows8_1;
			}
			if (OsVerInfoEx.dwMajorVersion == 10 && OsVerInfoEx.dwMinorVersion == 0 )
			{
				return Windows10;
			}

			break;
		}

	default:
		{
			return WinUnknown;
		}
	}

	return WinUnknown;
}









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


void PopFileInitialize (HWND hwnd)
{
	static TCHAR szFilter[] = TEXT ("应用程序扩展 (*.dll)\0*.dll\0\0");

	ofn.lStructSize       = sizeof (OPENFILENAME) ;
	ofn.hwndOwner         = hwnd ;
	ofn.hInstance         = NULL ;
	ofn.lpstrFilter       = szFilter ;
	ofn.lpstrCustomFilter = NULL ;
	ofn.nMaxCustFilter    = 0 ;
	ofn.nFilterIndex      = 0 ;
	ofn.lpstrFile         = NULL ;          // Set in Open and Close functions
	ofn.nMaxFile          = MAX_PATH ;
	ofn.lpstrFileTitle    = NULL ;          // Set in Open and Close functions
	ofn.nMaxFileTitle     = MAX_PATH ;
	ofn.lpstrInitialDir   = NULL ;
	ofn.lpstrTitle        = NULL ;
	ofn.Flags             = 0 ;             // Set in Open and Close functions
	ofn.nFileOffset       = 0 ;
	ofn.nFileExtension    = 0 ;
	ofn.lpstrDefExt       = TEXT ("dll") ;
	ofn.lCustData         = 0L ;
	ofn.lpfnHook          = NULL ;
	ofn.lpTemplateName    = NULL ;
}

BOOL PopFileOpenDlg (HWND hwnd, PTSTR pstrFileName, PTSTR pstrTitleName)
{
	ofn.hwndOwner         = hwnd ;
	ofn.lpstrFile         = pstrFileName ;
	ofn.lpstrFileTitle    = pstrTitleName ;
	ofn.Flags             = OFN_HIDEREADONLY | OFN_CREATEPROMPT ;

	return GetOpenFileName (&ofn) ;
}
