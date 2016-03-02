
// InlineHookSSSDT应用程序.h : PROJECT_NAME 应用程序的主头文件
//

#pragma once

#ifndef __AFXWIN_H__
	#error "在包含此文件之前包含“stdafx.h”以生成 PCH 文件"
#endif

#include "resource.h"		// 主符号


// CInlineHookSSSDT应用程序App:
// 有关此类的实现，请参阅 InlineHookSSSDT应用程序.cpp
//

class CInlineHookSSSDT应用程序App : public CWinApp
{
public:
	CInlineHookSSSDT应用程序App();

// 重写
public:
	virtual BOOL InitInstance();

// 实现

	DECLARE_MESSAGE_MAP()
};

extern CInlineHookSSSDT应用程序App theApp;