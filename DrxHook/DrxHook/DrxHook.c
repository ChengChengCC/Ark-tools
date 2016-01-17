

#ifndef CXX_DRXHOOK_H
#	include "DrxHook.h"
#endif

#include <ntimage.h>


KIRQL  Irql;
PDRIVER_OBJECT g_LocalDriverObj;
BOOLEAN        g_bHookSuccess;
ULONG_PTR      g_RtlDispatchExeceptionAddress;
ULONG_PTR      g_JmpOrigDispatchException;
UCHAR		   g_cDisExceptionCode[5];

ULONG_PTR g_JmpOrigNtOpenProcess;

void __declspec(naked)  NewNtOpenProcess()
{

	__asm
	{
		pushad
		pushfd

		call FilterNtOpenProcess

		popfd
		popad

		mov		edi , edi
		push	esp
		mov		ebp , esp
		//跳过NtOpenProcess的前五个字节，
		//避免再次触发异常
		jmp		g_JmpOrigNtOpenProcess
	}
}


void __declspec(naked) NewRtlDispatchException()
{
	__asm
	{
		mov   edi,edi
		push  ebp
		mov   ebp , esp
		pushad     //保存所有寄存器
		pushfd     //保存标志寄存器
		push	[ebp+0xc]
		push	[ebp+0x8]
		call	FilterRtlDispatchException
		//检测返回值是否为0
		test	eax , eax
		jz		__SafeExit  // 若eax为0 跳转__SafeExit
		popfd
		popad
		mov		esp , ebp
		pop		ebp
		//  将KiDispatchException中对于RtlDispatchException的返回值进行校验，
		//  如果为0 则对异常进行重新派发，为1则不再做处理
		mov		eax ,0x01   
		retn	0x8     //平衡堆栈，两个参数8字节

__SafeExit:

		popfd
		popad
		mov		esp , ebp
		pop		ebp

		//先执行RtlDispatchException原来的5个字节的内容
		mov		edi , edi
		push	ebp
		mov		ebp , esp
		jmp g_JmpOrigDispatchException
	}
}


NTSTATUS  _stdcall FilterNtOpenProcess ()
{
	DbgPrint("FilterNtOpenProcess---%s\r\n",(ULONG_PTR)PsGetCurrentProcess()+0x16c);
	return  STATUS_SUCCESS;
}



ULONG_PTR _stdcall
	FilterRtlDispatchException (
	IN PEXCEPTION_RECORD ExceptionRecord,
	IN PCONTEXT ContextRecord
	)
{

	//DbgPrint("Address:%x -- ExceptionCode:%x\r\n",ExceptionRecord->ExceptionAddress,ExceptionRecord->ExceptionCode);
	//如果是NtOpenProcess处的异常
	if (ExceptionRecord->ExceptionAddress == (PVOID)KeServiceDescriptorTable.ServiceTableBase[190])
	{
		KdPrint(("<Except addresss>:%X <seh callBack>:%X -- <Except code>:%X",
			ContextRecord->Eip,ExceptionRecord->ExceptionAddress,ExceptionRecord->ExceptionCode));

		//将执行的下一条指令置为NewNtOpenProcess() 函数的地址，CPU接着去执行NewNtOpenProcess
		ContextRecord->Eip = (ULONG_PTR)NewNtOpenProcess;
		//返回TRUE,异常不再进行派发
		return 1;
	}
	return 0;
}

VOID SetMonitor(PVOID Address)
{

	__asm
	{
		mov eax , Address
		mov DR0 , eax
		mov eax , 0x02  //全局的，仅当执行时产生异常
		mov DR7 , eax
	}
}


VOID CancelMonitor(PVOID Address)
{

	__asm
	{
		xor eax , eax
		mov DR0 , eax
		mov DR7 , eax
	}
}
NTSTATUS DriverEntry(IN PDRIVER_OBJECT pDriverObject, IN PUNICODE_STRING RegistryString)
{
	NTSTATUS	Status = STATUS_SUCCESS;
	g_LocalDriverObj = pDriverObject;
	HookRtlDispatchException();
	g_JmpOrigNtOpenProcess = (ULONG_PTR)(KeServiceDescriptorTable.ServiceTableBase[190] + 0x5);
	//为了方便，这里写死了，NtOpenProcess  Win7 x86 
	SetMonitor((PVOID)KeServiceDescriptorTable.ServiceTableBase[190]);
	return Status;
}

