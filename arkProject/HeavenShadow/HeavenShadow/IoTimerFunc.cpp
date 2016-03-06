#include "stdafx.h"
#include "IoTimerFunc.h"
#include "Common.h"
#include "ModuleFunc.h"
#include "ThreadFunc.h"
#include "SSDTFunc.h"
#include <vector>

#include "ProcessFunc.h"

using namespace std;

typedef struct _PCOMMUNICATE_IO_TIMER_  
{
	PLIST_ENTRY     TimerEntry;
	ULONG_PTR       DeviceObject;
	BOOLEAN         bStart;
}COMMUNICATE_IO_TIMER,*PCOMMUNICATE_IO_TIMER;
typedef struct _IO_TIMERS_
{
	ULONG_PTR TimerObject;
	ULONG_PTR DeviceObject;
	ULONG_PTR TimeDispatch;
	ULONG_PTR TimerEntry;
	ULONG     Status;
}IO_TIMERS, *PIO_TIMERS;

typedef struct _IO_TIMER_INFOR_
{
	ULONG ulCnt;
	ULONG ulRetCnt;
	IO_TIMERS IoTimer[1];
}IO_TIMER_INFOR, *PIO_TIMER_INFOR;

vector<IO_TIMERS> m_IOTimerVector;
extern HANDLE g_hDevice;
extern WIN_VERSION WinVersion;
extern BOOL bIsChecking;

COLUMNSTRUCT g_Column_IOTimer[] = 
{
	{	L"定时器对象",			125	},
	{	L"设备对象",				125	},
	{	L"状态",					45	},
	{	L"函数入口",				125	},
	{	L"模块路径",				180	},
	{	L"出品厂商",				125	}
};

ULONG_PTR m_ulIOTimerCount = 0;

UINT g_Column_IOTimer_Count = 6;

extern int dpix;
extern int dpiy;


VOID HsInitIOTimerList(CListCtrl *m_ListCtrl)
{
	while(m_ListCtrl->DeleteColumn(0));
	m_ListCtrl->DeleteAllItems();

	m_ListCtrl->SetExtendedStyle(LVS_EX_FULLROWSELECT|LVS_EX_GRIDLINES|LVS_EX_HEADERDRAGDROP);

	UINT i;
	for (i = 0;i<g_Column_IOTimer_Count;i++)
	{
		m_ListCtrl->InsertColumn(i, g_Column_IOTimer[i].szTitle,LVCFMT_LEFT,(int)(g_Column_IOTimer[i].nWidth*(dpix/96.0)));
	}
}



VOID HsLoadIOTimerList(CListCtrl *m_ListCtrl)
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

	HsSendStatusDetail(L"IOTimer正在加载...");
	HsSendStatusTip(L"IOTimer");

	HsQueryIOTimerList(m_ListCtrl);

	bIsChecking = FALSE;
}



