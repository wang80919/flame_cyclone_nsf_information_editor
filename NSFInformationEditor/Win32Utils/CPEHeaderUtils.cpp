#include "CPEHeaderUtils.h"

CPEHeader::CPEHeader()
{
    Clear();
}

CPEHeader::~CPEHeader()
{

}

void CPEHeader::Clear()
{
    memset(&m_DosHeader, 0, sizeof(m_DosHeader));
    memset(&m_NtHeaders32, 0, sizeof(m_NtHeaders32));
    memset(&m_NtHeaders64, 0, sizeof(m_NtHeaders64));
    memset(&m_RomHeaders, 0, sizeof(m_RomHeaders));
    m_NtHeadersMagic = 0;
}

bool CPEHeader::Load(const _tstring strFile)
{
    HMODULE hModule = NULL;
    bool fResult = false;
    LPVOID OldValue = NULL;
    BOOL isDisableWow64Fs = ::Wow64DisableWow64FsRedirection(&OldValue);

    Clear();

    do
    {
        hModule = ::LoadLibraryEx(strFile.c_str(), 0, LOAD_LIBRARY_AS_DATAFILE | LOAD_LIBRARY_AS_IMAGE_RESOURCE);
        if (NULL == hModule)
        {
            break;
        }

        LPBYTE pHeader = (BYTE*)hModule;
        BYTE* pImageData = (BYTE*)((ULONG_PTR)pHeader & ~((ULONG_PTR)0x03));
        PIMAGE_DOS_HEADER pDosHeader = (PIMAGE_DOS_HEADER)pImageData;
        if (pDosHeader->e_magic != IMAGE_DOS_SIGNATURE)
        {
            break;
        }

        m_DosHeader = *pDosHeader;

        PIMAGE_NT_HEADERS pNtHeader = (IMAGE_NT_HEADERS*)((BYTE*)(pDosHeader)+(DWORD)(pDosHeader->e_lfanew));
        if (IMAGE_NT_SIGNATURE != pNtHeader->Signature)
        {
            break;
        }

        // 检查 是否为 32位程序可选头
        if (IMAGE_NT_OPTIONAL_HDR32_MAGIC == pNtHeader->OptionalHeader.Magic)
        {
            m_NtHeaders32 = *((PIMAGE_NT_HEADERS32)pNtHeader);
        }
        // 检查 是否为 64位程序可选头
        else if (IMAGE_NT_OPTIONAL_HDR64_MAGIC == pNtHeader->OptionalHeader.Magic)
        {
            m_NtHeaders64 = *((PIMAGE_NT_HEADERS64)pNtHeader);
        }
        // ROM可选头
        else if (IMAGE_ROM_OPTIONAL_HDR_MAGIC == pNtHeader->OptionalHeader.Magic)
        {
            m_RomHeaders = *((PIMAGE_ROM_HEADERS)pNtHeader);
        }
        else
        {
            break;
        }

        m_NtHeadersMagic = pNtHeader->OptionalHeader.Magic;
        fResult = true;

    } while (false);

    if (isDisableWow64Fs)
    {
        ::Wow64RevertWow64FsRedirection(OldValue);
    }

    if (!fResult)
    {
        Clear();
    }

    if (NULL != hModule)
    {
        ::FreeLibrary(hModule);
    }

    return fResult;
}

bool CPEHeader::IsValidPEHeader() const
{
    if (IMAGE_NT_OPTIONAL_HDR32_MAGIC == m_NtHeadersMagic || 
        IMAGE_NT_OPTIONAL_HDR64_MAGIC == m_NtHeadersMagic)
    {
        return true;
    }

    return false;
}

WORD CPEHeader::GetMachine() const
{
    if (IMAGE_NT_OPTIONAL_HDR32_MAGIC == m_NtHeadersMagic)
    {
        return m_NtHeaders32.FileHeader.Machine;
    }

    if (IMAGE_NT_OPTIONAL_HDR64_MAGIC == m_NtHeadersMagic)
    {
        return m_NtHeaders64.FileHeader.Machine;
    }

    return 0;
}

