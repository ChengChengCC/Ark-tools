#include "stdafx.h"
#include "SSDTFunc.h"
#include "ModuleFunc.h"
#include "Common.h"
#include <vector>
#include <Strsafe.h>

using namespace std;

extern vector<DRIVER_INFO> m_DriverList;
extern HANDLE g_hDevice;
extern WIN_VERSION WinVersion;
extern BOOL bIsChecking;

COLUMNSTRUCT g_Column_SSDT[] = 
{
	{	L"序号",					50	},
	{	L"函数名称",				145	},
	{	L"函数当前地址",			125	},
	{	L"函数原地址",			125	},
	{	L"模块文件",				195	},
	{	L"状态",					85	}
};

UINT g_Column_SSDT_Count  = 6;	  //进程列表列数

ULONG_PTR m_KiServiceTable = 0;
ULONG_PTR m_SSDTFuncCount = 0;
ULONG_PTR m_HookFuncCount = 0;
CHAR  m_szNtosName[512];
CHAR  m_szTempNtosName[512];
ULONG_PTR m_NtosImageBase;
ULONG_PTR m_NtosBase;
ULONG_PTR m_TempNtosBase;
BOOL  m_bSSDTOk = FALSE;

extern int dpix;
extern int dpiy;


VOID HsInitSSDTList(CListCtrl *m_ListCtrl)
{
	while(m_ListCtrl->DeleteColumn(0));
	m_ListCtrl->DeleteAllItems();

	m_ListCtrl->SetExtendedStyle(LVS_EX_FULLROWSELECT|LVS_EX_GRIDLINES|LVS_EX_HEADERDRAGDROP);

	UINT i;
	for (i = 0;i<g_Column_SSDT_Count;i++)
	{
		m_ListCtrl->InsertColumn(i, g_Column_SSDT[i].szTitle,LVCFMT_LEFT,(int)(g_Column_SSDT[i].nWidth*(dpix/96.0)));
	}

}

VOID HsLoadSSDTList(CListCtrl *m_ListCtrl)
{
// 	if (bIsChecking == TRUE)
// 	{
// 		return;
// 	}

// 	while(bIsChecking == TRUE)
// 	{
// 		Sleep(10);
// 	}


	HsSendStatusDetail(L"SSDT正在加载...");
	HsSendStatusTip(L"SSDT");

	HsQuerySSDTList(m_ListCtrl);

	
	bIsChecking = FALSE;
}


VOID HsQuerySSDTList(CListCtrl *m_ListCtrl)
{


	SSDT_INFO si[0x1000] = {0};

	m_KiServiceTable = 0;
	m_HookFuncCount = 0;
	m_SSDTFuncCount = 0;

	m_DriverList.clear();

	BOOL bRet = FALSE;

	InitDataOfSSDT();

	bRet = EnumSSDT(si);

	if (bRet == FALSE)
	{
		HsSendStatusDetail(L"SSDT加载失败。");
		return;
	}

	bRet = EnumDriver();

	if (bRet == FALSE)
	{
		HsSendStatusDetail(L"SSDT加载失败。");
		return;
	}

	CString strIndex;

	int i = 0;
	int j = 0;
	for (i=0;i<m_SSDTFuncCount;i++)
	{

		strIndex.Format(L"%d",si[i].Id);
		int n = m_ListCtrl->InsertItem(m_ListCtrl->GetItemCount(),strIndex);

		CString strFuncName(si[i].szFuncName);
		m_ListCtrl->SetItemText(n,1,strFuncName);

		CString strCurAddr;
		strCurAddr.Format(L"0x%p",si[i].CurAddr);
		m_ListCtrl->SetItemText(n,2,strCurAddr);

		CString strOrigAddr;
		strOrigAddr.Format(L"0x%p",si[i].OriAddr);
		m_ListCtrl->SetItemText(n,3,strOrigAddr);


		CString strType;
		if (si[i].OriAddr!=si[i].CurAddr)
		{
			m_ListCtrl->SetItemData(n,1);

			strType = L"SSDTHook";
		}
		else
		{
			strType = L"正常";
		}


		m_ListCtrl->SetItemText(n,5,strType);

		CString strPath;

		strPath = GetDriverPath(si[i].CurAddr);
		m_ListCtrl->SetItemText(n,4,strPath);

		if (_wcsnicmp(L"正常",strType,wcslen(L"正常"))!=0)
		{
			j+=1;
		}

		CString StatusBarContext;
		StatusBarContext.Format(L"SSDT正在加载。 SSDT函数：%d，挂钩函数：%d",i+1,j);

		HsSendStatusDetail(StatusBarContext);
	}

	CString StatusBarContext;
	StatusBarContext.Format(L"SSDT加载完成。 SSDT函数：%d，挂钩函数：%d",m_SSDTFuncCount,m_HookFuncCount);

	HsSendStatusDetail(StatusBarContext);
}

