#include "stdafx.h"
#include "PModuleFunc.h"
#include "Common.h"

#include <Psapi.h>
#include "resource.h"


#pragma comment(lib,"psapi.lib")

extern WIN_VERSION GetWindowsVersion();
extern WIN_VERSION WinVersion;

extern ULONG_PTR g_ulProcessId;



COLUMNSTRUCT g_Column_ProcessModule[] = 
{
	{	L"地址",					125	},
	{	L"大小",					70	},
	{	L"入口地址",				125	},
	{	L"模块名称",				270	}
};

UINT g_Column_ProcessModule_Count  = 4;	  //进程列表列数



extern int dpix;
extern int dpiy;




VOID HsInitPModuleDetailList(CListCtrl *m_ListCtrl)
{
	while(m_ListCtrl->DeleteColumn(0));
	m_ListCtrl->DeleteAllItems();

	m_ListCtrl->SetExtendedStyle(LVS_EX_FULLROWSELECT|LVS_EX_GRIDLINES|LVS_EX_HEADERDRAGDROP);

	UINT i;
	for (i = 0;i<g_Column_ProcessModule_Count;i++)
	{
		if (i==1)
		{
			m_ListCtrl->InsertColumn(i, g_Column_ProcessModule[i].szTitle,LVCFMT_RIGHT,(int)(g_Column_ProcessModule[i].nWidth*(dpix/96.0)));
		}
		else
		{
			m_ListCtrl->InsertColumn(i, g_Column_ProcessModule[i].szTitle,LVCFMT_LEFT,(int)(g_Column_ProcessModule[i].nWidth*(dpix/96.0)));
		}
	}


	if (HsIs64BitWindows())
	{
		EnableDebugPri64();
	}
	else
	{
		EnableDebugPri32();
	}
}

VOID HsLoadPModuleDetailList(CListCtrl *m_ListCtrl)
{
	PROCESSMODULE_INFO mi[1024] = {0};

	m_ListCtrl->DeleteAllItems();


	ULONG_PTR ulCount = EnumProcessModule(mi);

	CString  strBase;
	CString  strSize;
	CString  strEntryPoint;


	if (ulCount>0)
	{
		UINT i = 0;
		for (i=0;i<ulCount;i++)
		{


			strBase.Format(L"0x%p",mi[i].ulBase);
			strSize.Format(L"0x%X",mi[i].ulSize);
			strEntryPoint.Format(L"0x%p",mi[i].ulEntryPoint);
			CString  strPathName(mi[i].szPath);


			int n = m_ListCtrl->InsertItem(m_ListCtrl->GetItemCount(),strBase);
			m_ListCtrl->SetItemText(n,1,strSize);
			m_ListCtrl->SetItemText(n,2,strEntryPoint);
			m_ListCtrl->SetItemText(n,3,strPathName);

		}
	}

}

BOOL EnableDebugPri64()
{
	typedef long (__fastcall *pfnRtlAdjustPrivilege64)(ULONG,ULONG,ULONG,PVOID);
	pfnRtlAdjustPrivilege64 RtlAdjustPrivilege;

	DWORD                  dwRetVal    = 0;
	LPTHREAD_START_ROUTINE FuncAddress = NULL;
#ifdef _UNICODE
	FuncAddress = (PTHREAD_START_ROUTINE)::GetProcAddress(::GetModuleHandle(_T("Kernel32")), "LoadLibraryW");
#else
	FuncAddress = (PTHREAD_START_ROUTINE)::GetProcAddress(::GetModuleHandle(_T("Kernel32")), "LoadLibraryA");
#endif

	if (FuncAddress==NULL)
	{
		return FALSE;
	}


	RtlAdjustPrivilege=(pfnRtlAdjustPrivilege64)GetProcAddress((HMODULE)(FuncAddress(L"ntdll.dll")),"RtlAdjustPrivilege");

	if (RtlAdjustPrivilege==NULL)
	{
		return FALSE;
	}
	RtlAdjustPrivilege(20,1,0,&dwRetVal);

	return TRUE;
}



BOOL EnableDebugPri32()
{

	HANDLE hToken;
	TOKEN_PRIVILEGES pTP;
	LUID uID;

	if (!OpenProcessToken(GetCurrentProcess(),TOKEN_ADJUST_PRIVILEGES|TOKEN_QUERY,&hToken))
	{
		printf("OpenProcessToken is Error\n");

		return FALSE;
	}

	if (!LookupPrivilegeValue(NULL,SE_DEBUG_NAME,&uID))
	{
		printf("LookupPrivilegeValue is Error\n");

		return FALSE;
	}


	pTP.PrivilegeCount = 1;
	pTP.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
	pTP.Privileges[0].Luid = uID;


	//在这里我们进行调整权限
	if (!AdjustTokenPrivileges(hToken,false,&pTP,sizeof(TOKEN_PRIVILEGES),NULL,NULL))
	{
		printf("AdjuestTokenPrivileges is Error\n");
		return  FALSE;
	}


	return TRUE;

}





ULONG_PTR EnumProcessModule(_PROCESSMODULE_INFO* mi)
{

	MODULEINFO ModInfor;
	char szModName[MAX_PATH];
	HMODULE hMods[1024];
	DWORD cbNeeded,i;
	HANDLE hProcess;
	hProcess = OpenProcess(PROCESS_ALL_ACCESS,0,(DWORD)g_ulProcessId);
	if (hProcess==0)
	{
		return 0;
	}
	if(EnumProcessModules(hProcess, hMods, sizeof(hMods), &cbNeeded))
	{
		for (i=0; i<=cbNeeded/sizeof(HMODULE); i++ )
		{
			GetModuleInformation(hProcess, hMods[i], &ModInfor, sizeof(MODULEINFO));
			GetModuleFileNameExA(hProcess, hMods[i], szModName, 260);
			mi[i].ulBase=(ULONG_PTR)(ModInfor.lpBaseOfDll);
			mi[i].ulSize=(ULONG_PTR)(ModInfor.SizeOfImage);
			mi[i].ulEntryPoint=(ULONG_PTR)(ModInfor.EntryPoint);
			strcpy_s(mi[i].szPath,szModName);
		}
		CloseHandle(hProcess);
		return cbNeeded/sizeof(HMODULE);
	}
	else
	{
		CloseHandle(hProcess);
		return 0;
	}
}
