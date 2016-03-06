
#include "stdafx.h"
#include "Common.h"



#ifndef NT_SUCCESS
#define NT_SUCCESS(Status) ((NTSTATUS)(Status) >= 0)
#endif
#ifndef STATUS_SUCCESS
#define STATUS_SUCCESS       ((NTSTATUS)0x00000000L)
#endif
#ifndef STATUS_UNSUCCESSFUL
#define STATUS_UNSUCCESSFUL ((NTSTATUS)0xC0000001L)
#endif



HANDLE g_hDevice = NULL;


extern WIN_VERSION WinVersion;
extern CWnd* g_wParent;



//在虚拟机中测试: WinXP, Vista, Win7, Win8(RP) 包含32/64位
typedef BOOL (WINAPI *LPFN_ISWOW64PROCESS) (HANDLE, PBOOL);
BOOL HsIs64BitWindows()
{
#if defined(_WIN64)
	return TRUE;  // 64位程序只在Win64系统中运行
#elif defined(_WIN32)
	// 32位程序在32/64位系统中运行。
	// 所以必须判断
	BOOL f64 = FALSE;
	LPFN_ISWOW64PROCESS fnIsWow64Process;

	fnIsWow64Process = (LPFN_ISWOW64PROCESS) GetProcAddress(GetModuleHandle(_T("kernel32")),"IsWow64Process");
	if(NULL != fnIsWow64Process)
	{
		return fnIsWow64Process(GetCurrentProcess(),&f64) && f64;
	}
	return FALSE;
#else
	return FALSE; // Win64不支持16位系统
#endif
}


VOID HsSendStatusDetail(LPCWSTR szBuffer)
{
	::SendMessageW(g_wParent->m_hWnd,HS_MESSAGE_STATUSDETAIL,NULL,(LPARAM)szBuffer);
}

VOID HsSendStatusTip(LPCWSTR szBuffer)
{
	::SendMessageW(g_wParent->m_hWnd,HS_MESSAGE_STATUSTIP,NULL,(LPARAM)szBuffer);
}


CString TrimPath(WCHAR * wzPath)
{
	CString strPath;

	if (wzPath[1] == ':' && wzPath[2] == '\\')
	{
		strPath = wzPath;
	}
	else if (wcslen(wzPath) > wcslen(L"\\SystemRoot\\") && 
		!_wcsnicmp(wzPath, L"\\SystemRoot\\", wcslen(L"\\SystemRoot\\")))
	{
		WCHAR szSystemDir[MAX_PATH] = {0};
		GetWindowsDirectory(szSystemDir, MAX_PATH);
		strPath.Format(L"%s\\%s", szSystemDir, wzPath + wcslen(L"\\SystemRoot\\"));
	}
	else if (wcslen(wzPath) > wcslen(L"system32\\") && 
		!_wcsnicmp(wzPath, L"system32\\", wcslen(L"system32\\")))
	{
		WCHAR szSystemDir[MAX_PATH] = {0};
		GetWindowsDirectory(szSystemDir, MAX_PATH);
		strPath.Format(L"%s\\%s", szSystemDir, wzPath/* + wcslen(L"system32\\")*/);
	}
	else if (wcslen(wzPath) > wcslen(L"\\??\\") &&
		!_wcsnicmp(wzPath, L"\\??\\", wcslen(L"\\??\\")))
	{
		strPath = wzPath + wcslen(L"\\??\\");
	}
	else if (wcslen(wzPath) > wcslen(L"%ProgramFiles%") &&
		!_wcsnicmp(wzPath, L"%ProgramFiles%", wcslen(L"%ProgramFiles%")))
	{
		WCHAR szSystemDir[MAX_PATH] = {0};
		if (GetWindowsDirectory(szSystemDir, MAX_PATH) != 0)
		{
			CString szTemp = szSystemDir;
			szTemp = szTemp.Left(szTemp.Find('\\'));
			szTemp += L"\\Program Files";
			szTemp += wzPath + wcslen(L"%ProgramFiles%"); 
			strPath = szTemp;
		}
	}
	else
	{
		strPath = wzPath;
	}

	strPath = GetLongPath(strPath);

	return strPath;
}

CString GetLongPath(CString szPath)
{
	CString strPath;

	if (szPath.Find(L'~') != -1)
	{
		WCHAR szLongPath[MAX_PATH] = {0};
		DWORD nRet = GetLongPathName(szPath, szLongPath, MAX_PATH);
		if (nRet >= MAX_PATH || nRet == 0)
		{
			strPath = szPath;
		}
		else
		{
			strPath = szLongPath;
		}
	}
	else
	{
		strPath = szPath;
	}

	return strPath;
}





