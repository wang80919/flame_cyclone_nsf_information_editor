#include "CPEHelper.h"
#include <tchar.h>
#include <imagehlp.h>

#define LDR_IS_DATAFILE(_handle)      (((ULONG_PTR)(_handle)) & (ULONG_PTR)1)
#define LDR_IS_IMAGEMAPPING(_handle)  (((ULONG_PTR)(_handle)) & (ULONG_PTR)2)
#define LDR_IS_RESOURCE(_handle)      (LDR_IS_IMAGEMAPPING(_handle) || LDR_IS_DATAFILE(_handle))

#pragma comment(lib, "Imagehlp.lib")

static LPCTSTR g_ResourctTypeName[] = {
    _T("RT_NONE"),
    _T("Cursor"),
    _T("Bitmap"),
    _T("Icon"),
    _T("Menu"),
    _T("Dialog"),
    _T("String Table"),
    _T("Font dir"),
    _T("Font"),
    _T("Accelerator"),
    _T("Application-defined resource (raw data"),
    _T("Message-table entry"),
    _T("Cursor Group"),
    _T("RT_NONE"),
    _T("Icon Group"),
    _T("RT_NONE"),
    _T("Version Info"),
    _T("RT_DLGINCLUDE"),
    _T("RT_NONE"),
    _T("Plug and Play resource"),
    _T("VXD"),
    _T("Animated cursor"),
    _T("Animated icon"),
    _T("Html"),
    _T("Manifest")
};

static std::string _WStrToMultiStr(UINT CodePage, const std::wstring& str)
{
    //计算缓冲区所需的字节长度
    int cbMultiByte = ::WideCharToMultiByte(CodePage, 0, str.c_str(), -1, NULL, 0, NULL, 0);
    std::string strResult(cbMultiByte, 0);

    //成功则返回写入到指示的缓冲区的字节数
    size_t nConverted = ::WideCharToMultiByte(CodePage, 0, str.c_str(), (int)str.size(), &strResult[0], (int)strResult.size(), NULL, NULL);

    //调整内容长度
    strResult.resize(nConverted);
    return strResult;
}

static std::wstring _MultiStrToWStr(UINT CodePage, const std::string& str)
{
    //计算缓冲区所需的字符长度
    int cchWideChar = ::MultiByteToWideChar(CodePage, 0, str.c_str(), -1, NULL, 0);
    std::wstring strResult(cchWideChar, 0);

    //成功则返回写入到指示的缓冲区的字符数
    size_t nConverted = ::MultiByteToWideChar(CodePage, 0, str.c_str(), (int)str.size(), &strResult[0], (int)strResult.size());

    //调整内容长度
    strResult.resize(nConverted);
    return strResult;
}

static std::string _WStrToAStr(const std::wstring& str)
{
    return _WStrToMultiStr(CP_ACP, str);
}

static std::string _WStrToU8Str(const std::wstring& str)
{
    return _WStrToMultiStr(CP_UTF8, str);
}

static _tstring _WStrToTStr(const std::wstring& str)
{
#ifdef _UNICODE
    return str;
#else
    return _WStrToMultiStr(CP_ACP, str);
#endif
}

static std::wstring _AStrToWStr(const std::string& str)
{
    return _MultiStrToWStr(CP_ACP, str);
}

static std::string _AStrToU8Str(const std::string& str)
{
    return _WStrToU8Str(_AStrToWStr(str));
}

static _tstring _AStrToTStr(const std::string& str)
{
#ifdef _UNICODE
    return _MultiStrToWStr(CP_ACP, str);
#else
    return str;
#endif
}

static std::wstring _U8StrToWStr(const std::string& str)
{
    return _MultiStrToWStr(CP_UTF8, str);
}

static std::string _U8StrToAStr(const std::string& str)
{
    return _WStrToAStr(_U8StrToWStr(str));
}

static _tstring _U8StrToTStr(const std::string& str)
{
#ifdef _UNICODE
    return _MultiStrToWStr(CP_UTF8, str);
#else
    return _WStrToAStr(_U8StrToWStr(str));
#endif
}

static std::string _TStrToAStr(const _tstring& str)
{
#ifdef _UNICODE
    return _WStrToMultiStr(CP_ACP, str);
#else
    return str;
#endif
}

static std::wstring _TStrToWStr(const _tstring& str)
{
#ifdef _UNICODE
    return str;
#else
    return _AStrToWStr(str);
#endif
}

static std::string _TStrToU8Str(const _tstring& str)
{
#ifdef _UNICODE
    return _WStrToU8Str(str);
#else
    return _WStrToU8Str(_AStrToWStr(str));
#endif
}

const int FORMAT_COUNT_MAX = (1024 * 1024 * 64);

static void ConsoleOutputA(LPCSTR pFormat, ...)
{
    size_t nCchCount = MAX_PATH;
    std::string strResult(nCchCount, 0);
    va_list args;

    va_start(args, pFormat);

    do
    {
        //格式化输出字符串
        int nSize = _vsnprintf_s(&strResult[0], nCchCount, _TRUNCATE, pFormat, args);
        if (-1 != nSize)
        {
            HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
            ::WriteConsoleA(console, strResult.c_str(), nSize, NULL, NULL);
            break;
        }

        //缓冲大小超限终止
        if (nCchCount >= FORMAT_COUNT_MAX)
        {
            break;
        }

        //重新分配缓冲
        nCchCount *= 2;
        strResult.resize(nCchCount);

    } while (true);

    va_end(args);
}

static void ConsoleOutputW(LPCWSTR pFormat, ...)
{
    size_t nCchCount = MAX_PATH;
    std::wstring strResult(nCchCount, 0);
    va_list args;

    va_start(args, pFormat);

    do
    {
        //格式化输出字符串
        int nSize = _vsnwprintf_s(&strResult[0], nCchCount, _TRUNCATE, pFormat, args);
        if (-1 != nSize)
        {
            HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
            ::WriteConsoleW(console, strResult.c_str(), nSize, NULL, NULL);
            break;
        }

        //缓冲大小超限终止
        if (nCchCount >= FORMAT_COUNT_MAX)
        {
            break;
        }

        //重新分配缓冲
        nCchCount *= 2;
        strResult.resize(nCchCount);

    } while (true);

    va_end(args);
}

static void ConsoleOutput(LPCTSTR pFormat, ...)
{
    size_t nCchCount = MAX_PATH;
    _tstring strResult(nCchCount, 0);
    va_list args;

    va_start(args, pFormat);

    do
    {
        //格式化输出字符串
        int nSize = _vsntprintf_s(&strResult[0], nCchCount, _TRUNCATE, pFormat, args);
        if (-1 != nSize)
        {
            HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
            ::WriteConsole(console, strResult.c_str(), nSize, NULL, NULL);
            break;
        }

        //缓冲大小超限终止
        if (nCchCount >= FORMAT_COUNT_MAX)
        {
            break;
        }

        //重新分配缓冲
        nCchCount *= 2;
        strResult.resize(nCchCount);

    } while (true);

    va_end(args);
}

CPEHelper::CPEHelper()
    :
    m_hFile(INVALID_HANDLE_VALUE),
    m_hModule(NULL)
{
    _Clear();
}

