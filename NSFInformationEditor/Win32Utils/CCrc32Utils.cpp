#include "CCrc32Utils.h"
#include <fstream>

#define CRC32_POLY          (0xEDB88320L)           // CRC32标准
#define CRC32_TABLE_SIZE    (256)                   // CRC查询表大小
#define CRC32_BLOCK_SIZE    (1024 * 1024 * 64)      // CRC缓冲块大小

#define _count_of(_array) (sizeof(_array) / sizeof(_array[0]))

namespace CCrc32Utils
{
    static uint32_t g_Crc32Table[CRC32_TABLE_SIZE] = { 0 }; // CRC查询表
    static bool g_bInitFlag = false;                        // CRC查询表初始化标记

    static void _InitCrc32Table(void);
    static uint32_t GetCrc32(const uint8_t* lpData, size_t byteSize);
    static uint32_t GetCrc32Data(uint32_t crcInit, const uint8_t* lpData, size_t byteSize);

    uint32_t GetCrc32Value(const void* lpData, size_t nSize)
    {
        if (!g_bInitFlag)
        {
            _InitCrc32Table();
            g_bInitFlag = true;
        }

        return GetCrc32((uint8_t*)lpData, nSize);
    }

    _tstring GetCrc32Str(const void* lpData, size_t nSize, bool bUpper/* = false*/)
    {
        return GetStrFromValue(GetCrc32Value((uint8_t*)lpData, nSize), bUpper);
    }

    uint32_t GetPartCrc32(uint32_t crcInit, const void* lpData, size_t byteSize)
    {
        if (0 == byteSize)
        {
            return crcInit;
        }

        if (!g_bInitFlag)
        {
            _InitCrc32Table();
            g_bInitFlag = true;
        }

        return GetCrc32Data(crcInit, (uint8_t*)lpData, byteSize);
    }

    uint32_t GetFileCrc32Value(const _tstring& strFilePath)
    {
        if (!g_bInitFlag)
        {
            _InitCrc32Table();
            g_bInitFlag = true;
        }

        uint32_t dwCrc32 = 0;
        std::ifstream inputFile(strFilePath.c_str(), std::ios::binary | std::ios::in);
        if (!inputFile.is_open())
        {
            return false;
        }

        uint8_t* lpBuf = new (std::nothrow) uint8_t[CRC32_BLOCK_SIZE];
        if (nullptr == lpBuf)
        {
            return dwCrc32;
        }

        while (!inputFile.eof())
        {
            inputFile.read((char*)lpBuf, CRC32_BLOCK_SIZE);
            size_t nByteSize = (size_t)inputFile.gcount();
            if (nByteSize > 0)
            {
                dwCrc32 = GetCrc32Data(dwCrc32, lpBuf, nByteSize);
            }
        }

        inputFile.close();
        if (nullptr != lpBuf)
        {
            delete[]lpBuf;
        }

        return dwCrc32;
    }

    _tstring GetFileCrc32Str(const _tstring& strFilePath, bool bUpper/* = false*/)
    {
        return GetStrFromValue(GetFileCrc32Value(strFilePath), bUpper);
    }

    _tstring GetStrFromValue(uint32_t uCrc32, bool bUpper/* = false*/)
    {
#ifdef _UNICODE
        wchar_t szBuf[32] = { 0 };
#else
        char szBuf[32] = { 0 };
#endif

#ifdef _UNICODE
        _snwprintf_s(szBuf, _count_of(szBuf), bUpper ? L"%0.8X" : L"%0.8x", uCrc32);
#else
        snprintf(szBuf, _count_of(szBuf), bUpper ? "%0.8X" : "%0.8x", uCrc32);
#endif

        return szBuf;
    }

    void _InitCrc32Table(void)
    {
        for (uint32_t i = 0; i < _count_of(g_Crc32Table); i++)
        {
            uint32_t crc = i;
            for (int j = 0; j < 8; j++)
            {
                if (crc & 0x00000001L)
                    crc = (crc >> 1) ^ CRC32_POLY;
                else
                    crc = crc >> 1;
            }
            g_Crc32Table[i] = crc;
        }
    }

    uint32_t GetCrc32(const uint8_t* lpData, size_t byteSize)
    {
        uint32_t dwCrc32 = 0;
        size_t nBlockSize = byteSize > CRC32_BLOCK_SIZE ? CRC32_BLOCK_SIZE : byteSize;
        uint8_t* lpBase = (uint8_t*)lpData;

        while (byteSize > 0)
        {
            dwCrc32 = GetCrc32Data(dwCrc32, lpBase, nBlockSize);
            lpBase += nBlockSize;
            byteSize -= nBlockSize;

            nBlockSize = byteSize > CRC32_BLOCK_SIZE ? CRC32_BLOCK_SIZE : byteSize;
        }

        return dwCrc32;
    }

    uint32_t GetCrc32Data(uint32_t crcInit, const uint8_t* lpData, size_t byteSize)
    {
        uint32_t crc = crcInit ^ 0xffffffff;

        while (byteSize--)
        {
            crc = (crc >> 8) ^ g_Crc32Table[(crc & 0xff) ^ *lpData++];
        }

        return crc ^ 0xffffffff;
    }
}