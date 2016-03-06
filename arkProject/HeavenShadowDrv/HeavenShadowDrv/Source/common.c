#include "common.h"


//////////////////////////////////////////////////////////////////////////
ULONG_PTR  ulBuildNumber = 0;	//GetWindowsVersion

ULONG_PTR    SYSTEM_ADDRESS_START = 0;

//////////////////////////////////////////////////////////////////////////


VOID HsWcharToChar(WCHAR *wzFuncName,CHAR *szFuncName)
{
	UNICODE_STRING UnicodeFuncName;
	ANSI_STRING AnsiFuncName;

	RtlInitUnicodeString(&UnicodeFuncName,wzFuncName);
	if (RtlUnicodeStringToAnsiString(&AnsiFuncName,&UnicodeFuncName,TRUE) == STATUS_SUCCESS){
		memcpy(szFuncName,AnsiFuncName.Buffer,AnsiFuncName.Length);
		RtlFreeAnsiString(&AnsiFuncName);
	}
}


//通过 函数名称 得到函数地址
PVOID 
HsGetFunctionAddressByName(WCHAR *szFunction)
{
	UNICODE_STRING uniFunction;  
	PVOID AddrBase = NULL;

	if (szFunction && wcslen(szFunction) > 0)
	{
		RtlInitUnicodeString(&uniFunction, szFunction);   //常量指针
		AddrBase = MmGetSystemRoutineAddress(&uniFunction);
	}

	return AddrBase;
}



WIN_VERSION HsGetWindowsVersion()
{


	RTL_OSVERSIONINFOEXW osverInfo = {sizeof(osverInfo)}; 
	pfnRtlGetVersion RtlGetVersion = NULL;
	WIN_VERSION WinVersion;
	WCHAR szRtlGetVersion[] = L"RtlGetVersion";


	RtlGetVersion = (pfnRtlGetVersion)HsGetFunctionAddressByName(szRtlGetVersion); 

	if (RtlGetVersion)
	{
		RtlGetVersion((PRTL_OSVERSIONINFOW)&osverInfo); 
	} 
	else 
	{
		PsGetVersion(&osverInfo.dwMajorVersion, &osverInfo.dwMinorVersion, &osverInfo.dwBuildNumber, NULL);
	}

	ulBuildNumber =  osverInfo.dwBuildNumber;

	if (osverInfo.dwMajorVersion == 5 && osverInfo.dwMinorVersion == 1) 
	{
		DbgPrint("WINDOWS_XP\r\n");
		WinVersion = WINDOWS_XP;
	}
	else if (osverInfo.dwMajorVersion == 6 && osverInfo.dwMinorVersion == 1)
	{
		DbgPrint("WINDOWS 7\r\n");
		WinVersion = WINDOWS_7;
	}
	else if (osverInfo.dwMajorVersion == 6 && 
		osverInfo.dwMinorVersion == 2 &&
		osverInfo.dwBuildNumber == 9200)
	{
		DbgPrint("WINDOWS 8\r\n");
		WinVersion = WINDOWS_8;
	}
	else if (osverInfo.dwMajorVersion == 6 && 
		osverInfo.dwMinorVersion == 3 && 
		osverInfo.dwBuildNumber == 9600)
	{
		DbgPrint("WINDOWS 8.1\r\n");
		WinVersion = WINDOWS_8_1;
	}
	else
	{
		DbgPrint("WINDOWS_UNKNOW\r\n");
		WinVersion = WINDOWS_UNKNOW;
	}

	return WinVersion;
}

NTSTATUS HsSafeCopyMemory(PVOID SrcAddr, PVOID DstAddr, ULONG SrcSize)
{
	PMDL SrcMdl, DstMdl;
	PUCHAR SrcAddress, DstAddress;
	NTSTATUS Status = STATUS_UNSUCCESSFUL;
	ULONG_PTR r;

	//为拷贝源创建一个MDL
	SrcMdl = IoAllocateMdl(SrcAddr, SrcSize, FALSE, FALSE, NULL);  
	if (MmIsAddressValid(SrcMdl))
	{
		MmBuildMdlForNonPagedPool(SrcMdl);
		SrcAddress = MmGetSystemAddressForMdlSafe(SrcMdl, NormalPagePriority);
		//系统为我们创建内存

		if (MmIsAddressValid(SrcAddress))
		{
			//为拷贝目标创建一个MDL
			DstMdl = IoAllocateMdl(DstAddr,SrcSize,FALSE,FALSE, NULL);
			if (MmIsAddressValid(DstMdl))
			{
				__try
				{
					MmProbeAndLockPages(DstMdl, KernelMode, IoWriteAccess);
					DstAddress = MmGetSystemAddressForMdlSafe(DstMdl, NormalPagePriority);
					if (MmIsAddressValid(DstAddress))
					{
						RtlZeroMemory(DstAddress,SrcSize);
						RtlCopyMemory(DstAddress, SrcAddress, SrcSize);
						Status = STATUS_SUCCESS;
					}
					MmUnlockPages(DstMdl);
				}
				__except(EXCEPTION_EXECUTE_HANDLER)
				{                 
					if (DstMdl)
					{
						MmUnlockPages(DstMdl);
					}

					if (DstMdl)
					{
						IoFreeMdl(DstMdl);
					}

					if (SrcMdl)
					{
						IoFreeMdl(SrcMdl);
					}

					return GetExceptionCode();
				}
				IoFreeMdl(DstMdl);
			}
		}            
		IoFreeMdl(SrcMdl);
	}
	return Status;
}


BOOLEAN HsIsUnicodeStringValid(PUNICODE_STRING uniString)
{
	BOOLEAN bRet = FALSE;

	__try
	{
		if (uniString->Length > 0	&&
			uniString->Buffer		&&
			MmIsAddressValid(uniString->Buffer) &&
			MmIsAddressValid(&uniString->Buffer[uniString->Length / sizeof(WCHAR) - 1]))
		{
			bRet = TRUE;
		}

	}
	__except(1)
	{	
		bRet = FALSE;
	}

	return bRet;
}



