#pragma once

#include <wtypesbase.h>
#include <tchar.h>
#include <string>
#include <map>

#ifdef _UNICODE
using _tstring = std::wstring;
#else
using _tstring = std::string;
#endif

namespace CVersionUtils
{
    // https://learn.microsoft.com/zh-cn/windows/win32/api/verrsrc/ns-verrsrc-vs_fixedfileinfo
    // 版本号结构
    typedef union _VERSON_NUMBER {
        DWORD   Data;
        struct {
            WORD wLow;      // 版本号低16位
            WORD wHigh;     // 版本号高16位
        }Version;
        _VERSON_NUMBER() : Data(0) {}
    }VERSON_NUMBER;

    typedef struct _VS_VER_FIXEDFILEINFO {

        DWORD dwSignature;                  // 0xFEEF04BD
        VERSON_NUMBER dwStrucVersion;       // 此结构的二进制版本号。 此成员的高序字包含主版本号，低序字包含次要版本号。
        VERSON_NUMBER dwFileVersionMS;      // 文件二进制版本号中最重要的 32 位。 此成员与 dwFileVersionLS 一起使用，形成用于数字比较的 64 位值。
        VERSON_NUMBER dwFileVersionLS;      // 文件二进制版本号中最低有效 32 位。 此成员与 dwFileVersionMS 一起使用，形成用于数字比较的 64 位值。
        VERSON_NUMBER dwProductVersionMS;   // 分发此文件的产品的二进制版本号中最重要的 32 位。 此成员与 dwProductVersionLS 一起使用，形成用于数字比较的 64 位值。
        VERSON_NUMBER dwProductVersionLS;   // 分发此文件的产品的二进制版本号中最低有效 32 位。 此成员与 dwProductVersionMS 一起使用，形成用于数字比较的 64 位值。

        DWORD dwFileFlagsMask;              // 包含指定 dwFileFlags 中的有效位的位掩码。 仅当在创建文件时定义位时，位才有效。

        struct {
            DWORD fVS_FF_DEBUG : 1;             // 该文件包含调试信息，或者在启用调试功能的情况下进行编译。
            DWORD fVS_FF_PRERELEASE : 1;        // 该文件是开发版本，而不是商业发布的产品。
            DWORD fVS_FF_PRIVATEBUILD : 1;      // 文件不是使用标准发布过程生成的。 如果设置了此标志， StringFileInfo 结构应包含 PrivateBuild 条目。
            DWORD fVS_FF_PATCHED : 1;           // 该文件已修改，与同一版本号的原始发货文件不同。
            DWORD fVS_FF_INFOINFERRED : 1;      // 文件的版本结构是动态创建的;因此，此结构中的某些成员可能为空或不正确。 切勿在文件的 VS_VERSIONINFO 数据中设置此标志。
            DWORD fVS_FF_SPECIALBUILD : 1;      // 该文件由原始公司使用标准发布过程生成，但是相同版本号的正常文件的变体。 如果设置了此标志， StringFileInfo 结构应包含 SpecialBuild 条目。
            DWORD fVS_FF_Reserved : 26;         // 保留未使用
        }dwFileFlags;   // 包含指定文件的布尔属性的位掩码。

        union _FILE_OS {
            DWORD dwData;
            struct {
                enum eFileOSLo : WORD {
                    eVOS_LO_UNKNOWN = 0x0000L,  // 系统不知道设计该文件的操作系统
                    eVOS_LO_WINDOWS16 = 0x0001L,  // 该文件是为 16 位 Windows 设计的
                    eVOS_LO_PM16 = 0x0002L,  // 该文件是为 16 位 Presentation Manager 设计的
                    eVOS_LO_PM32 = 0x0003L,  // 该文件是为 32 位 Presentation Manager 设计的
                    eVOS_LO_WINDOWS32 = 0x0004L,  // 该文件专为 32 位 Windows 设计
                }Low;

                enum eFileOSHi : WORD {
                    eVOS_HI_UNKNOWN = 0x0000L,  // 系统不知道设计该文件的操作系统
                    eVOS_HI_DOS = 0x0001L,  // 该文件是针对 MS-DOS 设计的
                    eVOS_HI_OS216 = 0x0002L,  // 该文件是为 16 位 OS/2 设计的
                    eVOS_HI_OS232 = 0x0003L,  // 该文件是为 32 位 OS/2 设计的
                    eVOS_HI_NT = 0x0004L,  // 该文件是为 Windows NT 设计的
                }High;
            };

        }dwFileOS;      // 为其设计此文件的操作系统

        enum eFileType : DWORD {
            eVFT_UNKNOWN = 0x00000000L,  // 系统不知道文件类型
            eVFT_APP = 0x00000001L,  // 该文件包含一个应用程序
            eVFT_DLL = 0x00000002L,  // 该文件包含一个 DLL
            eVFT_DRV = 0x00000003L,  // 该文件包含设备驱动程序
            eVFT_FONT = 0x00000004L,  // 该文件包含字体
            eVFT_VXD = 0x00000005L,  // 该文件包含一个虚拟设备
            eVFT_STATIC_LIB = 0x00000007L   // 该文件包含一个静态链接库
        }dwFileType;    // 文件的常规类型

        union {
            // 如果 dwFileTypeVFT_FONT， 则 dwFileSubtype 可以是以下值之一。
            enum eFileSubtypeDrv : DWORD {
                eVFT2_DRV_UNKNOWN = 0x00000000L,  //系统未知驱动程序类型。
                eVFT2_DRV_PRINTER = 0x00000001L,  //文件包含打印机驱动程序。
                eVFT2_DRV_KEYBOARD = 0x00000002L,  //文件包含键盘驱动程序。
                eVFT2_DRV_LANGUAGE = 0x00000003L,  //文件包含语言驱动程序。
                eVFT2_DRV_DISPLAY = 0x00000004L,  //文件包含显示驱动程序。
                eVFT2_DRV_MOUSE = 0x00000005L,  //文件包含鼠标驱动程序。
                eVFT2_DRV_NETWORK = 0x00000006L,  //文件包含网络驱动程序。
                eVFT2_DRV_SYSTEM = 0x00000007L,  //文件包含系统驱动程序。
                eVFT2_DRV_INSTALLABLE = 0x00000008L,  //文件包含可安装的驱动程序。
                eVFT2_DRV_SOUND = 0x00000009L,  //该文件包含声音驱动程序。
                eVFT2_DRV_COMM = 0x0000000AL,  //文件包含通信驱动程序。
                eVFT2_DRV_VERSIONED_PRINTER = 0x0000000CL,  //文件包含版本控制打印机驱动程序。
            }dwFileTypeVFT_DRV;     // 驱动文件类型

            // 如果 dwFileTypeVFT_VXD， 则 dwFileSubtype 包含虚拟设备控制块中包含的虚拟设备标识符。
            enum eFileSubtypeFont : DWORD {
                eVFT2_FONT_UNKNOWN = 0x00000000L,  //系统未知字体类型。
                eVFT2_FONT_RASTER = 0x00000001L,  //文件包含光栅字体。
                eVFT2_FONT_TRUETYPE = 0x00000003L,  //文件包含 TrueType 字体。
                eVFT2_FONT_VECTOR = 0x00000002L,  //文件包含矢量字体。
            }dwFileTypeVFT_FONT;    // 字体文件类型

        }dwFileSubtype;     // 文件的子类型

        DWORD dwFileDateMS; // 文件的 64 位二进制创建日期和时间戳中最高有效 32 位。
        DWORD dwFileDateLS; // 文件的 64 位二进制创建日期和时间戳的最低有效 32 位。
    } VS_VER_FIXEDFILEINFO, * PVS_VER_FIXEDFILEINFO;

