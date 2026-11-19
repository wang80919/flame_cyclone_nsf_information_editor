#pragma once
#include <wtypesbase.h>
#include <string>

#ifdef _UNICODE
using _tstring = std::wstring;
#else
using _tstring = std::string;
#endif

class CPEHeader
{
public:

    CPEHeader();
    ~CPEHeader();
    CPEHeader& operator = (const CPEHeader& r) = delete;

    // 从文件加载PE文件头信息
    bool Load(const _tstring strFile);

    // 是否为有效PE信息
    bool IsValidPEHeader() const;

    // 获取计算机的体系结构类型
    WORD GetMachine() const;

    // 获取映像文件特征
    WORD GetCharacteristics() const;

    // 获取运行映像所需的子系统
    WORD GetSubSystem() const;

    // 获取Dll特征
    WORD GetDllCharacteristics() const;

    // 获取校验和
    DWORD GetCheckSum() const;

    // 计算机的体系结构类型检查
    bool IsX86() const;                       //是否为 X86 平台
    bool IsX64() const;                       //是否为 X64 平台
    bool IsARM32() const;                     //是否为 ARM32 平台
    bool IsARM64() const;                     //是否为 ARM64 平台
    bool IsIA64() const;                      //是否为 英特尔的安腾(Intel Itanium) 平台

    // 文件特征检查
    bool IsDllFile() const;                   //映像是否为一个 DLL 文件
    bool IsExecutable() const;                //映像是否为一个 可执行文件
    bool IsSystemFile() const;                //映像是否为一个 系统文件
    bool IsLargeAddressAware() const;         //映像是否 可以处理大于 2 GB 的地址

    // 子系统检查
    bool IsWindowsGUI() const;                //是否运行于windows 图形用户界面 (GUI) 子系统
    bool IsWindowsCUI() const;                //是否运行于Windows 字符模式用户界面 (CUI) 子系统
    bool IsWindowsBootApplication() const;    //是否为启动应用程序

private:

    void Clear();

private:

    //基础PE信息
    IMAGE_DOS_HEADER    m_DosHeader;            //Dos头
    IMAGE_NT_HEADERS32  m_NtHeaders32;          //NT头(32位)
    IMAGE_NT_HEADERS64  m_NtHeaders64;          //NT头(64位)
    IMAGE_ROM_HEADERS   m_RomHeaders;           //ROM头
    WORD                m_NtHeadersMagic;       //NT头魔数
};
