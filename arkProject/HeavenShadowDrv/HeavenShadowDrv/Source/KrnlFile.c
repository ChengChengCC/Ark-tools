#include "KrnlFile.h"

#include <stdlib.h>


FILE_INFOR	 FileInfor;
char szModuleFileFullName[256] = {0};
ULONG_PTR  ulKernelBase = 0;
ULONG_PTR  ulKernelSize = 0;

PSYSTEM_MODULE_INFORMATION ModuleInforBuffer = NULL;
ULONG_PTR ModuleInforNeedSize = 0;
PFILE_INFOR PFileInfor = NULL;

NTSTATUS HsEnumKernelFileFunc(int KernelFile, PVOID OutputBuffer, ULONG_PTR ulOutputLen)
{
	NTSTATUS Status = STATUS_SUCCESS;

	switch(KernelFile)
	{
	case HS_KERNEL_KERNELFILE_NTOSKRNL_IAT:
		{
			DbgPrint("HS_KERNEL_KERNELFILE_NTOSKRNL_IAT:%d\r\n",ulOutputLen);
			Status = HsQueryKernelFileFuncIAT(OutputBuffer,ulOutputLen,"ntoskrnl.exe");
			break;
		}
	case HS_KERNEL_KERNELFILE_NTOSKRNL_EAT:
		{
			DbgPrint("HS_KERNEL_KERNELFILE_NTOSKRNL_EAT\r\n");
			Status = HsQueryKernelFileFuncEAT(OutputBuffer,ulOutputLen,"ntoskrnl.exe");
			break;
		}
	case HS_KERNEL_KERNELFILE_WIN32K_IAT:
		{
			DbgPrint("HS_KERNEL_KERNELFILE_WIN32K_IAT\r\n");
			Status = HsQueryKernelFileFuncIAT(OutputBuffer,ulOutputLen,"win32k.sys");
			break;
		}
	case HS_KERNEL_KERNELFILE_WIN32K_EAT:
		{
			DbgPrint("HS_KERNEL_KERNELFILE_WIN32K_EAT\r\n");
			Status = HsQueryKernelFileFuncEAT(OutputBuffer,ulOutputLen,"win32k.sys");
			break;
		}
	case HS_KERNEL_KERNELFILE_HALDLL_IAT:
		{
			DbgPrint("HS_KERNEL_KERNELFILE_HALDLL_IAT\r\n");
			Status = HsQueryKernelFileFuncIAT(OutputBuffer,ulOutputLen,"hal.dll");
			break;
		}
	case HS_KERNEL_KERNELFILE_HALDLL_EAT:
		{
			DbgPrint("HS_KERNEL_KERNELFILE_HALDLL_EAT\r\n");
			Status = HsQueryKernelFileFuncEAT(OutputBuffer,ulOutputLen,"hal.dll");
			break;
		}
	default:
		{
			DbgPrint("HS_KERNEL_KERNELFILE_UNKNOWN\r\n");
			Status = STATUS_UNSUCCESSFUL;
		}
	}

	return Status;
}



//////////////////////////////////////////////////////////////////////////

NTSTATUS HsQueryKernelFileFuncIAT(PVOID OutputBuffer, ULONG_PTR ulOutputLen, char* szModuleFile)
{

	NTSTATUS Status = STATUS_UNSUCCESSFUL;
	SYSTEM_MODULE_INFORMATION_ENTRY  Temp;
	BOOLEAN bRet = FALSE;

	DbgPrint("GetModuleInforNtoskrnl now:::\r\n");

	if (GetModuleInforKernelFile(&ulKernelBase,&ulKernelSize,szModuleFileFullName,szModuleFile))
	{
		DbgPrint("GetModuleInforNtoskrnl success\r\n");
	}
	FileInfor.BaseAddress = (PVOID)ulKernelBase;
	FileInfor.Size = ulKernelSize;

// 	if (GetModuleInfor(szModuleFile,szModuleFileFullName,&Temp))
// 	{
// 		DbgPrint("GetModuleInforNtoskrnl success\r\n");
// 	}
// 	FileInfor.BaseAddress = (PVOID)Temp.Base;
// 	FileInfor.Size = Temp.Size;

	strcpy(FileInfor.szFileFullName,szModuleFileFullName);
	ReadFileData(&FileInfor);

	Status = HsEnumIATTable((PMODULE_IAT)OutputBuffer,ulOutputLen);

	if (FileInfor.szFileData!=NULL)
	{
		ExFreePool(FileInfor.szFileData);
	}

	Status = STATUS_SUCCESS;

	return Status;
}

