// Wow64Injectx64.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "Wow64Injectx64.h"
#include <memory>
#include <string>
#include <Windows.h>
#include "wow64ext.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#pragma comment(lib,"wow64ext.lib")

// 唯一的应用程序对象
CWinApp theApp;

using namespace std;

typedef struct _UNICODE_STRING {
	USHORT    Length;     //UNICODE占用的内存字节数，个数*2；
	USHORT	  MaximumLength; 
	DWORD64   Buffer;     //注意这里指针的问题
} UNICODE_STRING ,*PUNICODE_STRING;



unsigned char shell_code[] = {
	0x48, 0x89, 0x4c, 0x24, 0x08,                               // mov       qword ptr [rsp+8],rcx 
	0x57,                                                       // push      rdi
	0x48, 0x83, 0xec, 0x20,                                     // sub       rsp,20h
	0x48, 0x8b, 0xfc,                                           // mov       rdi,rsp
	0xb9, 0x08, 0x00, 0x00, 0x00,                               // mov       ecx,8
	0xb8, 0xcc, 0xcc, 0xcc, 0xcc,                               // mov       eac,0CCCCCCCCh
	0xf3, 0xab,                                                 // rep stos  dword ptr [rdi]
	0x48, 0x8b, 0x4c, 0x24, 0x30,                               // mov       rcx,qword ptr [__formal]
	0x49, 0xb9, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // mov       r9,0  //PVOID*  BaseAddr opt
	0x49, 0xb8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // mov       r8,0  //PUNICODE_STRING Name
	0x48, 0xba, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // mov       rdx,0
	0x48, 0xb9, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // mov       rcx,0
	0x48, 0xb8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // mov       rax,0 
	0xff, 0xd0,                                                 // call      rax   LdrLoadDll
	0x48, 0xb9, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // mov       rcx,0
	0x48, 0xb8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // mov       rax,0
	0xff, 0xd0                                                  // call      rax
};


enum  InjectResult{
	OK,
	Error_NoSuchFile,
	Error_OpenProcess,
	Error_VirtualAllocEx,
	Error_GetProcAddress,
	Error_WriteProcessMemory,
	Error_CreateRemoteThread
};


InjectResult Wow64Injectx64(DWORD processid,const TCHAR* file_path);

int _tmain(int argc, TCHAR* argv[], TCHAR* envp[])
{
	cout<<"查看要注入进程的ID"<<endl;   
	ULONG_PTR ProcessID = 0;
	
	printf("Input ProcessID\r\n");
	cin>>ProcessID;
	WCHAR file_path[] = L"E:\\Messagebox.dll";

	
	if (OK==Wow64Injectx64(ProcessID,file_path))
	{
		printf("Inject Success!\n");
	}
	return 0;
}


InjectResult Wow64Injectx64(DWORD processid,const TCHAR* file_path)
{
	
	if (!PathFileExists(file_path))
	{
		return Error_NoSuchFile;
	}

	HANDLE handle = OpenProcess(PROCESS_ALL_ACCESS,FALSE,processid);
	if (INVALID_HANDLE_VALUE == handle)
	{
		return Error_OpenProcess;
	}

	size_t file_path_mem_length = (size_t)::_tcslen(file_path);
	size_t paramemter_size = (file_path_mem_length+1)*sizeof(TCHAR) + sizeof(UNICODE_STRING) + sizeof(DWORD64);
	DWORD64 paramemter_mem_addr = (DWORD64)VirtualAllocEx64(handle,NULL,paramemter_size,MEM_COMMIT,PAGE_READWRITE);
	DWORD64  shell_code_addr = (DWORD64)VirtualAllocEx64(handle,NULL,sizeof(shell_code),MEM_COMMIT,PAGE_EXECUTE_READWRITE);
	if ((!paramemter_mem_addr) || (!shell_code_addr))
	{
		return Error_VirtualAllocEx;
	}
	
	char * paramemter_mem_local = new char[paramemter_size];
	memset(paramemter_mem_local,0,paramemter_size);

	PUNICODE_STRING ptr_unicode_string = (PUNICODE_STRING)(paramemter_mem_local + sizeof(DWORD64));
	ptr_unicode_string->Length = file_path_mem_length;
	ptr_unicode_string->MaximumLength = file_path_mem_length*2;
	wcscpy((WCHAR*)(ptr_unicode_string+1),file_path);
	ptr_unicode_string->Buffer = (DWORD64)((char*)paramemter_mem_addr+sizeof(DWORD64)+sizeof(UNICODE_STRING));

	DWORD64 ntdll64 = GetModuleHandle64(L"ntdll.dll");
	DWORD64 ntdll_LdrLoadDll = GetProcAddress64(ntdll64,"LdrLoadDll");
	DWORD64 ntdll_RtlCreateUserThread = GetProcAddress64(ntdll64,"RtlCreateUserThread");
	DWORD64 ntdll_RtlExitThread = GetProcAddress64(ntdll64,"RtlExitUserThread");
	if (NULL == ntdll_LdrLoadDll || NULL==ntdll_RtlCreateUserThread || NULL==ntdll_RtlExitThread)
	{
		return Error_GetProcAddress;
	}

	//r9
	memcpy(shell_code+32,&paramemter_mem_addr,sizeof(DWORD64));

	//r8
	DWORD64 ptr = paramemter_mem_addr+sizeof(DWORD64);
	memcpy(shell_code+42,&ptr,sizeof(PUNICODE_STRING));

	//LdrLoaddll
	memcpy(shell_code+72,&ntdll_LdrLoadDll,sizeof(DWORD64));

	//RtlExitUserThread
	memcpy(shell_code+94,&ntdll_RtlExitThread,sizeof(DWORD64));
	size_t write_size = 0;
	if (!WriteProcessMemory64(handle,paramemter_mem_addr,paramemter_mem_local,paramemter_size,NULL) ||
		!WriteProcessMemory64(handle,shell_code_addr,shell_code,sizeof(shell_code),NULL))
	{
		return Error_WriteProcessMemory;
	}
	DWORD64 hRemoteThread = 0;
	struct {
		DWORD64 UniqueProcess;
		DWORD64 UniqueThread;
	} client_id;
	int a = X64Call(ntdll_RtlCreateUserThread,10,
		(DWORD64)handle,					// ProcessHandle
		(DWORD64)NULL,                      // SecurityDescriptor
		(DWORD64)FALSE,                     // CreateSuspended
		(DWORD64)0,                         // StackZeroBits
		(DWORD64)NULL,                      // StackReserved
		(DWORD64)NULL,                      // StackCommit
		shell_code_addr,					// StartAddress
		(DWORD64)NULL,                      // StartParameter
		(DWORD64)&hRemoteThread,            // ThreadHandle
		(DWORD64)&client_id);               // ClientID)
	if (INVALID_HANDLE_VALUE == (HANDLE)hRemoteThread)
	{
		return Error_CreateRemoteThread;
	}
	return OK;
}