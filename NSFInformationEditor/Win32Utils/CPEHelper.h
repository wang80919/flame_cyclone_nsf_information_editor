#pragma once

//
// @brief: PE文件解析助手类
// @copyright: Copyright 2024 FlameCyclone
// @license: 
// @birth: Created by Visual Studio 2022 on 2024-02-04
// @version: V1.0.0
// @revision: last revised by FlameCyclone on 2024-02-04
//

#include <stdint.h>
#include <wtypesbase.h>
#include <windows.h>
#include <string>
#include <vector>
#include <map>

#ifdef _UNICODE
using _tstring = std::wstring;
#else
using _tstring = std::string;
#endif

#pragma pack(push)
#pragma pack(1)

// https://learn.microsoft.com/zh-cn/windows/win32/menurc/newheader
typedef struct {
    WORD Reserved;				//保留;必须为零
    WORD ResType;				//资源类型 1: RES_ICON	2: RES_CURSOR
    WORD ResCount;				//资源组中的图标或游标组件数
} ICON_GROUP_HEADER, * LPICON_GROUP_HEADER;

// https://learn.microsoft.com/zh-cn/windows/win32/menurc/resdir
// https://learn.microsoft.com/zh-cn/windows/win32/menurc/iconresdir
typedef struct {
    BYTE Width;					//图标的宽度（以像素为单位）。 可接受的值为 16、32 和 64
    BYTE Height;				//图标的高度（以像素为单位）。 可接受的值为 16、32 和 64
    BYTE ColorCount;			//图标中的颜色数。 可接受的值为 2、8 和 16。
    BYTE reserved;				//保留;必须设置为与图标文件标头中保留字段的值相同的值
    WORD Planes;				//图标或光标位图中的颜色平面数
    WORD BitCount;				//图标或光标位图中每像素的位数
    DWORD BytesInRes;			//资源的大小（以字节为单位）
    WORD IconId;				//具有唯一序号标识符的图标或光标
} ICON_ENTRY, * LPICON_ENTRY;

typedef struct {
    ICON_GROUP_HEADER Header;	//图标组头部
    ICON_ENTRY	IconEntry[1];	//单个图标信息
}ICON_GROUP_DIR, * LPICON_GROUP_DIR;

// https://learn.microsoft.com/zh-cn/windows/win32/menurc/var-str
typedef struct {
    WORD  wLength;                  // Var 结构的长度（以字节为单位）
    WORD  wValueLength;             // Value 成员的长度（以字节为单位）
    WORD  wType;                    // 版本资源中的数据类型, 1: 资源包含文本数据 0: 版本资源包含二进制数据
    WCHAR szKey[12];                // Unicode 字符串 L“Translation”
    WORD  Padding;                  // 在 32 位边界上对齐 Value 成员所需的任意或零个 WORD

    struct {
        WORD LanguageID;            //低序字: Microsoft 语言标识符
        WORD CodePageID;            //高序字: IBM 代码页码
    }Value[1];
} Var;

// https://learn.microsoft.com/zh-cn/windows/win32/menurc/varfileinfo
typedef struct {
    WORD  wLength;                  // 整个 VarFileInfo 块（包括 Children 成员指示的所有结构）的长度（以字节为单位）
    WORD  wValueLength;             // 此成员始终等于零
    WORD  wType;                    // 版本资源中的数据类型, 1: 资源包含文本数据 0: 版本资源包含二进制数据
    WCHAR szKey[12];                // Unicode 字符串 L“VarFileInfo”
    WORD  Padding;                  // 在 32 位边界上对齐 Children 成员所需的任意或零个 WORD
    Var   Children[1];              // 通常包含应用程序或 DLL 支持的语言列表
} VarFileInfo;

// https://learn.microsoft.com/zh-cn/windows/win32/menurc/string-str
typedef struct {
    WORD  wLength;                  // 此 字符串 结构的长度（以字节为单位）
    WORD  wValueLength;             // Value 成员的大小（以字为单位）
    WORD  wType;                    // 版本资源中的数据类型, 1: 资源包含文本数据 0: 版本资源包含二进制数据
    WCHAR szKey[1];                 // 任意长度的 Unicode 字符串
    WORD  Padding;                  // 在 32 位边界上对齐 Value 成员所需的任意或零个 WORD
    WORD  Value[1];                 // 以零结尾的字符串
} String;

