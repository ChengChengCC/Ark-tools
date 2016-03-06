#include "Kernel.h"
#include "KrnlFile.h"

#include "GetFuncAddress.h"
//////////////////////////////////////////////////////////////////////////
extern     PDEVICE_OBJECT g_DeviceObject;
extern     PDRIVER_OBJECT g_DriverObject;
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
extern POBJECT_TYPE *IoDriverObjectType;
extern POBJECT_TYPE *IoDeviceObjectType;
//////////////////////////////////////////////////////////////////////////

extern
	WIN_VERSION WinVersion;
extern
	ULONG_PTR PreviousModeOffsetOf_KTHREAD;
extern
	ULONG_PTR ObjectTableOffsetOf_EPROCESS;
extern
	ULONG_PTR SYSTEM_ADDRESS_START;
extern
	ULONG_PTR ObjectHeaderSize;
extern
	ULONG_PTR ObjectTypeOffsetOf_Object_Header;


extern ULONG_PTR
	ulBuildNumber;
//////////////////////////////////////////////////////////////////////////

ULONG_PTR KiServiceTable = 0;
KIRQL Irql;

NTSTATUS HsDispatchControlForKernel(PIO_STACK_LOCATION  IrpSp, PVOID OutputBuffer, ULONG_PTR* ulRet)
{

	WCHAR* szOutputBuffer = (WCHAR*)OutputBuffer;
	ULONG				ulIoControlCode  = 0;
	NTSTATUS			Status = STATUS_UNSUCCESSFUL;
	PVOID               pvInputBuffer  = NULL;
	ULONG               ulInputLen     = 0;
	ULONG				ulOutputLen    = 0;


	ULONG     ulFuncIndex = 0;
	ULONG_PTR ulFuncAddr  = 0;
	ULONG_PTR IndexOffset = 0;


	pvInputBuffer   = IrpSp->Parameters.DeviceIoControl.Type3InputBuffer;
	ulInputLen      = IrpSp->Parameters.DeviceIoControl.InputBufferLength;
	ProbeForRead(pvInputBuffer,ulInputLen,sizeof(CHAR));

	ulOutputLen     = IrpSp->Parameters.DeviceIoControl.OutputBufferLength;

	ProbeForWrite(OutputBuffer,ulOutputLen,sizeof(CHAR));

	ulIoControlCode = IrpSp->Parameters.DeviceIoControl.IoControlCode;
	ulIoControlCode = (ulIoControlCode>>2)&0x00000FFF;

	DbgPrint("%x\r\n",ulIoControlCode);

	HsInitKernelGlobalVariable();

	switch(ulIoControlCode)
	{
	case HS_IOCTL_KRNL_KISRVTAB:
		{
			DbgPrint("HS_IOCTL_KRNL_KISRVTAB\r\n");

			switch(WinVersion)
			{
			case WINDOWS_7:
				{
					KiServiceTable = *((ULONG_PTR*)GetKeServiceDescriptorTable64());
					break;
				}
			case WINDOWS_XP:
				{
					KiServiceTable = *((ULONG_PTR*)HsGetFunctionAddressByName(L"KeServiceDescriptorTable"));
					break;
				}

			}

			memcpy(OutputBuffer, &KiServiceTable,sizeof(KiServiceTable));	
			*ulRet = sizeof(ULONG_PTR);
			Status = STATUS_SUCCESS;

			break;
		}
	case HS_IOCTL_KRNL_SSDTLIST:
		{
			ULONG_PTR SSDTDescriptor = 0;

			DbgPrint("HS_IOCTL_KRNL_SSDTLIST\r\n");

			memcpy(&ulFuncIndex, pvInputBuffer,sizeof(ULONG));
			switch(WinVersion)
			{
			case WINDOWS_7:
				{
					SSDTDescriptor = GetKeServiceDescriptorTable64();
					IndexOffset = 4;

					break;
				}

			case WINDOWS_XP:
				{
					SSDTDescriptor = (ULONG_PTR)HsGetFunctionAddressByName(L"KeServiceDescriptorTable");
					IndexOffset = 1;

					break;
				}
			}

			ulFuncAddr =  GetSSDTApiFunAddress(ulFuncIndex,SSDTDescriptor);

			DbgPrint("%p\r\n",ulFuncAddr);

			memcpy(OutputBuffer, &ulFuncAddr,sizeof(ULONG_PTR));
			*ulRet = sizeof(ULONG_PTR);
			Status = STATUS_SUCCESS;

			break;
		}

	case HS_IOCTL_KRNL_WIN32KSERVICE:
		{
			DbgPrint("HS_IOCTL_KRNL_WIN32KSERVICE\r\n");

			switch(WinVersion)
			{
			case WINDOWS_7:
				{
					KiServiceTable = GetKeServiceDescriptorTableShadow64();

					KiServiceTable = (ULONG_PTR)((PSYSTEM_SERVICE_TABLE64)KiServiceTable)->ServiceTableBase;
					break;
				}

			case WINDOWS_XP:
				{
					KiServiceTable = GetKeServiceDescriptorTableShadow32();

					KiServiceTable = (ULONG_PTR)((PSYSTEM_SERVICE_TABLE32)KiServiceTable)->ServiceTableBase;
					break;
				}
			}
			memcpy(OutputBuffer, &KiServiceTable,sizeof(KiServiceTable));	

			*ulRet = sizeof(ULONG_PTR);
			Status = STATUS_SUCCESS;
			break;
		}
	case HS_IOCTL_KRNL_RESUMESSDT:
		{
			RESUME_DATA  Data = {0};

			DbgPrint("HS_IOCTL_KRNL_RESUMESSDT\r\n");

			memcpy(&Data, pvInputBuffer,sizeof(Data));

			HsUnHookSSDT(Data.ulIndex,Data.ulFuncAddress);

			*ulRet = sizeof(ULONG_PTR);
			Status = STATUS_SUCCESS;
			break;
		}
	case HS_IOCTL_KRNL_SSSDTLIST:
		{
			ULONG_PTR SSSDTFuncAddress = 0;

			DbgPrint("HS_IOCTL_KRNL_WIN32KSERVICE\r\n");

			memcpy(&ulFuncIndex,pvInputBuffer,4);
			switch(WinVersion)
			{
			case WINDOWS_7:
				{

					KiServiceTable = GetKeServiceDescriptorTableShadow64();

					KiServiceTable = (ULONG_PTR)((PSYSTEM_SERVICE_TABLE64)KiServiceTable)->ServiceTableBase;

					SSSDTFuncAddress = GetSSSDTFunctionAddress64(ulFuncIndex,KiServiceTable);

					break;
				}

			case WINDOWS_XP:
				{
					KiServiceTable = GetKeServiceDescriptorTableShadow32();

					KiServiceTable = (ULONG_PTR)((PSYSTEM_SERVICE_TABLE32)KiServiceTable)->ServiceTableBase;

					SSSDTFuncAddress = GetSSSDTFunctionAddress32(ulFuncIndex,KiServiceTable);
					break;
				}
			}


			memcpy(OutputBuffer, &SSSDTFuncAddress,sizeof(SSSDTFuncAddress));	

			*ulRet = sizeof(ULONG_PTR);
			Status = STATUS_SUCCESS;
			break;
		}

	case HS_IOCTL_KRNL_KRNLFILE:
		{
			DbgPrint("HS_IOCTL_KRNL_KRNLFILE\r\n");

			Status = HsEnumKernelFileFunc(*(int*)pvInputBuffer,OutputBuffer,ulOutputLen);
			*ulRet = sizeof(ULONG_PTR);
			break;
		}
	case HS_IOCTL_KRNL_KRNLIAT:
		{
			DbgPrint("HS_IOCTL_KRNL_KRNLIAT\r\n");
			DbgPrint("EAT: %s\r\n",pvInputBuffer);

			Status = HsQueryKernelFileFuncIAT(OutputBuffer,ulOutputLen,(char*)pvInputBuffer);
			
			*ulRet = sizeof(ULONG_PTR);
			break;
		}
	case HS_IOCTL_KRNL_KRNLEAT:
		{
			DbgPrint("HS_IOCTL_KRNL_KRNLEAT\r\n");
			DbgPrint("EAT: %s\r\n",pvInputBuffer);

			Status = HsQueryKernelFileFuncEAT(OutputBuffer,ulOutputLen,(char*)pvInputBuffer);

			*ulRet = sizeof(ULONG_PTR);
			break;
		}
	default:
		{
			Status = STATUS_UNSUCCESSFUL;
		}
	}

	return Status;
}



