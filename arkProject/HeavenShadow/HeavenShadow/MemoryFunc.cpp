#include "stdafx.h"
#include "MemoryFunc.h"
#include "Common.h"
#include <vector>
#include "resource.h"

using namespace std;

typedef struct _MODULE_INFO_
{
	ULONG_PTR Base;
	ULONG_PTR Size;
	WCHAR Path[MAX_PATH]; 
}MODULE_INFO, *PMODULE_INFO;

typedef struct _ALL_MODULES_
{
	ULONG_PTR   ulCount;
	MODULE_INFO Modules[1];
}ALL_MODULES, *PALL_MODULES;


vector<PROTECT> m_vectorProtectType;
vector<MODULE_INFO> m_vectorModules;

extern ULONG_PTR g_ulProcessId;
extern HANDLE g_hDevice;
CMyList* g_ListCtrl; 

COLUMNSTRUCT g_Column_Memory[] = 
{
	{	L"内存地址",			125	},
	{	L"内存大小",			80	},
	{	L"属性",				100	},
	{	L"状态",				80	},
	{	L"类型",				80	},
	{	L"模块名",			125	}
};

UINT g_Column_Memory_Count  = 6;	  //进程列表列数

extern int dpix;
extern int dpiy;



void HsInitMemoryList(CMyList *m_ListCtrl)
{
	g_ListCtrl = m_ListCtrl;

	InitMemoryProtect();

	while(m_ListCtrl->DeleteColumn(0));
	m_ListCtrl->DeleteAllItems();

	m_ListCtrl->SetExtendedStyle(LVS_EX_FULLROWSELECT|LVS_EX_GRIDLINES|LVS_EX_HEADERDRAGDROP);

	UINT i;
	for (i = 0;i<g_Column_Memory_Count;i++)
	{
		if (i == 1)
		{
			m_ListCtrl->InsertColumn(i, g_Column_Memory[i].szTitle,LVCFMT_RIGHT,(int)(g_Column_Memory[i].nWidth*(dpix/96.0)));
		}
		else
		{
			m_ListCtrl->InsertColumn(i, g_Column_Memory[i].szTitle,LVCFMT_LEFT,(int)(g_Column_Memory[i].nWidth*(dpix/96.0)));
		}
		
	}
}



BOOL HsQueryProcessMemory(CMyList *m_ListCtrl)
{
	m_ListCtrl->DeleteAllItems();

	if (g_ulProcessId==0)
	{
		return FALSE;
	}

	SendIoControlCodeModule(g_ulProcessId);
	SendIoControlCodeMemory(g_ulProcessId);

	return TRUE;
}


BOOL SendIoControlCodeModule(ULONG_PTR ProcessID)
{
	ULONG ulReturnSize = 0;
	BOOL  bRet = 0;
	ULONG ulCount = 0x10;
	PALL_MODULES AllModules = NULL;

	do 
	{

		ULONG ulSize = 0;

		if (AllModules)
		{
			free(AllModules);
			AllModules = NULL;
		}

		ulSize = sizeof(ALL_MODULES) + ulCount * sizeof(MODULE_INFO);

		AllModules = (PALL_MODULES)malloc(ulSize);
		if (!AllModules)
		{
			break;
		}

		memset(AllModules,0,ulSize);


		bRet = DeviceIoControl(g_hDevice,HS_IOCTL(HS_IOCTL_PROC_PROCESSMODULE),
			&ProcessID,
			sizeof(ULONG_PTR),
			AllModules,
			ulSize,
			&ulReturnSize,
			NULL);


		ulCount = (ULONG)AllModules->ulCount + 1000;

	} while (bRet == FALSE && GetLastError() == ERROR_INSUFFICIENT_BUFFER);


	if (bRet==0)
	{


		return bRet;
	}

	if (bRet && AllModules->ulCount > 0)
	{

		for (ULONG i=0;i<AllModules->ulCount;i++)
		{
			MODULE_INFO Temp;

			Temp.Base = AllModules->Modules[i].Base;
			Temp.Size = AllModules->Modules[i].Size;
			CString szPath = TrimPath(AllModules->Modules[i].Path);

			wcsncpy_s(Temp.Path, MAX_PATH, szPath.GetBuffer(), szPath.GetLength());
			szPath.ReleaseBuffer();

			m_vectorModules.push_back(Temp);

		}
	}


	if (AllModules!=NULL)
	{
		free(AllModules);
		AllModules = NULL;
	}
	return bRet;
}













