#pragma once

#include <wtypesbase.h>
#include <string>
#include <tchar.h>
#include <vector>

#ifdef _UNICODE
using _tstring = std::wstring;
#else
using _tstring = std::string;
#endif

typedef struct _PARTION_INFO
{
    _tstring m_PathName;
    _tstring m_VolumeName;
    DWORD m_PartitionNumber;
    unsigned long long m_StartingOffset;
    unsigned long long m_PartitionLength;

    _PARTION_INFO() :
        m_PartitionNumber(0),
        m_PartitionLength(0),
        m_StartingOffset(0)
    {

    }

}PARTION_INFO;

typedef struct _VOLUME_INFO
{
    _tstring m_PathName;
    _tstring m_VolumeName;
    _tstring m_FileSystemName;
    DWORD m_DiskNumber;
    unsigned long long m_StartingOffset;
    unsigned long long m_Length;
    _VOLUME_INFO() :
        m_DiskNumber(0),
        m_StartingOffset(0),
        m_Length(0)
    {

    }

}VOLUME_INFO;

typedef struct _DISK_INFO
{
    _tstring m_ProductId;
    _tstring m_SerialNumber;
    _tstring m_AdapterSerialNumber;
    _tstring m_ProductRevision;
    _tstring m_BusTypeName;
    int m_Temperature;
    int m_BusType;
    unsigned long long m_DiskSize;
    DWORDLONG m_Attributes;
    std::vector<PARTION_INFO> m_Partitions;

    _DISK_INFO() :
        m_Temperature(0),
        m_BusType(0),
        m_DiskSize(0),
        m_Attributes(0)
    {

    }

    void clear()
    {
        m_Partitions.clear();
        m_ProductId.clear();
        m_SerialNumber.clear();
        m_AdapterSerialNumber.clear();
        m_ProductRevision.clear();
        m_BusTypeName.clear();
        m_Temperature = 0;
        m_BusType = 0;
        m_DiskSize = 0;
        m_Attributes = 0;
    }

}DISK_INFO;

class CDiskUtils
{
public:

    static bool GetDiskInfo(DWORD nIndex, DISK_INFO& info);
    static bool GetVolumeInfo(std::vector<VOLUME_INFO>& vVolumeInfo);
};