#include "Window.h"



extern
	WIN_VERSION  WinVersion;
extern       
	ULONG_PTR ulBuildNumber;

extern
	pfnNtUserBuildHwndList AddressNtUserBuildHwndList;
extern
	pfnNtUserQueryWindow AddressNtUserQueryWindow;




NTSTATUS HsEnumProcessWindow(PVOID InBuffer, ULONG_PTR InSize, PVOID OutBuffer, ULONG_PTR OutLen)
{
	NTSTATUS Status = STATUS_UNSUCCESSFUL;

	GetSSSDTApi();

	if (!InBuffer || 
		!OutBuffer)
	{
		return STATUS_INVALID_PARAMETER;
	}


	if (AddressNtUserBuildHwndList==NULL)
	{
		return Status;
	}

	else
	{

		Status = AddressNtUserBuildHwndList(
			NULL,
			NULL,
			FALSE,
			0,
			(OutLen - sizeof(ALL_WNDS)) / sizeof(WND_INFO),
			(HWND*)((ULONG)OutBuffer + sizeof(ULONG)),
			(ULONG*)OutBuffer);
	}

	if (NT_SUCCESS(Status))
	{
		DWORD ulCount = *((DWORD*)OutBuffer);
		ULONG i = 0;
		HWND* hWndBuffer = (HWND*)ExAllocatePool(NonPagedPool,sizeof(HWND) * ulCount);



		if (hWndBuffer)
		{
			PALL_WNDS Wnds = (PALL_WNDS)OutBuffer;
			memcpy(hWndBuffer, (PVOID)((ULONG)OutBuffer + sizeof(ULONG)), sizeof(HWND) * ulCount);

			for (i = 0; i < ulCount; i++)
			{
				ULONG Tid = 0, Pid = 0;
				HWND hWnd = hWndBuffer[i];

				Pid = AddressNtUserQueryWindow(hWnd, 0);

				if (WinVersion == WINDOWS_7)
				{
					Tid = AddressNtUserQueryWindow(hWnd, 2);
				}
				else
				{
					Tid = AddressNtUserQueryWindow(hWnd, 1);
				}

				Wnds->WndInfo[i].hWnd = hWnd;
				Wnds->WndInfo[i].uPid = Pid;
				Wnds->WndInfo[i].uTid = Tid;
			}

			Wnds->nCnt = ulCount;

			ExFreePool(hWndBuffer,0);
		}
	}

	return Status;
}

