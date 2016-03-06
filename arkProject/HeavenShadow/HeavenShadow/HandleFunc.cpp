#include "stdafx.h"
#include "HandleFunc.h"
#include "Common.h"

#include "resource.h"


extern ULONG_PTR g_ulProcessId;
extern HANDLE g_hDevice;

vector<HANDLE_INFO> m_HandleVector;

COLUMNSTRUCT g_Column_ProcessHandle[] = 
{
	{	L"句柄",					60	},
	{	L"句柄名称",				205	},
	{	L"句柄对象",				130	},
	{	L"句柄类型",				125	},
	{	L"引用计数",				70	}
};


enum HS_HANDLE_COLUMN
{
	HS_HANDLE_COLUMN_HANDLEVALUE,
	HS_HANDLE_COLUMN_OBJECTNAME,
	HS_HANDLE_COLUMN_OBJECT,
	HS_HANDLE_COLUMN_TYPENAME,
	HS_HANDLE_COLUMN_HANDLECOUNT
};

UINT g_Column_ProcessHandle_Count  = 5;	  //进程列表列数

extern int dpix;
extern int dpiy;


VOID HsInitProcessHandleList(CMyList *m_ListCtrl)
{
	while(m_ListCtrl->DeleteColumn(0));
	m_ListCtrl->DeleteAllItems();

	m_ListCtrl->SetExtendedStyle(LVS_EX_FULLROWSELECT|LVS_EX_GRIDLINES|LVS_EX_HEADERDRAGDROP);

	UINT i;
	for (i = 0;i<g_Column_ProcessHandle_Count;i++)
	{
		m_ListCtrl->InsertColumn(i, g_Column_ProcessHandle[i].szTitle,LVCFMT_LEFT,(int)(g_Column_ProcessHandle[i].nWidth*(dpix/96.0)));
	}
}




VOID HsQueryProcessHandle(CMyList *m_ListCtrl)
{
	ULONG_PTR  ProcessID = g_ulProcessId;

	if (ProcessID <= 4)
	{
		return;
	}

	m_ListCtrl->DeleteAllItems();
	m_HandleVector.clear();

	BOOL bOk = FALSE;

	ULONG_PTR ulCount = 0x1000;
	PALL_HANDLES Handles= NULL;
	BOOL bRet = FALSE;
	DWORD ulReturnSize = 0;

	do 
	{
		ULONG_PTR ulSize = 0;

		if (Handles)
		{
			free(Handles);
			Handles = NULL;
		}

		ulSize = sizeof(ALL_HANDLES) + ulCount * sizeof(HANDLE_INFO);

		Handles = (PALL_HANDLES)malloc(ulSize);
		if (!Handles)
		{
			break;
		}

		memset(Handles,0,ulSize);


		bRet = DeviceIoControl(g_hDevice,HS_IOCTL(HS_IOCTL_PROC_PROCESSHANDLE),
			&ProcessID,
			sizeof(ULONG),
			Handles,
			(DWORD)ulSize,
			&ulReturnSize,
			NULL);


		ulCount = Handles->ulCount + 1000;

	} while (bRet == FALSE && GetLastError() == ERROR_INSUFFICIENT_BUFFER);

	if (bRet && Handles)
	{
		for (ULONG i = 0; i < Handles->ulCount; i++)
		{
			m_HandleVector.push_back(Handles->Handles[i]);
		}

		bOk = TRUE;
	}

	if (Handles)
	{
		free(Handles);
		Handles = NULL;
	}

	if (m_HandleVector.empty())
	{
		return;
	}

	for (vector <HANDLE_INFO>::iterator Iter = m_HandleVector.begin( ); 
		Iter != m_HandleVector.end( ); 
		Iter++ )
	{
		CString strHandle, strObject, strReferenceCount;

		strHandle.Format(L"0x%04X", ((HANDLE_INFO)*Iter).Handle);
		strObject.Format(L"0x%08p", ((HANDLE_INFO)*Iter).Object);
		strReferenceCount.Format(L"%d", ((HANDLE_INFO)*Iter).ReferenceCount);

		int n = m_ListCtrl->InsertItem(m_ListCtrl->GetItemCount(), strHandle);
		m_ListCtrl->SetItemText(n, 1, ((HANDLE_INFO)*Iter).HandleName);
		m_ListCtrl->SetItemText(n, 2, strObject);
		m_ListCtrl->SetItemText(n, 3, ((HANDLE_INFO)*Iter).ObjectName);
		m_ListCtrl->SetItemText(n, 4, strReferenceCount);
	}

// 	ULONG dwReturnSize = 0;
// 	ULONG dwRet = 0;
// 
// 	ULONG nSize = 0x100000;
// 
// 	PHSHANDLE pHandleInfo = NULL;
// 
// 	if (ProcessID == 0)
// 	{
// 		return;
// 	}
// 
// 
// 	pHandleInfo = (PHSHANDLE)VirtualAlloc(0, nSize,MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
// 
// 	ZeroMemory(pHandleInfo,nSize);
// 
// 
// 	dwRet = DeviceIoControl(g_hDevice,HS_IOCTL(HS_IOCTL_PROC_PROCESSHANDLE),
// 		&ProcessID,
// 		sizeof(ULONG_PTR),
// 		pHandleInfo,
// 		nSize,
// 		&dwReturnSize,
// 		NULL);
// 
// 	if (dwRet == 0)
// 	{
// 		return;
// 	}
// 
// 	for (int i = 0; i < (int)pHandleInfo->NumberOfHandles; i++)
// 	{
// 		CString Temp;
// 		Temp.Format(L"0x%08X",pHandleInfo->Handles[i].HandleValue);
// 		m_ListCtrl->InsertItem(i,Temp);
// 		Temp.Empty();
// 
// 		Temp.Format(L"0x%p",pHandleInfo->Handles[i].Object);
// 		m_ListCtrl->SetItemText(i,HS_HANDLE_COLUMN_OBJECT,Temp);
// 		Temp.Empty();
// 
// 		Temp.Format(L"%d",pHandleInfo->Handles[i].ObjectTypeIndex);
// 		m_ListCtrl->SetItemText(i,HS_HANDLE_COLUMN_TYPEINDEX,Temp);
// 		Temp.Empty();
// 	}
}