// https://learn.microsoft.com/zh-cn/windows/win32/menurc/stringtable
typedef struct {
    WORD   wLength;                 // 此 StringTable 结构的长度（以字节为单位），包括 Children 成员指示的所有结构
    WORD   wValueLength;            // 此成员始终等于零
    WORD   wType;                   // 版本资源中的数据类型, 1: 资源包含文本数据 0: 版本资源包含二进制数据
    WCHAR  szKey[8];                // 存储为 Unicode 字符串的 8 位十六进制数
    WORD   Padding;                 // 在 32 位边界上对齐 Children 成员所需的任意或零个 WORD
    String Children[1];             // 一个或多个 String 结构的数组
} StringTable;

// https://learn.microsoft.com/zh-cn/windows/win32/menurc/stringfileinfo
typedef struct {
    WORD        wLength;            // 整个 StringFileInfo 块的长度（以字节为单位）
    WORD        wValueLength;       // 此成员始终等于零
    WORD        wType;              // 版本资源中的数据类型, 1: 资源包含文本数据 0: 版本资源包含二进制数据
    WCHAR       szKey[14];          // Unicode 字符串 L“StringFileInfo”
    WORD        Padding;            // 在 32 位边界上对齐 Children 成员所需的任意或零个 WORD
    StringTable Children[1];        // 一个或多个 StringTable 结构的数组
} StringFileInfo;

// VS 文件版本信息
// https://learn.microsoft.com/zh-cn/windows/win32/menurc/vs-versioninfo
typedef struct {
    WORD             wLength;       // VS_VERSIONINFO 结构的长度（以字节为单位）,此长度不包括在 32 位边界上对齐任何后续版本资源数据的填充
    WORD             wValueLength;  // Value 成员的长度（以字节为单位）
    WORD             wType;         // 版本资源中的数据类型, 1: 资源包含文本数据 0: 版本资源包含二进制数据
    WCHAR            szKey[15];     // Unicode 字符串 L“VS_VERSION_INFO”
    WORD             Padding1;      // 在 32 位边界上对齐 Children 成员所需的任意或零个 WORD
    //VS_FIXEDFILEINFO Value
    //WORD             Padding2
    //WORD             Children
} VS_VERSIONINFO, *PVS_VERSIONINFO;
#pragma pack(pop)

typedef struct _RESOURCE_ITEM
{
    WORD ID;                    //资源ID
    WORD LangID;                //语言ID
    DWORD OffsetToData;         //数据偏移
    DWORD Size;                 //数据大小
    DWORD CodePage;             //代码页

    _RESOURCE_ITEM()
    {
        memset(this, 0, sizeof(this));
    }
}RESOURCE_ITEM;

typedef struct _STRING_TEXT
{
    _tstring StrText;           //文本内容
    WORD ID;                    //字符串ID

    _STRING_TEXT()
        :
        ID(0)
    {

    }
}STRING_TEXT;

typedef struct _RESOURCE_GROUP_INFO
{
    _tstring TypeName;                  //类型名
    WORD TypeID;                        //类型ID
    std::vector<RESOURCE_ITEM> Items;   //资源信息

    _RESOURCE_GROUP_INFO()
        :
        TypeID(0)
    {

    }
}RESOURCE_GROUP_INFO;

typedef struct _STRING_INFO
{
    _tstring Text;
    WORD  wType;

    _STRING_INFO()
        :
        wType(0)
    {

    }
}STRING_INFO;

typedef struct _STRING_FILE_ITEM
{
    _tstring Key;                   // 值名字符串
    _tstring Value;                 // 值数据字符串
    WORD wType;                     // 版本资源中的数据类型, 1: 资源包含文本数据 0: 版本资源包含二进制数据
    std::vector<uint8_t> Data;      // 二进制数据

    _STRING_FILE_ITEM()
        :
        wType(0)
    {

    }
}STRING_FILE_ITEM;