    // 语言与代码页 https://learn.microsoft.com/zh-cn/windows/win32/menurc/varfileinfo-block
    typedef struct _LANGANDCODEPAGE {
        LANGID wLangId;
        WORD wCodePage; // https://learn.microsoft.com/zh-cn/windows/win32/menurc/varfileinfo-block

        _LANGANDCODEPAGE();
        bool operator < (const _LANGANDCODEPAGE& r) const;
        bool operator == (const _LANGANDCODEPAGE& r) const;

    }LANGANDCODEPAGE, * PLANGANDCODEPAGE;

    // 版本号辅助类
    class CVersionNumber
    {
    public:
        CVersionNumber();
        CVersionNumber(const _tstring& strVer);
        CVersionNumber(WORD v1, WORD v2, WORD v3, WORD v4);
        CVersionNumber& operator = (const _tstring& strVer);
        _tstring GetString() const;
        bool IsEmpty() const;
        void Clear();
        bool operator == (const CVersionNumber& ref);
        bool operator != (const CVersionNumber& ref);
        bool operator < (const CVersionNumber& ref);
        bool operator <= (const CVersionNumber& ref);
        bool operator > (const CVersionNumber& ref);
        bool operator >= (const CVersionNumber& ref);

    private:
        int _Compare(const CVersionNumber& ref) const;

