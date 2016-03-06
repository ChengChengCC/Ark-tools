#include "stdafx.h"
#include "DpcTimerFunc.h"
#include "SSDTFunc.h"
#include "ModuleFunc.h"
#include "ProcessFunc.h"
#include "Common.h"
#include <vector>

using namespace std;


vector<DPC_TIMER> m_DPCTimerVector;
ULONG             m_ulDPCCount;
extern HANDLE g_hDevice;
extern WIN_VERSION WinVersion;
extern BOOL bIsChecking;
extern vector<DRIVER_INFO> m_DriverList;



COLUMNSTRUCT g_Column_DPCTimer[] = 
{
	{	L"定时器对象",			125	},
	{	L"设备对象",				125	},
	{	L"触发周期",				70	},
	{	L"函数入口",				125	},
	{	L"模块文件",				155	},
	{	L"出品厂商",				125	}
};


UINT g_Column_DPCTimer_Count = 6;

extern int dpix;
extern int dpiy;


VOID HsInitDPCTimerList(CListCtrl *m_ListCtrl)
{
	while(m_ListCtrl->DeleteColumn(0));
	m_ListCtrl->DeleteAllItems();

	m_ListCtrl->SetExtendedStyle(LVS_EX_FULLROWSELECT|LVS_EX_GRIDLINES|LVS_EX_HEADERDRAGDROP);

	UINT i;
	for (i = 0;i<g_Column_DPCTimer_Count;i++)
	{
		m_ListCtrl->InsertColumn(i, g_Column_DPCTimer[i].szTitle,LVCFMT_LEFT,(int)(g_Column_DPCTimer[i].nWidth*(dpix/96.0)));
	}
}



VOID HsLoadDPCTimerList(CListCtrl *m_ListCtrl)
{
	if (bIsChecking == TRUE)
	{
		return;
	}

	// 	while(bIsChecking == TRUE)
	// 	{
	// 		Sleep(10);
	// 	}

	bIsChecking = TRUE;

	HsSendStatusDetail(L"DPCTimer正在加载...");
	HsSendStatusTip(L"DPCTimer");

	HsQueryDPCTimerList(m_ListCtrl);

	bIsChecking = FALSE;
}



VOID HsQueryDPCTimerList(CListCtrl *m_ListCtrl)
{
	BOOL bRet = FALSE;

	m_ListCtrl->DeleteAllItems();
	m_DPCTimerVector.clear();

	bRet = EnumDriver();
	if (bRet == FALSE)
	{
		HsSendStatusDetail(L"驱动模块初始化失败。");
		return;
	}

	bRet = HsGetDPCTimerList();
	if (bRet == FALSE)
	{
		HsSendStatusDetail(L"DPCTimer初始化失败。");
		return;
	}

	HsInsertDPCTimerItem(m_ListCtrl);
}


BOOL HsGetDPCTimerList()
{
	ULONG_PTR ulCnt = 100;
	PDPC_TIMER_INFOR TimerInfor = NULL;
	BOOL bRet = FALSE;
	DWORD ulReturnSize = 0;


	do 
	{
		ULONG_PTR ulSize = sizeof(DPC_TIMER_INFOR) + ulCnt * sizeof(DPC_TIMER);

		if (TimerInfor)
		{
			free(TimerInfor);
			TimerInfor = NULL;
		}

		TimerInfor = (PDPC_TIMER_INFOR)malloc(ulSize);

		if (TimerInfor)
		{
			memset(TimerInfor, 0, ulSize);
			TimerInfor->ulCnt = (ULONG)ulCnt;		
			bRet = DeviceIoControl(g_hDevice,HS_IOCTL(HS_IOCTL_SYSK_DPCTIMER),
				NULL,
				0,
				TimerInfor,
				(DWORD)ulSize,
				&ulReturnSize,
				NULL);
		}

		ulCnt =TimerInfor->ulCnt + 10;

	} while (!bRet && TimerInfor->ulRetCnt > TimerInfor->ulCnt);

	if (bRet &&
		TimerInfor->ulCnt >= TimerInfor->ulRetCnt)
	{
		for (ULONG i = 0; i < TimerInfor->ulRetCnt; i++)
		{
			m_DPCTimerVector.push_back(TimerInfor->DpcTimer[i]);
		}
	}

	if (TimerInfor)
	{
		free(TimerInfor);
		TimerInfor = NULL;
	}

	return bRet;
}



VOID HsInsertDPCTimerItem(CListCtrl* m_ListCtrl)
{
	m_ulDPCCount = 0;
	for (vector<DPC_TIMER>::iterator itor = m_DPCTimerVector.begin(); itor != m_DPCTimerVector.end(); itor++)
	{
		CString strTimerObject, strPeriod, strDispatch, strPath, strDpc;

		strTimerObject.Format(L"0x%p", itor->TimerObject);
		strPeriod.Format(L"%d", itor->Period / 1000);
		strDispatch.Format(L"0x%p", itor->TimeDispatch);
		strPath = GetDriverPath(itor->TimeDispatch);
		strDpc.Format(L"0x%p", itor->Dpc);

		int n = m_ListCtrl->InsertItem(m_ListCtrl->GetItemCount(),strTimerObject);
		m_ListCtrl->SetItemText(n, 1, strDpc);
		m_ListCtrl->SetItemText(n, 2, strPeriod);
		m_ListCtrl->SetItemText(n, 3, strDispatch);
		m_ListCtrl->SetItemText(n, 4, strPath);
		m_ListCtrl->SetItemText(n, 5, HsGetFileCompanyName(strPath));


		m_ulDPCCount++;

		CString StatusBarContext;
		StatusBarContext.Format(L"DPCTimer正在加载。 定时器数：%d",m_ulDPCCount);
		HsSendStatusDetail(StatusBarContext);

	}

	CString StatusBarContext;
	StatusBarContext.Format(L"DPCTimer加载完成。 定时器数：%d",m_ulDPCCount);
	HsSendStatusDetail(StatusBarContext);

}




VOID HsRemoveDPCTimerItem(CListCtrl* m_ListCtrl)
{
	BOOL bRet = FALSE;
	DWORD ulReturnSize = 0;
	int Index = m_ListCtrl->GetSelectionMark();

	if (Index<0)
	{
		return;
	}

	CString Temp = m_ListCtrl->GetItemText(Index,0);

	REMOVE_DPCTIMER  RemoveDPCTimer;

	for ( vector <DPC_TIMER>::iterator Iter = m_DPCTimerVector.begin( ); Iter != m_DPCTimerVector.end( ); Iter++ )
	{
		CString strTimerObject;
		strTimerObject.Format(L"0x%08p", Iter->TimerObject);
		if (!strTimerObject.CompareNoCase(Temp))
		{

			RemoveDPCTimer.TimerObject = Iter->TimerObject;
			bRet = DeviceIoControl(g_hDevice,HS_IOCTL(HS_IOCTL_SYSK_REMOVEDPCTIMER),
				&RemoveDPCTimer,
				sizeof(REMOVE_DPCTIMER),
				NULL,
				0,
				&ulReturnSize,
				NULL);


			break;
		}	
	}

	m_ulDPCCount--;
	m_ListCtrl->DeleteItem(Index);

	CString StatusBarContext;
	StatusBarContext.Format(L"DPCTimer加载完成。 定时器数：%d",m_ulDPCCount);
	HsSendStatusDetail(StatusBarContext);


	bIsChecking = FALSE;
}