VOID HsInitKernelGlobalVariable()
{
	switch(WinVersion)
	{
	case WINDOWS_XP:
		{
			PreviousModeOffsetOf_KTHREAD = 0x140;
			ObjectHeaderSize = 0x18;
			ObjectTypeOffsetOf_Object_Header = 0x8;
			ObjectTableOffsetOf_EPROCESS = 0x0c4;
			SYSTEM_ADDRESS_START = 0x80000000;
			break;
		}

	case WINDOWS_7:
		{
			PreviousModeOffsetOf_KTHREAD = 0x1f6;
			ObjectTableOffsetOf_EPROCESS = 0x200;
			ObjectHeaderSize = 0x30;
			SYSTEM_ADDRESS_START = 0x80000000000;
			break;
		}
	}
}



ULONG_PTR GetKeServiceDescriptorTableShadow64()
{
	PUCHAR StartSearchAddress = (PUCHAR)__readmsr(0xC0000082);
	PUCHAR EndSearchAddress = StartSearchAddress + 0x500;
	PUCHAR i = NULL;
	UCHAR b1=0,b2=0,b3=0;
	ULONG_PTR Temp = 0;
	ULONG_PTR Address = 0;
	for(i=StartSearchAddress;i<EndSearchAddress;i++)
	{
		if( MmIsAddressValid(i) && MmIsAddressValid(i+1) && MmIsAddressValid(i+2) )
		{
			b1=*i;
			b2=*(i+1);
			b3=*(i+2);
			if( b1==0x4c && b2==0x8d && b3==0x1d ) //4C8D1D
			{
				memcpy(&Temp,i+3,4);
				Address = (ULONG_PTR)Temp + (ULONG_PTR)i + 7;



				Address+=32;

				return Address;
			}
		}
	}
	return 0;
}