NTSTATUS HsEnumIATTable(PMODULE_IAT OutBuffer, ULONG_PTR OutSize)
{

	NTSTATUS						 Status = STATUS_UNSUCCESSFUL;
	ULONG_PTR                        NeedSize = 0;
	PSYSTEM_MODULE_INFORMATION       Buffer = NULL;
	ULONG_PTR                        KernelModuleCount = 0;
	PIMAGE_IMPORT_DESCRIPTOR	     ImportTable = NULL;
	ULONG_PTR                        ImportSize  = 0;
	ULONG_PTR                        ImportDiff  = 0;

	PIMAGE_THUNK_DATA				 ImportFirstThunk  = NULL;
	ULONG_PTR						 IATSize     = 0;
	ULONG_PTR						 IATDiff     = 0;

	PSYSTEM_MODULE_INFORMATION       ModuleInfor = NULL;
	char                             szIATOriImageFile[60] = {0};
	PFILE_INFOR                      ModuleFile = NULL;
	ULONG_PTR                        OriginalFuncAddress      = 0;

	PIMAGE_IMPORT_DESCRIPTOR		 ImportDescArray[64];
	ULONG_PTR						 ImportDescIndex = 0;
	char*                            ImportModuleName         = NULL;
	ULONG_PTR                        Max = 0;
	PIMAGE_THUNK_DATA                ImportOriFirstThunk      = NULL;
	PIMAGE_IMPORT_BY_NAME            OrdinalName              = NULL;
	ULONG                            x = 0;
	ULONG_PTR                        ulCount = (OutSize - sizeof(MODULE_IAT)) / sizeof(IAT_INFO);

	if(FileInfor.BaseAddress!=0)
	{
		ImportTable = (PIMAGE_IMPORT_DESCRIPTOR)GetDirectoryAddr((PUCHAR)FileInfor.szFileData, 
			IMAGE_DIRECTORY_ENTRY_IMPORT, &ImportSize, &ImportDiff, TRUE);	//文件内容

		ImportFirstThunk = (PIMAGE_THUNK_DATA)GetDirectoryAddr((PUCHAR)FileInfor.BaseAddress, 
			IMAGE_DIRECTORY_ENTRY_IAT, &IATSize, &IATDiff, FALSE);			//内存内容

		if (IATSize/sizeof(ULONG)> ulCount)
		{
			OutBuffer->ulCount = IATSize/sizeof(ULONG);

			Status = STATUS_BUFFER_TOO_SMALL;

			goto Exit;
		}

		if(!ImportFirstThunk || !ImportTable)
		{
			goto Exit;
		}

		while (ImportTable->Name)  //导入模块不为空  从自己的内存中获得所有的模块
		{
			ImportDescArray[ImportDescIndex] = ImportTable;

			ImportDescIndex++;
			ImportTable++;
		}

		if (ImportDescIndex==0)
		{
			goto Exit;  //该模块没有任何的导入模块
		}

		Max = ImportDescIndex;
		ImportDescIndex = 0;
		ImportTable = ImportDescArray[ImportDescIndex];
		ImportDescIndex++;

		ImportModuleName = MakePtr(char*,FileInfor.szFileData,ImportTable->Name - ImportDiff);  //文件内容

		if (!ImportModuleName)
		{
			goto Exit;
		}

		ImportOriFirstThunk = MakePtr(PIMAGE_THUNK_DATA,FileInfor.szFileData,
			ImportTable->OriginalFirstThunk - ImportDiff);//文件内容

		//通过导入的模块名找到导入模块

		while (ImportOriFirstThunk && ImportModuleName && //导入模块的函数名称  导入的模块名称 
			IATSize && ImportFirstThunk->u1.Function) 
		{

			if (_stricmp(szIATOriImageFile,ImportModuleName)!=0)
			{
				if (ModuleFile!=NULL && ModuleFile->szFileData != NULL && MmIsAddressValid(ModuleFile->szFileData))
				{
					DbgPrint("ExFreePool(ModuleFile->szFileData)\r\n");
					ExFreePool(ModuleFile->szFileData);
				}
				ModuleFile = CreateFileData(NULL,ImportModuleName);
				if (ModuleFile == NULL)
				{
					DbgPrint("CreateFileData Failed\r\n");
					goto Exit;
				}

				memset(szIATOriImageFile,0,60);
				strcpy(szIATOriImageFile,ImportModuleName);
			}

			strcpy(OutBuffer->Data[x].szModuleName,ImportModuleName);

			OrdinalName   = MakePtr(PIMAGE_IMPORT_BY_NAME, 
				FileInfor.szFileData, 
				(ULONG_PTR)ImportOriFirstThunk->u1.AddressOfData - ImportDiff);  //通过Original(文件)获得函数名称

			//DbgPrint("函数名称:%s   函数地址:0x%p\r\n",OrdinalName->Name,ImportFirstThunk->u1.Function);  //从First(内存)中获得函数地址

			if(GetAddrOfExportFuncAddr((PUCHAR)ModuleFile->szFileData,(CHAR*)OrdinalName->Name, &OriginalFuncAddress, 
				TRUE))//从导入的模块中获得函数地址
			{
				OriginalFuncAddress += (ULONG_PTR)ModuleFile->BaseAddress;
				DbgPrint("ExportFuncAddr: %p\r\n",OriginalFuncAddress);
			}


			strcpy(OutBuffer->Data[x].szFunctionName,(char*)OrdinalName->Name);
			OutBuffer->Data[x].CurFuncAddress = ImportFirstThunk->u1.Function;
			OutBuffer->Data[x].OriFuncAddress = OriginalFuncAddress;

			x++;

			ImportFirstThunk++;
			IATSize   -= sizeof(ULONG);
			ImportOriFirstThunk++;

			if (IATSize==0)
			{
				break;
			}

			if(ImportFirstThunk->u1.Function == 0)  //一个导入模块已经遍历完成
			{
				ImportFirstThunk++; 
				IATSize -= sizeof(ULONG);

				ImportTable = ImportDescArray[ImportDescIndex];  // 从数组中获得下一个导出模块

				if (ImportDescIndex==Max)
				{
					break;
				}

				ImportDescIndex++;

				ImportModuleName = MakePtr(char*,FileInfor.szFileData,ImportTable->Name - ImportDiff);
				ImportOriFirstThunk = MakePtr(PIMAGE_THUNK_DATA,FileInfor.szFileData,
					ImportTable->OriginalFirstThunk - ImportDiff);//文件内容

				if( ImportOriFirstThunk == NULL || 
					ImportModuleName	 == NULL || 
					(PULONG)ImportFirstThunk->u1.Function == NULL || 
					IATSize == 0)
				{
					break;
				}
			}
		}
	}

	OutBuffer->ulCount = x;

	Status = STATUS_SUCCESS;

Exit:

	if (ModuleFile!=NULL && ModuleFile->szFileData != NULL && MmIsAddressValid(ModuleFile->szFileData))
	{
		DbgPrint("ExFreePool(ModuleFile->szFileData)\r\n");
		ExFreePool(ModuleFile->szFileData);
	}

	return Status;
}