CPEHelper::~CPEHelper()
{
    Close();
}

bool CPEHelper::LoadFile(
    const _tstring& strPath,
    bool fCheckSum/* = false*/,
    bool fByModule/* = false*/
)
{
    bool bResult = false;
    Close();
    _Clear();

    if (fByModule)
    {
        bResult = _LoadByModule(strPath, fCheckSum);
    }
    else
    {
        bResult = _LoadByFile(strPath, fCheckSum);
    }

    if (bResult)
    {
        _LoadAllInformation();
    }
    else
    {
        Close();
    }

    return bResult;
}

bool CPEHelper::_LoadByFile(
    const _tstring& strPath,
    bool fCheckSum/* = false*/
)
{
    bool bResult = false;

    // 读取文件方式解析
    m_hFile = ::CreateFile(
        strPath.c_str(),
        GENERIC_READ,
        FILE_SHARE_READ,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );

    if (INVALID_HANDLE_VALUE == m_hFile)
    {
        return false;
    }

    do
    {
        DWORD HeaderSum = 0;
        DWORD CheckSum = 0;

        // 计算校验和
        if (fCheckSum && (CHECKSUM_SUCCESS != ::MapFileAndCheckSum(strPath.c_str(), &HeaderSum, &CheckSum)))
        {
            break;
        }

        // 文件头中存在校验和才进行检查
        if (0 != HeaderSum && HeaderSum != CheckSum)
        {
            break;
        }

        // 读取 DOS 头 64字节
        IMAGE_DOS_HEADER dosHeader = { 0 };
        DWORD dwBytesRead = 0;
        if (!_ReadFile(&dosHeader, sizeof(dosHeader), &dwBytesRead, 0, FILE_BEGIN, true) || IMAGE_DOS_SIGNATURE != dosHeader.e_magic)
        {
            break;
        }

        // 读取 NT头(32位版本) 248字节
        IMAGE_NT_HEADERS32 ntHeader32 = { 0 };
        if (!_ReadFile(&ntHeader32, sizeof(ntHeader32), &dwBytesRead, dosHeader.e_lfanew, FILE_BEGIN, true))
        {
            break;
        }

        // 读取 NT头(64位版本) 264字节
        IMAGE_NT_HEADERS64 ntHeader64 = { 0 };
        if (!_ReadFile(&ntHeader64, sizeof(ntHeader64), &dwBytesRead, dosHeader.e_lfanew, FILE_BEGIN, true))
        {
            break;
        }

        // 检查 NT头 签名
        if (IMAGE_NT_SIGNATURE != ntHeader32.Signature)
        {
            break;
        }

        // 检查 是否为 32位程序可选头
        if (IMAGE_NT_OPTIONAL_HDR32_MAGIC == ntHeader32.OptionalHeader.Magic)
        {
            m_OptionalHeaderMagic = IMAGE_NT_OPTIONAL_HDR32_MAGIC;
            m_NtHeader32 = ntHeader32;
            memcpy(&m_DataDirectory, &m_NtHeader32.OptionalHeader.DataDirectory, sizeof(m_DataDirectory));
        }
        // 检查 是否为 64位程序可选头
        else if (IMAGE_NT_OPTIONAL_HDR64_MAGIC == ntHeader64.OptionalHeader.Magic)
        {
            m_OptionalHeaderMagic = IMAGE_NT_OPTIONAL_HDR64_MAGIC;
            m_NtHeader64 = ntHeader64;
            memcpy(&m_DataDirectory, &m_NtHeader64.OptionalHeader.DataDirectory, sizeof(m_DataDirectory));
        }
        // ROM可选头
        else if (IMAGE_ROM_OPTIONAL_HDR_MAGIC == ntHeader32.OptionalHeader.Magic)
        {
            m_OptionalHeaderMagic = IMAGE_ROM_OPTIONAL_HDR_MAGIC;
        }
        else
        {
            break;
        }

        // 保存Dos头
        m_DosHeader = dosHeader;

        // 节标头偏移
        DWORD dwSectionOffset = (DWORD)(dosHeader.e_lfanew + FIELD_OFFSET(IMAGE_NT_HEADERS64, OptionalHeader) + ntHeader32.FileHeader.SizeOfOptionalHeader);
        WORD wNumberOfSections = ntHeader32.FileHeader.NumberOfSections;
        PIMAGE_SECTION_HEADER pSectionHeader = (PIMAGE_SECTION_HEADER)::HeapAlloc(::GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(IMAGE_SECTION_HEADER) * wNumberOfSections);
        if (nullptr == pSectionHeader)
        {
            break;
        }

        //保存标头信息
        do
        {
            if (!_ReadFile(pSectionHeader, sizeof(IMAGE_SECTION_HEADER) * wNumberOfSections, &dwBytesRead, dwSectionOffset, FILE_BEGIN, true))
            {
                break;
            }

            for (int i = 0; i < wNumberOfSections; i++)
            {
                m_SectionHeader.push_back(pSectionHeader[i]);
            }

        } while (false);

        // 释放节标头缓冲
        if (pSectionHeader)
        {
            ::HeapFree(::GetProcessHeap(), 0, pSectionHeader);
        }

        bResult = true;

    } while (false);

    return bResult;
}

