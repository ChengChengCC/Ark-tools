#pragma once

#include "stdafx.h"

#include <WinIoCtl.h>

typedef struct _COLUMNSTRUCT
{
	WCHAR*	szTitle;
	UINT    nWidth;
}COLUMNSTRUCT;


enum HS_ENUM_DLG_MSG	//子对话框、主对话框之间通信枚举
{
	HS_MESSAGE_STATUSDETAIL = 5425,	//修改状态栏详细信息
	HS_MESSAGE_STATUSTIP
};

enum HS_ENUM_IOCTL
{
	HS_IOCTL_PROC = 0x100,					//进程相关
	HS_IOCTL_PROC_SENDSELFPID,				//发送自己的PID到驱动层
	HS_IOCTL_PROC_PROCESSCOUNT,				//进程计数
	HS_IOCTL_PROC_PROCESSLIST,				//列举进程列表
	HS_IOCTL_PROC_PROTECTPROCESS,			//保护进程
	HS_IOCTL_PROC_KILLPROCESSBYFORCE,		//强制关闭进程
	HS_IOCTL_PROC_PROCESSTHREAD,			//线程
	HS_IOCTL_PROC_PROCESSTHREADMODULE,		//线程模块
	HS_IOCTL_PROC_PROCESSPRIVILEGE,			//列举进程权限
	HS_IOCTL_PROC_PRIVILEGE_ADJUST,			//改变进程权限
	HS_IOCTL_PROC_PROCESSHANDLE,			//句柄
	HS_IOCTL_PROC_PROCESSWINDOW,			//窗口
	HS_IOCTL_PROC_PROCESSMODULE,			//进程模块
	HS_IOCTL_PROC_PROCESSMEMORY,			//进程内存

	HS_IOCTL_MODU = 0x180,					//模块相关
	HS_IOCTL_MODU_MODULELIST,				//列举系统模块列表
	HS_IOCTL_MODU_REMOVEMODULE,				//卸载系统模块

	HS_IOCTL_KRNL = 0x200,					//内核钩子相关
	HS_IOCTL_KRNL_SSDTLIST,					//列举SSDT列表
	HS_IOCTL_KRNL_KISRVTAB,					//获取服务表
	HS_IOCTL_KRNL_RESUMESSDT,				//恢复SSDT函数
	HS_IOCTL_KRNL_SSSDTLIST,				//列举SSSDT列表
	HS_IOCTL_KRNL_WIN32KSERVICE,			//获取Win32k服务表
	HS_IOCTL_KRNL_KRNLFILE,					//获取内核文件		//具体哪个文件在InputBuffer的布尔型中确定
	HS_IOCTL_KRNL_KRNLIAT,					//获取内核导入表
	HS_IOCTL_KRNL_KRNLEAT,					//获取内核导出表

	HS_IOCTL_SYSK = 0x280,					//内核相关
	HS_IOCTL_SYSK_SYSTHREAD,				//内核线程
	HS_IOCTL_SYSK_IOTIMER,					//IOTIMER
	HS_IOCTL_SYSK_OPERIOTIMER,				//IOTIMER切换
	HS_IOCTL_SYSK_REMOVEIOTIMER,			//IOTIMER移除
	HS_IOCTL_SYSK_CALLBACKLIST,				//系统回调：驱动加载
	HS_IOCTL_SYSK_REMOVECALLBACK,			//删除系统回调
	HS_IOCTL_SYSK_DPCTIMER,					//DPCTimer
	HS_IOCTL_SYSK_REMOVEDPCTIMER,			//DPCTimer删除
	HS_IOCTL_SYSK_FILTERDRIVER,				//过滤驱动
	HS_IOCTL_SYSK_FILTERUNLOAD,				//卸载过滤驱动
};

enum HS_DLG_NUM
{
	HS_DIALOG_ABOUT = 1,		//关于对话框
	HS_DIALOG_PROCESS,			//进程对话框
	HS_DIALOG_MODULE,			//模块对话框
	HS_DIALOG_SYSTEM,			//内核对话框
	HS_DIALOG_KERNEL,			//内核钩子对话框
	HS_DIALOG_SERVICE,			//服务对话框
	HS_DIALOG_FILE,				//文件对话框
	HS_DIALOG_SETTING,			//设置对话框
	HS_DIALOG_TOOLS				//工具箱对话框
};

typedef enum _WIN_VERSION
{
	WindowsNT,
	Windows2000,
	WindowsXP,
	Windows2003,
	WindowsVista,
	Windows7,
	Windows8,
	Windows8_1,
	Windows10,
	WinUnknown
}WIN_VERSION;

//不支持符号链接用户相关性
#define HS_DEVICE_NAME                  L"\\Device\\HeavenShadowDevice"             // Driver Name
#define HS_LINK_NAME					L"\\\\.\\HeavenShadowLink"                  // Win32 Link Name

#define HS_EVENT_PROCESS_NAME			L"\\BaseNamedObjects\\HeavenShadowProcessEvent"

#define HS_DRIVER_NAME                  L"HeavenShadowDrv"
#define HS_DRIVER_PATH                  L"HeavenShadowDrv.sys"




#define HS_IOCTL(i)			 \
	CTL_CODE                 \
	(                        \
	FILE_DEVICE_UNKNOWN,     \
	i,						 \
	METHOD_NEITHER,          \
	FILE_ANY_ACCESS          \
	)

// HS_IOCTL(HS_IOCTL_PROC_PROCESSLIST)


//////////////////////////////////////////////////////////////////////////
//公有变量





//////////////////////////////////////////////////////////////////////////
//公有函数
HANDLE OpenDevice(LPCTSTR lpDevicePath);

BOOL HsIs64BitWindows();

VOID HsSendStatusDetail(LPCWSTR szBuffer);

VOID HsSendStatusTip(LPCWSTR szBuffer);

CString TrimPath(WCHAR * wzPath);

CString GetLongPath(CString szPath);

CHAR* HsLoadDllContext(char* szFileName);

DWORD FileLen(char* szFileName);

ULONG_PTR HsGetKernelBase(char* szNtosName);

char *Strcat(char *Str1, char *Str2);

int HsReloc(ULONG_PTR NewBase, ULONG_PTR OrigBase);

CHAR *HsGetTempNtdll();

CHAR* HsGetSystemDir();

DWORD HsGetSpecialIndex(char *FunctionName);

DWORD HsGetSSDTFunctionIndex(char *FunctionName);

CHAR* HsGetTempWin32k();

ULONG_PTR HsGetWin32kBase();

ULONG_PTR HsGetWin32kImageBase(char *szFileName);