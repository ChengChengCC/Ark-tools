#include "FilterDriver.h"
#include "Module.h"

extern ULONG_PTR		SYSTEM_ADDRESS_START;
extern PDRIVER_OBJECT	g_DriverObject;
extern WIN_VERSION		WinVersion;

ULONG_PTR ulVolumeStartCount = 0;
ULONG_PTR ulFileSystemStartCount = 0;

NTSTATUS HsEnumFilterDriver(PFILTER_DRIVER FilterDriverInfor)
{
	NTSTATUS Status = STATUS_UNSUCCESSFUL;

	// 写上所有的驱动名。

	ulVolumeStartCount = 0;
	ulFileSystemStartCount = 0;

	Status = GetFilterDriverByDriverName(L"\\Driver\\Disk", FilterDriverInfor,Disk);
	Status = GetFilterDriverByDriverName(L"\\Driver\\volmgr", FilterDriverInfor,Volume);
	Status = GetFilterDriverByDriverName(L"\\FileSystem\\ntfs", FilterDriverInfor,File);
	Status = GetFilterDriverByDriverName(L"\\FileSystem\\fastfat", FilterDriverInfor,File);
	Status = GetFilterDriverByDriverName(L"\\FileSystem\\Raw", FilterDriverInfor,RAW);
	Status = GetFilterDriverByDriverName(L"\\driver\\kbdclass", FilterDriverInfor,Keyboard);
	Status = GetFilterDriverByDriverName(L"\\driver\\mouclass", FilterDriverInfor,Mouse);
	Status = GetFilterDriverByDriverName(L"\\driver\\i8042prt", FilterDriverInfor,I8042prt);
	Status = GetFilterDriverByDriverName(L"\\Driver\\tdx", FilterDriverInfor,Tdx);
	Status = GetFilterDriverByDriverName(L"\\Driver\\NDIS", FilterDriverInfor,NDIS);
	Status = GetFilterDriverByDriverName(L"\\Driver\\PnpManager", FilterDriverInfor,PnpManager);

	return Status;
}



NTSTATUS HsUnloadFilterDriver(PUNLOAD_FILTER UnloadFilter)
{
	NTSTATUS Status;

	DbgPrint("FilterType: %d\r\n",UnloadFilter->Type);
	DbgPrint("DeviceObject: %d\r\n",UnloadFilter->DeviceObject);

	switch(UnloadFilter->Type)
	{
	case Disk:
		{
			Status = ClearFilters(L"\\Driver\\Disk", UnloadFilter->DeviceObject);
			break;
		}
	default:
		{

		}
	}

	return Status;
}




NTSTATUS ClearFilters(WCHAR* wzDriverName,ULONG_PTR DeviceObject)
{
	UNICODE_STRING uniDriverName;
	PDRIVER_OBJECT DriverObject;
	PDEVICE_OBJECT CurrentDevice;
	NTSTATUS Status;


	RtlInitUnicodeString(&uniDriverName, wzDriverName);
	Status = ObReferenceObjectByName(&uniDriverName,
		OBJ_CASE_INSENSITIVE,
		NULL,
		0,
		*IoDriverObjectType,
		KernelMode,
		NULL,
		(PVOID*)&DriverObject);

	if (!NT_SUCCESS(Status)) return ;
	if(!DriverObject) return ;
	CurrentDevice = DriverObject->DeviceObject;
	while(CurrentDevice != NULL )
	{
		if ((ULONG_PTR)CurrentDevice->AttachedDevice == DeviceObject)
		{

			CurrentDevice->AttachedDevice = ((PDEVICE_OBJECT)DeviceObject)->AttachedDevice;

		}
		CurrentDevice = CurrentDevice->NextDevice;
	}
	ObDereferenceObject(DriverObject);

	return Status;
}



NTSTATUS GetFilterDriverByDriverName(WCHAR *wzDriverName, PFILTER_DRIVER FilterDriverInfor, FILTER_TYPE Type)
{
	NTSTATUS Status = STATUS_UNSUCCESSFUL;
	UNICODE_STRING uniDriverName;
	PDRIVER_OBJECT DriverObject = NULL;
	POBJECT_TYPE DriverObjectType = *IoDriverObjectType;


	if (!DriverObjectType)
	{
		return Status;
	}

	RtlInitUnicodeString(&uniDriverName, wzDriverName);

	Status = ObReferenceObjectByName(
		&uniDriverName,
		OBJ_CASE_INSENSITIVE,
		NULL,
		0,
		DriverObjectType,
		KernelMode,
		NULL,
		(PVOID*)&DriverObject);

	if (NT_SUCCESS(Status) && DriverObject)
	{
		PDEVICE_OBJECT DeviceObject = NULL;

		for ( DeviceObject = DriverObject->DeviceObject;
			  DeviceObject;
			  DeviceObject = DeviceObject->NextDevice )
		{


			PDRIVER_OBJECT AttachedDriverObject = DeviceObject->DriverObject;
			PDEVICE_OBJECT AttachDeviceObject = NULL;

			for ( AttachDeviceObject = DeviceObject->AttachedDevice; 
				  AttachDeviceObject; 
				  AttachDeviceObject = AttachDeviceObject->AttachedDevice )
			{
				Status = AddFilterInfo( AttachDeviceObject, AttachedDriverObject, FilterDriverInfor, Type );
				AttachedDriverObject = AttachDeviceObject->DriverObject;
			}

		}

		ObfDereferenceObject(DriverObject);
	}

	return Status;
}