bool CPEHelper::_LoadByModule(
    const _tstring& strPath,
    bool fCheckSum/* = false*/
)
{
    bool bResult = false;

    // 资源模块加载方式解析
    DWORD dwFlags = LOAD_LIBRARY_AS_DATAFILE;

    // 先尝试 数据文件 模式加载
    m_hModule = ::LoadLibraryEx(strPath.c_str(), 0, dwFlags);
    if (NULL == m_hModule)
    {
        // 加载失败则尝试 数据文件 + 映像资源 模式加载 
        dwFlags |= LOAD_LIBRARY_AS_IMAGE_RESOURCE;
        m_hModule = ::LoadLibraryEx(strPath.c_str(), 0, dwFlags);
    }

    if (NULL == m_hModule)
    {
        return false;
    }

    do
    {
        DWORD HeaderSum = 0;
        DWORD CheckSum = 0;

        // 计算校验和
        if (fCheckSum && (CHECKSUM_SUCCESS != ::MapFileAndCheckSum(strPath.c_str(), &HeaderSum, &CheckSum)))
        {
            break;
        }

        // 文件头中存在校验和才进行检查
        if (0 != HeaderSum && HeaderSum != CheckSum)
        {
            break;
        }

        LPBYTE pHeader = (BYTE*)m_hModule;
        m_pImageData = (BYTE*)((ULONG_PTR)pHeader & ~((ULONG_PTR)0x03));
        PIMAGE_DOS_HEADER pDosHeader = (PIMAGE_DOS_HEADER)m_pImageData;

        // 读取 DOS 头 64字节
        if (IMAGE_DOS_SIGNATURE != pDosHeader->e_magic)
        {
            break;
        }

        PIMAGE_NT_HEADERS pNtHeader = (IMAGE_NT_HEADERS*)((BYTE*)(pDosHeader)+(DWORD)(pDosHeader->e_lfanew));

        // 检查 NT头 签名
        if (IMAGE_NT_SIGNATURE != pNtHeader->Signature)
        {
            break;
        }

        // 检查 是否为 32位程序可选头
        if (IMAGE_NT_OPTIONAL_HDR32_MAGIC == pNtHeader->OptionalHeader.Magic)
        {
            m_NtHeader32 = *((PIMAGE_NT_HEADERS32)pNtHeader);
            memcpy(&m_DataDirectory, &m_NtHeader32.OptionalHeader.DataDirectory, sizeof(m_DataDirectory));
        }
        // 检查 是否为 64位程序可选头
        else if (IMAGE_NT_OPTIONAL_HDR64_MAGIC == pNtHeader->OptionalHeader.Magic)
        {
            m_OptionalHeaderMagic = IMAGE_NT_OPTIONAL_HDR64_MAGIC;
            m_NtHeader64 = *((PIMAGE_NT_HEADERS64)pNtHeader);
            memcpy(&m_DataDirectory, &m_NtHeader64.OptionalHeader.DataDirectory, sizeof(m_DataDirectory));
        }
        // ROM可选头
        else if (IMAGE_ROM_OPTIONAL_HDR_MAGIC == pNtHeader->OptionalHeader.Magic)
        {
            m_OptionalHeaderMagic = IMAGE_ROM_OPTIONAL_HDR_MAGIC;
        }
        else
        {
            break;
        }

        m_OptionalHeaderMagic = pNtHeader->OptionalHeader.Magic;

        // 保存Dos头
        m_DosHeader = *pDosHeader;

        // 节标头偏移
        DWORD dwSectionOffset = (DWORD)(m_DosHeader.e_lfanew + FIELD_OFFSET(IMAGE_NT_HEADERS64, OptionalHeader) + pNtHeader->FileHeader.SizeOfOptionalHeader);
        WORD wNumberOfSections = pNtHeader->FileHeader.NumberOfSections;
        PIMAGE_SECTION_HEADER pSectionHeader = (PIMAGE_SECTION_HEADER)(m_pImageData + dwSectionOffset);

        //保存标头信息
        do
        {
            for (int i = 0; i < wNumberOfSections; i++)
            {
                m_SectionHeader.push_back(pSectionHeader[i]);
            }

        } while (false);

        bResult = true;

    } while (false);

    return bResult;
}

void CPEHelper::Close()
{
    if (INVALID_HANDLE_VALUE != m_hFile)
    {
        ::CloseHandle(m_hFile);
        m_hFile = INVALID_HANDLE_VALUE;
    }

    if (NULL != m_hModule)
    {
        ::FreeLibrary(m_hModule);
        m_hModule = NULL;
    }
}

const std::map<_tstring, std::vector<IMPORT_FUNCTION_INFO>>& CPEHelper::GetImportTable() const
{
    return m_ImportTable;
}

const std::map<_tstring, std::vector<EXPORT_FUNCTION_INFO>>& CPEHelper::GetExportTable() const
{
    return m_ExportTable;
}

const RESOURCE_INFO& CPEHelper::GetResourceInfo() const
{
    return m_ResourceInfo;
}

void CPEHelper::_Clear()
{
    memset(&m_DosHeader, 0, sizeof(m_DosHeader));
    memset(&m_NtHeader32, 0, sizeof(m_NtHeader32));
    memset(&m_NtHeader64, 0, sizeof(m_NtHeader64));
    memset(&m_DataDirectory, 0, sizeof(m_DataDirectory));

    m_ResourceDirectoryEntrys.clear();
    m_SectionHeader.clear();
    m_ImportTable.clear();
    m_ExportTable.clear();
    m_ResourceInfo.clear();

    m_OptionalHeaderMagic = 0;
    m_pImageData = NULL;
}

void CPEHelper::_PrintfByte(LPVOID lpData, size_t size)
{
    if (NULL == lpData)
    {
        return;
    }

    for (size_t i = 0; i < size; i++)
    {
        if (i != size - 1)
        {
            ConsoleOutput(_T("%02X "), ((LPBYTE)lpData)[i]);
        }
        else
        {
            ConsoleOutput(_T("%02X"), ((LPBYTE)lpData)[i]);
        }
    }
}

bool CPEHelper::_ReadFile(
    LPVOID lpBuffer,
    DWORD dwSize,
    LPDWORD lpBytesRead/* = nullptr*/,
    LONGLONG llPos/* = 0*/,
    DWORD dwFlag/* = FILE_CURRENT*/,
    bool fCheckResdSize/* = false*/
)
{
    LARGE_INTEGER  liDistanceToMove = { 0 };
    DWORD dwBytesRead = 0;
    bool bResult = FALSE;

    if (INVALID_HANDLE_VALUE == m_hFile)
    {
        return false;
    }

    liDistanceToMove.QuadPart = llPos;
    ::SetFilePointerEx(m_hFile, liDistanceToMove, NULL, dwFlag);
    bResult = ::ReadFile(m_hFile, lpBuffer, dwSize, &dwBytesRead, NULL);

    if (nullptr != lpBytesRead)
    {
        *lpBytesRead = dwBytesRead;
    }

    //设置了读取大小检查, 则当实际读取数据量与指定读取数据量相同才认为读取成功
    if (fCheckResdSize)
    {
        bResult = (dwBytesRead == dwSize);
    }

    return bResult;
}

LONGLONG CPEHelper::_GetSectionDataOffset(
    const PIMAGE_SECTION_HEADER lpSectionHeader,
    ULONGLONG ullVirtualAddr
)
{
    ULONGLONG VirtualAddrBegin = lpSectionHeader->VirtualAddress;
    ULONGLONG VirtualAddrEnd = VirtualAddrBegin + lpSectionHeader->SizeOfRawData;
    ULONGLONG ullOffset = ullVirtualAddr;
    if (ullOffset < VirtualAddrBegin || ullOffset >= VirtualAddrEnd)
    {
        return -1;
    }

    ullOffset -= VirtualAddrBegin;

    return ullOffset;
}

LPVOID CPEHelper::_GetSectionDataAddr(
    LPCVOID lpBase,
    const PIMAGE_SECTION_HEADER pSectionHeader,
    ULONGLONG ullVirtualAddr
)
{
    ULONGLONG VirtualAddrBegin = pSectionHeader->VirtualAddress;
    ULONGLONG VirtualAddrEnd = VirtualAddrBegin + pSectionHeader->SizeOfRawData;
    ULONGLONG ullOffset = VirtualAddrBegin + ullVirtualAddr - pSectionHeader->VirtualAddress;
    if (ullOffset < VirtualAddrBegin || ullOffset >= VirtualAddrEnd)
    {
        return NULL;
    }

    ullOffset -= VirtualAddrBegin;

    return (LPBYTE)lpBase + ullOffset;
}

bool CPEHelper::_GetSectionHeader(
    ULONGLONG ullVirtualAddr,
    PIMAGE_SECTION_HEADER pSectinHeader
)
{
    for (const auto& item : m_SectionHeader)
    {
        ULONGLONG VirtualAddrBegin = item.VirtualAddress;
        ULONGLONG VirtualAddrEnd = item.VirtualAddress + item.SizeOfRawData;

        if ((ullVirtualAddr >= VirtualAddrBegin) && (ullVirtualAddr < VirtualAddrEnd))
        {
            if (pSectinHeader)
            {
                *pSectinHeader = item;
            }

            return true;
        }
    }

    return false;
}

