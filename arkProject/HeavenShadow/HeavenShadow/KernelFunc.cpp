#include "stdafx.h"
#include "KernelFunc.h"
#include "Common.h"
#include "SSDTFunc.h"
#include "ProcessFunc.h"
#include "ModuleFunc.h"
#include <vector>

using namespace std;

BOOL bNowSelKernelFile = 255;

extern BOOL bIsChecking;

extern HANDLE g_hDevice;
extern vector<DRIVER_INFO> m_DriverList;


COLUMNSTRUCT g_Column_KernelFunc[] = 
{
	{	L"序号",					50	},
	{	L"函数名称",				205	},
	{	L"函数当前地址",			135	},
	{	L"函数原地址",			135	},
	{	L"模块文件",				115	},
	{	L"状态",					85	}
};

UINT g_Column_KernelFunc_Count  = 6;	  //进程列表列数

extern int dpix;
extern int dpiy;


void HsInitKernelFuncList(CListBox* m_ListBox, CListCtrl* m_ListCtrl)
{
	if (m_ListBox->GetCount())
	{
		while(m_ListBox->DeleteString(0));
	}
	

	m_ListBox->AddString(L"ntoskrnl.exe [IAT]");
	m_ListBox->InsertString(HS_KERNEL_KERNELFILE_NTOSKRNL_EAT,L"ntoskrnl.exe [EAT]");
	m_ListBox->InsertString(HS_KERNEL_KERNELFILE_WIN32K_IAT,L"win32k.sys [IAT]");
	m_ListBox->InsertString(HS_KERNEL_KERNELFILE_WIN32K_EAT,L"win32k.sys [EAT]");
	m_ListBox->InsertString(HS_KERNEL_KERNELFILE_HALDLL_IAT,L"hal.dll [IAT]");
	m_ListBox->InsertString(HS_KERNEL_KERNELFILE_HALDLL_EAT,L"hal.dll [EAT]");

	m_ListCtrl->DeleteAllItems();
	while(m_ListCtrl->DeleteColumn(0));

	//////////////////////////////////////////////////////////////////////////

	while(m_ListCtrl->DeleteColumn(0));
	m_ListCtrl->DeleteAllItems();

	m_ListCtrl->SetExtendedStyle(LVS_EX_FULLROWSELECT|LVS_EX_GRIDLINES|LVS_EX_HEADERDRAGDROP);

	UINT i;
	for (i = 0;i<g_Column_KernelFunc_Count;i++)
	{
		m_ListCtrl->InsertColumn(i, g_Column_KernelFunc[i].szTitle,LVCFMT_LEFT,(int)(g_Column_KernelFunc[i].nWidth*(dpix/96.0)));
	}

	bNowSelKernelFile = 255;
}


void HsInitKernelFileList(CListBox* m_ListBox, CListCtrl* m_ListCtrl)
{
	if (m_ListBox->GetCount())
	{
		while(m_ListBox->DeleteString(0));
	}

	m_ListCtrl->DeleteAllItems();
	while(m_ListCtrl->DeleteColumn(0));

	//////////////////////////////////////////////////////////////////////////

	m_ListCtrl->SetExtendedStyle(LVS_EX_FULLROWSELECT|LVS_EX_GRIDLINES|LVS_EX_HEADERDRAGDROP);

	UINT i;
	for (i = 0;i<g_Column_KernelFunc_Count;i++)
	{
		m_ListCtrl->InsertColumn(i, g_Column_KernelFunc[i].szTitle,LVCFMT_LEFT,(int)(g_Column_KernelFunc[i].nWidth*(dpix/96.0)));
	}

	//////////////////////////////////////////////////////////////////////////

	m_DriverList.clear();

	BOOL bRet = FALSE;

	bRet = EnumDriver();

	if (bRet == FALSE)
	{
		HsSendStatusDetail(L"内核导入表加载失败。");
		return;
	}


	i = 0;

	CString ModuleName;

	for (vector<DRIVER_INFO>::iterator itor = m_DriverList.begin(); 
		itor != m_DriverList.end(); 
		itor++)
	{
		ModuleName = wcsrchr(itor->wzDriverPath,L'\\');

		ModuleName = ModuleName.GetBuffer()+1;

		if (i==0)
		{
			m_ListBox->AddString(ModuleName);
		}
		else if (wcslen(itor->wzDriverPath)>0)
		{
			m_ListBox->InsertString(i,ModuleName);
		}
		i++;
		
	}

	bNowSelKernelFile = 255;
}


