

#ifndef CXX_HIDEIDTHOOK_H
#define CXX_HIDEIDTHOOK_H


#include <ntifs.h>
#include <devioctl.h>
#include <WINDEF.H>

#endif	


typedef struct _KGDTENTRY                 // 3 elements, 0x8 bytes (sizeof)  
{                                                                            
	/*0x000*/     UINT16       LimitLow;                                                   
	/*0x002*/     UINT16       BaseLow;                                                    
	union                                 // 2 elements, 0x4 bytes (sizeof)  
	{                                                                        
		struct                            // 4 elements, 0x4 bytes (sizeof)  
		{                                                                    
			/*0x004*/             UINT8        BaseMid;                                            
			/*0x005*/             UINT8        Flags1;                                             
			/*0x006*/             UINT8        Flags2;                                             
			/*0x007*/             UINT8        BaseHi;                                             
		}Bytes;                                                              
		struct                            // 10 elements, 0x4 bytes (sizeof) 
		{                                                                    
			/*0x004*/             ULONG32      BaseMid : 8;     // 0 BitPosition                   
			/*0x004*/             ULONG32      Type : 5;        // 8 BitPosition                   
			/*0x004*/             ULONG32      Dpl : 2;         // 13 BitPosition                  
			/*0x004*/             ULONG32      Pres : 1;        // 15 BitPosition                  
			/*0x004*/             ULONG32      LimitHi : 4;     // 16 BitPosition                  
			/*0x004*/             ULONG32      Sys : 1;         // 20 BitPosition                  
			/*0x004*/             ULONG32      Reserved_0 : 1;  // 21 BitPosition                  
			/*0x004*/             ULONG32      Default_Big : 1; // 22 BitPosition                  
			/*0x004*/             ULONG32      Granularity : 1; // 23 BitPosition                  
			/*0x004*/             ULONG32      BaseHi : 8;      // 24 BitPosition                  
		}Bits;                                                               
	}HighWord;                                                               
}KGDTENTRY, *PKGDTENTRY;

typedef struct _IDTR{
	USHORT   IDT_limit;
	USHORT   IDT_LOWbase;
	USHORT   IDT_HIGbase;
}IDTR,*PIDTR;

typedef struct _IDTENTRY
{
	unsigned short LowOffset;
	unsigned short selector;
	unsigned char retention:5;
	unsigned char zero1:3;
	unsigned char gate_type:1;
	unsigned char zero2:1;
	unsigned char interrupt_gate_size:1;
	unsigned char zero3:1;
	unsigned char zero4:1;
	unsigned char DPL:2;
	unsigned char P:1;
	unsigned short HiOffset;
} IDTENTRY,*PIDTENTRY;

typedef struct _X86_KTRAP_FRAME {
	ULONG   DbgEbp;
	ULONG   DbgEip;
	ULONG   DbgArgMark;
	ULONG   DbgArgPointer;
	ULONG   TempSegCs;
	ULONG   TempEsp;
	ULONG   Dr0;
	ULONG   Dr1;
	ULONG   Dr2;
	ULONG   Dr3;
	ULONG   Dr6;
	ULONG   Dr7;
	ULONG   SegGs;
	ULONG   SegEs;
	ULONG   SegDs;
	ULONG   Edx;
	ULONG   Ecx;
	ULONG   Eax;
	ULONG   PreviousPreviousMode;
	ULONG   ExceptionList;
	ULONG   SegFs;
	ULONG   Edi;
	ULONG   Esi;
	ULONG   Ebx;
	ULONG   Ebp;
	ULONG   ErrCode;

	ULONG   Eip;
	ULONG   SegCs;
	ULONG   EFlags;
	ULONG   HardwareEsp;    // WARNING - segSS:esp are only here for stacks
	ULONG   HardwareSegSs;  // that involve a ring transition.
	ULONG   V86Es;          // these will be present for all transitions from
	ULONG   V86Ds;          // V86 mode
	ULONG   V86Fs;
	ULONG   V86Gs;
} X86_KTRAP_FRAME, *PX86_KTRAP_FRAME;

VOID __stdcall FilterExceptionInfo(PX86_KTRAP_FRAME pTrapFrame);

void DriverUnLoad(PDRIVER_OBJECT pDriverObject);

VOID WPOFF();
VOID WPON();