//////////////////////////////////////////////////////////////////////////

NTSTATUS HsQueryKernelFileFuncEAT(PVOID OutputBuffer, ULONG_PTR ulOutputLen, char* szModuleFile)
{
	NTSTATUS Status = STATUS_UNSUCCESSFUL;

	DbgPrint("GetModuleInforNtoskrnl now:::\r\n");

	if (GetModuleInforKernelFile(&ulKernelBase,&ulKernelSize,szModuleFileFullName,szModuleFile))
	{
		DbgPrint("GetModuleInforNtoskrnl success\r\n");
	}

	Status = HsEnumEATTable((PVOID)ulKernelBase,(PMODULE_EAT)OutputBuffer,ulOutputLen,szModuleFile);

	return Status;
}


NTSTATUS HsEnumEATTable(PVOID  KernelBase,PMODULE_EAT OutBuffer, ULONG_PTR OutSize, char* szModuleFileName)
{
	PIMAGE_DOS_HEADER  DosHeader;
	PIMAGE_NT_HEADERS  NtHeader;
	IMAGE_OPTIONAL_HEADER  OptionHead;
	PIMAGE_EXPORT_DIRECTORY ExportTable;
	ULONG*             ArrayOfFunctionAddress;
	ULONG*             ArrayOfFunctionName;
	short*             ArrayOfFunctionOrdinals;
	ULONG_PTR          Base; 
	ULONG_PTR          x;
	char*              FunctionName;
	ULONG_PTR          FunctionOrdinals;
	ULONG_PTR          FunctionAddress;
	ULONG_PTR          ulCount = (OutSize - sizeof(MODULE_EAT)) / sizeof(EAT_INFO);

	PFILE_INFOR        ModuleFile = NULL;
	ULONG_PTR          OriginalFuncAddress      = 0;

	DosHeader = (PIMAGE_DOS_HEADER)KernelBase;

	if (DosHeader->e_magic!=IMAGE_DOS_SIGNATURE)
	{
		return STATUS_UNSUCCESSFUL;
	}

	NtHeader =  (PIMAGE_NT_HEADERS)((ULONG_PTR)DosHeader + DosHeader->e_lfanew);

	if (NtHeader->Signature!=IMAGE_NT_SIGNATURE)
	{

		return STATUS_UNSUCCESSFUL;
	}

	//////////////////////////////////////////////////////////////////////////
	ModuleFile = CreateFileData(NULL,szModuleFileName);

	if (ModuleFile == NULL)
	{
		DbgPrint("CreateFileData Failed\r\n");
		return STATUS_UNSUCCESSFUL;
	}
	//////////////////////////////////////////////////////////////////////////

	OptionHead = NtHeader->OptionalHeader;

	ExportTable = (PIMAGE_EXPORT_DIRECTORY)((ULONG_PTR)DosHeader + 
		OptionHead.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress);  //获得导出表的RVA


	ArrayOfFunctionAddress = (ULONG*)((ULONG_PTR)DosHeader + ExportTable->AddressOfFunctions);
	ArrayOfFunctionName = (ULONG*)((ULONG_PTR)DosHeader+ExportTable->AddressOfNames);
	ArrayOfFunctionOrdinals = (short*)((ULONG_PTR)DosHeader+ExportTable->AddressOfNameOrdinals);

	Base = ExportTable->Base;


	if (ExportTable->NumberOfFunctions > ulCount)
	{
		OutBuffer->ulCount = ExportTable->NumberOfFunctions ;

		return STATUS_BUFFER_TOO_SMALL;
	}

	for (x=0;x<ExportTable->NumberOfFunctions;x++)
	{
		FunctionName = (char*)((ULONG_PTR)DosHeader+ArrayOfFunctionName[x]);
		FunctionOrdinals = ArrayOfFunctionOrdinals[x] + Base -1;
		FunctionAddress  = (ULONG_PTR)((ULONG_PTR)DosHeader +  
			ArrayOfFunctionAddress[FunctionOrdinals]);

		if(GetAddrOfExportFuncAddr((PUCHAR)ModuleFile->szFileData,(CHAR*)FunctionName, &OriginalFuncAddress, 
			TRUE))//从导入的模块中获得函数地址
		{
			OriginalFuncAddress += (ULONG_PTR)ModuleFile->BaseAddress;
			DbgPrint("OriFuncAddr: %p\r\n",OriginalFuncAddress);
		}

		//DbgPrint("%s    %p\r\n",FunctionName,FunctionAddress);
		OutBuffer->Data[x].CurFuncAddress = FunctionAddress;
		OutBuffer->Data[x].OriFuncAddress = OriginalFuncAddress;

		memcpy(OutBuffer->Data[x].szFunctionName,FunctionName,strlen(FunctionName));
	}
	OutBuffer->ulCount = ExportTable->NumberOfFunctions;

	//////////////////////////////////////////////////////////////////////////
	if (ModuleFile!=NULL && ModuleFile->szFileData != NULL && MmIsAddressValid(ModuleFile->szFileData))
	{
		DbgPrint("ExFreePool(ModuleFile->szFileData)\r\n");
		ExFreePool(ModuleFile->szFileData);
	}
	//////////////////////////////////////////////////////////////////////////

	return STATUS_SUCCESS;
}