inline LPBYTE CPEHelper::_GetAlignAddr(
    LPCVOID lpBase,
    LPCVOID lpAddr,
    DWORD dwAlign
)
{
    DWORD_PTR dwOffset = 0;
    if (dwAlign)
    {
        dwOffset = (DWORD_PTR)lpBase & (dwAlign - 1);
    }

    DWORD_PTR dwPadding = ((DWORD_PTR)lpAddr - dwOffset) % dwAlign;
    return dwPadding ? (LPBYTE)lpAddr + (dwAlign - dwPadding) : (LPBYTE)lpAddr;
}

bool CPEHelper::_LoadAllInformation()
{
    // 加载导入表
    auto _FunImportTable = [this]() {
        IMAGE_SECTION_HEADER sectionHeaderImportTable = { 0 };
        LPBYTE lpSectionImportTable = NULL;

        if (_GetDirectorySection(IMAGE_DIRECTORY_ENTRY_IMPORT, &lpSectionImportTable, &sectionHeaderImportTable))
        {
            _LoadImportTable(lpSectionImportTable, m_DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress, &sectionHeaderImportTable);
            if (INVALID_HANDLE_VALUE != m_hFile)
            {
                ::HeapFree(::GetProcessHeap(), 0, lpSectionImportTable);
            }
        }
        };

    // 加载导出表
    auto _FunLoadExportTable = [this]() {
        IMAGE_SECTION_HEADER sectionHeaderExportTable = { 0 };
        LPBYTE lpSectionExportTable = NULL;

        if (_GetDirectorySection(IMAGE_DIRECTORY_ENTRY_EXPORT, &lpSectionExportTable, &sectionHeaderExportTable))
        {
            _LoadExportTable(lpSectionExportTable, m_DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress, &sectionHeaderExportTable);
            if (INVALID_HANDLE_VALUE != m_hFile)
            {
                ::HeapFree(::GetProcessHeap(), 0, lpSectionExportTable);
            }
        }
        };

    // 加载资源表
    auto _FunLoadResourceTable = [this]() {
        IMAGE_SECTION_HEADER sectionHeaderResourceTable = { 0 };
        LPBYTE lpSectionResourceTable = NULL;

        if (_GetDirectorySection(IMAGE_DIRECTORY_ENTRY_RESOURCE, &lpSectionResourceTable, &sectionHeaderResourceTable))
        {
            _LoadResourceTable(lpSectionResourceTable, m_DataDirectory[IMAGE_DIRECTORY_ENTRY_RESOURCE].VirtualAddress, &sectionHeaderResourceTable);
            if (INVALID_HANDLE_VALUE != m_hFile)
            {
                ::HeapFree(::GetProcessHeap(), 0, lpSectionResourceTable);
            }
        }
        };

    _FunImportTable();
    _FunLoadExportTable();
    _FunLoadResourceTable();
    
    return true;
}

bool CPEHelper::_GetDirectorySection(
    int nDirectortIndex,
    LPBYTE* lppSection,
    PIMAGE_SECTION_HEADER lpSectionHeader
)
{
    if (NULL == lppSection)
    {
        return false;
    }

    DWORD VirtualAddress = m_DataDirectory[nDirectortIndex].VirtualAddress;
    if (0 == VirtualAddress)
    {
        return false;
    }

    if (INVALID_HANDLE_VALUE != m_hFile)
    {
        LPBYTE lpSectionData = nullptr;
        bool fResult = false;

        do
        {
            // 获取虚拟地址所在节信息
            if (!_GetSectionHeader(VirtualAddress, lpSectionHeader))
            {
                break;
            }

            //分配节数据缓冲
            lpSectionData = (LPBYTE)::HeapAlloc(::GetProcessHeap(), HEAP_ZERO_MEMORY, lpSectionHeader->SizeOfRawData);
            if (NULL == lpSectionData)
            {
                break;
            }

            // 读取整个节数据
            DWORD dwReadBytes = 0;
            if (!_ReadFile(lpSectionData, lpSectionHeader->SizeOfRawData, &dwReadBytes, lpSectionHeader->PointerToRawData, FILE_BEGIN, true))
            {
                break;
            }

            fResult = true;

        } while (false);

        if (!fResult && lpSectionData)
        {
            ::HeapFree(::GetProcessHeap(), 0, lpSectionData);
        }

        if (fResult)
        {
            *lppSection = lpSectionData;
        }
    }

    if (NULL != m_hModule)
    {
        do
        {
            // 获取虚拟地址所在节信息
            if (!_GetSectionHeader(VirtualAddress, lpSectionHeader))
            {
                break;
            }

            if (LDR_IS_DATAFILE(m_hModule))
            {
                *lppSection = m_pImageData + lpSectionHeader->PointerToRawData;
            }
            else if (LDR_IS_IMAGEMAPPING(m_hModule))
            {
                *lppSection = m_pImageData + lpSectionHeader->VirtualAddress;
            }
            else
            {
                break;
            }

        } while (false);
    }

    return NULL != *lppSection;
}


bool CPEHelper::_LoadExportTable(
    LPCBYTE lpSectionData,
    ULONGLONG ullVirtualAddr,
    const PIMAGE_SECTION_HEADER lpSectionHeader
)
{
    bool fResult = false;

    if (!lpSectionData || !lpSectionHeader)
    {
        return false;
    }

    do
    {
        // 获取导出描述信息所在偏移
        ULONGLONG VirtualAddrBegin = m_DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress;
        ULONGLONG VirtualAddrEnd = VirtualAddrBegin + m_DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].Size;
        LONGLONG ullExportDescOffset = _GetSectionDataOffset(lpSectionHeader, ullVirtualAddr);
        if (-1 == ullExportDescOffset)
        {
            break;
        }

        // 获取导出目录信息位置
        PIMAGE_EXPORT_DIRECTORY pExportDirectory = (PIMAGE_EXPORT_DIRECTORY)((LPBYTE)lpSectionData + ullExportDescOffset);
        LPCSTR lpDllName = (LPCSTR)_GetSectionDataAddr(lpSectionData, lpSectionHeader, pExportDirectory->Name);
        if (NULL == lpDllName)
        {
            break;
        }

        // 遍历导出表
        LPDWORD pFuncAddr = (LPDWORD)_GetSectionDataAddr(lpSectionData, lpSectionHeader, pExportDirectory->AddressOfFunctions);
        LPDWORD pNameAddr = (LPDWORD)_GetSectionDataAddr(lpSectionData, lpSectionHeader, pExportDirectory->AddressOfNames);
        LPWORD pNameOrdinalsName = (LPWORD)_GetSectionDataAddr(lpSectionData, lpSectionHeader, pExportDirectory->AddressOfNameOrdinals);
        if ((NULL == pFuncAddr) || (NULL == pNameAddr) || (NULL == pNameOrdinalsName))
        {
            break;
        }

        std::vector<EXPORT_FUNCTION_INFO> FunList;

        //预先分配空间, 速度比每次 push_back 一个信息快
        FunList.resize(pExportDirectory->NumberOfFunctions);
        for (DWORD i = 0; i < pExportDirectory->NumberOfFunctions; i++)
        {
            FunList[i].Addr = pFuncAddr[i];
            FunList[i].Ordinal = pExportDirectory->Base + i;
        }

        for (DWORD i = 0; i < pExportDirectory->NumberOfNames; i++)
        {
            LPCSTR lpFnName = (LPCSTR)_GetSectionDataAddr(lpSectionData, lpSectionHeader, pNameAddr[i]);
            if (NULL == lpFnName)
            {
                break;
            }

            DWORD dwOrdinal = pNameOrdinalsName[i];
            FunList[dwOrdinal].Name = _AStrToTStr(lpFnName);

            //转发函数
            if ((FunList[dwOrdinal].Addr >= VirtualAddrBegin && FunList[dwOrdinal].Addr < VirtualAddrEnd))
            {
                LPCSTR lpForwarderName = (LPCSTR)_GetSectionDataAddr(lpSectionData, lpSectionHeader, FunList[dwOrdinal].Addr);
                if (NULL == lpForwarderName)
                {
                    break;
                }

                FunList[dwOrdinal].ForwarderName = _AStrToTStr(lpForwarderName);
            }
        }

        m_ExportTable.emplace(_AStrToTStr(lpDllName), FunList);

        fResult = true;

    } while (false);

    return fResult;
}