CHAR* HsLoadDllContext(char* szFileName)
{
	DWORD dwReadWrite, LenOfFile=FileLen(szFileName);
	HANDLE hFile = CreateFileA(szFileName, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, 0, OPEN_EXISTING, 0, 0);
	if (hFile != INVALID_HANDLE_VALUE)
	{
		PCHAR Buffer=(PCHAR)malloc(LenOfFile);
		SetFilePointer(hFile, 0, 0, FILE_BEGIN);
		ReadFile(hFile, Buffer, LenOfFile, &dwReadWrite, 0);
		CloseHandle(hFile);
		return Buffer;
	}
	return NULL;
}


DWORD FileLen(char* szFileName)
{
	WIN32_FIND_DATAA FileInfo= {0};
	DWORD FileSize = 0;
	HANDLE hFind;
	hFind = FindFirstFileA(szFileName ,&FileInfo);
	if(hFind != INVALID_HANDLE_VALUE)
	{
		FileSize = FileInfo.nFileSizeLow;
		FindClose(hFind);
	}
	return FileSize;
}




ULONG_PTR HsGetKernelBase(char* szNtosName)
{
	typedef long (__stdcall *pfnZwQuerySystemInformation)
		(
		IN ULONG SystemInformationClass,
		IN PVOID SystemInformation,
		IN ULONG SystemInformationLength,
		IN PULONG ReturnLength OPTIONAL
		);
	typedef struct _SYSTEM_MODULE_INFORMATION_ENTRY64
	{
		ULONG Unknow1;
		ULONG Unknow2;
		ULONG Unknow3;
		ULONG Unknow4;
		PVOID Base;
		ULONG Size;
		ULONG Flags;
		USHORT Index;
		USHORT NameLength;
		USHORT LoadCount;
		USHORT ModuleNameOffset;
		char ImageName[256];
	} SYSTEM_MODULE_INFORMATION_ENTRY64, *PSYSTEM_MODULE_INFORMATION_ENTRY64;



	typedef struct _SYSTEM_MODULE_INFORMATION_ENTRY32
	{
		ULONG  Reserved[2];  
		ULONG  Base;        
		ULONG  Size;         
		ULONG  Flags;        
		USHORT Index;       
		USHORT Unknown;     
		USHORT LoadCount;   
		USHORT ModuleNameOffset;
		CHAR   ImageName[256];   
	} SYSTEM_MODULE_INFORMATION_ENTRY32, *PSYSTEM_MODULE_INFORMATION_ENTRY32;

#ifdef _WIN64
#define SYSTEM_MODULE_INFORMATION_ENTRY SYSTEM_MODULE_INFORMATION_ENTRY64
#else
#define SYSTEM_MODULE_INFORMATION_ENTRY SYSTEM_MODULE_INFORMATION_ENTRY32
#endif


	typedef struct _SYSTEM_MODULE_INFORMATION
	{
		ULONG Count;
		SYSTEM_MODULE_INFORMATION_ENTRY Module[1];
	} SYSTEM_MODULE_INFORMATION, *PSYSTEM_MODULE_INFORMATION;
#define SystemModuleInformation 11
#define STATUS_INFO_LENGTH_MISMATCH ((NTSTATUS)0xC0000004L)
	pfnZwQuerySystemInformation ZwQuerySystemInformationAddress = NULL;
	PSYSTEM_MODULE_INFORMATION  SystemModuleInformationPoint;
	ULONG NeedSize, BufferSize = 0x5000;
	PVOID Buffer = NULL;
	NTSTATUS bOk;
	ZwQuerySystemInformationAddress = (pfnZwQuerySystemInformation)GetProcAddress(GetModuleHandleA("ntdll.dll"),"ZwQuerySystemInformation");
	do
	{
		Buffer = malloc(BufferSize);
		if(Buffer == NULL ) 
		{
			return 0;
		}
		bOk = ZwQuerySystemInformationAddress( SystemModuleInformation, Buffer, BufferSize, &NeedSize );
		if( bOk == STATUS_INFO_LENGTH_MISMATCH )
		{
			free(Buffer);
			BufferSize *= 2;
		}
		else if(!NT_SUCCESS(bOk))
		{
			free(Buffer);
			return 0;
		}
	}
	while(bOk==STATUS_INFO_LENGTH_MISMATCH );
	SystemModuleInformationPoint = (PSYSTEM_MODULE_INFORMATION)Buffer;
	ULONG_PTR Address = (ULONG_PTR)(SystemModuleInformationPoint->Module[0].Base);

	if(szNtosName!=NULL)
	{
		//*////////////////////////////////////////////////////////////////////////
		memcpy(
			szNtosName,
			SystemModuleInformationPoint->Module[0].ImageName+SystemModuleInformationPoint->Module[0].ModuleNameOffset,
			strlen(SystemModuleInformationPoint->Module[0].ImageName+SystemModuleInformationPoint->Module[0].ModuleNameOffset)
			);
	}

	free(Buffer);

	return Address;
}