//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////



BOOLEAN  ReadFileData(PFILE_INFOR FileInfor)
{

	WCHAR     wzFileName[256];
	HANDLE    hFile  = NULL;
	IO_STATUS_BLOCK   iosb;
	OBJECT_ATTRIBUTES oa;
	UNICODE_STRING    uniFileName;
	NTSTATUS          Status;
	FILE_STANDARD_INFORMATION   FileStandardInfor;
	LARGE_INTEGER               Offset = {0,0};

	mbstowcs(wzFileName,FileInfor->szFileFullName,256);


	//DbgPrint("%S\r\n",uniFileName);

	RtlInitUnicodeString(&uniFileName,wzFileName);

	//初始化我们的对象属性

	InitializeObjectAttributes(&oa,&uniFileName,OBJ_CASE_INSENSITIVE,NULL,NULL);


	Status = ZwCreateFile(&hFile,GENERIC_READ,&oa,&iosb,NULL,
		FILE_ATTRIBUTE_NORMAL,FILE_SHARE_READ|FILE_SHARE_WRITE,FILE_OPEN,FILE_SYNCHRONOUS_IO_NONALERT,NULL,0);

	if(Status!=STATUS_SUCCESS)
	{

		return FALSE;
	}
	else
	{
		//	DbgPrint("Get File Success\n");
	}


	//获得文件信息

	Status = ZwQueryInformationFile(hFile,&iosb,&FileStandardInfor,sizeof(FILE_STANDARD_INFORMATION),
		FileStandardInformation);


	if (!NT_SUCCESS(Status))
	{
		ZwClose(hFile);

		return FALSE;
	}

	//动态申请内存

	if (FileStandardInfor.AllocationSize.u.LowPart == 0)
	{
		ZwClose(hFile);

		return FALSE;
	}

	FileInfor->szFileData = (char*)ExAllocatePool(PagedPool,FileStandardInfor.AllocationSize.u.LowPart);

	if (FileInfor->szFileData==NULL)
	{
		ZwClose(hFile);

		return FALSE;
	}


	//读取文件长度

	Status = ZwReadFile(hFile,
		NULL,
		NULL,
		NULL,
		&iosb,
		FileInfor->szFileData ,
		FileStandardInfor.AllocationSize.u.LowPart,
		&Offset,
		NULL);

	if (!NT_SUCCESS(Status))
	{
		//DbgPrint("Sys文件读取失败\r\n");
		ExFreePool(FileInfor->szFileData);
		ZwClose(hFile);
		return FALSE;
	}
	ZwClose(hFile);

	return TRUE;

}