bool CPEHelper::_LoadImportTable(
    LPCBYTE lpSectionData,
    ULONGLONG ullVirtualAddr,
    const PIMAGE_SECTION_HEADER lpSectionHeader
)
{
    IMAGE_IMPORT_DESCRIPTOR emptyImportDesc = { 0 };
    bool fResult = false;

    if (!lpSectionData || !lpSectionHeader)
    {
        return false;
    }

    do
    {
        // 获取导入描述信息所在偏移
        LONGLONG ullImportDescOffset = _GetSectionDataOffset(lpSectionHeader, ullVirtualAddr);
        if (-1 == ullImportDescOffset)
        {
            break;
        }

        // 导入描述
        PIMAGE_IMPORT_DESCRIPTOR pImportDesc = (PIMAGE_IMPORT_DESCRIPTOR)(LPBYTE)(lpSectionData + ullImportDescOffset);

        // 32位导入表
        if (IMAGE_NT_OPTIONAL_HDR32_MAGIC == m_OptionalHeaderMagic)
        {
            // 遍历导入表
            for (DWORD i = 0; 0 != ::memcmp(pImportDesc + i, &emptyImportDesc, sizeof(emptyImportDesc)); i++)
            {
                LPCSTR lpDllName = (LPCSTR)_GetSectionDataAddr(lpSectionData, lpSectionHeader, pImportDesc[i].Name);
                if (NULL == lpDllName)
                {
                    break;
                }

                PIMAGE_THUNK_DATA32 pThunkData = (PIMAGE_THUNK_DATA32)_GetSectionDataAddr(lpSectionData, lpSectionHeader, pImportDesc[i].FirstThunk);
                if (NULL == pThunkData)
                {
                    break;
                }

                //先统计一下导入函数数量
                PIMAGE_THUNK_DATA32 pThunkDataTmp = pThunkData;
                size_t nThunkCount = 0;
                while (pThunkDataTmp->u1.AddressOfData)
                {
                    nThunkCount++;
                    pThunkDataTmp++;
                }

                std::vector<IMPORT_FUNCTION_INFO> FunList;
                //预先分配空间, 速度比每次 push_back 一个信息快
                FunList.resize(nThunkCount);

                size_t nIndex = 0;
                while (pThunkData->u1.AddressOfData)
                {
                    if (IMAGE_SNAP_BY_ORDINAL32(pThunkData->u1.AddressOfData))
                    {
                        FunList[nIndex].Hint = IMAGE_ORDINAL32(pThunkData->u1.AddressOfData);
                    }
                    else
                    {
                        PIMAGE_IMPORT_BY_NAME pImportName = (PIMAGE_IMPORT_BY_NAME)_GetSectionDataAddr(lpSectionData, lpSectionHeader, pThunkData->u1.AddressOfData);
                        if (NULL == pImportName)
                        {
                            break;
                        }

                        FunList[nIndex].Hint = pImportName->Hint;
                        FunList[nIndex].Name = _AStrToTStr((LPCSTR)&pImportName->Name);
                    }
                    nIndex++;
                    pThunkData++;
                }

                auto result = m_ImportTable.emplace(_AStrToTStr(lpDllName), FunList);
                if (!result.second)
                {
                    result.first->second.insert(result.first->second.end(), FunList.begin(), FunList.end());
                }
            }
        }

        // 64位导入表
        if (IMAGE_NT_OPTIONAL_HDR64_MAGIC == m_OptionalHeaderMagic)
        {
            // 遍历导入表
            for (DWORD i = 0; 0 != ::memcmp(pImportDesc + i, &emptyImportDesc, sizeof(emptyImportDesc)); i++)
            {
                LPCSTR lpDllName = (LPCSTR)_GetSectionDataAddr(lpSectionData, lpSectionHeader, pImportDesc[i].Name);
                if (NULL == lpDllName)
                {
                    break;
                }

                PIMAGE_THUNK_DATA64 pThunkData = (PIMAGE_THUNK_DATA64)_GetSectionDataAddr(lpSectionData, lpSectionHeader, pImportDesc[i].FirstThunk);
                if (NULL == pThunkData)
                {
                    break;
                }

                //先统计一下导入函数数量
                PIMAGE_THUNK_DATA64 pThunkDataTmp = pThunkData;
                size_t nThunkCount = 0;
                while (pThunkDataTmp->u1.AddressOfData)
                {
                    nThunkCount++;
                    pThunkDataTmp++;
                }

                std::vector<IMPORT_FUNCTION_INFO> FunList;
                //预先分配空间, 速度比每次 push_back 一个信息快
                FunList.resize(nThunkCount);

                size_t nIndex = 0;
                while (pThunkData->u1.AddressOfData)
                {
                    if (IMAGE_SNAP_BY_ORDINAL64(pThunkData->u1.AddressOfData))
                    {
                        FunList[nIndex].Hint = IMAGE_ORDINAL64(pThunkData->u1.AddressOfData);
                    }
                    else
                    {
                        PIMAGE_IMPORT_BY_NAME pImportName = (PIMAGE_IMPORT_BY_NAME)_GetSectionDataAddr(lpSectionData, lpSectionHeader, pThunkData->u1.AddressOfData);
                        if (NULL == pImportName)
                        {
                            break;
                        }

                        FunList[nIndex].Hint = pImportName->Hint;
                        FunList[nIndex].Name = _AStrToTStr((LPCSTR)&pImportName->Name);
                    }
                    nIndex++;
                    pThunkData++;
                }

                auto result = m_ImportTable.emplace(_AStrToTStr(lpDllName), FunList);
                if (!result.second)
                {
                    result.first->second.insert(result.first->second.end(), FunList.begin(), FunList.end());
                }
            }
        }

        fResult = true;

    } while (false);

    return fResult;
}