CString GetDriverPath(ULONG_PTR Address)
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


BOOL EnumSSDT(SSDT_INFO* si)
{
	DWORD i=0,SSDTFuncCount = 0;


	CHAR* szTempNtdll = NULL;
	DWORD dwLen = 0;
	DWORD dwStart = 0;
	DWORD dwEnd = 0;
	ULONG_PTR SSDTOriAddr = 0;
	ULONG_PTR SSDTCurAddr = 0;
	szTempNtdll=HsGetTempNtdll();

	if(!CopyFileA(Strcat(HsGetSystemDir(),"ntdll.dll"),szTempNtdll,0))
	{
		return FALSE;
	}

	dwLen = FileLen(szTempNtdll);

	char szFuncStart[]="ZwAcceptConnectPort", szFuncEnd[]="ZwYieldExecution"; //每个函数名之间隔着\0

	char* szNtdll = HsLoadDllContext(szTempNtdll);
	char* szTemp  = szNtdll;
	for(i=0; i<dwLen; i++)
	{
		if(memcmp(szTemp+i,szFuncStart,strlen(szFuncStart))==0)
		{
			dwStart = i;
		}
		if(memcmp(szTemp+i,szFuncEnd,strlen(szFuncEnd))==0)
		{
			dwEnd = i;
			break;
		}
	}
	szTemp =szTemp+dwStart;
	//这里不能显示到最后一个函数
	while(strcmp(szTemp,szFuncEnd)!=0)
	{
		DWORD dwFuncIndex = HsGetSSDTFunctionIndex(szTemp);
		if(dwFuncIndex<0x1000)
		{
			SSDTOriAddr = HsGetFunctionOriginalAddress(dwFuncIndex);

			SendIoCodeSSDT(dwFuncIndex,&SSDTCurAddr);


			if (SSDTCurAddr!=SSDTOriAddr)
			{
				m_HookFuncCount++;
			}

			szTemp[0]='N';
			szTemp[1]='t';		//寻找的是Zw***，但是应该显示Nt***
		}
		else
		{

			dwFuncIndex = HsGetSpecialIndex(szTemp);
			SSDTOriAddr=HsGetFunctionOriginalAddress(dwFuncIndex);
			SendIoCodeSSDT(dwFuncIndex,&SSDTCurAddr);

			if (SSDTCurAddr!=SSDTOriAddr)
			{
				m_HookFuncCount++;
			}
			szTemp[0]='N';
			szTemp[1]='t';
		}
		si[SSDTFuncCount].Id = dwFuncIndex;
		si[SSDTFuncCount].CurAddr = SSDTCurAddr;
		si[SSDTFuncCount].OriAddr = SSDTOriAddr;
		strcpy_s(si[SSDTFuncCount].szFuncName,szTemp);
		szTemp=szTemp+strlen(szTemp)+1;
		SSDTFuncCount++;
		dwFuncIndex=0;
	}
	//显示完最后一个函数
	DWORD dwFuncIndex = HsGetSSDTFunctionIndex(szTemp);
	SSDTOriAddr = HsGetFunctionOriginalAddress(dwFuncIndex);
	SendIoCodeSSDT(dwFuncIndex,&SSDTCurAddr);

	if (SSDTCurAddr!=SSDTOriAddr)
	{
		m_HookFuncCount++;
	}
	szTemp[0]='N';
	szTemp[1]='t';
	si[SSDTFuncCount].Id = dwFuncIndex;
	si[SSDTFuncCount].CurAddr = SSDTCurAddr;
	si[SSDTFuncCount].OriAddr = SSDTOriAddr;
	strcpy_s(si[SSDTFuncCount].szFuncName,szTemp);
	SSDTFuncCount++;

	m_SSDTFuncCount = SSDTFuncCount;
	DeleteFileA(szTempNtdll);
	DeleteFileA(m_szTempNtosName);
	free(szNtdll);

	return TRUE;
}


BOOL SendIoCodeSSDT(DWORD dwFuncIndex,PULONG_PTR SSDTCurAddr)
{
	DWORD dwReturnSize = 0;
	DWORD dwRet = 0;

	//发送IO 控制码
	dwRet = DeviceIoControl(g_hDevice,HS_IOCTL(HS_IOCTL_KRNL_SSDTLIST),
		&dwFuncIndex,
		sizeof(DWORD),
		SSDTCurAddr,
		sizeof(ULONG_PTR),
		&dwReturnSize,
		NULL);


	if (dwRet==0)
	{
		return FALSE;
	}

	return TRUE;
}




ULONG_PTR HsGetFunctionOriginalAddress(ULONG_PTR dwIndex)	//通过INDEX获得函数原始地址
{
	ULONG_PTR RVA = m_KiServiceTable - m_NtosBase;
	ULONG_PTR Temp=*(PULONG_PTR)(m_TempNtosBase+RVA+sizeof(ULONG_PTR)*(ULONG_PTR)dwIndex);
	return Temp;
}