void SelchangeListKrnlIAT(CListBox* m_ListBox,  CListCtrl* m_ListCtrl)
{
	HsSendStatusTip(L"内核函数");

	int nCurSel = m_ListBox->GetCurSel();

	if (bIsChecking == TRUE || bNowSelKernelFile == nCurSel)	//
	{
		m_ListBox->SetCurSel(bNowSelKernelFile);
		m_ListCtrl->SetFocus();
		return;
	}

	bIsChecking = TRUE;

	bNowSelKernelFile = nCurSel;

	CString ModuleFile;

	m_ListBox->GetText(m_ListBox->GetCurSel(), ModuleFile);

	CString DetailContext;
	DetailContext.Format(L"%s的IAT正在加载...",ModuleFile.GetBuffer());

	HsSendStatusDetail(DetailContext);

	//////////////////////////////////////////////////////////////////////////

	m_ListCtrl->DeleteAllItems();

	BOOL bOk = FALSE;

	ULONG_PTR   ulCount = 0x1000;
	PMODULE_IAT IAT = NULL;
	BOOL bRet = FALSE;
	DWORD ulReturnSize = 0;

	do 
	{
		ULONG_PTR ulSize = 0;

		if (IAT)
		{
			free(IAT);
			IAT = NULL;
		}

		ulSize = sizeof(MODULE_IAT) + ulCount * sizeof(IAT_INFO);

		IAT = (PMODULE_IAT)malloc(ulSize);
		if (!IAT)
		{
			break;
		}

		memset(IAT,0,ulSize);

		CStringA ModuleFileA(ModuleFile);

		bRet = DeviceIoControl(g_hDevice,HS_IOCTL(HS_IOCTL_KRNL_KRNLIAT),
			ModuleFileA.GetBuffer(),
			ModuleFileA.GetLength(),
			IAT,
			(DWORD)ulSize,
			&ulReturnSize,
			NULL);


		ulCount = IAT->ulCount + 1000;

	} while (bRet == FALSE && GetLastError() == ERROR_INSUFFICIENT_BUFFER);

	if (bRet && IAT)
	{
		for (ULONG i = 0; i < IAT->ulCount; i++)
		{
			CString strIndex;
			strIndex.Format(L"%d",i+1);
			int n = m_ListCtrl->InsertItem(m_ListCtrl->GetItemCount(), strIndex);

			CString strFuncName(IAT->Data[i].szFunctionName);
			m_ListCtrl->SetItemText(n, 1, strFuncName);

			CString strCurFuncAddress;
			strCurFuncAddress.Format(L"0x%p",IAT->Data[i].CurFuncAddress);
			m_ListCtrl->SetItemText(n, 2, strCurFuncAddress);

			CString strOriFuncAddress;
			strOriFuncAddress.Format(L"0x%p",IAT->Data[i].OriFuncAddress);
			m_ListCtrl->SetItemText(n, 3, strOriFuncAddress);

			CString strModuleName(IAT->Data[i].szModuleName);
			m_ListCtrl->SetItemText(n, 4, strModuleName);

			CString strFuncStatus;
			if (IAT->Data[i].CurFuncAddress == IAT->Data[i].OriFuncAddress)
			{
				strFuncStatus = L"正常";
			}
			else
			{
				strFuncStatus = L"Hooked";
				m_ListCtrl->SetItemData(n,1);
			}
			m_ListCtrl->SetItemText(n, 5, strFuncStatus);

			CString StatusBarContext;
			StatusBarContext.Format(L"%s的IAT正在加载。 IAT函数：%d",ModuleFile,i);
			HsSendStatusDetail(StatusBarContext);
		}

		CString StatusBarContext;
		StatusBarContext.Format(L"%s的IAT加载完成。 IAT函数：%d",ModuleFile,IAT->ulCount);
		HsSendStatusDetail(StatusBarContext);

		bOk = TRUE;
	}
	else
	{
		CString StatusBarContext;
		StatusBarContext.Format(L"%s的IAT加载失败。",ModuleFile);
		HsSendStatusDetail(ModuleFile);
	}

	if (IAT)
	{
		free(IAT);
		IAT = NULL;
	}

	bIsChecking = FALSE;

}