PVOID
	GetDirectoryAddr(PUCHAR AddressBase,USHORT DirectoryIndex,
	ULONG_PTR* ulSize,ULONG_PTR* ulDiff,BOOLEAN IsFile)
{
	PIMAGE_DOS_HEADER  DosHeader = NULL;
	PIMAGE_NT_HEADERS  NtHeader  = NULL;
	PIMAGE_SECTION_HEADER  SectionHeader = NULL;
	PVOID			   DirAddr   = NULL;


	DosHeader = (PIMAGE_DOS_HEADER)AddressBase;

	if (!MmIsAddressValid(DosHeader))
	{
		return NULL;
	}

	if (DosHeader->e_magic!=IMAGE_DOS_SIGNATURE)
	{
		return NULL;
	}


	NtHeader = MakePtr(PIMAGE_NT_HEADERS,DosHeader,DosHeader->e_lfanew);


	if (!MmIsAddressValid(NtHeader))
	{
		return NULL;
	}

	if (NtHeader->Signature!=IMAGE_NT_SIGNATURE)
	{
		return NULL;
	}



	if (IsFile)  //这里判断是否是文件映射 还是将文件读入到内存的形式
	{


		//判断导入表属于那个节表中
		SectionHeader = GetSectionHeaderFromRva(NtHeader->OptionalHeader.DataDirectory[DirectoryIndex].VirtualAddress,
			NtHeader);   


		if (SectionHeader==NULL)
		{
			return NULL;
		}

		//  PointerToRawData == 0x200       VirtualAddress == 0x1000    
		//                                              RVA = 0x1030

		//	那么导出表在文件中的偏移就是0x230  返回	

		//节在内存中的RVA - 在文件中节的偏移 = 该节被提高了多少        
		*ulDiff = (int)(SectionHeader->VirtualAddress - SectionHeader->PointerToRawData);   
		//0x1000 - 0x200 = 0xE00	

	}

	else
	{
		*ulDiff = 0;
	}


	DirAddr = MakePtr(PVOID,DosHeader,NtHeader->OptionalHeader.DataDirectory[DirectoryIndex].VirtualAddress - *ulDiff); 


	if (DirAddr == (VOID*)NtHeader)
	{
		return NULL;
	}


	if (ulSize)
	{
		*ulSize = MakePtr(ULONG_PTR,DosHeader,NtHeader->OptionalHeader.DataDirectory[DirectoryIndex].Size); 


		if (ulSize)
		{
			*ulSize -= (ULONG_PTR)DosHeader; 
		}

	}


	return DirAddr;
}