bool CPEHelper::_LoadResourceTable(
    LPCBYTE lpSectionData,
    ULONGLONG ullVirtualAddr,
    const PIMAGE_SECTION_HEADER lpSectionHeader
)
{
    bool fResult = false;

    UNREFERENCED_PARAMETER(ullVirtualAddr);

    if (!lpSectionData || !lpSectionHeader)
    {
        return false;
    }

    do
    {
        PIMAGE_RESOURCE_DIRECTORY pDirectoryRoot = (PIMAGE_RESOURCE_DIRECTORY)lpSectionData;
        DWORD dwTpyeCount = pDirectoryRoot->NumberOfNamedEntries + pDirectoryRoot->NumberOfIdEntries;
        m_ResourceInfo.ResourceTable.resize(dwTpyeCount);
        for (DWORD i = 0; i < dwTpyeCount; i++)
        {
            //资源类型目录子项
            PIMAGE_RESOURCE_DIRECTORY_ENTRY pEntryType = (PIMAGE_RESOURCE_DIRECTORY_ENTRY)(pDirectoryRoot + 1) + i;
            m_ResourceDirectoryEntrys.push_back(*pEntryType);

            RESOURCE_GROUP_INFO GroupInfo;

            // 资源类型ID为字符串
            if (pEntryType->NameIsString)
            {
                PIMAGE_RESOURCE_DIR_STRING_U pStrName = (PIMAGE_RESOURCE_DIR_STRING_U)((LPBYTE)pDirectoryRoot + pEntryType->NameOffset);
                GroupInfo.TypeName = _WStrToTStr(std::wstring(pStrName->NameString, pStrName->Length));
                GroupInfo.TypeID = 0;
            }
            else
            {
                // 预定义资源类型名字符串
                if (pEntryType->Id < _countof(g_ResourctTypeName))
                {
                    GroupInfo.TypeName = g_ResourctTypeName[pEntryType->Id];
                }

                GroupInfo.TypeID = pEntryType->Id;
            }

            // 资源类型ID子项
            PIMAGE_RESOURCE_DIRECTORY pDirectoryDataID = (PIMAGE_RESOURCE_DIRECTORY)((LPBYTE)pDirectoryRoot + pEntryType->OffsetToDirectory);
            DWORD dwCount = pDirectoryDataID->NumberOfNamedEntries + pDirectoryDataID->NumberOfIdEntries;
            for (DWORD j = 0; j < dwCount; j++)
            {
                RESOURCE_ITEM info;

                // 资源类型ID子项
                PIMAGE_RESOURCE_DIRECTORY_ENTRY pEntryDataID = (PIMAGE_RESOURCE_DIRECTORY_ENTRY)(pDirectoryDataID + 1) + j;
                info.ID = pEntryDataID->Id;

                //各种语言版本的资源数据
                PIMAGE_RESOURCE_DIRECTORY pDirectoryLanguage = (PIMAGE_RESOURCE_DIRECTORY)((LPBYTE)pDirectoryRoot + pEntryDataID->OffsetToDirectory);
                DWORD dwLanguageCount = pDirectoryLanguage->NumberOfNamedEntries + pDirectoryLanguage->NumberOfIdEntries;
                GroupInfo.Items.resize(dwLanguageCount);
                for (DWORD k = 0; k < dwLanguageCount; k++)
                {
                    //资源ID与数据偏移, 数据大小
                    PIMAGE_RESOURCE_DIRECTORY_ENTRY pEntryLanguage = (PIMAGE_RESOURCE_DIRECTORY_ENTRY)(pDirectoryLanguage + 1) + k;
                    PIMAGE_RESOURCE_DATA_ENTRY pDataEntry = (PIMAGE_RESOURCE_DATA_ENTRY)((LPBYTE)pDirectoryRoot + pEntryLanguage->OffsetToDirectory);

                    info.LangID = pEntryLanguage->Id;
                    info.OffsetToData = pDataEntry->OffsetToData;
                    info.Size = pDataEntry->Size;
                    info.CodePage = pDataEntry->CodePage;

                    GroupInfo.Items[k] = info;
                }
            }

            m_ResourceInfo.ResourceTable[i] = GroupInfo;
        }

        fResult = true;

        _LoadResourceInformation(lpSectionData, lpSectionHeader);

    } while (false);

    return fResult;
}

bool CPEHelper::_LoadResourceInformation(
    LPCBYTE lpSectionData,
    const PIMAGE_SECTION_HEADER lpSectionHeader
)
{
    // 解析各种资源类型
    for (const auto& item : m_ResourceInfo.ResourceTable)
    {
        if (MAKEINTRESOURCE(item.TypeID) == RT_STRING)
        {
            // 加载 字符串表 资源
            _LoadResourceStringTable(lpSectionData, lpSectionHeader, item);
        }
        else if (MAKEINTRESOURCE(item.TypeID) == RT_MANIFEST)
        {
            // 加载 Manifest 资源
            _LoadResourceManifest(lpSectionData, lpSectionHeader, item);
        }
        else if (MAKEINTRESOURCE(item.TypeID) == RT_VERSION)
        {
            // 加载 版本 资源
            _LoadResourceVersion(lpSectionData, lpSectionHeader, item);
        }
    }

    return true;
}

bool CPEHelper::_LoadResourceStringTable(
    LPCBYTE lpSection,
    const PIMAGE_SECTION_HEADER lpSectionHeader,
    const RESOURCE_GROUP_INFO& info
)
{
    for (const auto& str : info.Items)
    {
        // 数据范围检查
        if (str.OffsetToData < lpSectionHeader->VirtualAddress ||
            str.OffsetToData >= (lpSectionHeader->VirtualAddress + lpSectionHeader->SizeOfRawData))
        {
            continue;
        }

        std::vector<STRING_TEXT> strList;

        DWORD SectionOffset = str.OffsetToData - lpSectionHeader->VirtualAddress;
        LPWORD lpLengthBegin = (LPWORD)((LPBYTE)lpSection + SectionOffset);
        LPWORD lpLengthEnd = (LPWORD)((LPBYTE)lpLengthBegin + str.Size);
        DWORD wIdBegin = (str.ID - 1) * 16;
        DWORD wIdEnd = wIdBegin + 16;
        LPWORD lpStrLength = lpLengthBegin;
        for (DWORD i = wIdBegin; i < wIdEnd; i++)
        {
            // 数据范围检查
            if (lpStrLength >= lpLengthEnd || 0 == *lpStrLength)
            {
                break;
            }

            STRING_TEXT strText;
            LPCWSTR lpStrAddr = (LPCWSTR)lpStrLength + 1;
            if ((LPBYTE)lpStrAddr < ((LPBYTE)lpLengthEnd))
            {
                strText.StrText = _WStrToTStr(std::wstring(lpStrAddr, *lpStrLength));
                strText.ID = i;
                strList.push_back(strText);
            }

            lpStrLength = (LPWORD)lpStrAddr + *lpStrLength;
        }

        auto itFind = m_ResourceInfo.StringTable.find(str.LangID);
        if (m_ResourceInfo.StringTable.end() == itFind)
        {
            m_ResourceInfo.StringTable.emplace(str.LangID, strList);
        }
        else
        {
            itFind->second.insert(itFind->second.end(), strList.begin(), strList.end());
        }
    }

    return true;
}