char *Strcat(char *Str1, char *Str2) 
{
	DWORD dwLen = (DWORD)(strlen(Str1)+strlen(Str2)+1);
	char* Str3 =(char*)malloc(dwLen);
	memcpy(Str3,Str1,strlen(Str1));
	memcpy(Str3+strlen(Str1),Str2,strlen(Str2)+1);
	return Str3;
}



int HsReloc(ULONG_PTR NewBase, ULONG_PTR OrigBase)
{
	PIMAGE_DOS_HEADER		DosHeader;
	PIMAGE_NT_HEADERS		NtHeader;
	PIMAGE_BASE_RELOCATION	RelocTable;
	ULONG i,dwOldProtect;
	DosHeader = (PIMAGE_DOS_HEADER)NewBase;
	if (DosHeader->e_magic != IMAGE_DOS_SIGNATURE)
	{
		return 0;
	}
	NtHeader = (PIMAGE_NT_HEADERS)((ULONG_PTR)NewBase + DosHeader->e_lfanew );
	if (NtHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].Size)//是否存在重定位表
	{
		RelocTable=(PIMAGE_BASE_RELOCATION)((ULONG_PTR)NewBase + NtHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].VirtualAddress);
		do
		{
			ULONG	NumOfReloc=(RelocTable->SizeOfBlock-sizeof(IMAGE_BASE_RELOCATION))/2;
			SHORT	MiniOffset=0;
			PUSHORT RelocData =(PUSHORT)((ULONG_PTR)RelocTable+sizeof(IMAGE_BASE_RELOCATION));
			for (i=0; i<NumOfReloc; i++) 
			{
				PULONG_PTR RelocAddress;//需要重定位的地址

				if (((*RelocData)>>12)==IMAGE_REL_BASED_DIR64||((*RelocData)>>12)==IMAGE_REL_BASED_HIGHLOW)//判断重定位类型是否为IMAGE_REL_BASED_HIGHLOW[32]或IMAGE_REL_BASED_DIR64[64]
				{

					MiniOffset=(*RelocData)&0xFFF;//小偏移

					RelocAddress=(PULONG_PTR)(NewBase+RelocTable->VirtualAddress+MiniOffset);

					VirtualProtect((PVOID)RelocAddress,sizeof(ULONG_PTR),PAGE_EXECUTE_READWRITE, &dwOldProtect);

					*RelocAddress=*RelocAddress+OrigBase-NtHeader->OptionalHeader.ImageBase;

					VirtualProtect((PVOID)RelocAddress, sizeof(ULONG_PTR),dwOldProtect,&dwOldProtect);
				}
				//下一个重定位数据
				RelocData++;
			}
			//下一个重定位块
			RelocTable=(PIMAGE_BASE_RELOCATION)((ULONG_PTR)RelocTable+RelocTable->SizeOfBlock);
		}
		while (RelocTable->VirtualAddress);
		return TRUE;
	}
	return FALSE;
}



CHAR *HsGetTempNtdll()
{
	char *szPath;
	szPath=(char*)malloc(260);
	memset(szPath,0,260);
	GetTempPathA(260,szPath);   //没有释放内存  
	return Strcat(szPath,"ntdll.dll");    
}


CHAR* HsGetSystemDir()
{
	char* szPath;
	szPath =(char *)malloc(20);
	memset(szPath,0,20);
	GetWindowsDirectoryA(szPath,20);
	return Strcat(szPath,"\\system32\\");
}





DWORD HsGetSSDTFunctionIndex(char *FunctionName)
{

	ULONG_PTR IndexOffset = 0;

	switch(WinVersion)
	{
	case Windows7:
		{		
			IndexOffset = 4;

			break;
		}

	case WindowsXP:
		{

			IndexOffset = 1;

			break;
		}
	}



	return *(DWORD*)((PUCHAR)GetProcAddress(GetModuleHandleW(L"ntdll.dll"),FunctionName)+IndexOffset);
}


DWORD HsGetSpecialIndex(char *FunctionName)
{
	switch(WinVersion)
	{
	case Windows7:
		{
			if(!_stricmp(FunctionName,"ZwQuerySystemTime"))
			{
				return 0x57;
			}
			return 0;
		}

	default:
		return 0;
	}
}



CHAR* HsGetTempWin32k()
{
	char* szPath = NULL;
	szPath=(char*)malloc(260);
	memset(szPath,0,260);
	GetTempPathA(260,szPath);
	return Strcat(szPath,"win32k.sys");
}



