#pragma once
#include "stdafx.h"

#include "MyList.h"


enum HS_ENUM_PROC_COLUMN	//进程列表列枚举
{
	HS_PROCESS_COLUMN_NAME = 0,
	HS_PROCESS_COLUMN_PID,
	HS_PROCESS_COLUMN_PPID,
	HS_PROCESS_COLUMN_PATH,
	HS_PROCESS_COLUMN_EPROCESS,
	HS_PROCESS_COLUMN_USERACCESS,
	HS_PROCESS_COLUMN_COMPANY
};



enum HS_PROCESSDIG_SEND_TYPE
{
	HS_PROCESSDIG_SEND_INSERT = 555,	//插入条目
	HS_PROCESSDIG_SEND_DELETE			//删除条目
};


//结构体//////////////////////////////////////////////////////////////////

typedef struct HS_PROCESSINFO
{

	WCHAR     Name[100];
	WCHAR     Path[260];
	WCHAR     CompanyName[100];
	BOOL      UserAccess;
	ULONG_PTR Pid;
	ULONG_PTR PPid;
	ULONG_PTR Eprocess;
	LONGLONG  CreateTime;

}HSPROCESSINFO, *PHSPROCESSINFO;



//函数////////////////////////////////////////////////////////////////////

void HsInitProcessList(CMyList *m_ListCtrl);

DWORD WINAPI HsQueryProcessFunction(CMyList *m_ListCtrl);



void HsQueryProcessList(CMyList *m_ListCtrl);

BOOL HsR3CanOpenProcess(DWORD dwPid);

CString HsGetFileCompanyName(CString strPath);

//查看文件属性
VOID HsCheckAttribute(CString strPath);

//定位文件位置
VOID HsLocationExplorer(CString strPath);





BOOL HsGetDefaultTextFileName(CString& strFile);

CString HsGetExcelDriver();
BOOL HsGetDefaultExcelFileName(CString& strExcelFile);

BOOL __stdcall HsIs32BitFile(const WCHAR * pwszFullPath);


//获取权限
BOOL HsDebugPrivilege(const WCHAR *pName, BOOL bEnable);


//排序比较函数
static int CALLBACK HsProcessListCompareProc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);


BOOL HsKillProcessByForce(CMyList* m_ListCtrl);


ULONG_PTR
HsQuerySystemProcessCount(CMyList *m_ListCtrl);