VOID HookRtlDispatchException()
{

	PLDR_DATA_TABLE_ENTRY Ldr = NULL;
	//构建RtlDispatchException 的特征码
	// 	nt!KiDispatchException+0x160:
	// 	83eff040 53              push    ebx
	// 	83eff041 ff750c          push    dword ptr [ebp+0Ch]
	// 	83eff044 ff7510          push    dword ptr [ebp+10h]
	// 	83eff047 ff15bc49fb83    call    dword ptr [nt!KiDebugRoutine (83fb49bc)]
	// 	83eff04d 84c0            test    al,al
	// 	83eff04f 0f859d000000    jne     nt!KiDispatchException+0x211 (83eff0f2)
	// 	83eff055 57              push    edi
	// 	83eff056 53              push    ebx
	// 	83eff057 e8 a372ffff      call    nt!RtlDispatchException (83ef62ff)


	// 	kd> u 83ef62ff
	// 	nt!RtlDispatchException:
	// 	83ef62ff 8bff            mov     edi,edi
	// 	83ef6301 55              push    ebp
	// 	83ef6302 8bec            mov     ebp,esp


	// 	83ef6304 83e4f8          and     esp,0FFFFFFF8h
	// 	83ef6307 83ec6c          sub     esp,6Ch
	// 	83ef630a 53              push    ebx
	// 	83ef630b 56              push    esi
	// 	83ef630c 57              push    edi


	SIGNATURE_INFO SignCode[] = {{0x84,10},{0xc0,9},{0x57,2},{0x53,1},{0xE8,0}};
#ifndef _DEBUG
	__asm int 3
#endif 

	g_bHookSuccess  = FALSE;
	Ldr = SearchDriver(g_LocalDriverObj,L"ntoskrnl.exe");
	if (!Ldr)   return;
	g_RtlDispatchExeceptionAddress = SearchAddressForSignFromPE((ULONG_PTR)(Ldr->DllBase),Ldr->SizeOfImage,SignCode);	
	if (!MmIsAddressValid((PVOID)g_RtlDispatchExeceptionAddress))  return;
	//利用偏移转成绝对地址                                    +5 过e8 a372ffff 这五个字节
	g_RtlDispatchExeceptionAddress = g_RtlDispatchExeceptionAddress+5 + *(ULONG_PTR*)(g_RtlDispatchExeceptionAddress+1);
	//过被占的前5个字节，继续执行的代码
	DbgPrint("RtlDispatchExceptionAddresss:%x",g_RtlDispatchExeceptionAddress);
	g_JmpOrigDispatchException = g_RtlDispatchExeceptionAddress + 5;
	g_bHookSuccess = Jmp_HookFunction(g_RtlDispatchExeceptionAddress,(ULONG_PTR)NewRtlDispatchException,g_cDisExceptionCode);
}

//搜索整个PE文件的
ULONG_PTR SearchAddressForSignFromPE(ULONG_PTR uStartBase,ULONG_PTR uSearchLength,SIGNATURE_INFO SignatureInfo[5])
{
	UCHAR *p;
	ULONG_PTR u_index1,u_index2;

	//ULONG uIndex;
	PIMAGE_DOS_HEADER pimage_dos_header;
	PIMAGE_NT_HEADERS pimage_nt_header;
	PIMAGE_SECTION_HEADER pimage_section_header;

	if(!MmIsAddressValid((PVOID)uStartBase))
	{	return 0;	}

	pimage_dos_header = (PIMAGE_DOS_HEADER)uStartBase;
	pimage_nt_header = (PIMAGE_NT_HEADERS)((ULONG)uStartBase+pimage_dos_header->e_lfanew);
	pimage_section_header = (PIMAGE_SECTION_HEADER)((ULONG)pimage_nt_header+sizeof(IMAGE_NT_HEADERS));

	for (u_index1 = 0;u_index1<pimage_nt_header->FileHeader.NumberOfSections;u_index1++)
	{
		//#define IMAGE_SCN_MEM_EXECUTE                0x20000000  // Section is executable.
		//#define IMAGE_SCN_MEM_READ                   0x40000000  // Section is readable.
		//#define IMAGE_SCN_MEM_WRITE                  0x80000000  // Section is writeable.
		//0x60000000 = IMAGE_SCN_MEM_EXECUTE|IMAGE_SCN_MEM_READ
		if (pimage_section_header[u_index1].Characteristics&0x60000000)
		{
			p = (UCHAR*)uStartBase + pimage_section_header[u_index1].VirtualAddress;
			for (u_index2 = 0;u_index2<pimage_section_header[u_index1].Misc.VirtualSize;u_index2++)
			{
				if (!MmIsAddressValid((p-SignatureInfo[0].Offset))||
					!MmIsAddressValid((p-SignatureInfo[4].Offset)))
				{
					p++;
					continue;
				}
				__try{
					if (*(p-SignatureInfo[0].Offset)==SignatureInfo[0].cSingature&&
						*(p-SignatureInfo[1].Offset)==SignatureInfo[1].cSingature&&
						*(p-SignatureInfo[2].Offset)==SignatureInfo[2].cSingature&&
						*(p-SignatureInfo[3].Offset)==SignatureInfo[3].cSingature&&
						*(p-SignatureInfo[4].Offset)==SignatureInfo[4].cSingature)
					{
						return (ULONG_PTR)p;
					}

				}__except(EXCEPTION_EXECUTE_HANDLER){
					DbgPrint("Search error!");
				}
				p++;
			}
		}
	}

	return 0;
}