// 版本资源中的数据, 包含可为特定语言和代码页显示的版本信息
typedef struct
{
    _tstring StringCode;
    std::vector<STRING_FILE_ITEM> StringInfos;
}STRING_FILE_INFO;

typedef struct _TRANSLATION_INFO
{
    WORD LanguageID;
    WORD CodePageID;

    _TRANSLATION_INFO()
        :
        LanguageID(0),
        CodePageID(0)
    {

    }
}TRANSLATION_INFO;

// 版本信息
typedef struct
{
    VS_FIXEDFILEINFO FixedFileInfo;
    std::vector<STRING_FILE_INFO> StringFileInfo;   // 版本资源中的数据的组织
    std::vector<TRANSLATION_INFO> TranslationList;  // 应用程序或 DLL 支持的语言列表
}VERSION_INFO;

typedef struct _IMPORT_FUNCTION_INFO
{
    _tstring Name;          //函数名
    WORD Hint;              //索引, 可能为0

    _IMPORT_FUNCTION_INFO()
        :
        Hint(0)
    {

    }
}IMPORT_FUNCTION_INFO;

typedef struct _EXPORT_FUNCTION_INFO
{
    _tstring Name;          //函数名
    _tstring ForwarderName; //转发函数名
    DWORD Addr;             //函数相对偏移地址
    DWORD Ordinal;          //函数顺序

    _EXPORT_FUNCTION_INFO()
        :
        Addr(0),
        Ordinal(0)
    {

    }
}EXPORT_FUNCTION_INFO;

typedef struct _RESOURCE_INFO
{
    std::vector<RESOURCE_GROUP_INFO> ResourceTable;             //资源表信息
    std::map<WORD, std::vector<STRING_TEXT>> StringTable;       //资源字符串表
    VERSION_INFO VersionInfo;                                   //资源版本信息
    _tstring Manifest;                                          //资源清单

    void clear()
    {
        ResourceTable.clear();
        StringTable.clear();
        VersionInfo.StringFileInfo.clear();
        VersionInfo.TranslationList.clear();
        memset(&VersionInfo.FixedFileInfo, 0, sizeof(VersionInfo.FixedFileInfo));
    }

}RESOURCE_INFO;

class CPEHelper
{
public:
    CPEHelper();
    ~CPEHelper();

    //
    // @brief: 加载PE文件信息
    // @param: strPath          文件路径
    // @param: fCheckSum        检查映像文件校验和, 校验失败此函数将直接返回false
    // @param: fByModule        是否通过模块加载
    // @ret: bool               操作成功与否
    bool LoadFile(
        const _tstring& strPath, 
        bool fCheckSum = false, 
        bool fByModule = false
    );

    //
    // @brief: 关闭文件占用
    // @ret: bool               操作成功与否
    void Close();

    //
    // @brief: 获取导入表信息
    // @ret: std::map<_tstring, std::vector<IMPORT_FUNCTION_INFO>>  导入表信息
    const std::map<_tstring, std::vector<IMPORT_FUNCTION_INFO>>& GetImportTable() const;

    //
    // @brief: 获取导出表信息
    // @ret: std::map<_tstring, std::vector<IMPORT_FUNCTION_INFO>>  导出表信息
    const std::map<_tstring, std::vector<EXPORT_FUNCTION_INFO>>& GetExportTable() const;

    //
    // @brief: 获取资源表信息
    // @ret: RESOURCE_INFO                                          资源信息
    const RESOURCE_INFO& GetResourceInfo() const;

    //
    // @brief: 打印导出表信息
    // @param: fShowModule      显示模块信息
    // @param: fShowFunList     显示函数信息
    // @ret: void
    void PrintExportTable(bool fShowModule = true, bool fShowFunList = true);

    //
    // @brief: 打印导入表信息
    // @param: fShowModule      显示模块信息
    // @param: fShowFunList     显示函数信息
    // @ret: void
    void PrintImportTable(bool fShowModule = true, bool fShowFunList = true);

    //
    // @brief: 打印资源表信息
    // @param: fShowDetail      显示详情
    // @ret: void
    void PrintResourceTable(bool fShowDetail = true);

