#pragma once
#include "stdafx.h"

#include "MyList.h"

#include "ProcessFunc.h"

typedef struct _PROCESSMODULE_INFO
{
	ULONG_PTR	ulBase;
	ULONG_PTR	ulSize;
	ULONG_PTR	ulEntryPoint;
	CHAR	    szPath[264];
} PROCESSMODULE_INFO,*PPROCESSMODULE_INFO;


VOID HsInitPModuleDetailList(CListCtrl *m_ListCtrl);


BOOL EnableDebugPri64();


BOOL EnableDebugPri32();


ULONG_PTR EnumProcessModule(_PROCESSMODULE_INFO* mi);


VOID HsLoadPModuleDetailList(CListCtrl *m_ListCtrl);
