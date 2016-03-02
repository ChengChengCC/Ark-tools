// KeInjectApc.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <iostream>
#include <Windows.h>
#include <WinIoCtl.h>
using namespace std;




#define CTL_KEINJECTAPC \
	CTL_CODE(FILE_DEVICE_UNKNOWN,0x830,METHOD_BUFFERED,FILE_ANY_ACCESS)



typedef struct _INJECT_INFO
{
	ULONG ProcessId;
	wchar_t DllName[1024];
}INJECT_INFO,*PINJECT_INFO;

int _tmain(int argc, _TCHAR* argv[])
{
	HANDLE hFile;
	INJECT_INFO InjectInfo;
	hFile=CreateFile(L"\\\\.\\DriverLink",
		GENERIC_READ|GENERIC_WRITE,FILE_SHARE_READ|FILE_SHARE_WRITE,
		NULL,OPEN_EXISTING,0,NULL);

	if(hFile==INVALID_HANDLE_VALUE)
	{
		printf("\nError: Unable to connect to the driver (%d)\n",GetLastError());
		return -1;
	}

	memset(&InjectInfo,0,sizeof(INJECT_INFO));
	scanf("%d",&(InjectInfo.ProcessId));
	wscanf(L"%s",InjectInfo.DllName);


	DWORD dwReturnSize = 0;
	DWORD dwRet = 0;
	//发送IO 控制码
	dwRet = DeviceIoControl(hFile,CTL_KEINJECTAPC,   //
		&InjectInfo,
		sizeof(INJECT_INFO),
		NULL,
		NULL,
		&dwReturnSize,
		NULL);

	CloseHandle(hFile);
	return 0;
}

