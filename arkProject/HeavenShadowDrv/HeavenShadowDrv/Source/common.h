/**************************************************************************************
* AUTHOR : HeavenShadow
* DATE   : 2014-10-28
* MODULE : common.h
*
* Command: 
*	IOCTRL 通用头文件
*
* Description:
*	驱动和应用程序的IoCtrl的通用数据
*
****************************************************************************************
* Copyright (C) 2015 HeavenShadow.
****************************************************************************************/

#pragma once 

#include <ntifs.h>

#if DBG
#define dprintf DbgPrint
#else
#define dprintf
#endif

//不支持符号链接用户相关性
#define HS_DEVICE_NAME                  L"\\Device\\HeavenShadowDevice"
#define HS_LINK_NAME                    L"\\DosDevices\\HeavenShadowLink"

#define HS_EVENT_PROCESS_NAME           L"\\BaseNamedObjects\\HeavenShadowProcessEvent"

#define HS_WIN32_LINK_NAME              "\\\\.\\HeavenShadowLink"

//支持符号链接用户相关性
#define SYMBOLIC_LINK_GLOBAL_NAME       L"\\DosDevices\\Global\\HeavenShadowDevice"

#define DATA_TO_APP                     "HeavenShadow"


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
	HS_IOCTL_SYSK_REMOVEIOTIMER,			//IOTIMER删除
	HS_IOCTL_SYSK_CALLBACKLIST,				//系统回调：驱动加载
	HS_IOCTL_SYSK_REMOVECALLBACK,			//删除系统回调
	HS_IOCTL_SYSK_DPCTIMER,					//DPCTimer
	HS_IOCTL_SYSK_REMOVEDPCTIMER,			//DPCTimer删除
	HS_IOCTL_SYSK_FILTERDRIVER,				//过滤驱动
	HS_IOCTL_SYSK_FILTERUNLOAD,				//卸载过滤驱动
};


#define HS_IOCTL(i)			 \
	CTL_CODE                 \
	(                        \
	FILE_DEVICE_UNKNOWN,     \
	i,						 \
	METHOD_BUFFERED,         \
	FILE_ANY_ACCESS          \
	)

// HS_IOCTL(HS_IOCTL_PROC_PROCESSLIST)




typedef 
NTSTATUS 
(*pfnRtlGetVersion)(OUT PRTL_OSVERSIONINFOW lpVersionInformation);

typedef enum WIN_VERSION {
	WINDOWS_UNKNOW,
	WINDOWS_XP,
	WINDOWS_7,
	WINDOWS_8,
	WINDOWS_8_1
} WIN_VERSION;

WIN_VERSION HsGetWindowsVersion();





//////////////////////////////////////////////////////////////////////////
///自定义函数
//////////////////////////////////////////////////////////////////////////


//通过 函数名称 得到函数地址
PVOID 
HsGetFunctionAddressByName(WCHAR *szFunction);

NTSTATUS HsSafeCopyMemory(PVOID SrcAddr, PVOID DstAddr, ULONG SrcSize);

VOID HsWcharToChar(WCHAR *wzFuncName,CHAR *szFuncName);


CHAR HsChangePreMode(PETHREAD EThread);
VOID HsRecoverPreMode(PETHREAD EThread, CHAR PreMode);

extern NTSTATUS PsLookupProcessByProcessId(PVOID ProcessID, PEPROCESS *Process);

BOOLEAN HsIsUnicodeStringValid(PUNICODE_STRING uniString);