ULONG_PTR GetSSSDTFunctionAddress32(ULONG_PTR ulIndex,ULONG_PTR ServiceTableBase)
{

	return (ULONG_PTR)(((ULONG*)ServiceTableBase)[(ULONG)ulIndex]);
}


ULONG_PTR GetSSSDTFunctionAddress64(ULONG_PTR ulIndex,ULONG_PTR ServiceTableBase)
{
	LONG dwTemp=0;
	ULONG_PTR qwTemp=0;
	ULONG_PTR FuncAddress =0;
	qwTemp = ServiceTableBase + 4 * (ulIndex);
	dwTemp = *(PLONG)qwTemp;
	dwTemp = dwTemp>>4;
	FuncAddress = ServiceTableBase + (ULONG_PTR)dwTemp;
	return FuncAddress;
}



ULONG_PTR GetKeServiceDescriptorTableShadow32()
{
	WCHAR szKeAddSystemServiceTable[] = L"KeAddSystemServiceTable";
	ULONG_PTR KeAddSystemServiceTableAddress = NULL;
	ULONG_PTR Temp = 0;
	ULONG_PTR Address = 0;
	int i = 0;
	PUCHAR StartSearchAddress;
	KeAddSystemServiceTableAddress = (ULONG_PTR)HsGetFunctionAddressByName(szKeAddSystemServiceTable);

	if (KeAddSystemServiceTableAddress==NULL)
	{
		return 0;
	}


	for (StartSearchAddress = (PUCHAR)KeAddSystemServiceTableAddress; 
			StartSearchAddress < (PUCHAR)KeAddSystemServiceTableAddress + PAGE_SIZE; 
			StartSearchAddress++)
		{
			if (ulBuildNumber < 8000)
			{
			
				if (*(unsigned short*)StartSearchAddress == 0x888d)
				{
					Temp = *(ULONG_PTR*)(StartSearchAddress+2);
			
					Address = Temp + 16;
				
				
					/*
					kd> dd 80553f60
					80553f60  80502b8c 00000000 0000011c 80503000    SSDT
					80553f70  bf999b80 00000000 0000029b bf99a890    ShadowSSDT  所以要加16
					80553f80  00000000 00000000 00000000 00000000
					80553f90  00000000 00000000 00000000 00000000
					80553fa0  80502b8c 00000000 0000011c 80503000
					80553fb0  00000000 00000000 00000000 00000000
					80553fc0  00000000 00000000 00000000 00000000
					80553fd0  00000000 00000000 00000000 00000000
					*/
					break;
				}
			}

			//其他版本
			else if (ulBuildNumber>=8000)
			{
				if (*(unsigned short*)StartSearchAddress==0xb983)
				{
					i++;
					if (i==1)
					{
					}

					if (i==2)
					{
						Temp = *(ULONG_PTR*)(StartSearchAddress+2);

						Address = Temp + 16;
						break;
					}
				}
			}
		}



	return Address;

}


ULONG GetOffsetAddress(ULONG_PTR FuncAddr)
{
	ULONG dwTemp=0;
	dwTemp= (ULONG)(FuncAddr-(ULONG_PTR)KiServiceTable);
	return dwTemp<<4;
}


VOID HsUnHookSSDT(ULONG Index, ULONG_PTR FuncAddr)
{
	ULONG dwTemp;
	PULONG ServiceTableBase=NULL;

	switch(WinVersion)
	{
	case WINDOWS_7:
		{
			dwTemp=GetOffsetAddress(FuncAddr);
			ServiceTableBase=(PULONG)KiServiceTable;
			WPOFF();
			ServiceTableBase[Index]=dwTemp;
			WPON();

			break;
		}

	case WINDOWS_XP:
		{
			ServiceTableBase=(PULONG)KiServiceTable;
			WPOFF();
			ServiceTableBase[Index]=FuncAddr;
			WPON();

			break;
		}
	}

}


VOID WPOFF()
{
	ULONG_PTR cr0 = 0;
	Irql = KeRaiseIrqlToDpcLevel();
	cr0 =__readcr0();
	cr0 &= 0xfffffffffffeffff;
	__writecr0(cr0);
	//_disable();                      //这句话 屏蔽也没有啥

}

VOID WPON()
{

	ULONG_PTR cr0=__readcr0();
	cr0 |= 0x10000;
	//_enable();                      //这句话 屏蔽也没有啥
	__writecr0(cr0);
	KeLowerIrql(Irql);
}