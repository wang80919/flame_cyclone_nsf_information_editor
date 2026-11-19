#include "CDiskUtils.h"
#include "CStrUtils.h"
#include <strsafe.h>

static LPCTSTR s_szBusTypes[] = {
    _T("Unknown"),
    _T("SCSI"),
    _T("ATAPI"),
    _T("ATA"),
    _T("IEEE 1394"),
    _T("SSA"),
    _T("FIBRE"),
    _T("USB"),
    _T("RAID"),
    _T("ISCSI"),
    _T("SAS"),
    _T("SATA"),
    _T("SD"),
    _T("MMC"),
    _T("VIRTUAL"),
    _T("FILEBACKEDVIRTUAL"),
    _T("SPACES"),
    _T("NVMe"),
    _T("SCM"),
    _T("UFS"),
    _T("MAX"),
    _T("MAXRESERVED")
};

bool CDiskUtils::GetDiskInfo(DWORD nIndex, DISK_INFO& info)
{
    TCHAR szBuf[MAX_PATH] = { 0 };
    CHAR szOut[1024] = { 0 };
    HANDLE hDevice = INVALID_HANDLE_VALUE;
    STORAGE_PROPERTY_QUERY spq;
    std::vector<VOLUME_INFO> vVolumeInfo;
    DWORD dwBytesReturned = 0;
    bool bSuccess = false;

    ZeroMemory(&spq, sizeof(spq));
    (void)GetVolumeInfo(vVolumeInfo);

    info.clear();

    (void)::StringCchPrintf(szBuf, _countof(szBuf), _T(R"(\\.\PhysicalDrive%d)"), nIndex);
    hDevice = ::CreateFile(szBuf,
        GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
        NULL, OPEN_EXISTING, 0, NULL);
    if (INVALID_HANDLE_VALUE == hDevice)
    {
        goto L_CleanUp;
    }

    spq.PropertyId = StorageDeviceProperty;
    spq.QueryType = PropertyStandardQuery;
    if (::DeviceIoControl(hDevice, IOCTL_STORAGE_QUERY_PROPERTY, &spq, sizeof(spq), szOut, sizeof(szOut), &dwBytesReturned, NULL))
    {
        PSTORAGE_DEVICE_DESCRIPTOR pDesc = (PSTORAGE_DEVICE_DESCRIPTOR)szOut;
        info.m_ProductId = CStrUtils::AStrToTStr((LPCSTR)((LPBYTE)pDesc + pDesc->ProductIdOffset));
        info.m_ProductRevision = CStrUtils::AStrToTStr((LPCSTR)((LPBYTE)pDesc + pDesc->ProductRevisionOffset));
        info.m_SerialNumber = CStrUtils::AStrToTStr((LPCSTR)((LPBYTE)pDesc + pDesc->SerialNumberOffset));
        info.m_BusType = pDesc->BusType;
        info.m_BusTypeName = s_szBusTypes[pDesc->BusType];
    }

    spq.PropertyId = StorageDeviceTemperatureProperty;
    if (::DeviceIoControl(hDevice, IOCTL_STORAGE_QUERY_PROPERTY, &spq, sizeof(spq), szOut, sizeof(szOut), &dwBytesReturned, NULL))
    {
        PSTORAGE_TEMPERATURE_DATA_DESCRIPTOR pDesc = (PSTORAGE_TEMPERATURE_DATA_DESCRIPTOR)szOut;
        info.m_Temperature = pDesc->TemperatureInfo->Temperature;
    }

    spq.PropertyId = StorageAdapterSerialNumberProperty;
    if (::DeviceIoControl(hDevice, IOCTL_STORAGE_QUERY_PROPERTY, &spq, sizeof(spq), szOut, sizeof(szOut), &dwBytesReturned, NULL))
    {
        PSTORAGE_ADAPTER_SERIAL_NUMBER pDesc = (PSTORAGE_ADAPTER_SERIAL_NUMBER)szOut;
        info.m_AdapterSerialNumber = CStrUtils::WStrToTStr(pDesc->SerialNumber);
    }

    if (::DeviceIoControl(hDevice, IOCTL_DISK_GET_DRIVE_GEOMETRY_EX, NULL, 0, &szOut, sizeof(szOut), &dwBytesReturned, NULL))
    {
        PDISK_GEOMETRY_EX pDesc = (PDISK_GEOMETRY_EX)szOut;
        info.m_DiskSize = *(unsigned long long*)& pDesc->DiskSize;
    }

    if (::DeviceIoControl(hDevice, IOCTL_DISK_GET_DRIVE_LAYOUT_EX, NULL, 0, &szOut, sizeof(szOut), &dwBytesReturned, NULL))
    {
        PDRIVE_LAYOUT_INFORMATION_EX pDesc = (PDRIVE_LAYOUT_INFORMATION_EX)szOut;
        for (DWORD i = 0; i < pDesc->PartitionCount; i++)
        {
            PPARTITION_INFORMATION_EX pPartInfo = &pDesc->PartitionEntry[i];
            if (0 == pPartInfo->PartitionNumber)
            {
                continue;
            }

            PARTION_INFO partionInfo;
            partionInfo.m_PartitionLength = *(unsigned long long*) &pPartInfo->PartitionLength;
            partionInfo.m_PartitionNumber = pPartInfo->PartitionNumber;
            partionInfo.m_StartingOffset = *(unsigned long long*) & pPartInfo->StartingOffset;
            info.m_Partitions.push_back(partionInfo);
        }
    }

    if (::DeviceIoControl(hDevice, IOCTL_DISK_GET_DISK_ATTRIBUTES, NULL, 0, &szOut, sizeof(szOut), &dwBytesReturned, NULL))
    {
        PGET_DISK_ATTRIBUTES pDesc = (PGET_DISK_ATTRIBUTES)szOut;
        info.m_Attributes = pDesc->Attributes;
    }

    if (::DeviceIoControl(hDevice, IOCTL_DISK_GET_CACHE_INFORMATION, NULL, 0, &szOut, sizeof(szOut), &dwBytesReturned, NULL))
    {
        PDISK_CACHE_INFORMATION pDesc = (PDISK_CACHE_INFORMATION)szOut;
        pDesc = (PDISK_CACHE_INFORMATION)szOut;
    }

    // 从卷信息设置分区信息
    for (auto& item : info.m_Partitions)
    {
        for (const auto& volume : vVolumeInfo)
        {
            if (nIndex == volume.m_DiskNumber &&
                item.m_StartingOffset == volume.m_StartingOffset
                )
            {
                item.m_PathName = volume.m_PathName;
                item.m_VolumeName = volume.m_VolumeName;
                break;
            }
        }
    }

    bSuccess = true;

L_CleanUp:

    if (INVALID_HANDLE_VALUE != hDevice)
    {
        ::CloseHandle(hDevice);
    }

    return bSuccess;
}