BOOLEAN	Jmp_HookFunction(
	IN ULONG Destination,
	IN ULONG Source,
	IN UCHAR *Ori_Code
	)
{
	ULONG	jmp_offset;
	UCHAR	jmp_code[5] = {0xE9};

	KSPIN_LOCK lock;
	KIRQL irql;

	if (Destination==0||Source==0)
	{
		DbgPrint("Params error!");
		return FALSE;
	}

	RtlCopyMemory(Ori_Code,(PVOID)Destination,5);
	jmp_offset = Source - (Destination+5);

	*(ULONG*)&jmp_code[1] = jmp_offset;   //放入偏移

	KeInitializeSpinLock (&lock );
	KeAcquireSpinLock(&lock,&irql);

	WPOFF();
	RtlCopyMemory((PVOID)Destination,jmp_code,5);
	WPON();

	KeReleaseSpinLock (&lock,irql);

	return TRUE;
}




VOID WPOFF()
{
	ULONG_PTR cr0 = 0;
	Irql = KeRaiseIrqlToDpcLevel();
	cr0 =__readcr0();
	cr0 &= 0xfffffffffffeffff;
	__writecr0(cr0);

}





VOID WPON()
{

	ULONG_PTR cr0=__readcr0();
	cr0 |= 0x10000;
	__writecr0(cr0);
	KeLowerIrql(Irql);
}


//简单的通过链表获得内核模块的基本信息
PLDR_DATA_TABLE_ENTRY SearchDriver(PDRIVER_OBJECT pDriverObject,wchar_t *strDriverName)
{
	LDR_DATA_TABLE_ENTRY	*pdata_table_entry,*ptemp_data_table_entry;
	PLIST_ENTRY				plist;
	UNICODE_STRING			str_module_name;

	RtlInitUnicodeString(&str_module_name,strDriverName);
	pdata_table_entry = (LDR_DATA_TABLE_ENTRY*)pDriverObject->DriverSection;
	if (!pdata_table_entry)
	{
		return 0;
	}
	plist = pdata_table_entry->InLoadOrderLinks.Flink;

	while(plist!= &pdata_table_entry->InLoadOrderLinks)
	{
		ptemp_data_table_entry = (LDR_DATA_TABLE_ENTRY *)plist;

		//DbgPrint("%wZ",&pTempDataTableEntry->BaseDllName);
		if (0==RtlCompareUnicodeString(&ptemp_data_table_entry->BaseDllName,&str_module_name,FALSE))
		{
			return ptemp_data_table_entry;
		}

		plist = plist->Flink;
	}

	return 0;
}





VOID UnloadDriver(PDRIVER_OBJECT DriverObject)
{
	if (g_bHookSuccess)
	{
		ResumeHookFunction(g_RtlDispatchExeceptionAddress,g_cDisExceptionCode,0x5);
	}

}


VOID ResumeHookFunction(
	IN ULONG	Destination,
	IN UCHAR	*Ori_Code,
	IN ULONG	Length
	)
{
	KSPIN_LOCK lock;
	KIRQL irql;

	if (Destination==0||Ori_Code==0)	return;	

	KeInitializeSpinLock (&lock );
	KeAcquireSpinLock(&lock,&irql);

	WPOFF();
	RtlCopyMemory((PVOID)Destination,Ori_Code,Length);
	WPON();

	KeReleaseSpinLock (&lock,irql);
}
