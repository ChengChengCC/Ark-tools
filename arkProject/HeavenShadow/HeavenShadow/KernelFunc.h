#pragma once
#include "stdafx.h"


//////////////////////////////////////////////////////////////////////////

enum HS_KERNEL_KERNELFILE
{
	HS_KERNEL_KERNELFILE_NTOSKRNL_IAT,
	HS_KERNEL_KERNELFILE_NTOSKRNL_EAT,
	HS_KERNEL_KERNELFILE_WIN32K_IAT,
	HS_KERNEL_KERNELFILE_WIN32K_EAT,
	HS_KERNEL_KERNELFILE_HALDLL_IAT,
	HS_KERNEL_KERNELFILE_HALDLL_EAT,
};

//////////////////////////////////////////////////////////////////////////

#define MAX_NAME 60

typedef struct _IAT_INFO_
{
	ULONG_PTR CurFuncAddress;
	ULONG_PTR OriFuncAddress;
	CHAR  szFunctionName[MAX_NAME];
	CHAR  szModuleName[MAX_NAME];
}IAT_INFO, *PIAT_INFO;

typedef struct _MODULE_IAT_
{
	ULONG_PTR ulCount;
	IAT_INFO  Data[1];
}MODULE_IAT, *PMODULE_IAT;

typedef struct _EAT_INFO_
{
	ULONG_PTR CurFuncAddress;
	ULONG_PTR OriFuncAddress;
	CHAR  szFunctionName[MAX_NAME];
}EAT_INFO, *PEAT_INFO;

typedef struct _MODULE_EAT_
{
	ULONG_PTR ulCount;
	EAT_INFO  Data[1];
}MODULE_EAT, *PMODULE_EAT;



void HsInitKernelFuncList(CListBox* m_ListBox, CListCtrl* m_ListCtrl);

void HsInitKernelFileList(CListBox* m_ListBox, CListCtrl* m_ListCtrl);

void SelchangeListKrnlFunc(CListBox* m_ListBox,  CListCtrl* m_ListCtrl);

void SelchangeListKrnlIAT(CListBox* m_ListBox,  CListCtrl* m_ListCtrl);
void SelchangeListKrnlEAT(CListBox* m_ListBox,  CListCtrl* m_ListCtrl);

void HsEnumKernelFuncNameNtoskrnlIAT(CListCtrl* m_ListCtrl);

void HsEnumKernelFuncNameWin32kIAT(CListCtrl* m_ListCtrl);

void HsEnumKernelFuncNameHaldllIAT(CListCtrl* m_ListCtrl);

//////////////////////////////////////////////////////////////////////////

void HsEnumKernelFuncNameNtoskrnlEAT(CListCtrl* m_ListCtrl);

void HsEnumKernelFuncNameWin32kEAT(CListCtrl* m_ListCtrl);

void HsEnumKernelFuncNameHaldllEAT(CListCtrl* m_ListCtrl);