bool CDiskUtils::GetVolumeInfo(std::vector<VOLUME_INFO>& vVolumeInfo)
{
    CHAR szOutBuffer[1024] = { 0 };
    TCHAR szBuf[MAX_PATH] = { 0 };
    DWORD dwBytesReturned = 0;

    if (0 == GetLogicalDriveStrings(_countof(szBuf), szBuf))
    {
        return false;
    }

    LPCTSTR lpDriveString = szBuf;
    while (_T('\0') != *lpDriveString)
    {
        TCHAR szDeviceBuf[MAX_PATH] = { 0 };
        HANDLE hDevice = INVALID_HANDLE_VALUE;

        ::StringCchCopy(szDeviceBuf, _countof(szDeviceBuf), _T(R"(\\.\)"));
        ::StringCchCatN(szDeviceBuf, _countof(szDeviceBuf), lpDriveString, 2);
        hDevice = ::CreateFile(szDeviceBuf, GENERIC_READ | GENERIC_WRITE,
            FILE_SHARE_READ | FILE_SHARE_WRITE,
            NULL,
            OPEN_EXISTING,
            FILE_FLAG_SEQUENTIAL_SCAN,
            NULL);

        if (INVALID_HANDLE_VALUE == hDevice)
        {
            lpDriveString += 4;
            continue;
        }

        if (::DeviceIoControl(hDevice,
            IOCTL_VOLUME_GET_VOLUME_DISK_EXTENTS,
            NULL,
            0,
            &szOutBuffer,
            sizeof(szOutBuffer),
            &dwBytesReturned,
            NULL))
        {
            PVOLUME_DISK_EXTENTS pDesc = (PVOLUME_DISK_EXTENTS)szOutBuffer;

            //当用户尝试获取有关没有软盘的软盘驱动器或没有光盘的 CD-ROM 驱动器的信息时，
            //系统会显示一个消息框, 防止系统显示此消息框
            (void)::SetErrorMode(SEM_FAILCRITICALERRORS);

            //获取卷信息 (卷中的磁盘数可以跨多个磁盘) 。
            for (DWORD i = 0; i < pDesc->NumberOfDiskExtents; i++)
            {
                PDISK_EXTENT pDiskExtent = &pDesc->Extents[i];
                TCHAR szVolumeName[MAX_PATH] = { 0 };
                DWORD dwVolumeSerialNumber = 0;
                DWORD dwMaximumComponentLength = 0;
                DWORD dwFileSystemFlags = 0;
                TCHAR szFileSystemName[MAX_PATH] = { 0 };
                ::GetVolumeInformation(lpDriveString,
                    szVolumeName,
                    _countof(szVolumeName),
                    &dwVolumeSerialNumber,
                    &dwMaximumComponentLength,
                    &dwFileSystemFlags,
                    szFileSystemName,
                    _countof(szFileSystemName)
                );

                VOLUME_INFO info;
                info.m_PathName = _tstring(lpDriveString).substr(0, 2);
                info.m_VolumeName = szVolumeName;
                info.m_FileSystemName = szFileSystemName;
                info.m_DiskNumber = pDiskExtent->DiskNumber;
                info.m_StartingOffset = *(unsigned long long*) & pDiskExtent->StartingOffset;
                info.m_Length = *(unsigned long long*) & pDiskExtent->ExtentLength;
                vVolumeInfo.push_back(info);
            }
        }

        ::CloseHandle(hDevice);
        lpDriveString += 4;
    }

    return true;
}