ULONG_PTR HsGetWin32kBase()
{
	typedef long (__stdcall *pfnZwQuerySystemInformation)
		(
		IN ULONG SystemInformationClass,
		IN PVOID SystemInformation,
		IN ULONG SystemInformationLength,
		IN PULONG ReturnLength OPTIONAL
		);
	typedef struct _SYSTEM_MODULE_INFORMATION_ENTRY64
	{
		ULONG Unknow1;
		ULONG Unknow2;
		ULONG Unknow3;
		ULONG Unknow4;
		PVOID Base;
		ULONG Size;
		ULONG Flags;
		USHORT Index;
		USHORT NameLength;
		USHORT LoadCount;
		USHORT ModuleNameOffset;
		char ImageName[256];
	} SYSTEM_MODULE_INFORMATION_ENTRY64, *PSYSTEM_MODULE_INFORMATION_ENTRY64;



	typedef struct _SYSTEM_MODULE_INFORMATION_ENTRY32
	{
		ULONG  Reserved[2];  
		ULONG  Base;        
		ULONG  Size;         
		ULONG  Flags;        
		USHORT Index;       
		USHORT Unknown;     
		USHORT LoadCount;   
		USHORT ModuleNameOffset;
		CHAR   ImageName[256];   
	} SYSTEM_MODULE_INFORMATION_ENTRY32, *PSYSTEM_MODULE_INFORMATION_ENTRY32;



#ifdef _WIN64
#define SYSTEM_MODULE_INFORMATION_ENTRY SYSTEM_MODULE_INFORMATION_ENTRY64
#else
#define SYSTEM_MODULE_INFORMATION_ENTRY SYSTEM_MODULE_INFORMATION_ENTRY32
#endif


	typedef struct _SYSTEM_MODULE_INFORMATION
	{
		ULONG Count;
		SYSTEM_MODULE_INFORMATION_ENTRY Module[1];
	} SYSTEM_MODULE_INFORMATION, *PSYSTEM_MODULE_INFORMATION;
#define SystemModuleInformation 11
#define STATUS_INFO_LENGTH_MISMATCH ((NTSTATUS)0xC0000004L)
	pfnZwQuerySystemInformation ZwQuerySystemInformationAddress = NULL;
	PSYSTEM_MODULE_INFORMATION  SystemModuleInformationPoint;
	ULONG NeedSize, BufferSize = 0x5000;
	PVOID Buffer = NULL;
	NTSTATUS bOk;
	ZwQuerySystemInformationAddress = (pfnZwQuerySystemInformation)GetProcAddress(GetModuleHandleA("ntdll.dll"),"ZwQuerySystemInformation");
	do
	{
		Buffer = malloc(BufferSize);
		if(Buffer == NULL ) 
		{
			return 0;
		}
		bOk = ZwQuerySystemInformationAddress( SystemModuleInformation, Buffer, BufferSize, &NeedSize );
		if( bOk == STATUS_INFO_LENGTH_MISMATCH )
		{
			free(Buffer);
			BufferSize *= 2;
		}
		else if(!NT_SUCCESS(bOk))
		{
			free(Buffer);
			return 0;
		}
	}
	while(bOk==STATUS_INFO_LENGTH_MISMATCH );
	SystemModuleInformationPoint = (PSYSTEM_MODULE_INFORMATION)Buffer;
	ULONG_PTR ModuleCount = SystemModuleInformationPoint->Count;

	ULONG_PTR Address = NULL;
	//遍历所有的模块
	int i = 0;
	for(i=0; i<ModuleCount;i++ )
	{
		if(_stricmp(SystemModuleInformationPoint->Module[i].ImageName+SystemModuleInformationPoint->Module[i].ModuleNameOffset,"win32k.sys")==0 )
		{
			Address=(ULONG_PTR)SystemModuleInformationPoint->Module[i].Base;
			break;
		}
	}
	free(Buffer);

	return Address;
}



ULONG_PTR HsGetWin32kImageBase(char *szFileName)
{
	PIMAGE_NT_HEADERS NtHeader;
	PIMAGE_DOS_HEADER DosHeader;
	ULONG_PTR ImageBaseAddr = 0;
	char* szNtosFileData=NULL;
	szNtosFileData = HsLoadDllContext(szFileName);
	DosHeader = (PIMAGE_DOS_HEADER)szNtosFileData;
	NtHeader  = (PIMAGE_NT_HEADERS)(szNtosFileData+DosHeader->e_lfanew);
	ImageBaseAddr=NtHeader->OptionalHeader.ImageBase;
	return ImageBaseAddr;
}