bool CPEHelper::_LoadResourceManifest(
    LPCBYTE lpSection,
    const PIMAGE_SECTION_HEADER lpSectionHeader,
    const RESOURCE_GROUP_INFO& info
)
{
    for (const auto& str : info.Items)
    {
        // 数据范围检查
        if (str.OffsetToData < lpSectionHeader->VirtualAddress ||
            str.OffsetToData >= (lpSectionHeader->VirtualAddress + lpSectionHeader->SizeOfRawData))
        {
            continue;
        }

        DWORD SectionOffset = str.OffsetToData - lpSectionHeader->VirtualAddress;
        LPCSTR lpStr = (LPCSTR)((LPBYTE)lpSection + SectionOffset);
        m_ResourceInfo.Manifest = _U8StrToTStr(std::string(lpStr, str.Size));
    }

    return true;
}

bool CPEHelper::_LoadResourceVersion(
    LPCBYTE lpSection,
    const PIMAGE_SECTION_HEADER lpSectionHeader,
    const RESOURCE_GROUP_INFO& info
)
{
    for (const auto& str : info.Items)
    {
        // 数据范围检查
        if (str.OffsetToData < lpSectionHeader->VirtualAddress ||
            str.OffsetToData >= (lpSectionHeader->VirtualAddress + lpSectionHeader->SizeOfRawData))
        {
            continue;
        }

        DWORD SectionOffset = str.OffsetToData - lpSectionHeader->VirtualAddress;
        PVS_VERSIONINFO lpVersion = (PVS_VERSIONINFO)((LPBYTE)lpSection + SectionOffset);

        // 存在 VS_FIXEDFILEINFO 信息
        if (0 != lpVersion->wValueLength)
        {
            VS_FIXEDFILEINFO* pFixedFileInfo = (VS_FIXEDFILEINFO*)((LPBYTE)lpVersion + sizeof(VS_VERSIONINFO));
            pFixedFileInfo = (VS_FIXEDFILEINFO*)_GetAlignAddr(lpVersion, (LPVOID)pFixedFileInfo, sizeof(DWORD));
            m_ResourceInfo.VersionInfo.FixedFileInfo = *pFixedFileInfo;
        }

        {
            VS_FIXEDFILEINFO FixedFileInfo = { 0 };
            LPBYTE lpVerBeg = (LPBYTE)lpVersion;

            // 存在 VS_FIXEDFILEINFO 信息
            if (0 != lpVersion->wValueLength)
            {
                VS_FIXEDFILEINFO* pFixedFileInfo = (VS_FIXEDFILEINFO*)((LPBYTE)lpVersion + sizeof(VS_VERSIONINFO));
                pFixedFileInfo = (VS_FIXEDFILEINFO*)_GetAlignAddr(lpVersion, pFixedFileInfo, sizeof(DWORD));
                FixedFileInfo = *pFixedFileInfo;
                lpVerBeg = (LPBYTE)lpVerBeg + sizeof(VS_VERSIONINFO) + lpVersion->wValueLength;
            }

            lpVerBeg = _GetAlignAddr(lpVersion, lpVerBeg, sizeof(DWORD));
            LPBYTE lpVerEnd = (LPBYTE)lpVersion + lpVersion->wLength;

            while (lpVerBeg < lpVerEnd)
            {
                StringFileInfo* pStringFileInfo = (StringFileInfo*)_GetAlignAddr(lpVersion, lpVerBeg, sizeof(DWORD));
                if (0 == pStringFileInfo->wLength)
                {
                    break;
                }

                // 解析 "StringFileInfo" 块
                if (0 == _wcsnicmp(L"StringFileInfo", pStringFileInfo->szKey, 14))
                {
                    LPBYTE lpStringFileInfoBeg = (LPBYTE)pStringFileInfo;
                    LPBYTE lpStringFileInfoEnd = (LPBYTE)pStringFileInfo + pStringFileInfo->wLength;
                    lpStringFileInfoBeg = (LPBYTE)(pStringFileInfo->Children);
                    while (lpStringFileInfoBeg < lpStringFileInfoEnd)
                    {
                        STRING_FILE_INFO strFileInfo;
                        StringTable* pStringTable = (StringTable*)lpStringFileInfoBeg;
                        if (0 == pStringTable->wLength)
                        {
                            break;
                        }

                        strFileInfo.StringCode = _WStrToTStr(std::wstring(pStringTable->szKey, _countof(pStringTable->szKey)));

                        LPBYTE lpStringBeg = (LPBYTE)pStringTable->Children;
                        LPBYTE lpStringEnd = (LPBYTE)pStringTable + pStringTable->wLength;
                        while (lpStringBeg < lpStringEnd)
                        {
                            const String* pString = (String*)lpStringBeg;
                            if (0 == pString->wLength)
                            {
                                break;
                            }

                            //值名
                            std::wstring strKey = pString->szKey;

                            STRING_FILE_ITEM item;

                            //值数据
                            LPCWSTR lpStrValue = pString->szKey + strKey.size() + 1;

                            //值数据位置
                            lpStrValue = (LPCWSTR)_GetAlignAddr(lpVersion, lpStrValue, sizeof(DWORD));
                            item.wType = pString->wType;

                            item.Key = _WStrToTStr(strKey);

                            //字符串类型
                            if (1 == pString->wType)
                            {
                                WORD wLength = pString->wValueLength;
                                if (wLength > 0 && L'\0' == lpStrValue[wLength - 1])
                                {
                                    wLength--;
                                }
                                std::wstring strValue = std::wstring(lpStrValue, wLength);
                                item.Value = _WStrToTStr(strValue);
                            }
                            //字节数据类型
                            else
                            {
                                DWORD dwValueSize = pString->wValueLength;
                                item.Value = _WStrToTStr(std::wstring(lpStrValue, dwValueSize));
                                item.Data.resize(dwValueSize);
                                for (DWORD i = 0; i < dwValueSize; i++)
                                {
                                    item.Data[i] = ((LPBYTE)lpStrValue)[i];
                                }
                            }

                            strFileInfo.StringInfos.push_back(item);

                            lpStringBeg = _GetAlignAddr(lpVersion, lpStringBeg + pString->wLength, sizeof(DWORD));
                        }

                        m_ResourceInfo.VersionInfo.StringFileInfo.push_back(strFileInfo);
                        lpStringFileInfoBeg = _GetAlignAddr(lpVersion, (LPBYTE)pStringTable + pStringTable->wLength, sizeof(DWORD));
                    }
                }
                // 解析 "VarFileInfo" 块
                else if (0 == _wcsnicmp(L"VarFileInfo", pStringFileInfo->szKey, 11))
                {
                    VarFileInfo* pVerFileInfo = (VarFileInfo*)_GetAlignAddr(lpVersion, pStringFileInfo, sizeof(DWORD));
                    Var* pVar = (Var*)pVerFileInfo->Children;
                    if (0 == _wcsnicmp(L"Translation", pVar->szKey, 11))
                    {
                        for (DWORD i = 0; i < pVar->wValueLength / sizeof(DWORD); i++)
                        {
                            TRANSLATION_INFO transInfo;
                            transInfo.CodePageID = pVar->Value[i].CodePageID;
                            transInfo.LanguageID = pVar->Value[i].LanguageID;
                            m_ResourceInfo.VersionInfo.TranslationList.push_back(transInfo);
                        }
                    }
                }

                lpVerBeg = _GetAlignAddr(lpVersion, lpVerBeg + pStringFileInfo->wLength, sizeof(DWORD));
            }
        }
    }

    return true;
}