PIMAGE_SECTION_HEADER
	GetSectionHeaderFromRva(ULONG RVA,PIMAGE_NT_HEADERS NtHeader)  //判断RVA是在那个节表当中
{
	ULONG   i = 0;
	PIMAGE_SECTION_HEADER  SectionHeader = IMAGE_FIRST_SECTION(NtHeader);

	for (i=0;i<NtHeader->FileHeader.NumberOfSections;i++,SectionHeader++)
	{

		if ((RVA>=SectionHeader->VirtualAddress)&&
			(RVA<(SectionHeader->VirtualAddress + SectionHeader->Misc.VirtualSize)))
		{
			return SectionHeader;
		}
	}

	return NULL;	
}

//////////////////////////////////////////////////////////////////////////

BOOLEAN
	GetModuleInfor(char* szModuleName,char* szFullName, SYSTEM_MODULE_INFORMATION_ENTRY* Temp)
{
	ULONG_PTR  NeedSize;
	ULONG_PTR  KernelModuleCount = 0;
	SYSTEM_MODULE_INFORMATION_ENTRY* ModuleInfor;
	ULONG_PTR  i = 0;
	BOOLEAN    bOk = FALSE;
	//PSYSTEM_MODULE_INFORMATION Buffer = NULL;

	PETHREAD EThread = NULL;
	CHAR     PreMode = 0;

	EThread = PsGetCurrentThread();
	PreMode = HsChangePreMode(EThread);

	NtQuerySystemInformation(SystemModuleInformation,NULL, 
		0,&NeedSize);

	if (ModuleInforNeedSize != NeedSize && ModuleInforNeedSize > 0)	//如果大小有变，则清空缓冲区
	{
		ExFreePool(ModuleInforBuffer);
		DbgPrint("ExFreePool\r\n");
		ModuleInforBuffer = NULL;
		ModuleInforNeedSize = NeedSize;	//更新大小全局变量
	}
	if (ModuleInforBuffer == NULL)
	{
		DbgPrint("NeedSize: %d\r\n");
		ModuleInforBuffer = (PSYSTEM_MODULE_INFORMATION)ExAllocatePool(NonPagedPool,NeedSize);
	}
	if (ModuleInforBuffer == NULL)
	{
		DbgPrint("ModuleInforBuffer ExAllocatePool Failed");
	}

	if(NtQuerySystemInformation(SystemModuleInformation,ModuleInforBuffer,NeedSize,&NeedSize)
		==STATUS_SUCCESS)
	{
		char szKernelModuleName[60] = {0};

		KernelModuleCount = ModuleInforBuffer->ulCount;
		ModuleInfor = ModuleInforBuffer->smi;

		strcpy(szKernelModuleName, ModuleInfor->ImageName + ModuleInfor->ModuleNameOffset);

		for (i=0;i<KernelModuleCount;i++)
		{
			//DbgPrint("%s : %s\r\n",szModuleName,ModuleInfor->ImageName + ModuleInfor->ModuleNameOffset);

			if(_stricmp(szModuleName, ModuleInfor->ImageName + ModuleInfor->ModuleNameOffset) == 0 || 
				(_stricmp(szModuleName, "ntoskrnl.exe") == 0 && 
				 _stricmp(szKernelModuleName, ModuleInfor->ImageName + ModuleInfor->ModuleNameOffset) == 0 ))
			{
				//DbgPrint("%s\r\n",ModuleInfor->ImageName);  //这里是没有盘符的我在这里偷懒写死了

				//////////////////////////////////////////////////////////////////////////
				if (strstr(ModuleInfor->ImageName,"SystemRoot")==NULL)
				{
					Temp->Base = ModuleInfor->Base;
					Temp->Size = ModuleInfor->Size;
					strcpy(szFullName,ModuleInfor->ImageName);
				}
				else
				{
					CHAR ImageFull[260] = {0};

					Temp->Base = ModuleInfor->Base;
					Temp->Size = ModuleInfor->Size;

					strcpy(szFullName,"\\??\\C:");
					strcat(szFullName,ModuleInfor->ImageName);
				}

				//////////////////////////////////////////////////////////////////////////
				Temp->Base = ModuleInfor->Base;
				Temp->Size = ModuleInfor->Size;

				if (strnicmp(ModuleInfor->ImageName,"\\Windows\\",strlen("\\Windows\\"))==0)
				{
					char* Temp = NULL;
					strcpy(szFullName,"\\??\\C:\\");

					Temp = ModuleInfor->ImageName;

					strcat(szFullName,Temp);
				}
				else
				{
					strcpy(szFullName,ModuleInfor->ImageName);
				}

				DbgPrint("%s\r\n",szFullName);
				DbgPrint("%s\r\n",ModuleInfor->ImageName);

				bOk = TRUE;
				break;
			}
			ModuleInfor++;
		}
	}
	else
	{
		DbgPrint("NtQuerySystemInformation Failed\r\n");
	}

	HsRecoverPreMode(EThread, PreMode);

	return bOk;
}