void SelchangeListKrnlEAT(CListBox* m_ListBox,  CListCtrl* m_ListCtrl)
{
	HsSendStatusTip(L"内核函数");

	int nCurSel = m_ListBox->GetCurSel();

	if (bIsChecking == TRUE || bNowSelKernelFile == nCurSel)	//
	{
		m_ListBox->SetCurSel(bNowSelKernelFile);
		m_ListCtrl->SetFocus();
		return;
	}

	bIsChecking = TRUE;

	bNowSelKernelFile = nCurSel;

	CString ModuleFile;

	m_ListBox->GetText(m_ListBox->GetCurSel(), ModuleFile);

	CString DetailContext;
	DetailContext.Format(L"%s的EAT正在加载...",ModuleFile.GetBuffer());

	HsSendStatusDetail(DetailContext);

	//////////////////////////////////////////////////////////////////////////

	m_ListCtrl->DeleteAllItems();

	BOOL bOk = FALSE;

	ULONG_PTR   ulCount = 0x1000;
	PMODULE_EAT EAT = NULL;
	BOOL bRet = FALSE;
	DWORD ulReturnSize = 0;

	do 
	{
		ULONG_PTR ulSize = 0;

		if (EAT)
		{
			free(EAT);
			EAT = NULL;
		}

		ulSize = sizeof(MODULE_EAT) + ulCount * sizeof(EAT_INFO);

		EAT = (PMODULE_EAT)malloc(ulSize);
		if (!EAT)
		{
			break;
		}

		memset(EAT,0,ulSize);

		CStringA ModuleFileA(ModuleFile);

		bRet = DeviceIoControl(g_hDevice,HS_IOCTL(HS_IOCTL_KRNL_KRNLEAT),
			ModuleFileA.GetBuffer(),
			ModuleFileA.GetLength(),
			EAT,
			(DWORD)ulSize,
			&ulReturnSize,
			NULL);


		ulCount = EAT->ulCount + 1000;

	} while (bRet == FALSE && GetLastError() == ERROR_INSUFFICIENT_BUFFER);

	if (bRet && EAT)
	{
		for (ULONG i = 0; i < EAT->ulCount; i++)
		{
			CString strIndex;
			strIndex.Format(L"%d",i+1);
			int n = m_ListCtrl->InsertItem(m_ListCtrl->GetItemCount(), strIndex);

			CString strFuncName(EAT->Data[i].szFunctionName);
			m_ListCtrl->SetItemText(n, 1, strFuncName);

			CString strCurFuncAddress;
			strCurFuncAddress.Format(L"0x%p",EAT->Data[i].CurFuncAddress);
			m_ListCtrl->SetItemText(n, 2, strCurFuncAddress);

			CString strOriFuncAddress;
			strOriFuncAddress.Format(L"0x%p",EAT->Data[i].OriFuncAddress);
			m_ListCtrl->SetItemText(n, 3, strOriFuncAddress);

			CString strModuleName(ModuleFile);
			m_ListCtrl->SetItemText(n, 4, strModuleName);

			CString strFuncStatus;
			if (EAT->Data[i].CurFuncAddress == EAT->Data[i].OriFuncAddress)
			{
				strFuncStatus = L"正常";
			}
			else
			{
				strFuncStatus = L"Hooked";
				m_ListCtrl->SetItemData(n,1);
			}
			m_ListCtrl->SetItemText(n, 5, strFuncStatus);

			CString StatusBarContext;
			StatusBarContext.Format(L"%s的EAT正在加载。 EAT函数：%d",ModuleFile,i);
			HsSendStatusDetail(StatusBarContext);
		}

		CString StatusBarContext;
		StatusBarContext.Format(L"%s的EAT加载完成。 EAT函数：%d",ModuleFile,EAT->ulCount);
		HsSendStatusDetail(StatusBarContext);

		bOk = TRUE;
	}
	else
	{
		CString StatusBarContext;
		StatusBarContext.Format(L"%s的EAT加载失败。",ModuleFile);
		HsSendStatusDetail(ModuleFile);
	}

	if (EAT)
	{
		free(EAT);
		EAT = NULL;
	}

	bIsChecking = FALSE;

}


