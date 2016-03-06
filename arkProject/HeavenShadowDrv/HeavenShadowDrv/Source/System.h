/**************************************************************************************
* AUTHOR : HeavenShadow
* DATE   : 2014-10-28
* MODULE : System.h
*
* Command: 
*	内核大功能的主文件
*
* Description:
*	与内核相关的所有功能集合文件
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


#define MAX_PATH 260


NTSTATUS HsDispatchControlForSystem(PIO_STACK_LOCATION  IrpSp, PVOID OutputBuffer, ULONG_PTR* ulRet);

VOID HsInitSystemGlobalVariable();