//////////////////////////////////////////////////////////////////////////



PFILE_INFOR
	CreateFileData(SYSTEM_MODULE_INFORMATION_ENTRY* ModuleInfor,char* ModuleName)
{
	BOOLEAN        bOk     = FALSE;
	char           szFullName[256] = {0};

	SYSTEM_MODULE_INFORMATION_ENTRY  Temp;

	if (!ModuleInfor)  //通过文件名称获得文件全路径
	{

		bOk = GetModuleInfor(ModuleName,szFullName,&Temp);

		if (bOk==FALSE)
		{
			DbgPrint("GetModuleInfor Failed\r\n");
			return NULL;
		}
	}

	if (PFileInfor == NULL)
	{
		PFileInfor = (PFILE_INFOR)ExAllocatePool(PagedPool,sizeof(FILE_INFOR));
	}
 	if (PFileInfor == NULL)
 	{
 		DbgPrint("ExAllocatePool Failed\r\n");
 		return NULL;
 	}


	RtlZeroMemory(PFileInfor,sizeof(FILE_INFOR));

	if (ModuleInfor)
	{
		PFileInfor->BaseAddress = ModuleInfor->Base;
		PFileInfor->Size        = ModuleInfor->Size;
		strcpy(PFileInfor->szFileFullName,ModuleInfor->ImageName);


		//DbgPrint("%x    %d    %s\r\n",RvrFile->BaseAddress,RvrFile->Size,RvrFile->FileName);
	}

	else
	{
		PFileInfor->BaseAddress = (PVOID)Temp.Base;
		PFileInfor->Size        = Temp.Size;
		memcpy(PFileInfor->szFileFullName,szFullName,strlen(szFullName)+1);

	}
	//读取文件

	bOk = ReadFileData(PFileInfor);

	if (bOk==FALSE)
	{
		//ExFreePool(FileInfor);
		DbgPrint("ReadFileData Failed\r\n");
		return NULL;
	}

	return PFileInfor;

}




BOOLEAN GetAddrOfExportFuncAddr(
	UCHAR*  Base,				  
	CHAR*	FunctionName,	      
	ULONG_PTR*  AddrOfExportFuncAddr, BOOLEAN IsFile)
{
	PIMAGE_EXPORT_DIRECTORY 		ExportTable = NULL;
	ULONG*	    FuncName = NULL;
	ULONG*      FuncAddr = NULL;
	ULONG_PTR   j = 0;
	ULONG_PTR	ExportDiff;
	PSHORT		Ordinal;

	ExportTable = 
		(PIMAGE_EXPORT_DIRECTORY)GetDirectoryAddr(Base, 
		IMAGE_DIRECTORY_ENTRY_EXPORT, 
		NULL, &ExportDiff, IsFile);

	if(!ExportTable) 
	{
		return FALSE;
	}


	FuncName = MakePtr(PULONG, Base, ExportTable->AddressOfNames - ExportDiff);
	FuncAddr = MakePtr(PULONG, Base, ExportTable->AddressOfFunctions - ExportDiff);
	Ordinal  = MakePtr(PSHORT, Base, ExportTable->AddressOfNameOrdinals - ExportDiff);


	for(j = 0; j < ExportTable->NumberOfNames; j++, FuncName++){

		if (_stricmp(FunctionName, (CHAR*)Base + *FuncName - ExportDiff) == 0)
		{

			*AddrOfExportFuncAddr = *(FuncAddr + Ordinal[j]);
			return TRUE;
		}
	}


	return FALSE;   
}