    //
    // @brief: 打印版本信息
    // @ret: void
    void PrintVersion();

    //
    // @brief: 打印字符串表
    // @ret: void
    void PrintStringTable();

    //
    // @brief: 打印清单信息
    // @ret: void
    void PrintManifest();

private:

    //
    // @brief: 加载PE文件信息
    // @param: strPath          文件路径
    // @param: fCheckSum        检查映像文件校验和, 校验失败此函数将直接返回false
    // @ret: bool               操作成功与否
    bool _LoadByFile(
        const _tstring& strPath,
        bool fCheckSum = false
    );

    //
    // @brief: 加载PE文件信息
    // @param: strPath          文件路径
    // @param: fCheckSum        检查映像文件校验和, 校验失败此函数将直接返回false
    // @ret: bool               操作成功与否
    bool _LoadByModule(
        const _tstring& strPath,
        bool fCheckSum = false
    );

    //
    // @brief: 清空数据
    // @ret: void
    void _Clear();

    //
    // @brief: 输出字节信息
    // @ret: void
    void _PrintfByte(LPVOID lpData, size_t size);

    //
    // @brief: 读取文件
    // @param: lpBuffer         读取数据存放缓冲
    // @param: dwSize           缓冲大小(字节)
    // @param: lpBytesRead      实际读取大小(字节)
    // @param: llPos            读取文件数据位置
    // @param: dwFlag           设置位置标志 FILE_BEGIN: 文件开头 FILE_CURRENT: 当前文件指针 FILE_END: 文件结束位置
    // @param: fCheckResdSize   是否检查读取长度
    // @ret: bool               操作成功与否
    bool _ReadFile(
        LPVOID lpBuffer, 
        DWORD dwSize, 
        LPDWORD lpBytesRead = nullptr,
        LONGLONG llPos = 0,
        DWORD dwFlag = FILE_CURRENT, 
        bool fCheckResdSize = false
    );

    //
    // @brief: 获取虚拟地址在节数据中的相对偏移
    // @param: pSectionHeader   节信息头
    // @param: ullVirtualAddr   虚拟地址
    // @ret: LONGLONG           虚拟地址在节数据中的相对偏移
    LONGLONG _GetSectionDataOffset(
        const PIMAGE_SECTION_HEADER lpSectionHeader,
        ULONGLONG ullVirtualAddr
    );

    //
    // @brief: 获取虚拟地址在节数据内存中的位置
    // @param: lpBase           节数据内存位置
    // @param: pSectionHeader   节信息头
    // @param: ullVirtualAddr   虚拟地址
    // @ret: LONGLONG           虚拟地址在节数据内存中的位置
    LPVOID _GetSectionDataAddr(
        LPCVOID lpBase,
        const PIMAGE_SECTION_HEADER pSectionHeader,
        ULONGLONG ullVirtualAddr
    );

    //
    // @brief: 获取虚拟地址在节数据内存中的位置
    // @param: ullVirtualAddr   虚拟地址
    // @param: pSectinHeader    节信息头输出缓冲
    // @ret: bool               操作成功与否
    bool _GetSectionHeader(
        ULONGLONG ullVirtualAddr, 
        PIMAGE_SECTION_HEADER pSectinHeader
    );

    //
    // @brief: 获取地址对齐后的地址
    // @param: lpBase           起始地址
    // @param: lpAddr           地址
    // @param: dwAlign          对齐粒度
    // @ret: LPBYTE             对齐后指针
    LPBYTE _GetAlignAddr(
        LPCVOID lpBase,
        LPCVOID lpAddr,
        DWORD dwAlign = sizeof(DWORD)
    );

    //
    // @brief: 加载所有信息
    // @ret: bool               操作成功与否
    bool _LoadAllInformation();

    //
    // @brief: 获取目录节数据与节标头信息
    // @param: nDirectortIndex  目录索引
    // @param: lppSection       节数据指针输出指针
    // @param: lpSectionHeader  节标头数据输出指针
    // @ret: bool               操作成功与否
    bool _GetDirectorySection(
        int nDirectortIndex, 
        LPBYTE* lppSection, 
        PIMAGE_SECTION_HEADER lpSectionHeader
    );

