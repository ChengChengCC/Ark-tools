/***************************************************************************************
* AUTHOR : HeavenShadow
* DATE   : 2014-10-28
* MODULE : HeavenShadowDrv.H
*
* 天影 驱动程序主文件
*
* Description:
*		天影 驱动程序主文件
*
****************************************************************************************
* Copyright (C) 2015 HeavenShadow.
****************************************************************************************/


#ifndef _HeavenShadowDrv_H
#define _HeavenShadowDrv_H


#include <ntifs.h>
#include <devioctl.h>
#include "common.h"

#endif	//_HeavenShadowDrv_H


/***************************************************************************************
* NAME:			DriverEntry
*
* DESCRIPTION:	注册派遣例程
*					
* PARAMETERS:		DriverObject					IN		
*						被NT创建用于此驱动的DRIVER_OBJECT的地址
*					RegisterPath					IN		
*						代表了驱动相关的服务注册表项的UNICODE_STRING 	
*
* RETURNS:		NTSTATUS
****************************************************************************************/
NTSTATUS DriverEntry(IN PDRIVER_OBJECT DriverObject, IN PUNICODE_STRING RegisterPath);

/***************************************************************************************
* NAME:			HsDriverUnload
*
* DESCRIPTION:	动态卸载入口点的例程
*					
* PARAMETERS:		DriverObject					IN		DRIVER_OBJECT的地址
*
* RETURNS:		None
****************************************************************************************/
VOID HsDriverUnload(IN PDRIVER_OBJECT DriverObject);

/***************************************************************************************
* NAME:			HsDispatchControl
*
* DESCRIPTION:	IRP_MJ_DEVICE_CONTROL的派遣入口点
*					
* PARAMETERS:		DriverObject				IN		DRIVER_OBJECT的地址
*					Irp							IN		IRP的地址
*
* RETURNS:		NTSTATUS
*
* NOTES:			IRP_MJ_DEVICE_CONTROL
*					Parameters:
*					Parameters.DeviceIoControl.OutputBufferLength	OutBuffer的长度
*						字节计 (来自GUI的缓冲区长度)
*					Parameters.DeviceIoControl.InputBufferLength	InBuffer的长度
*						字节计 (来自DRIVER的缓冲区长度)
*					Parameters.DeviceIoControl.ControlCode			I/O 控制码
****************************************************************************************/
NTSTATUS HsDispatchControl(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp);



NTSTATUS
HsDispatchPass(PDEVICE_OBJECT DeviceObject,PIRP Irp);



