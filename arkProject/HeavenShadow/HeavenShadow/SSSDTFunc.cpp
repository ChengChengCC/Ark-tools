#include "stdafx.h"
#include "SSDTFunc.h"
#include "SSSDTFunc.h"
#include "ModuleFunc.h"
#include "Common.h"
#include <vector>
#include <Strsafe.h>

using namespace std;

extern vector<DRIVER_INFO> m_DriverList;
extern HANDLE g_hDevice;
extern WIN_VERSION WinVersion;
extern BOOL bIsChecking;
extern CHAR XPProcName[667][100];
extern CHAR WIN7ProcName[827][100];

char* m_szTempWin32k;
ULONG_PTR m_Win32kServiceTable = 0;
ULONG_PTR m_Win32kBase = 0;
ULONG_PTR m_Win32kImageBase = 0;
ULONG_PTR m_TempWin32kBase = 0;
BOOL      m_bSSSDTOk = FALSE;

COLUMNSTRUCT g_Column_SSSDT[] = 
{
	{	L"序号",					50	},
	{	L"函数名称",				145	},
	{	L"函数当前地址",			125	},
	{	L"函数原地址",			125	},
	{	L"模块文件",				195	},
	{	L"状态",					85	}
};


UINT g_Column_SSSDT_Count  = 6;	  //进程列表列数

extern int dpix;
extern int dpiy;


VOID HsInitSSSDTList(CListCtrl *m_ListCtrl)
{
	while(m_ListCtrl->DeleteColumn(0));
	m_ListCtrl->DeleteAllItems();

	m_ListCtrl->SetExtendedStyle(LVS_EX_FULLROWSELECT|LVS_EX_GRIDLINES|LVS_EX_HEADERDRAGDROP);

	UINT i;
	for (i = 0;i<g_Column_SSSDT_Count;i++)
	{
		m_ListCtrl->InsertColumn(i, g_Column_SSSDT[i].szTitle,LVCFMT_LEFT,(int)(g_Column_SSSDT[i].nWidth*(dpix/96.0)));
	}

}

VOID HsLoadSSSDTList(CListCtrl *m_ListCtrl)
{
// 	if (bIsChecking == TRUE)
// 	{
// 		return;
// 	}

// 	while(bIsChecking == TRUE)
// 	{
// 		Sleep(10);
// 	}


	HsSendStatusDetail(L"ShadowSSDT正在加载...");
	HsSendStatusTip(L"ShadowSSDT");

	HsQuerySSSDTList(m_ListCtrl);

	
	bIsChecking = FALSE;
}


VOID HsQuerySSSDTList(CListCtrl *m_ListCtrl)
{
	SSSDT_INFO si[0x1000];

	if(EnumDriver()==FALSE)
	{
		return;
	}

	EnumSSSDT(si,m_ListCtrl);
}