//////////////////////////////////////////////////////////////////////////

BOOLEAN	GetModuleInforKernelFile(PULONG_PTR ulKernelBase,PULONG_PTR ulKernelSize,char* szModuleFileFullName, char* szModuleFile)
{
	NTSTATUS  Status;
	ULONG_PTR NeedSize = 0;
	PSYSTEM_MODULE_INFORMATION  ModuleList = NULL;

	int ModuleId = 0;

	PETHREAD EThread = NULL;
	CHAR     PreMode = 0;

	EThread = PsGetCurrentThread();
	PreMode = HsChangePreMode(EThread);

	Status = NtQuerySystemInformation(SystemModuleInformation,NULL,
		0,&NeedSize);  //不能被Hook


	if (Status!=STATUS_INFO_LENGTH_MISMATCH)
	{
		DbgPrint("STATUS_INFO_LENGTH_MISMATCH FAILED\r\n");
		return FALSE;
	}

	ModuleList = (PSYSTEM_MODULE_INFORMATION)ExAllocatePool(PagedPool,NeedSize);

	if (ModuleList==NULL)
	{
		DbgPrint("ModuleList==NULL\r\n");
		return FALSE;
	}

	Status = NtQuerySystemInformation(SystemModuleInformation,ModuleList,NeedSize,&NeedSize);

	if (!NT_SUCCESS(Status))
	{
		DbgPrint("NtQuerySystemInformation Failed\r\n");
		ExFreePool(ModuleList);
		return FALSE;
	}


	for (ModuleId = 0; ModuleId < ModuleList->ulCount; ModuleId++)
	{
		if (_stricmp(szModuleFile, "ntoskrnl.exe") == 0)
		{
			ModuleId = 0;
			DbgPrint("Win32k ModuleId: %d\r\n",ModuleId);
			break;
		}
		else if (_stricmp(
			ModuleList->smi[ModuleId].ImageName+ModuleList->smi[ModuleId].ModuleNameOffset,
			szModuleFile) == 0)
		{

			DbgPrint("Win32k ModuleId: %d\r\n",ModuleId);
			break;
		}
	}

	DbgPrint("%s\r\n",ModuleList->smi[ModuleId].ImageName);

	*ulKernelBase = ModuleList->smi[ModuleId].Base;
	*ulKernelSize = ModuleList->smi[ModuleId].Size;

	if (memcmp(ModuleList->smi[ModuleId].ImageName,"\\??\\",strlen("\\??\\"))==0)
	{
		strcpy(szModuleFileFullName,ModuleList->smi[ModuleId].ImageName);
	}
	else if (strnicmp(ModuleList->smi[ModuleId].ImageName,"\\SystemRoot\\",strlen("\\SystemRoot\\"))==0)
	{
		char* Temp = NULL;
		strcpy(szModuleFileFullName,"\\??\\C:\\Windows\\"); /*System32\\*/
		
		Temp = ModuleList->smi[ModuleId].ImageName + strlen("\\SystemRoot\\");

		strcat(szModuleFileFullName,Temp);
	}
	else if (strnicmp(ModuleList->smi[ModuleId].ImageName,"\\Windows\\",strlen("\\Windows\\"))==0)
	{
		char* Temp = NULL;
		strcpy(szModuleFileFullName,"\\??\\C:\\");

		Temp = ModuleList->smi[ModuleId].ImageName;

		strcat(szModuleFileFullName,Temp);
	}
	

	DbgPrint("szModuleFileFullName: %s\r\n",szModuleFileFullName);

	ExFreePool(ModuleList);

	HsRecoverPreMode(EThread, PreMode);

	return TRUE;

}