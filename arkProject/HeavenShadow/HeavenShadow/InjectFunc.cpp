#include "stdafx.h"
#include "InjectFunc.h"
#include "Common.h"
#include "ProcessFunc.h"

WIN_VERSION  GetWindowsVersion();
WIN_VERSION  WinVersion = WinUnknown;

typedef long (__fastcall *pfnRtlAdjustPrivilege64)(ULONG,ULONG,ULONG,PVOID);
typedef long (__fastcall *pfnRtlAdjustPrivilege32)(ULONG,ULONG,ULONG,PVOID);
pfnRtlAdjustPrivilege64 RtlAdjustPrivilege64;
pfnRtlAdjustPrivilege32 RtlAdjustPrivilege32;

DWORD WINAPI HsRemoteThreadInjectDll(CListCtrl *m_ListCtrl)
{
	CString ProcessPath;
	ULONG_PTR ProcessId = 0;
	CString CmdLine;

	POSITION pos = m_ListCtrl->GetFirstSelectedItemPosition();

	while(pos)
	{
		int nItem = m_ListCtrl->GetNextSelectedItem(pos);

		ProcessPath = m_ListCtrl->GetItemText(nItem,HS_PROCESS_COLUMN_PATH);
		ProcessId = _ttoi(m_ListCtrl->GetItemText(nItem,HS_PROCESS_COLUMN_PID));

		CmdLine += L"-d ";
		CmdLine += m_ListCtrl->GetItemText(nItem,HS_PROCESS_COLUMN_PID);
		CmdLine += L" -p ";
		CmdLine += ProcessPath;
	}
	
	


	if (ProcessId<=4)
	{
		return FALSE;
	}

	if (HsIs32BitFile(ProcessPath.GetBuffer())==TRUE)
	{
		WinVersion = GetWindowsVersion();

		if (WinVersion >= WindowsVista)
		{
			WCHAR wzInjectPath[260] = {0};
			WCHAR* p = NULL;
			CString InjectPath;
			HMODULE module = GetModuleHandle(0);
			GetModuleFileName(module,wzInjectPath,sizeof(wzInjectPath));
			p = wcsrchr(wzInjectPath,L'\\');
			*p = 0;
			InjectPath = wzInjectPath;
			InjectPath += L"\\injectdll32.exe";

			SHELLEXECUTEINFO sei = {sizeof(SHELLEXECUTEINFO)};
			sei.lpVerb = L"runas";
			sei.lpFile = InjectPath.GetBuffer();
			sei.hwnd = GetDesktopWindow();
			sei.nShow = SW_NORMAL;
			sei.lpParameters = CmdLine.GetBuffer();

			if (!ShellExecuteEx(&sei))
			{
				MessageBox(NULL,L"注入失败",L"注入失败",0);
			}
		}
		else
		{
			ShellExecuteW(NULL,
				L"open",
				L"injectdll32.exe",
				CmdLine.GetBuffer(),
				L"",
				SW_SHOWMAXIMIZED);
		}

	}
	else
	{
		if (WinVersion >= WindowsVista)
		{
			WCHAR wzInjectPath[260] = {0};
			WCHAR* p = NULL;
			CString InjectPath;
			HMODULE module = GetModuleHandle(0);
			GetModuleFileName(module,wzInjectPath,sizeof(wzInjectPath));
			p = wcsrchr(wzInjectPath,L'\\');
			*p = 0;
			InjectPath = wzInjectPath;
			InjectPath += L"\\injectdll64.exe";

			SHELLEXECUTEINFO sei = {sizeof(SHELLEXECUTEINFO)};
			sei.lpVerb = L"runas";
			sei.lpFile = InjectPath.GetBuffer();
			sei.hwnd = GetDesktopWindow();
			sei.nShow = SW_NORMAL;
			sei.lpParameters = CmdLine.GetBuffer();

			if (!ShellExecuteEx(&sei))
			{
				MessageBox(NULL,L"注入失败",L"注入失败",0);
			}

		}
		else
		{
			ShellExecuteW(NULL,
				L"open",
				L"injectdll64.exe",
				CmdLine.GetBuffer(),
				L"",
				SW_SHOWMAXIMIZED);
		}
	}


	
	return TRUE;

	WCHAR wzFileFilter[] = L"应用程序扩展 (*.dll)\0*.dll\0所有文件 (*.*)\0*.*\0";
	WCHAR wzFileChoose[] = L"打开文件";


	CFileDialog FileDlg(TRUE);
	FileDlg.m_ofn.lpstrTitle  = wzFileChoose;
	FileDlg.m_ofn.lpstrFilter = wzFileFilter;

	if (IDOK != FileDlg.DoModal())
	{
		return FALSE;
	}

	

	CString strDllPath = FileDlg.GetPathName();

	BOOL bResult = FALSE;

	if (PathFileExists(strDllPath) && ProcessId > 4)   //注意这里的判断如果是要选择64位的Dll就要编译成64位
	{
		if (HsIs32BitFile(ProcessPath.GetBuffer())==TRUE &&
			HsIs32BitFile(strDllPath.GetBuffer())==TRUE)
		{
			bResult = HsInjectDll(TRUE,&strDllPath,ProcessId);
		}
		else if (HsIs32BitFile(ProcessPath.GetBuffer())==FALSE &&
			HsIs32BitFile(strDllPath.GetBuffer())==FALSE)
		{
			bResult = HsInjectDll(FALSE,&strDllPath,ProcessId);
		}
	}

	if (bResult == FALSE)
	{
		::MessageBox(NULL,L"远程线程注入失败。",L"天影卫士",0);

	}
	else
	{
		::MessageBox(NULL,L"远程线程注入成功。",L"天影卫士",0);
	}

	return bResult;
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

				bResult = HsInjectDllByRemoteThreadWin7(wzPath,ProcessID,Is32Bit);

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

	if (Is32Bit == FALSE)
	{
		RtlAdjustPrivilege64=(pfnRtlAdjustPrivilege64)GetProcAddress((HMODULE)(FuncAddress(L"ntdll.dll")),"RtlAdjustPrivilege");

		if (RtlAdjustPrivilege64==NULL)
		{
			return FALSE;
		}
		RtlAdjustPrivilege64(20,1,0,&dwRetVal);  //19
	}
	else
	{
		RtlAdjustPrivilege32=(pfnRtlAdjustPrivilege32)GetProcAddress((HMODULE)(FuncAddress(L"ntdll.dll")),"RtlAdjustPrivilege");

		if (RtlAdjustPrivilege32==NULL)
		{
			return FALSE;
		}
		RtlAdjustPrivilege32(20,1,0,&dwRetVal);  //19
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



