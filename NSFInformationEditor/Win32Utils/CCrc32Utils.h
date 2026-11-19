#pragma once

#include <string>
#include <stdint.h>

#ifdef _UNICODE
using _tstring = std::wstring;
#else
using _tstring = std::string;
#endif

namespace CCrc32Utils
{
    uint32_t GetCrc32Value(const void* lpData, size_t nSize);
    _tstring GetCrc32Str(const void* lpData, size_t nSize, bool bUpper = false);
    uint32_t GetFileCrc32Value(const _tstring& strFilePath);
    _tstring GetFileCrc32Str(const _tstring& strFilePath, bool bUpper = false);
    uint32_t GetPartCrc32(uint32_t crcInit, const void* lpData, size_t byteSize);
    _tstring GetStrFromValue(uint32_t uCrc32, bool bUpper = false);
}