    //
    // @brief: 加载导出表
    // @param: lpSection        节数据
    // @param: ullVirtualAddr   目录虚拟地址
    // @param: lpSectionHeader  节标头数据输出指针
    // @ret: bool               操作成功与否
    bool _LoadExportTable(
        LPCBYTE lpSection, 
        ULONGLONG ullVirtualAddr, 
        const PIMAGE_SECTION_HEADER lpSectionHeader
    );

    //
    // @brief: 加载导入表
    // @param: lpSection        节数据
    // @param: ullVirtualAddr   目录虚拟地址
    // @param: lpSectionHeader  节标头
    // @ret: bool               操作成功与否
    bool _LoadImportTable(
        LPCBYTE lpSection, 
        ULONGLONG ullVirtualAddr, 
        const PIMAGE_SECTION_HEADER lpSectionHeader
    );

    //
    // @brief: 加载资源表
    // @param: lpSection        节数据
    // @param: ullVirtualAddr   目录虚拟地址
    // @param: lpSectionHeader  节标头
    // @ret: bool               操作成功与否
    bool _LoadResourceTable(
        LPCBYTE lpSection, 
        ULONGLONG ullVirtualAddr, 
        const PIMAGE_SECTION_HEADER lpSectionHeader
    );

    //
    // @brief: 加载资源信息
    // @param: lpSection        节数据内存指针
    // @ret: bool               操作成功与否
    bool _LoadResourceInformation(
        LPCBYTE lpSectionData, 
        const PIMAGE_SECTION_HEADER lpSectionHeader
    );

    //
    // @brief: 加载资源字符串表
    // @param: lpSection        节数据内存指针
    // @ret: info               资源组信息
    // @ret: bool               操作成功与否
    bool _LoadResourceStringTable(
        LPCBYTE lpSection,
        const PIMAGE_SECTION_HEADER lpSectionHeader,
        const RESOURCE_GROUP_INFO& info
    );

    //
    // @brief: 加载资源Manifest
    // @param: lpSection        节数据内存指针
    // @ret: info               资源组信息
    // @ret: bool               操作成功与否
    bool _LoadResourceManifest(
        LPCBYTE lpSection,
        const PIMAGE_SECTION_HEADER lpSectionHeader,
        const RESOURCE_GROUP_INFO& info
    );

    //
    // @brief: 加载资源版本信息
    // @param: lpSection        节数据内存指针
    // @ret: info               资源组信息
    // @ret: bool               操作成功与否
    bool _LoadResourceVersion(
        LPCBYTE lpSection,
        const PIMAGE_SECTION_HEADER lpSectionHeader,
        const RESOURCE_GROUP_INFO& info
    );

private:

    HANDLE m_hFile;                                                                 //PE文件句柄
    HMODULE m_hModule;                                                              //PE文件模块句柄
    LPBYTE m_pImageData;                                                            //数据地址

    //基础PE信息
    IMAGE_DOS_HEADER m_DosHeader;                                                   //Dos头
    IMAGE_NT_HEADERS32 m_NtHeader32;                                                //NT头(32位)
    IMAGE_NT_HEADERS64 m_NtHeader64;                                                //NT头(64位)
    IMAGE_DATA_DIRECTORY m_DataDirectory[IMAGE_NUMBEROF_DIRECTORY_ENTRIES];         //数据目录
    std::vector<IMAGE_SECTION_HEADER> m_SectionHeader;                              //节头信息
    std::vector<IMAGE_RESOURCE_DIRECTORY_ENTRY> m_ResourceDirectoryEntrys;          //资源目录信息

    //数据目录解析相关成员
    std::map<_tstring, std::vector<IMPORT_FUNCTION_INFO>> m_ImportTable;            //导入表信息
    std::map<_tstring, std::vector<EXPORT_FUNCTION_INFO>> m_ExportTable;            //导出表信息
    RESOURCE_INFO m_ResourceInfo;                                                   //资源信息
    WORD m_OptionalHeaderMagic;                                                     //可选头魔数
};
