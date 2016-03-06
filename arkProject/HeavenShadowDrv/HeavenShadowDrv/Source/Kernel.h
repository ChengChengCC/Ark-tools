/**************************************************************************************
* AUTHOR : HeavenShadow
* DATE   : 2014-10-28
* MODULE : Kernel.h
*
* Command: 
*	内核钩子大功能的主文件
*
* Description:
*	与内核钩子相关的所有功能集合文件
*
****************************************************************************************
* Copyright (C) 2015 HeavenShadow.
****************************************************************************************/


#pragma once 


#if DBG
#define dprintf DbgPrint
#else
#define dprintf
#endif

#include <ntifs.h>
#include "common.h"

typedef struct _RESUME_DATA_ 
{
	ULONG ulIndex;
	ULONG_PTR ulFuncAddress;
}RESUME_DATA,*PRESUME_DATA;

#define MAX_PATH 260


NTSTATUS HsDispatchControlForKernel(PIO_STACK_LOCATION  IrpSp, PVOID OutputBuffer, ULONG_PTR* ulRet);

VOID HsInitKernelGlobalVariable();

ULONG_PTR GetKeServiceDescriptorTableShadow64();

ULONG_PTR GetSSSDTFunctionAddress64(ULONG_PTR ulIndex,ULONG_PTR ServiceTableBase);

ULONG_PTR GetKeServiceDescriptorTableShadow32();

ULONG_PTR GetSSSDTFunctionAddress32(ULONG_PTR ulIndex,ULONG_PTR ServiceTableBase);

VOID HsUnHookSSDT(ULONG Index, ULONG_PTR FuncAddr);

ULONG GetOffsetAddress(ULONG_PTR FuncAddr);

VOID WPOFF();

VOID WPON();