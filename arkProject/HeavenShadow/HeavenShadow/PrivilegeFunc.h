#pragma once
#include "stdafx.h"

#include "MyList.h"






VOID HsInitPrivilegeList(CMyList *m_ListCtrl);


//查询进程权限
VOID HsQueryProcessPrivilege(CMyList *m_ListCtrl);

VOID HsProcessPrivilegePopupMenu(CMyList *m_ListCtrl, CWnd* parent);

//设置启用权限
BOOL HsAdjustPrivilege(ULONG_PTR ProcessID, WCHAR* PrivilegeName, BOOL bIsEnable);