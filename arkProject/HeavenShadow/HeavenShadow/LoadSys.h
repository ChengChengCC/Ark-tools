#pragma once
#include "stdafx.h"
#include "afxcmn.h"

BOOL ExtractFile(void);

BOOL HsLoadNTDriver(WCHAR* lpszDriverName,WCHAR* lpszDriverPath);

BOOL HsUnloadNTDriver(WCHAR* szSvrName);