void SelchangeListKrnlFunc(CListBox* m_ListBox, CListCtrl* m_ListCtrl)
{
	HsSendStatusTip(L"内核函数");

	int nCurSel = m_ListBox->GetCurSel();

	switch (nCurSel)
	{
	case HS_KERNEL_KERNELFILE_NTOSKRNL_IAT:
		{
			if (bIsChecking == TRUE || bNowSelKernelFile == HS_KERNEL_KERNELFILE_NTOSKRNL_IAT)	//
			{
				m_ListBox->SetCurSel(bNowSelKernelFile);
				m_ListCtrl->SetFocus();
				return;
			}

			bIsChecking = TRUE;

			bNowSelKernelFile = nCurSel;

			//////////////////////////////////////////////////////////////////////////

			HsSendStatusDetail(L"ntoskrnl.exe的IAT正在加载...");

			CloseHandle(
				CreateThread(NULL,0, 
				(LPTHREAD_START_ROUTINE)HsEnumKernelFuncNameNtoskrnlIAT,m_ListCtrl, 0,NULL)
				);

			//HsEnumKernelFuncNameNtoskrnlIAT(m_ListCtrl);

			break;
		}
	case HS_KERNEL_KERNELFILE_NTOSKRNL_EAT:
		{
			if (bIsChecking == TRUE || bNowSelKernelFile == HS_KERNEL_KERNELFILE_NTOSKRNL_EAT)	//
			{
				m_ListBox->SetCurSel(bNowSelKernelFile);
				m_ListCtrl->SetFocus();
				return;
			}

			bIsChecking = TRUE;

			bNowSelKernelFile = nCurSel;

			//////////////////////////////////////////////////////////////////////////

			HsSendStatusDetail(L"ntoskrnl.exe的EAT正在加载...");

			CloseHandle(
				CreateThread(NULL,0, 
				(LPTHREAD_START_ROUTINE)HsEnumKernelFuncNameNtoskrnlEAT,m_ListCtrl, 0,NULL)
				);

			//HsEnumKernelFuncNameNtoskrnlEAT(m_ListCtrl);

			break;
		}
	case HS_KERNEL_KERNELFILE_WIN32K_IAT:
		{
			if (bIsChecking == TRUE || bNowSelKernelFile == HS_KERNEL_KERNELFILE_WIN32K_IAT)	//
			{
				m_ListBox->SetCurSel(bNowSelKernelFile);
				m_ListCtrl->SetFocus();
				return;
			}

			bIsChecking = TRUE;

			bNowSelKernelFile = nCurSel;

			//////////////////////////////////////////////////////////////////////////

			HsSendStatusDetail(L"ntoskrnl.exe的IAT正在加载...");

			CloseHandle(
				CreateThread(NULL,0, 
				(LPTHREAD_START_ROUTINE)HsEnumKernelFuncNameWin32kIAT,m_ListCtrl, 0,NULL)
				);

			//HsEnumKernelFuncNameWin32kIAT(m_ListCtrl);

			break;
		}
	case HS_KERNEL_KERNELFILE_WIN32K_EAT:
		{
			if (bIsChecking == TRUE || bNowSelKernelFile == HS_KERNEL_KERNELFILE_WIN32K_EAT)	//
			{
				m_ListBox->SetCurSel(bNowSelKernelFile);
				m_ListCtrl->SetFocus();
				return;
			}

			bIsChecking = TRUE;

			bNowSelKernelFile = nCurSel;

			//////////////////////////////////////////////////////////////////////////

			HsSendStatusDetail(L"ntoskrnl.exe的EAT正在加载...");

			CloseHandle(
				CreateThread(NULL,0, 
				(LPTHREAD_START_ROUTINE)HsEnumKernelFuncNameWin32kEAT,m_ListCtrl, 0,NULL)
				);

			//HsEnumKernelFuncNameWin32kEAT(m_ListCtrl);

			break;
		}
	case HS_KERNEL_KERNELFILE_HALDLL_IAT:
		{
			if (bIsChecking == TRUE || bNowSelKernelFile == HS_KERNEL_KERNELFILE_HALDLL_IAT)	//
			{
				m_ListBox->SetCurSel(bNowSelKernelFile);
				m_ListCtrl->SetFocus();
				return;
			}

			bIsChecking = TRUE;

			bNowSelKernelFile = nCurSel;

			//////////////////////////////////////////////////////////////////////////

			HsSendStatusDetail(L"hal.dll的IAT正在加载...");

			CloseHandle(
				CreateThread(NULL,0, 
				(LPTHREAD_START_ROUTINE)HsEnumKernelFuncNameHaldllIAT,m_ListCtrl, 0,NULL)
				);

			//HsEnumKernelFuncNameHaldllIAT(m_ListCtrl);

			break;
		}
	case HS_KERNEL_KERNELFILE_HALDLL_EAT:
		{
			if (bIsChecking == TRUE || bNowSelKernelFile == HS_KERNEL_KERNELFILE_HALDLL_EAT)	//
			{
				m_ListBox->SetCurSel(bNowSelKernelFile);
				m_ListCtrl->SetFocus();
				return;
			}

			bIsChecking = TRUE;

			bNowSelKernelFile = nCurSel;

			//////////////////////////////////////////////////////////////////////////

			HsSendStatusDetail(L"hal.dll的EAT正在加载...");

			CloseHandle(
				CreateThread(NULL,0, 
				(LPTHREAD_START_ROUTINE)HsEnumKernelFuncNameHaldllEAT,m_ListCtrl, 0,NULL)
				);

			//HsEnumKernelFuncNameHaldllEAT(m_ListCtrl);

			break;
		}
	}

}



