

#ifndef CXX_HIDEIDTHOOK_H
#	include "HideIDTHook.h"
#endif


KIRQL  Irql;
ULONG_PTR g_jmp_offset = 0;
ULONG_PTR  OldBase;
PKGDTENTRY NewGDTAddr;
ULONG_PTR g_OrigKiTrap03;
unsigned short OldSelector;
IDTENTRY*  idt_entries;

__declspec(naked) void NewKiTrap03()
{
	__asm
	{
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

NTSTATUS
DriverEntry(IN PDRIVER_OBJECT pDriverObj, IN PUNICODE_STRING pRegistryString)
{
	ULONG	oriaddr=0;
	ULONG	newaddr=0;
	PKGDTENTRY GDT_Addr;
	KGDTENTRY  GDTInfo;
	PKGDTENTRY Gdt_Addr3e0;
	PKGDTENTRY Gdt_Addr8;
	ULONG	jmpoffset=0;
	IDTR    idt_info;    
	
	unsigned short selector;

#ifdef _DBG
	__asm int 3
#endif
	pDriverObj->DriverUnload = DriverUnLoad;

	__asm 
	{
		sidt  idt_info
			
		push edx
		sgdt [esp-2]
		pop edx
		mov GDT_Addr,edx

	}
	idt_entries = (IDTENTRY*) MAKELONG(idt_info.IDT_LOWbase,idt_info.IDT_HIGbase);
	g_OrigKiTrap03 = MAKELONG(idt_entries[3].LowOffset,idt_entries[3].HiOffset);
	jmpoffset	=	(ULONG)NewKiTrap03 - g_OrigKiTrap03;
	selector = idt_entries[1].selector;
	//我选择的是索引为0x10的，空白的GDT表项
	NewGDTAddr = GDT_Addr + 0x10;

	//保存原来的
 	memcpy((UCHAR*)&OldBase,(char*)(&(NewGDTAddr->BaseLow)),2);
 	memcpy((UCHAR*)&OldBase+2,(char*)(&(NewGDTAddr->HighWord.Bytes.BaseMid)),1);
 	memcpy((UCHAR*)&OldBase+3,(char*)(&(NewGDTAddr->HighWord.Bytes.BaseHi)),1);

	//修改
	WPOFF();
	memcpy((char*)(&(NewGDTAddr->BaseLow)),(UCHAR*)&jmpoffset,2);
	memcpy((char*)(&(NewGDTAddr->HighWord.Bytes.BaseMid)),(UCHAR*)(&jmpoffset)+2,1);
	memcpy((char*)(&(NewGDTAddr->HighWord.Bytes.BaseHi)),(UCHAR*)(&jmpoffset)+3,1);
	OldSelector = idt_entries[3].selector;
	idt_entries[3].selector  = 0x80;
	WPON();

	return  STATUS_SUCCESS;
}



void DriverUnLoad(PDRIVER_OBJECT pDriverObject)
{
	WPOFF();
	memcpy((char*)(&(NewGDTAddr->BaseLow)),(UCHAR*)(&OldBase),2);
	memcpy((char*)(&(NewGDTAddr->HighWord.Bytes.BaseMid)),(UCHAR*)(&OldBase)+2,1);
	memcpy((char*)(&(NewGDTAddr->HighWord.Bytes.BaseHi)),(UCHAR*)(&OldBase)+3,1);
	idt_entries[3].selector = OldSelector;
	WPON();
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