BOOL InitDataOfSSDT()
{
	BOOL b1=FALSE,b2=FALSE;
	BOOL bOk = FALSE;
	b1 = GetNtosNameAndBase();
	b2 = GetNtosImageBase();

	m_TempNtosBase = (ULONG_PTR)LoadLibraryExA(m_szTempNtosName,0, DONT_RESOLVE_DLL_REFERENCES);

	if (m_bSSDTOk==FALSE)
	{
		HsReloc((ULONG_PTR)m_TempNtosBase,m_NtosBase);
		m_bSSDTOk = TRUE;
	}

	if (GetKiServiceTable()==FALSE)
	{
		return FALSE;
	}

	if(b1 && b2)
	{
		bOk = TRUE;
	}	
	else
	{
		bOk = FALSE;
	}

	return bOk;
}



BOOL GetNtosImageBase()
{
	PIMAGE_NT_HEADERS NtHeader;
	PIMAGE_DOS_HEADER DosHeader;
	char* szNtosFileData=NULL;
	szNtosFileData = HsLoadDllContext(m_szTempNtosName);
	if(szNtosFileData==NULL)
	{
		return FALSE;
	}
	DosHeader = (PIMAGE_DOS_HEADER)szNtosFileData;
	NtHeader  = (PIMAGE_NT_HEADERS)(szNtosFileData+DosHeader->e_lfanew);
	m_NtosImageBase = NtHeader->OptionalHeader.ImageBase;
	return TRUE;
}

BOOL GetNtosNameAndBase()    //获得Ntos的路径和加载地址
{
	char szFileName[260]= {0},*szFullName;
	m_NtosBase = HsGetKernelBase(szFileName);
	szFullName = Strcat(HsGetSystemDir(),szFileName);
	strcpy_s(m_szNtosName,szFullName);
	return GetTempNtosName();
}



BOOL GetTempNtosName()   //根据Ntos的路径 进行拷贝构建ntosxxxx.exe
{
	char *szPath;
	szPath=(char *)malloc(260);
	memset(szPath,0,260);
	GetTempPathA(260,szPath);
	szPath = Strcat(szPath,"ntosxxxx.exe");
	strcpy_s(m_szTempNtosName,szPath);

	if(CopyFileA(m_szNtosName,m_szTempNtosName,0))
	{
		free(szPath);
		return TRUE;
	}
	else
	{
		free(szPath);
		return FALSE;
	}
}




BOOL GetKiServiceTable()
{
	DWORD dwReturnSize = 0;
	DWORD dwRet = 0;

	if (g_hDevice==NULL)
	{
		return FALSE;
	}

	dwRet = DeviceIoControl(
		g_hDevice,
		HS_IOCTL(HS_IOCTL_KRNL_KISRVTAB),
		NULL,
		0,
		&m_KiServiceTable,
		sizeof(ULONG_PTR),
		&dwReturnSize,
		NULL);


	if (dwRet==0)
	{
		return FALSE;
	}

	return TRUE;
}





BOOL EnumDriver()
{
	ULONG ulReturnSize = 0;
	BOOL bRet = FALSE;

	m_DriverList.clear();

	ULONG ulCount = 1000;
	PALL_DRIVERS Drivers = NULL;

	if (g_hDevice==NULL)
	{
		MessageBox(NULL,L"设备获取失败",L"Error",0);
		return FALSE;
	}

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


		bRet = DeviceIoControl(
			g_hDevice,
			HS_IOCTL(HS_IOCTL_MODU_MODULELIST),
			NULL,
			0,
			Drivers,
			ulSize,
			&ulReturnSize,
			NULL);

		ulCount = (ULONG)Drivers->ulCount + 100;

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


void HsResumeSSDTHook(CListCtrl* m_ListCtrl)
{

	int Index = m_ListCtrl->GetSelectionMark();

	if (Index<0)
	{
		return;
	}

	CString Temp = m_ListCtrl->GetItemText(Index,0);

	RESUME_DATA  Data = {0};  

	swscanf_s(Temp.GetBuffer(),L"%d",&Data.ulIndex);

	Temp  = m_ListCtrl->GetItemText(Index,3);
	swscanf_s(Temp.GetBuffer()+2,L"%p",&Data.ulFuncAddress);

	BOOL dwRet = FALSE;

	if (g_hDevice==NULL)
	{
		return;
	}

	DWORD dwReturnSize = 0;


	dwRet = DeviceIoControl(g_hDevice,HS_IOCTL(HS_IOCTL_KRNL_RESUMESSDT),
		&Data,
		sizeof(Data),
		NULL,
		0,
		&dwReturnSize,
		NULL);

	if (dwRet==0)
	{
		return;
	}

	m_ListCtrl->SetItemText(Index,5,L"正常");
	m_ListCtrl->SetItemData(Index,0);

}