void HsEnumKernelFuncNameNtoskrnlIAT(CListCtrl* m_ListCtrl)
{
	m_ListCtrl->DeleteAllItems();

	BOOL bOk = FALSE;

	ULONG_PTR   ulCount = 0x1000;
	PMODULE_IAT IAT = NULL;
	BOOL bRet = FALSE;
	DWORD ulReturnSize = 0;

	do 
	{
		ULONG_PTR ulSize = 0;

		if (IAT)
		{
			free(IAT);
			IAT = NULL;
		}

		ulSize = sizeof(MODULE_IAT) + ulCount * sizeof(IAT_INFO);

		IAT = (PMODULE_IAT)malloc(ulSize);
		if (!IAT)
		{
			break;
		}

		memset(IAT,0,ulSize);

		int KernelFile = HS_KERNEL_KERNELFILE_NTOSKRNL_IAT;

		bRet = DeviceIoControl(g_hDevice,HS_IOCTL(HS_IOCTL_KRNL_KRNLFILE),
			&KernelFile,
			sizeof(int),
			IAT,
			(DWORD)ulSize,
			&ulReturnSize,
			NULL);


		ulCount = IAT->ulCount + 1000;

	} while (bRet == FALSE && GetLastError() == ERROR_INSUFFICIENT_BUFFER);

	if (bRet && IAT)
	{
		for (ULONG i = 0; i < IAT->ulCount; i++)
		{
			CString strIndex;
			strIndex.Format(L"%d",i+1);
			int n = m_ListCtrl->InsertItem(m_ListCtrl->GetItemCount(), strIndex);

			CString strFuncName(IAT->Data[i].szFunctionName);
			m_ListCtrl->SetItemText(n, 1, strFuncName);

			CString strCurFuncAddress;
			strCurFuncAddress.Format(L"0x%p",IAT->Data[i].CurFuncAddress);
			m_ListCtrl->SetItemText(n, 2, strCurFuncAddress);

			CString strOriFuncAddress;
			strOriFuncAddress.Format(L"0x%p",IAT->Data[i].OriFuncAddress);
			m_ListCtrl->SetItemText(n, 3, strOriFuncAddress);

			CString strModuleName(IAT->Data[i].szModuleName);
			m_ListCtrl->SetItemText(n, 4, strModuleName);

			CString strFuncStatus;
			if (IAT->Data[i].CurFuncAddress == IAT->Data[i].OriFuncAddress)
			{
				strFuncStatus = L"正常";
			}
			else
			{
				strFuncStatus = L"Hooked";
				m_ListCtrl->SetItemData(n,1);
			}
			m_ListCtrl->SetItemText(n, 5, strFuncStatus);

			CString StatusBarContext;
			StatusBarContext.Format(L"ntoskrnl.exe的IAT正在加载。 IAT函数：%d",i);

			HsSendStatusDetail(StatusBarContext);
		}

		CString StatusBarContext;
		StatusBarContext.Format(L"ntoskrnl.exe的IAT加载完成。 IAT函数：%d",IAT->ulCount);

		HsSendStatusDetail(StatusBarContext);

		bOk = TRUE;
	}
	else
	{
		HsSendStatusDetail(L"ntoskrnl.exe的IAT加载失败。");
	}
	//m_strIATCount.Format(L"%d",IAT->ulCount);
	if (IAT)
	{
		free(IAT);
		IAT = NULL;
	}

	
	bIsChecking = FALSE;
}


void HsEnumKernelFuncNameNtoskrnlEAT(CListCtrl* m_ListCtrl)
{
	m_ListCtrl->DeleteAllItems();

	BOOL bOk = FALSE;

	ULONG_PTR   ulCount = 0x1000;
	PMODULE_EAT EAT = NULL;
	BOOL bRet = FALSE;
	DWORD ulReturnSize = 0;

	CString ModuleFile(L"ntoskrnl.exe");

	do 
	{
		ULONG_PTR ulSize = 0;

		if (EAT)
		{
			free(EAT);
			EAT = NULL;
		}

		ulSize = sizeof(MODULE_EAT) + ulCount * sizeof(EAT_INFO);

		EAT = (PMODULE_EAT)malloc(ulSize);
		if (!EAT)
		{
			break;
		}

		memset(EAT,0,ulSize);

		int KernelFile = HS_KERNEL_KERNELFILE_NTOSKRNL_EAT;

		bRet = DeviceIoControl(g_hDevice,HS_IOCTL(HS_IOCTL_KRNL_KRNLFILE),
			&KernelFile,
			sizeof(int),
			EAT,
			(DWORD)ulSize,
			&ulReturnSize,
			NULL);

		ulCount = EAT->ulCount + 1000;

	} while (bRet == FALSE && GetLastError() == ERROR_INSUFFICIENT_BUFFER);

	if (bRet && EAT)
	{
		for (ULONG i = 0; i < EAT->ulCount; i++)
		{
			CString strIndex;
			strIndex.Format(L"%d",i+1);
			int n = m_ListCtrl->InsertItem(m_ListCtrl->GetItemCount(), strIndex);

			CString strFuncName(EAT->Data[i].szFunctionName);
			m_ListCtrl->SetItemText(n, 1, strFuncName);

			CString strCurFuncAddress;
			strCurFuncAddress.Format(L"0x%p",EAT->Data[i].CurFuncAddress);
			m_ListCtrl->SetItemText(n, 2, strCurFuncAddress);

			CString strOriFuncAddress;
			strOriFuncAddress.Format(L"0x%p",EAT->Data[i].OriFuncAddress);
			m_ListCtrl->SetItemText(n, 3, strOriFuncAddress);

			CString strModuleName(ModuleFile);
			m_ListCtrl->SetItemText(n, 4, strModuleName);

			CString strFuncStatus;
			if (EAT->Data[i].CurFuncAddress == EAT->Data[i].OriFuncAddress)
			{
				strFuncStatus = L"正常";
			}
			else
			{
				strFuncStatus = L"Hooked";
				m_ListCtrl->SetItemData(n,1);
			}
			m_ListCtrl->SetItemText(n, 5, strFuncStatus);

			CString StatusBarContext;
			StatusBarContext.Format(L"ntoskrnl.exe的EAT正在加载。 EAT函数：%d",i);

			HsSendStatusDetail(StatusBarContext);
		}

		CString StatusBarContext;
		StatusBarContext.Format(L"ntoskrnl.exe的EAT加载完成。 EAT函数：%d",EAT->ulCount);

		HsSendStatusDetail(StatusBarContext);

		bOk = TRUE;
	}
	else
	{
		HsSendStatusDetail(L"ntoskrnl.exe的EAT加载失败。");
	}
	//m_strEATCount.Format(L"%d",EAT->ulCount);
	if (EAT)
	{
		free(EAT);
		EAT = NULL;
	}

	
	bIsChecking = FALSE;
}