BOOL SendIoControlCodeMemory(ULONG_PTR ProcessID)
{
	ULONG ulReturnSize = 0;
	BOOL  bRet = 0;
	ULONG ulCount = 0x10;
	PALL_MEMORYS Memorys = NULL;

	do 
	{

		ULONG ulSize = 0;

		if (Memorys)
		{
			free(Memorys);
			Memorys = NULL;
		}

		ulSize = sizeof(ALL_MEMORYS) + ulCount * sizeof(MEMORY_INFO);

		Memorys = (PALL_MEMORYS)malloc(ulSize);
		if (!Memorys)
		{
			break;
		}

		memset(Memorys,0,ulSize);


		bRet = DeviceIoControl(g_hDevice,HS_IOCTL(HS_IOCTL_PROC_PROCESSMEMORY),
			&ProcessID,
			sizeof(ULONG_PTR),
			Memorys,
			ulSize,
			&ulReturnSize,
			NULL);


		ulCount = (ULONG)Memorys->ulCount + 1000;

	} while (bRet == FALSE && GetLastError() == ERROR_INSUFFICIENT_BUFFER);


	if (bRet==0)
	{


		return bRet;
	}

	if (bRet && Memorys->ulCount > 0)
	{

		for (ULONG i=0;i<Memorys->ulCount;i++)
		{
			CString strBase, strSize, strProtect, strType, strImageName, strState;

			strBase.Format(L"0x%08p", Memorys->Memorys[i].ulBase);
			strSize.Format(L"0x%X",  Memorys->Memorys[i].ulSize);
			strProtect = GetMemoryProtect(Memorys->Memorys[i].ulProtect);
			strType = GetMemoryType(Memorys->Memorys[i].ulType);
			strState = GetMemoryState(Memorys->Memorys[i].ulState);

			if (Memorys->Memorys[i].ulType == MEM_IMAGE)
			{
				strImageName = GetModuleImageName( Memorys->Memorys[i].ulBase);  //在我们的另一个数据结构中查找模块的名称
			}

			int n = g_ListCtrl->GetItemCount();
			int j = g_ListCtrl->InsertItem(n, strBase);
			g_ListCtrl->SetItemText(j, MemorySize, strSize);
			g_ListCtrl->SetItemText(j, MemoryProtect, strProtect);
			g_ListCtrl->SetItemText(j, MemoryState, strState);
			g_ListCtrl->SetItemText(j, MemoryType, strType);
			g_ListCtrl->SetItemText(j, MmeoryModuleName, strImageName);

			g_ListCtrl->SetItemData(j,j);

		}
	}


	if (Memorys!=NULL)
	{
		free(Memorys);
		Memorys = NULL;
	}


	m_vectorProtectType.clear();
	m_vectorModules.clear();
	return bRet;
}


CString GetModuleImageName(ULONG_PTR ulBase)
{
	CString strImageName = L"";

	for ( vector <MODULE_INFO>::iterator Iter = m_vectorModules.begin( ); Iter != m_vectorModules.end( ); Iter++)
	{	
		MODULE_INFO entry = *Iter;

		if (ulBase >= entry.Base && ulBase <= (entry.Base + entry.Size))
		{
			CString strPath = entry.Path;
			strImageName = strPath.Right(strPath.GetLength() - strPath.ReverseFind('\\') - 1);
			break;
		}
	}

	return strImageName;
}



CString GetMemoryState(ULONG State)
{
	CString szState = L"";

	if (State == MEM_COMMIT)
	{
		szState = L"Commit";
	}
	else if (State == MEM_FREE)
	{
		szState = L"Free";
	}
	else if (State == MEM_RESERVE)
	{
		szState = L"Reserve";
	}
	else if (State == MEM_DECOMMIT)
	{
		szState = L"Decommit";
	}
	else if (State == MEM_RELEASE)
	{
		szState = L"Release";
	}

	return szState;
}