VOID HsQueryIOTimerList(CListCtrl *m_ListCtrl)
{
	BOOL bRet = FALSE;

	bRet = EnumDriver();
	if (bRet == FALSE)
	{
		HsSendStatusDetail(L"驱动模块初始化失败...");
		return;
	}

	ULONG_PTR ulCnt = 100;
	PIO_TIMER_INFOR IOTimerInfor = NULL;
	DWORD ulReturnSize = 0;

	m_ListCtrl->DeleteAllItems();
	m_IOTimerVector.clear();

	do 
	{
		ULONG_PTR ulSize = sizeof(IO_TIMER_INFOR) + ulCnt * sizeof(IO_TIMERS);

		if (IOTimerInfor)
		{
			free(IOTimerInfor);
			IOTimerInfor = NULL;
		}

		IOTimerInfor = (PIO_TIMER_INFOR)malloc(ulSize);

		if (IOTimerInfor)
		{
			memset(IOTimerInfor, 0, ulSize);
			IOTimerInfor->ulCnt = (ULONG)ulCnt;		
			bRet = DeviceIoControl(g_hDevice,HS_IOCTL(HS_IOCTL_SYSK_IOTIMER),
				NULL,
				0,
				IOTimerInfor,
				(DWORD)ulSize,
				&ulReturnSize,
				NULL);
		}

		ulCnt =IOTimerInfor->ulCnt + 10;


	} while (!bRet && IOTimerInfor->ulRetCnt > IOTimerInfor->ulCnt);

	if (!bRet)
	{
		HsSendStatusDetail(L"IOTimer加载失败。");
		return;
	}

	if (bRet &&
		IOTimerInfor->ulCnt >= IOTimerInfor->ulRetCnt)
	{
		for (ULONG i = 0; i < IOTimerInfor->ulRetCnt; i++)
		{
			m_IOTimerVector.push_back(IOTimerInfor->IoTimer[i]);
		}
	}

	if (IOTimerInfor)
	{
		free(IOTimerInfor);
		IOTimerInfor = NULL;
	}

	//////////////////////////////////////////////////////////////////////////

	m_ulIOTimerCount = 0;

	for (vector<IO_TIMERS>::iterator itor = m_IOTimerVector.begin(); itor != m_IOTimerVector.end(); itor++)
	{
		CString strTimer, strDeviceObject, strPath, strStatus, strDispatch;

		strTimer.Format(L"0x%p", itor->TimerObject);
		strDeviceObject.Format(L"0x%p", itor->DeviceObject);
		strDispatch.Format(L"0x%p", itor->TimeDispatch);
		strPath = GetDriverPath(itor->TimeDispatch);

		if (itor->Status)
		{
			strStatus = L"运行";
		}
		else
		{
			strStatus = L"停止";
		}


		int n = m_ListCtrl->InsertItem(m_ListCtrl->GetItemCount(),strTimer);
		m_ListCtrl->SetItemText(n, 1, strDeviceObject);
		m_ListCtrl->SetItemText(n, 2, strStatus);
		m_ListCtrl->SetItemText(n, 3, strDispatch);
		m_ListCtrl->SetItemText(n, 4, strPath);
		m_ListCtrl->SetItemText(n, 5, HsGetFileCompanyName(strPath));

		m_ListCtrl->SetItemData(n, itor->TimerEntry);

		m_ulIOTimerCount++;

		CString StatusBarContext;
		StatusBarContext.Format(L"IOTimer正在加载。 定时器数：%d",m_ulIOTimerCount);
		HsSendStatusDetail(StatusBarContext);
	}

	CString StatusBarContext;
	StatusBarContext.Format(L"IOTimer加载完成。 定时器数：%d",m_ulIOTimerCount);
	HsSendStatusDetail(StatusBarContext);

}



VOID HsOperIOTimer(CListCtrl* m_ListCtrl)
{
	BOOL bRet = FALSE;
	DWORD ulReturnSize = 0;
	int Index = m_ListCtrl->GetSelectionMark();

	if (Index<0)
	{
		return;
	}

	CString Temp = m_ListCtrl->GetItemText(Index,1);

	COMMUNICATE_IO_TIMER  IOTimer;

	for ( vector <IO_TIMERS>::iterator Iter = m_IOTimerVector.begin( ); Iter != m_IOTimerVector.end( ); Iter++ )
	{
		CString strObject;
		strObject.Format(L"0x%p", Iter->DeviceObject);
		if (!strObject.CompareNoCase(Temp))
		{

			IOTimer.DeviceObject = Iter->DeviceObject;

			if (Iter->Status)
			{

				IOTimer.bStart = 0;
			}
			else
			{

				IOTimer.bStart = 1;
			}


			bRet = DeviceIoControl(g_hDevice,HS_IOCTL(HS_IOCTL_SYSK_OPERIOTIMER),
				&IOTimer,
				sizeof(COMMUNICATE_IO_TIMER),
				NULL,
				0,
				&ulReturnSize,
				NULL);

			break;
		}	
	}

	if (bRet == TRUE)
	{
		HsQueryIOTimerList(m_ListCtrl);
	}

	bIsChecking = FALSE;
}




VOID HsRemoveIOTimerItem(CListCtrl* m_ListCtrl)
{
	BOOL bRet = FALSE;
	DWORD ulReturnSize = 0;
	int Index = m_ListCtrl->GetSelectionMark();

	if (Index<0)
	{
		return;
	}

	CString Temp = m_ListCtrl->GetItemText(Index,3);

	COMMUNICATE_IO_TIMER  IOTimer;

	for ( vector <IO_TIMERS>::iterator Iter = m_IOTimerVector.begin( ); Iter != m_IOTimerVector.end( ); Iter++ )
	{

		if (m_ListCtrl->GetItemData(Index)==Iter->TimerEntry)
		{

			IOTimer.TimerEntry = (PLIST_ENTRY)Iter->TimerEntry;


			bRet = DeviceIoControl(g_hDevice,HS_IOCTL(HS_IOCTL_SYSK_REMOVEIOTIMER),
				&IOTimer,
				sizeof(COMMUNICATE_IO_TIMER),
				NULL,
				0,
				&ulReturnSize,
				NULL);


			break;
		}	
	}
	

	if (bRet)
	{
		HsQueryIOTimerList(m_ListCtrl);
	}


	bIsChecking = FALSE;
}