void HsEnumKernelFuncNameWin32kIAT(CListCtrl* m_ListCtrl)
{
	m_ListCtrl->DeleteAllItems();

	BOOL bOk = FALSE;

	ULONG_PTR   ulCount = 0x1000;
	PMODULE_IAT IAT = NULL;
	BOOL bRet = FALSE;
	DWORD ulReturnSize = 0;

	do 
	{
		ULONG_PTR ulSize = 0;

		if (IAT)
		{
			free(IAT);
			IAT = NULL;
		}

		ulSize = sizeof(MODULE_IAT) + ulCount * sizeof(IAT_INFO);

		IAT = (PMODULE_IAT)malloc(ulSize);
		if (!IAT)
		{
			break;
		}

		memset(IAT,0,ulSize);

		int KernelFile = HS_KERNEL_KERNELFILE_WIN32K_IAT;

		bRet = DeviceIoControl(g_hDevice,HS_IOCTL(HS_IOCTL_KRNL_KRNLFILE),
			&KernelFile,
			sizeof(int),
			IAT,
			(DWORD)ulSize,
			&ulReturnSize,
			NULL);


		ulCount = IAT->ulCount + 1000;

	} while (bRet == FALSE && GetLastError() == ERROR_INSUFFICIENT_BUFFER);

	if (bRet && IAT)
	{
		for (ULONG i = 0; i < IAT->ulCount; i++)
		{
			CString strIndex;
			strIndex.Format(L"%d",i+1);
			int n = m_ListCtrl->InsertItem(m_ListCtrl->GetItemCount(), strIndex);

			CString strFuncName(IAT->Data[i].szFunctionName);
			m_ListCtrl->SetItemText(n, 1, strFuncName);

			CString strCurFuncAddress;
			strCurFuncAddress.Format(L"0x%p",IAT->Data[i].CurFuncAddress);
			m_ListCtrl->SetItemText(n, 2, strCurFuncAddress);

			CString strOriFuncAddress;
			strOriFuncAddress.Format(L"0x%p",IAT->Data[i].OriFuncAddress);
			m_ListCtrl->SetItemText(n, 3, strOriFuncAddress);

			CString strModuleName(IAT->Data[i].szModuleName);
			m_ListCtrl->SetItemText(n, 4, strModuleName);

			CString strFuncStatus;
			if (IAT->Data[i].CurFuncAddress == IAT->Data[i].OriFuncAddress)
			{
				strFuncStatus = L"正常";
			}
			else
			{
				strFuncStatus = L"Hooked";
				m_ListCtrl->SetItemData(n,1);
			}
			m_ListCtrl->SetItemText(n, 5, strFuncStatus);

			CString StatusBarContext;
			StatusBarContext.Format(L"win32k.sys的IAT正在加载。 IAT函数：%d",i);

			HsSendStatusDetail(StatusBarContext);

		}

		CString StatusBarContext;
		StatusBarContext.Format(L"win32k.sys的IAT加载完成。 IAT函数：%d",IAT->ulCount);

		HsSendStatusDetail(StatusBarContext);

		bOk = TRUE;
	}
	else
	{
		HsSendStatusDetail(L"win32k.sys的IAT加载失败。");
	}
	//m_strIATCount.Format(L"%d",IAT->ulCount);
	if (IAT)
	{
		free(IAT);
		IAT = NULL;
	}

	
	bIsChecking = FALSE;
}