    private:
        WORD m_nVer[4];         //版本号
    };

    // PE文件信息
    typedef struct _VERSION_INFO
    {
        LANGANDCODEPAGE langAndCodePage;            // 语言代码页
        LANGID   wLangId;                           // 语言ID
        _tstring strLocaleName;                     // 区域名
        _tstring strLangName;                       // 语言名

        _tstring strComments;                       // 文件注释
        _tstring strInternalName;                   // 内部名称
        _tstring strProductName;                    // 产品名称
        _tstring strCompanyName;                    // 公司名称
        _tstring strLegalCopyright;                 // 法律版权
        _tstring strProductVersion;                 // 产品版本
        _tstring strFileDescription;                // 文件描述
        _tstring strLegalTrademarks;                // 合法商标
        _tstring strPrivateBuild;                   // 私有构建
        _tstring strFileVersion;                    // 文件版本
        _tstring strOriginalFilename;               // 原始文件名
        _tstring strSpecialBuild;                   // 特殊构建
        _tstring strFileVersionEx;                  // 文件版本(从 VS_FIXEDFILEINFO 中获取)
        _tstring strProductVersionEx;               // 产品版本(从 VS_FIXEDFILEINFO 中获取)
        CVersionNumber FileVerNumber;               // 文件版本号
        CVersionNumber ProductVerNumber;            // 产品版本号
        VS_VER_FIXEDFILEINFO vsFixedInfo;           // 固定文件信息
    }VERSION_INFO;

    // 语言信息
    typedef struct _LANG_INFO
    {
        LANGID   wLangId;                   // 语言ID
        _tstring strLocaleName;             // 区域名
        _tstring strLangName;               // 语言名

    }LANG_INFO;

    // 语言列表
    using LANG_LIST = std::map<LANGID, LANG_INFO>;

    // 文件版本信息列表
    using VERSION_LIST = std::map<_tstring, VERSION_INFO>;

    //
    // 获取语言信息
    // @param: dwLang               语言ID
    // @ret: LANG_INFO              语言信息
    LANG_INFO GetLanguageInfo(LANGID dwLang);

    //
    // 获取系统安装的语言列表
    // @ret: LANG_LIST              语言列表
    LANG_LIST GetLangList();

    //
    // @brief: 获取文件版本列表
    // @param: strFile              文件路径
    // @param: fLocalised           本地化
    // @ret: VERSION_LIST           文件版本信息列表
    VERSION_LIST GetFileVersionList(const _tstring& strFile, bool fMultilingual = true);

    //
    // @brief: 获取文件版本
    // @param: strFile              文件路径
    // @param: fLocalised           本地化
    // @ret: VERSION_INFO           文件版本信息
    VERSION_INFO GetFileVersion(const _tstring& strFile, bool fMultilingual = true);
}
