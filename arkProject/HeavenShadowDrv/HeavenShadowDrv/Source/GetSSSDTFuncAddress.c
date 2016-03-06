#include "GetSSSDTFuncAddress.h"
#include "common.h"


ULONG32	NtUserBuildHwndListIndex32WinXP = 312;
ULONG32	NtUserQueryWindowIndex32WinXP = 483;


ULONG64	NtUserQueryWindowIndex64Win7 = 0x1010;
ULONG64	NtUserBuildHwndListIndex64Win7 = 0x101C;

pfnNtUserBuildHwndList AddressNtUserBuildHwndList = 0;
pfnNtUserQueryWindow AddressNtUserQueryWindow = 0;

extern 
WIN_VERSION WinVersion;
extern
ULONG_PTR ulBuildNumber;

ULONG_PTR  GetSSSDTApi()
{
	ULONG_PTR       SSSDTDescriptor = 0;
	ULONG_PTR       SSSDTFuncAddress = 0;

	switch(WinVersion)
	{
	case WINDOWS_7:
		{

			SSSDTDescriptor = GetKeServiceDescriptorTableShadow64Win7();

			AddressNtUserBuildHwndList = (pfnNtUserBuildHwndList)GetSSSDTFunctionAddress64Win7(NtUserBuildHwndListIndex64Win7,SSSDTDescriptor);

			AddressNtUserQueryWindow = (pfnNtUserQueryWindow)GetSSSDTFunctionAddress64Win7(NtUserQueryWindowIndex64Win7,SSSDTDescriptor);
			break;
		}

	case WINDOWS_XP:
		{
			SSSDTDescriptor = GetKeServiceDescriptorTableShadow32WinXP();

			AddressNtUserBuildHwndList = (pfnNtUserBuildHwndList)GetSSSDTFunctionAddress32WinXP(NtUserBuildHwndListIndex32WinXP,SSSDTDescriptor);

			AddressNtUserQueryWindow = (pfnNtUserQueryWindow)GetSSSDTFunctionAddress32WinXP(NtUserQueryWindowIndex32WinXP,SSSDTDescriptor);
			break;
		}
	}


	return SSSDTFuncAddress;

}





ULONG_PTR GetKeServiceDescriptorTableShadow32WinXP()
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




ULONG_PTR GetKeServiceDescriptorTableShadow64Win7()
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



ULONG_PTR GetSSSDTFunctionAddress32WinXP(ULONG_PTR ulIndex,ULONG_PTR SSSDTDescriptor)
{
	ULONG_PTR ServiceTableBase= 0 ;
	PSYSTEM_SERVICE_TABLE_SSSDT32 SSSDT = (PSYSTEM_SERVICE_TABLE_SSSDT32)SSSDTDescriptor;

	ServiceTableBase=(ULONG_PTR)(SSSDT ->ServiceTableBase);

	return (ULONG_PTR)(((ULONG*)ServiceTableBase)[(ULONG)ulIndex]);
}



ULONG_PTR GetSSSDTFunctionAddress64Win7(ULONG_PTR ulIndex,ULONG_PTR SSSDTDescriptor)
{
	LONG dwTemp=0;
	ULONG_PTR qwTemp=0;
	ULONG_PTR ServiceTableBase= 0 ;
	ULONG_PTR FuncAddress =0;
	PSYSTEM_SERVICE_TABLE_SSSDT64 SSSDT = (PSYSTEM_SERVICE_TABLE_SSSDT64)SSSDTDescriptor;
	ServiceTableBase=(ULONG_PTR)(SSSDT ->ServiceTableBase);
	qwTemp = ServiceTableBase + 4 * (ulIndex-0x1000);
	dwTemp = *(PLONG)qwTemp;
	dwTemp = dwTemp>>4;
	FuncAddress = ServiceTableBase + (ULONG_PTR)dwTemp;
	return FuncAddress;
}