NTSTATUS AddFilterInfo(PDEVICE_OBJECT AttachDeviceObject, PDRIVER_OBJECT AttachedDriverObject, PFILTER_DRIVER FilterDriverInfor, FILTER_TYPE Type)
{
	if (AttachDeviceObject && AttachedDriverObject && FilterDriverInfor)
	{
		if (FilterDriverInfor->ulCnt > FilterDriverInfor->ulRetCnt)
		{
			ULONG_PTR ulRetCnt = FilterDriverInfor->ulRetCnt;
			PDRIVER_OBJECT   AttachDriverObject = AttachDeviceObject->DriverObject;
			PKLDR_DATA_TABLE_ENTRY Entry = NULL;

			BOOLEAN bIsExist = FALSE;
			ULONG_PTR i = 0;
			char Temp[260] = {0};


			if (Type == File || Type == RAW)
			{
				if (ulFileSystemStartCount == 0)
				{
					ulFileSystemStartCount = ulRetCnt;
				}
				for (i = ulFileSystemStartCount; i < ulRetCnt; i++)
				{
					if (_wcsnicmp(FilterDriverInfor->Filter[i].wzFilterDriverName,
						AttachDriverObject->DriverName.Buffer,
						wcslen(FilterDriverInfor->Filter[i].wzFilterDriverName))==0 && 
						_wcsnicmp(FilterDriverInfor->Filter[i].wzAttachedDriverName,
						AttachedDriverObject->DriverName.Buffer,
						wcslen(FilterDriverInfor->Filter[i].wzAttachedDriverName))==0)
					{
						return STATUS_SUCCESS;
					}
				}
			}
			if (Type == Volume)
			{
				if (ulVolumeStartCount == 0)
				{
					ulVolumeStartCount = ulRetCnt;
				}
				for (i = 0; i < ulRetCnt; i++)
				{
					if (_wcsnicmp(FilterDriverInfor->Filter[i].wzFilterDriverName,
						AttachDriverObject->DriverName.Buffer,
						wcslen(FilterDriverInfor->Filter[i].wzFilterDriverName))==0 && 
						_wcsnicmp(FilterDriverInfor->Filter[i].wzAttachedDriverName,
						AttachedDriverObject->DriverName.Buffer,
						wcslen(FilterDriverInfor->Filter[i].wzAttachedDriverName))==0)
					{
						return STATUS_SUCCESS;
					}
				}

			}
			

			FilterDriverInfor->Filter[ulRetCnt].FileterDeviceObject = (ULONG_PTR)AttachDeviceObject;
			FilterDriverInfor->Filter[ulRetCnt].Type = Type;

			if (IsUnicodeStringValid(&(AttachDriverObject->DriverName)))
			{
				RtlZeroMemory(FilterDriverInfor->Filter[ulRetCnt].wzAttachedDriverName,100);
				memcpy(
					FilterDriverInfor->Filter[ulRetCnt].wzFilterDriverName, 
					AttachDriverObject->DriverName.Buffer,
					AttachDriverObject->DriverName.Length);
			}

			if (IsUnicodeStringValid(&(AttachedDriverObject->DriverName)))
			{
				RtlZeroMemory(FilterDriverInfor->Filter[ulRetCnt].wzAttachedDriverName,100);
				memcpy(
					FilterDriverInfor->Filter[ulRetCnt].wzAttachedDriverName,
					AttachedDriverObject->DriverName.Buffer, 
					AttachedDriverObject->DriverName.Length);
			}

			Entry = (PKLDR_DATA_TABLE_ENTRY)AttachDriverObject->DriverSection;

			if ((ULONG_PTR)Entry > SYSTEM_ADDRESS_START)
			{
				if (IsUnicodeStringValid(&(Entry->FullDllName)))
				{

					memcpy(FilterDriverInfor->Filter[ulRetCnt].wzPath, Entry->FullDllName.Buffer,Entry->FullDllName.Length);
				}
				else if (IsUnicodeStringValid(&(Entry->BaseDllName)))
				{

					memcpy(FilterDriverInfor->Filter[ulRetCnt].wzPath, Entry->BaseDllName.Buffer, Entry->BaseDllName.Length);
				}
			}
		}

		else
		{

			return STATUS_BUFFER_TOO_SMALL;
		}

		FilterDriverInfor->ulRetCnt++;

		return STATUS_SUCCESS;
	}
}





