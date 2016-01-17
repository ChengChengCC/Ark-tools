
#ifndef CXX_IDTHOOK_H
#	include "IDTHook.h"
#endif
#include <WINDEF.H>


ULONG_PTR g_OrigKiTrap03;
KIRQL  Irql;


_declspec(naked) void NewKiTrap03()
{

	__asm
	{
		//测试
		//jmp g_OrigKiTrap03

		//构建Trap03的异常帧
		//保存现场环境,和原始Trap03一样
		push    0   ;ErrorCode
		push    ebp
		push    ebx
		push    esi
		push    edi
		push    fs
		mov     ebx,30h
		mov     fs,bx
		mov     ebx,dword ptr fs:[0]
		push    ebx
		sub     esp,4
		push    eax
		push    ecx
		push    edx
		push    ds
		push    es
		push    gs

		sub     esp,30h    //esp此时就指向陷阱帧

		push    esp         //FilterExceptionInfo自己清理了

		call   FilterExceptionInfo   //过滤函数

		add     esp , 0x30
		pop		gs
		pop		es
		pop		ds
		pop		edx
		pop		ecx
		pop		eax
		add		esp , 4
		pop		ebx
		pop		fs
		pop		edi
		pop		esi
		pop		ebx
		pop		ebp
		add     esp , 0x4
		jmp     g_OrigKiTrap03
	}
}



VOID __stdcall FilterExceptionInfo(PX86_KTRAP_FRAME pTrapFrame)
{

	//eip的值减一过int3，汇编代码分析中dec， 
	DbgPrint("Eip:%x\r\n",(pTrapFrame->Eip)-1);
}


NTSTATUS DriverEntry(IN PDRIVER_OBJECT DriverObject, IN PUNICODE_STRING RegistryString)
{
	NTSTATUS	Status = STATUS_SUCCESS;
	IDTR Idtr;
	PIDTENTRY pIdtArray = NULL;
	ULONG_PTR Index = 0;

	DriverObject->DriverUnload = UnloadDriver;
    __asm sidt Idtr
	//虚拟机是单核的，只用一个就可以了
	if(KeGetIdt(&pIdtArray))
	{
		DbgPrint("%x---%x\r\n",Idtr.base,Idtr.limit);
		for (Index =0;Index<(Idtr.limit+1)/sizeof(IDTENTRY);Index++) 		
		{
			DbgPrint("TrapHandle[%d]:%x\r\n",Index,MAKELONG(pIdtArray[Index].LowOffset,pIdtArray[Index].HiOffset));
		}

		g_OrigKiTrap03 = MAKELONG(pIdtArray[3].LowOffset,pIdtArray[3].HiOffset);

		WPOFF();
		pIdtArray[3].LowOffset = (ULONG_PTR)NewKiTrap03 & 0xFFFF;  //低16位
		pIdtArray[3].HiOffset =  (ULONG_PTR)NewKiTrap03 >> 16;     //高16位
		WPON();

	}
	
	//limit 0x7ff (包含0)  0x800  = 2048  Entry每项大小8字节，就2048/8 = 256 成员
	//!idt -a   0ff  = 256 

	//MAKELONG
	//#define MAKELONG(a, b)      ((LONG)(((WORD)(((DWORD_PTR)(a)) & 0xffff)) | ((DWORD)((WORD)(((DWORD_PTR)(b)) & 0xffff))) << 16))
	return Status;
}





BOOLEAN KeGetIdt(PIDTENTRY *pIdtArray)
{
	ULONG Index,Affinity,CurrentAffinity;
	pfnKESETAFFINITYTHREAD fnpKeSetAffinityThread;

	UNICODE_STRING usFuncName;
	PIDTENTRY pIdtEntry;

	RtlInitUnicodeString(&usFuncName,L"KeSetAffinityThread");
	fnpKeSetAffinityThread = (pfnKESETAFFINITYTHREAD)MmGetSystemRoutineAddress(&usFuncName);

	if (fnpKeSetAffinityThread==0)
	{
		return FALSE;
	}

	Affinity = KeQueryActiveProcessors();                    
	//KeQueryActiveProcessors获取处理器相关的位图
	//(这里的位图可以理解为个数，比如返回1代表一个处理器，返回3表示两个处理器，返回7表示三个处理器，依此类推。
	//也就是说从有多少个处理器，那么Affinity的值就会从低位到高位依此填充多少位)

	CurrentAffinity = 1;
	Index = 0;
	while(Affinity)
	{
		//下面只是个简单的算法，使当前线程运行到不同的处理器上
		Affinity &= ~CurrentAffinity;
		fnpKeSetAffinityThread(PsGetCurrentThread(),(KAFFINITY)CurrentAffinity);
		CurrentAffinity <<= 1;

		__asm{
			push        eax
			mov         eax,fs:[0x38]
			mov         pIdtEntry,eax
			pop         eax
		}
		//得到我们要的东西
		pIdtArray[Index] = pIdtEntry;
		Index++;
	}

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



VOID UnloadDriver(PDRIVER_OBJECT DriverObject)
{
	//恢复
	PIDTENTRY pIdtEntry;
	if (g_OrigKiTrap03 && KeGetIdt(&pIdtEntry))
	{
		WPOFF();
		pIdtEntry[3].LowOffset = g_OrigKiTrap03 & 0xFFFF;
		pIdtEntry[3].HiOffset = g_OrigKiTrap03 >> 16;
		WPON();
	}
}