BOOL EnumSSSDT(SSSDT_INFO* si, CListCtrl* m_ListCtrl)
{

	m_ListCtrl->DeleteAllItems();
	ULONG_PTR SSSDTFuncCount = 0;
	ULONG_PTR HookFuncCount = 0;


	ULONG_PTR SSSDTOriAddr = 0;
	ULONG_PTR SSSDTCurAddr = 0;

	switch(WinVersion)
	{
	case Windows7:
		{

			int i = 0;
			int j = 0;
			for (i=0;i<sizeof(WIN7ProcName)/100;i++)
			{


				SendIoCodeSSSDT(i,&SSSDTCurAddr);
				SSSDTOriAddr = GetOriSSSDTAddress(i);


				si[SSSDTFuncCount].Id = i;
				si[SSSDTFuncCount].CurAddr = SSSDTCurAddr;
				si[SSSDTFuncCount].OriAddr = SSSDTOriAddr;
				strcpy_s(si[SSSDTFuncCount].szFuncName,WIN7ProcName[i]);


				if (SSSDTCurAddr!=SSSDTOriAddr)
				{
					HookFuncCount++;
				}

				SSSDTFuncCount++;
			}

			CString strIndex;
			for (i=0;i<SSSDTFuncCount;i++)
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


				CString strPath;

				strPath = GetDriverPath(si[i].CurAddr);
				m_ListCtrl->SetItemText(n,4,strPath);

				CString strType;
				if (si[i].OriAddr!=si[i].CurAddr)
				{
					m_ListCtrl->SetItemData(n,1);

					strType = L"SSSDTHook";
				}
				else
				{
					strType = L"正常";
				}


				m_ListCtrl->SetItemText(n,5,strType);

				if (_wcsnicmp(L"正常",strType,wcslen(L"正常"))!=0)
				{
					j+=1;
				}

				CString StatusBarContext;
				StatusBarContext.Format(L"ShadowSSDT正在加载。 ShadowSSDT函数：%d，挂钩函数：%d",i+1,j);

				HsSendStatusDetail(StatusBarContext);



			}

			CString StatusBarContext;
			StatusBarContext.Format(L"ShadowSSDT加载完成。 ShadowSSDT函数：%d，挂钩函数：%d",SSSDTFuncCount,HookFuncCount);

			HsSendStatusDetail(StatusBarContext);

			break;
		}

	case WindowsXP:
		{

			int i = 0;
			int j = 0;
			for (i=0;i<sizeof(XPProcName)/100;i++)
			{


				SendIoCodeSSSDT(i,&SSSDTCurAddr);
				SSSDTOriAddr = GetOriSSSDTAddress(i);


				si[SSSDTFuncCount].Id = i;
				si[SSSDTFuncCount].CurAddr = SSSDTCurAddr;
				si[SSSDTFuncCount].OriAddr = SSSDTOriAddr;
				strcpy_s(si[SSSDTFuncCount].szFuncName,XPProcName[i]);


				if (SSSDTCurAddr!=SSSDTOriAddr)
				{
					HookFuncCount++;
				}

				SSSDTFuncCount++;

			}

			CString strIndex;
			for (i=0;i<SSSDTFuncCount;i++)
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


				CString strPath;



				strPath = GetDriverPath(si[i].CurAddr);
				m_ListCtrl->SetItemText(n,4,strPath);


				CString strType;
				if (si[i].OriAddr!=si[i].CurAddr)
				{
					m_ListCtrl->SetItemData(n,1);

					strType = L"SSSDTHook";
				}
				else
				{
					strType = L"正常";
				}

				m_ListCtrl->SetItemText(n,5,strType);


				if (_wcsnicmp(L"正常",strType,wcslen(L"正常"))!=0)
				{
					j+=1;
				}

				CString StatusBarContext;
				StatusBarContext.Format(L"ShadowSSDT正在加载。 ShadowSSDT函数：%d，挂钩函数：%d",i+1,j);

				HsSendStatusDetail(StatusBarContext);

			}

			CString StatusBarContext;
			StatusBarContext.Format(L"ShadowSSDT加载完成。 ShadowSSDT函数：%d，挂钩函数：%d",SSSDTFuncCount,HookFuncCount);

			HsSendStatusDetail(StatusBarContext);

			break;
		}
	}

	return TRUE;
}



VOID SendIoCodeSSSDT(DWORD dwFuncIndex,PULONG_PTR SSSDTCurAddr)
{
	DWORD dwReturnSize = 0;
	DWORD dwRet = 0;

	//发送IO 控制码
	dwRet = DeviceIoControl(g_hDevice,HS_IOCTL(HS_IOCTL_KRNL_SSSDTLIST),
		&dwFuncIndex,
		sizeof(DWORD),
		SSSDTCurAddr,
		sizeof(ULONG_PTR),
		&dwReturnSize,
		NULL);


	if (dwRet==0)
	{
		return;
	}

	return;
}


ULONG_PTR GetOriSSSDTAddress(ULONG_PTR Index)
{

	m_szTempWin32k = HsGetTempWin32k();

	if(m_Win32kServiceTable==0 )
	{
		SendIoCodeWin32kServiceTable();
	}

	if(m_Win32kBase==0 )
	{
		m_Win32kBase = HsGetWin32kBase();
	}
	if(m_Win32kImageBase==0)
	{
		if(!CopyFileA(Strcat(HsGetSystemDir(),"win32k.sys"),m_szTempWin32k,0))
		{
			return 0;
		}
		m_Win32kImageBase = HsGetWin32kImageBase(m_szTempWin32k);
	}
	if( m_TempWin32kBase==0 )
	{
		m_TempWin32kBase = (ULONG_PTR)LoadLibraryExA(m_szTempWin32k,0, DONT_RESOLVE_DLL_REFERENCES);
	}


	if (m_bSSSDTOk==FALSE)
	{
		if(!HsReloc(m_TempWin32kBase,m_Win32kBase))
		{
			return 0;
		}

		m_bSSSDTOk = TRUE;
	}

	ULONG_PTR RVA = m_Win32kServiceTable - m_Win32kBase;
	ULONG_PTR Address = *(ULONG_PTR*)(m_TempWin32kBase+RVA+sizeof(ULONG_PTR)*Index);



	return Address;
}



VOID SendIoCodeWin32kServiceTable()
{
	DWORD dwReturnSize = 0;
	DWORD dwRet = 0;

	//发送IO 控制码
	dwRet = DeviceIoControl(g_hDevice,HS_IOCTL(HS_IOCTL_KRNL_WIN32KSERVICE),   //
		NULL,
		0,
		&m_Win32kServiceTable,
		sizeof(ULONG_PTR),
		&dwReturnSize,
		NULL);


	if (dwRet==0)
	{
		return;
	}

	return;
}


//////////////////////////////////////////////////////////////////////////