WORD CPEHeader::GetCharacteristics() const
{
    if (IMAGE_NT_OPTIONAL_HDR32_MAGIC == m_NtHeadersMagic)
    {
        return m_NtHeaders32.FileHeader.Characteristics;
    }

    if (IMAGE_NT_OPTIONAL_HDR64_MAGIC == m_NtHeadersMagic)
    {
        return m_NtHeaders64.FileHeader.Characteristics;
    }

    return 0;
}

WORD CPEHeader::GetSubSystem() const
{
    if (IMAGE_NT_OPTIONAL_HDR32_MAGIC == m_NtHeadersMagic)
    {
        return m_NtHeaders32.OptionalHeader.Subsystem;
    }

    if (IMAGE_NT_OPTIONAL_HDR64_MAGIC == m_NtHeadersMagic)
    {
        return m_NtHeaders64.OptionalHeader.Subsystem;
    }

    return 0;
}

WORD CPEHeader::GetDllCharacteristics() const
{
    if (IMAGE_NT_OPTIONAL_HDR32_MAGIC == m_NtHeadersMagic)
    {
        return m_NtHeaders32.OptionalHeader.DllCharacteristics;
    }

    if (IMAGE_NT_OPTIONAL_HDR64_MAGIC == m_NtHeadersMagic)
    {
        return m_NtHeaders64.OptionalHeader.DllCharacteristics;
    }

    return 0;
}

DWORD CPEHeader::GetCheckSum() const
{
    if (IMAGE_NT_OPTIONAL_HDR32_MAGIC == m_NtHeadersMagic)
    {
        return m_NtHeaders32.OptionalHeader.CheckSum;
    }

    if (IMAGE_NT_OPTIONAL_HDR64_MAGIC == m_NtHeadersMagic)
    {
        return m_NtHeaders64.OptionalHeader.CheckSum;
    }

    return 0;
}

bool CPEHeader::IsX86() const
{
    return IMAGE_FILE_MACHINE_I386 == GetMachine();
}

bool CPEHeader::IsX64() const
{
    return IMAGE_FILE_MACHINE_AMD64 == GetMachine();
}

bool CPEHeader::IsARM32() const
{
    WORD wMachine = GetMachine();
    return wMachine >= IMAGE_FILE_MACHINE_ARM && wMachine <= IMAGE_FILE_MACHINE_ARMNT;
}

bool CPEHeader::IsARM64() const
{
    return IMAGE_FILE_MACHINE_ARM64 == GetMachine();
}

bool CPEHeader::IsIA64() const
{
    return IMAGE_FILE_MACHINE_IA64 == GetMachine();
}

bool CPEHeader::IsDllFile() const
{
    return IMAGE_FILE_DLL & GetCharacteristics();
}

bool CPEHeader::IsExecutable() const
{
    return IMAGE_FILE_EXECUTABLE_IMAGE & GetCharacteristics();
}

bool CPEHeader::IsSystemFile() const
{
    return IMAGE_FILE_SYSTEM & GetCharacteristics();
}

bool CPEHeader::IsLargeAddressAware() const
{
    return IMAGE_FILE_LARGE_ADDRESS_AWARE & GetCharacteristics();
}

bool CPEHeader::IsWindowsGUI() const
{
    return IMAGE_SUBSYSTEM_WINDOWS_GUI == GetSubSystem();
}

bool CPEHeader::IsWindowsCUI() const
{
    return IMAGE_SUBSYSTEM_WINDOWS_CUI == GetSubSystem();
}

bool CPEHeader::IsWindowsBootApplication() const
{
    return IMAGE_SUBSYSTEM_WINDOWS_BOOT_APPLICATION == GetSubSystem();
}