void HsEnumKernelFuncNameWin32kEAT(CListCtrl* m_ListCtrl)
{
	m_ListCtrl->DeleteAllItems();

	BOOL bOk = FALSE;

	ULONG_PTR   ulCount = 0x1000;
	PMODULE_EAT EAT = NULL;
	BOOL bRet = FALSE;
	DWORD ulReturnSize = 0;

	CString ModuleFile(L"win32k.sys");

	do 
	{
		ULONG_PTR ulSize = 0;

		if (EAT)
		{
			free(EAT);
			EAT = NULL;
		}

		ulSize = sizeof(MODULE_EAT) + ulCount * sizeof(EAT_INFO);

		EAT = (PMODULE_EAT)malloc(ulSize);
		if (!EAT)
		{
			break;
		}

		memset(EAT,0,ulSize);

		int KernelFile = HS_KERNEL_KERNELFILE_WIN32K_EAT;

		bRet = DeviceIoControl(g_hDevice,HS_IOCTL(HS_IOCTL_KRNL_KRNLFILE),
			&KernelFile,
			sizeof(int),
			EAT,
			(DWORD)ulSize,
			&ulReturnSize,
			NULL);

		ulCount = EAT->ulCount + 1000;

	} while (bRet == FALSE && GetLastError() == ERROR_INSUFFICIENT_BUFFER);

	if (bRet && EAT)
	{
		for (ULONG i = 0; i < EAT->ulCount; i++)
		{
			CString strIndex;
			strIndex.Format(L"%d",i+1);
			int n = m_ListCtrl->InsertItem(m_ListCtrl->GetItemCount(), strIndex);

			CString strFuncName(EAT->Data[i].szFunctionName);
			m_ListCtrl->SetItemText(n, 1, strFuncName);

			CString strCurFuncAddress;
			strCurFuncAddress.Format(L"0x%p",EAT->Data[i].CurFuncAddress);
			m_ListCtrl->SetItemText(n, 2, strCurFuncAddress);

			CString strOriFuncAddress;
			strOriFuncAddress.Format(L"0x%p",EAT->Data[i].OriFuncAddress);
			m_ListCtrl->SetItemText(n, 3, strOriFuncAddress);

			CString strModuleName(ModuleFile);
			m_ListCtrl->SetItemText(n, 4, strModuleName);

			CString strFuncStatus;
			if (EAT->Data[i].CurFuncAddress == EAT->Data[i].OriFuncAddress)
			{
				strFuncStatus = L"正常";
			}
			else
			{
				strFuncStatus = L"Hooked";
				m_ListCtrl->SetItemData(n,1);
			}
			m_ListCtrl->SetItemText(n, 5, strFuncStatus);

			CString StatusBarContext;
			StatusBarContext.Format(L"win32k.sys的EAT正在加载。 EAT函数：%d",i);

			HsSendStatusDetail(StatusBarContext);
		}

		CString StatusBarContext;
		StatusBarContext.Format(L"win32k.sys的EAT加载完成。 EAT函数：%d",EAT->ulCount);

		HsSendStatusDetail(StatusBarContext);

		bOk = TRUE;
	}
	else
	{
		HsSendStatusDetail(L"win32k.sys的EAT加载失败。");
	}

	if (EAT)
	{
		free(EAT);
		EAT = NULL;
	}

	bIsChecking = FALSE;
}

void HsEnumKernelFuncNameHaldllIAT(CListCtrl* m_ListCtrl)
{
	m_ListCtrl->DeleteAllItems();

	BOOL bOk = FALSE;

	ULONG_PTR   ulCount = 0x1000;
	PMODULE_IAT IAT = NULL;
	BOOL bRet = FALSE;
	DWORD ulReturnSize = 0;

	do 
	{
		ULONG_PTR ulSize = 0;

		if (IAT)
		{
			free(IAT);
			IAT = NULL;
		}

		ulSize = sizeof(MODULE_IAT) + ulCount * sizeof(IAT_INFO);

		IAT = (PMODULE_IAT)malloc(ulSize);
		if (!IAT)
		{
			break;
		}

		memset(IAT,0,ulSize);

		int KernelFile = HS_KERNEL_KERNELFILE_HALDLL_IAT;

		bRet = DeviceIoControl(g_hDevice,HS_IOCTL(HS_IOCTL_KRNL_KRNLFILE),
			&KernelFile,
			sizeof(int),
			IAT,
			(DWORD)ulSize,
			&ulReturnSize,
			NULL);


		ulCount = IAT->ulCount + 1000;

	} while (bRet == FALSE && GetLastError() == ERROR_INSUFFICIENT_BUFFER);

	if (bRet && IAT)
	{
		for (ULONG i = 0; i < IAT->ulCount; i++)
		{
			CString strIndex;
			strIndex.Format(L"%d",i+1);
			int n = m_ListCtrl->InsertItem(m_ListCtrl->GetItemCount(), strIndex);

			CString strFuncName(IAT->Data[i].szFunctionName);
			m_ListCtrl->SetItemText(n, 1, strFuncName);

			CString strCurFuncAddress;
			strCurFuncAddress.Format(L"0x%p",IAT->Data[i].CurFuncAddress);
			m_ListCtrl->SetItemText(n, 2, strCurFuncAddress);

			CString strOriFuncAddress;
			strOriFuncAddress.Format(L"0x%p",IAT->Data[i].OriFuncAddress);
			m_ListCtrl->SetItemText(n, 3, strOriFuncAddress);

			CString strModuleName(IAT->Data[i].szModuleName);
			m_ListCtrl->SetItemText(n, 4, strModuleName);

			CString strFuncStatus;
			if (IAT->Data[i].CurFuncAddress == IAT->Data[i].OriFuncAddress)
			{
				strFuncStatus = L"正常";
			}
			else
			{
				strFuncStatus = L"Hooked";
				m_ListCtrl->SetItemData(n,1);
			}
			m_ListCtrl->SetItemText(n, 5, strFuncStatus);

			CString StatusBarContext;
			StatusBarContext.Format(L"hal.dll的IAT正在加载。 IAT函数：%d",i);

			HsSendStatusDetail(StatusBarContext);

		}

		CString StatusBarContext;
		StatusBarContext.Format(L"hal.dll的IAT加载完成。 IAT函数：%d",IAT->ulCount);

		HsSendStatusDetail(StatusBarContext);

		bOk = TRUE;
	}
	else
	{
		HsSendStatusDetail(L"hal.dll的IAT加载失败。");
	}
	//m_strIATCount.Format(L"%d",IAT->ulCount);
	if (IAT)
	{
		free(IAT);
		IAT = NULL;
	}


	bIsChecking = FALSE;
}