void CPEHelper::PrintResourceTable(bool fShowDetail/* = true*/)
{
    ConsoleOutput(_T("资源类型数量: %4d\n"), (int)m_ResourceInfo.ResourceTable.size());
    int i = 0;
    for (const auto& ResourceInfo : m_ResourceInfo.ResourceTable)
    {
        if (0 != ResourceInfo.TypeID)
        {
            ConsoleOutput(_T("    %4d 类型ID: %4d 类型名: %s 数量: %d\n"), i++, ResourceInfo.TypeID, ResourceInfo.TypeName.c_str(), (int)ResourceInfo.Items.size());
        }
        else
        {
            ConsoleOutput(_T("    %4d 类型ID: \"%s\" 数量: %d\n"), i++, ResourceInfo.TypeName.c_str(), (int)ResourceInfo.Items.size());
        }

        if (!fShowDetail)
        {
            continue;
        }

        int j = 0;
        for (const auto& item : ResourceInfo.Items)
        {
            ConsoleOutput(_T("        %4d 资源ID: %4d  节数据偏移: %08X 大小: %d 语言: %d\n"), j++, item.ID, item.OffsetToData, item.Size, item.LangID);
        }
    }
}

void CPEHelper::PrintExportTable(bool fShowModule/* = true*/, bool fShowFunList/* = true*/)
{
    int nFunctionCount = 0;
    for (const auto& item : m_ExportTable)
    {
        nFunctionCount += (int)item.second.size();
    }

    ConsoleOutput(_T("Export module count: %d function count: %d\n"), (int)m_ExportTable.size(), nFunctionCount);
    int i = 0;
    for (const auto& item : m_ExportTable)
    {
        if (fShowModule)
        {
            ConsoleOutput(_T("    %4d %s function count: %4d\n"), i++, item.first.c_str(), (int)item.second.size());
        }

        if (!fShowFunList)
        {
            continue;
        }

        int j = 0;
        for (const auto& fun : item.second)
        {
            if (!fun.Name.empty())
            {
                if (fun.ForwarderName.empty())
                {
                    ConsoleOutput(_T("        %4d Ordinal: %4d(%04x) Addr: %08x %s \n"), j++, fun.Ordinal, fun.Ordinal, fun.Addr, fun.Name.c_str());
                }
                else
                {
                    ConsoleOutput(_T("        %4d Ordinal: %4d(%04x) Addr: %08x %s -> %s \n"), j++, fun.Ordinal, fun.Ordinal, fun.Addr, fun.Name.c_str(), fun.ForwarderName.c_str());
                }
            }
            else
            {
                ConsoleOutput(_T("        %4d Ordinal: %4d(%04x) Addr: %08x \n"), j++, fun.Ordinal, fun.Ordinal, fun.Addr);
            }
        }
    }
}

void CPEHelper::PrintImportTable(bool fShowModule/* = true*/, bool fShowFunList/* = true*/)
{
    int nFunctionCount = 0;
    for (const auto& item : m_ImportTable)
    {
        nFunctionCount += (int)item.second.size();
    }

    ConsoleOutput(_T("Import module count: %d function count: %d\n"), (int)m_ImportTable.size(), nFunctionCount);
    int i = 0;
    for (const auto& item : m_ImportTable)
    {
        if (fShowModule)
        {
            ConsoleOutput(_T("    %d %s count: %d\n"), i++, item.first.c_str(), (int)item.second.size());
        }

        if (!fShowFunList)
        {
            continue;
        }

        int j = 0;
        for (const auto& fun : item.second)
        {
            if (!fun.Name.empty())
            {
                ConsoleOutput(_T("        %4d %4d(%04x) %s\n"), j++, fun.Hint, fun.Hint, fun.Name.c_str());
            }
            else
            {
                ConsoleOutput(_T("        %4d %4d(%04x)\n"), j++, fun.Hint, fun.Hint);
            }
        }
    }
}

void CPEHelper::PrintVersion()
{
    ConsoleOutput(_T("Version:\n"));
    ConsoleOutput(_T("    StringFileInfo conut: %d\n"), (int)m_ResourceInfo.VersionInfo.StringFileInfo.size());
    for (const auto& item : m_ResourceInfo.VersionInfo.StringFileInfo)
    {
        ConsoleOutput(_T("        %s\n"), item.StringCode.c_str());

        VS_FIXEDFILEINFO& FixedFileInfo = m_ResourceInfo.VersionInfo.FixedFileInfo;
        ConsoleOutput(_T("            FileVersion: %d.%d.%d.%d\n"),
            HIWORD(FixedFileInfo.dwFileVersionMS),
            LOWORD(FixedFileInfo.dwFileVersionMS),
            HIWORD(FixedFileInfo.dwFileVersionLS),
            LOWORD(FixedFileInfo.dwFileVersionLS));

        for (const auto& info : item.StringInfos)
        {
            if (info.wType)
            {
                ConsoleOutput(_T("            %s: %s\n"), info.Key.c_str(), info.Value.c_str());
            }
            else
            {
                ConsoleOutput(_T("            %s: %s\n"), info.Key.c_str(), info.Value.c_str());
                //ConsoleOutput(_T("            %s: "), info.Key.c_str());
                //_PrintfByte((LPVOID)info.Data.data(), info.Data.size());
                //ConsoleOutput(_T("\n"));
            }
        }
    }

    ConsoleOutput(_T("    Translation conut: %d\n"), (int)m_ResourceInfo.VersionInfo.TranslationList.size());
    for (const auto& item : m_ResourceInfo.VersionInfo.TranslationList)
    {
        ConsoleOutput(_T("        0x%04X 0x%04X\n"), item.LanguageID, item.CodePageID);
    }
}

void CPEHelper::PrintStringTable()
{
    ConsoleOutput(_T("StringTable: count: %d\n"), (int)m_ResourceInfo.StringTable.size());
    for (const auto& item : m_ResourceInfo.StringTable)
    {
        ConsoleOutput(_T("    ID: %d, count: %d\n"), item.first, (int)item.second.size());

        for (const auto& info : item.second)
        {
            ConsoleOutput(_T("        %04d: %s\n"), info.ID, info.StrText.c_str());
        }
    }
}

void CPEHelper::PrintManifest()
{
    ConsoleOutput(_T("Manifest:\n"));
    ConsoleOutput(_T("%s\n"), m_ResourceInfo.Manifest.c_str());
}
