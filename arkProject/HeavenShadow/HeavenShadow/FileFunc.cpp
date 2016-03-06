#include "stdafx.h"
#include "FileFunc.h"
#include "Common.h"



COLUMNSTRUCT g_Column_Drive[] = 
{
	{	L"名称",			150	},
	{	L"类型",			150	},
	{	L"总大小",			150	},
	{	L"可用空间",		150	}
};

UINT g_Column_Drive_Count  = 4;	  //进程列表列数


COLUMNSTRUCT g_Column_File[] = 
{
	{	L"文件名称",		130	},
	{	L"大小",			100	},
	{	L"文件类型",		125	},
	{	L"创建时间",		125	},
	{	L"修改时间",		125	}
};

UINT g_Column_File_Count  = 5;	  //进程列表列数

extern int dpix;
extern int dpiy;


void HsInitDriveList(CListCtrl *m_ListCtrl)
{
	m_ListCtrl->SetExtendedStyle(LVS_EX_FULLROWSELECT|LVS_EX_GRIDLINES|LVS_EX_HEADERDRAGDROP);

	while(m_ListCtrl->DeleteColumn(0));

	UINT i;
	for (i = 0;i<g_Column_Drive_Count;i++)
	{
		if (i == 0 || i == 1)
		{
			m_ListCtrl->InsertColumn(i, g_Column_Drive[i].szTitle,LVCFMT_LEFT,(int)(g_Column_Drive[i].nWidth*(dpix/96.0)));
		}
		else
		{
			m_ListCtrl->InsertColumn(i, g_Column_Drive[i].szTitle,LVCFMT_RIGHT,(int)(g_Column_Drive[i].nWidth*(dpix/96.0)));
		}
		
	}
}