void HsEnumKernelFuncNameHaldllEAT(CListCtrl* m_ListCtrl)
{
	m_ListCtrl->DeleteAllItems();

	BOOL bOk = FALSE;

	ULONG_PTR   ulCount = 0x1000;
	PMODULE_EAT EAT = NULL;
	BOOL bRet = FALSE;
	DWORD ulReturnSize = 0;

	CString ModuleFile(L"hal.dll");

	do 
	{
		ULONG_PTR ulSize = 0;

		if (EAT)
		{
			free(EAT);
			EAT = NULL;
		}

		ulSize = sizeof(MODULE_EAT) + ulCount * sizeof(EAT_INFO);

		EAT = (PMODULE_EAT)malloc(ulSize);
		if (!EAT)
		{
			break;
		}

		memset(EAT,0,ulSize);

		int KernelFile = HS_KERNEL_KERNELFILE_HALDLL_EAT;

		bRet = DeviceIoControl(g_hDevice,HS_IOCTL(HS_IOCTL_KRNL_KRNLFILE),
			&KernelFile,
			sizeof(int),
			EAT,
			(DWORD)ulSize,
			&ulReturnSize,
			NULL);

		ulCount = EAT->ulCount + 1000;

	} while (bRet == FALSE && GetLastError() == ERROR_INSUFFICIENT_BUFFER);

	if (bRet && EAT)
	{
		for (ULONG i = 0; i < EAT->ulCount; i++)
		{
			CString strIndex;
			strIndex.Format(L"%d",i+1);
			int n = m_ListCtrl->InsertItem(m_ListCtrl->GetItemCount(), strIndex);

			CString strFuncName(EAT->Data[i].szFunctionName);
			m_ListCtrl->SetItemText(n, 1, strFuncName);

			CString strCurFuncAddress;
			strCurFuncAddress.Format(L"0x%p",EAT->Data[i].CurFuncAddress);
			m_ListCtrl->SetItemText(n, 2, strCurFuncAddress);

			CString strOriFuncAddress;
			strOriFuncAddress.Format(L"0x%p",EAT->Data[i].OriFuncAddress);
			m_ListCtrl->SetItemText(n, 3, strOriFuncAddress);

			CString strModuleName(ModuleFile);
			m_ListCtrl->SetItemText(n, 4, strModuleName);

			CString strFuncStatus;
			if (EAT->Data[i].CurFuncAddress == EAT->Data[i].OriFuncAddress)
			{
				strFuncStatus = L"正常";
			}
			else
			{
				strFuncStatus = L"Hooked";
				m_ListCtrl->SetItemData(n,1);
			}
			m_ListCtrl->SetItemText(n, 5, strFuncStatus);

			CString StatusBarContext;
			StatusBarContext.Format(L"hal.dll的EAT正在加载。 EAT函数：%d",i);

			HsSendStatusDetail(StatusBarContext);
		}

		CString StatusBarContext;
		StatusBarContext.Format(L"hal.dll的EAT加载完成。 EAT函数：%d",EAT->ulCount);

		HsSendStatusDetail(StatusBarContext);

		bOk = TRUE;
	}
	else
	{
		HsSendStatusDetail(L"hal.dll的EAT加载失败。");
	}

	if (EAT)
	{
		free(EAT);
		EAT = NULL;
	}

	bIsChecking = FALSE;
}