CString GetMemoryType(ULONG Type)
{
	CString szType = L"";

	if (Type == MEM_PRIVATE)
	{
		szType = L"Private";
	}
	else if (Type == MEM_MAPPED)
	{
		szType = L"Map";
	}
	else if (Type == MEM_IMAGE)
	{
		szType = L"Image";
	}

	return szType;
}




VOID InitMemoryProtect()
{
	PROTECT protect;

	memset(&protect, 0, sizeof(PROTECT));
	protect.uType = PAGE_NOACCESS;
	wcsncpy_s(protect.szTypeName, L"No Access", wcslen(L"No Access"));
	m_vectorProtectType.push_back(protect);

	memset(&protect, 0, sizeof(PROTECT));
	protect.uType = PAGE_READONLY;
	wcsncpy_s(protect.szTypeName, L"Read", wcslen(L"Read"));
	m_vectorProtectType.push_back(protect);

	memset(&protect, 0, sizeof(PROTECT));
	protect.uType = PAGE_READWRITE;
	wcsncpy_s(protect.szTypeName, L"ReadWrite", wcslen(L"ReadWrite"));
	m_vectorProtectType.push_back(protect);

	memset(&protect, 0, sizeof(PROTECT));
	protect.uType = PAGE_WRITECOPY;
	wcsncpy_s(protect.szTypeName, L"WriteCopy", wcslen(L"WriteCopy"));
	m_vectorProtectType.push_back(protect);

	memset(&protect, 0, sizeof(PROTECT));
	protect.uType = PAGE_EXECUTE;
	wcsncpy_s(protect.szTypeName, L"Execute", wcslen(L"Execute"));
	m_vectorProtectType.push_back(protect);

	memset(&protect, 0, sizeof(PROTECT));
	protect.uType = PAGE_EXECUTE_READ;
	wcsncpy_s(protect.szTypeName, L"ReadExecute", wcslen(L"ReadExecute"));
	m_vectorProtectType.push_back(protect);

	memset(&protect, 0, sizeof(PROTECT));
	protect.uType = PAGE_EXECUTE_READWRITE;
	wcsncpy_s(protect.szTypeName, L"ReadWriteExecute", wcslen(L"ReadWriteExecute"));
	m_vectorProtectType.push_back(protect);

	memset(&protect, 0, sizeof(PROTECT));
	protect.uType = PAGE_EXECUTE_WRITECOPY;
	wcsncpy_s(protect.szTypeName, L"WriteCopyExecute", wcslen(L"WriteCopyExecute"));
	m_vectorProtectType.push_back(protect);

	memset(&protect, 0, sizeof(PROTECT));
	protect.uType = PAGE_GUARD;
	wcsncpy_s(protect.szTypeName, L"Guard", wcslen(L"Guard"));
	m_vectorProtectType.push_back(protect);

	memset(&protect, 0, sizeof(PROTECT));
	protect.uType = PAGE_NOCACHE;
	wcsncpy_s(protect.szTypeName, L"No Cache", wcslen(L"No Cache"));
	m_vectorProtectType.push_back(protect);

	memset(&protect, 0, sizeof(PROTECT));
	protect.uType = PAGE_WRITECOMBINE;
	wcsncpy_s(protect.szTypeName, L"WriteCombine", wcslen(L"WriteCombine"));
	m_vectorProtectType.push_back(protect);
}



CString GetMemoryProtect(ULONG Protect)
{
	BOOL bFirst = TRUE;
	CString strProtect = L"";

	for ( vector <PROTECT>::iterator Iter = m_vectorProtectType.begin( ); 
		Iter != m_vectorProtectType.end( ); 
		Iter++ )
	{
		PROTECT item = *Iter;
		if (item.uType & Protect)
		{
			if (bFirst == TRUE)
			{
				strProtect = item.szTypeName;
				bFirst = FALSE;
			}
			else
			{
				strProtect += L" & ";
				strProtect += item.szTypeName;
			}
		}
	}

	return strProtect;
}
