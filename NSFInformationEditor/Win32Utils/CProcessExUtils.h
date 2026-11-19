#pragma once

#include <wtypesbase.h>
#include <tchar.h>
#include <vector>
#include <map>
#include <set>
#include <string>
#include <psapi.h>
#include <thread>

#pragma warning(disable: 4100)

#ifdef _UNICODE
using _tstring = std::wstring;
#else
using _tstring = std::string;
#endif

namespace CProcessExUtils
{
    // https://learn.microsoft.com/zh-cn/windows/win32/api/verrsrc/ns-verrsrc-vs_fixedfileinfo
    // 版本号结构
    typedef union _VERSON_NUMBER {
        DWORD   Data;
        struct {
            WORD wLow;      // 版本号低16位
            WORD wHigh;     // 版本号高16位
        }Version;
        _VERSON_NUMBER(DWORD val = 0) : Data(val) {}
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
                    eVOS_LO_UNKNOWN = 0x0000L,      // 系统不知道设计该文件的操作系统
                    eVOS_LO_WINDOWS16 = 0x0001L,    // 该文件是为 16 位 Windows 设计的
                    eVOS_LO_PM16 = 0x0002L,         // 该文件是为 16 位 Presentation Manager 设计的
                    eVOS_LO_PM32 = 0x0003L,         // 该文件是为 32 位 Presentation Manager 设计的
                    eVOS_LO_WINDOWS32 = 0x0004L,    // 该文件专为 32 位 Windows 设计
                }Low;

                enum eFileOSHi : WORD {
                    eVOS_HI_UNKNOWN = 0x0000L,  // 系统不知道设计该文件的操作系统
                    eVOS_HI_DOS = 0x0001L,      // 该文件是针对 MS-DOS 设计的
                    eVOS_HI_OS216 = 0x0002L,    // 该文件是为 16 位 OS/2 设计的
                    eVOS_HI_OS232 = 0x0003L,    // 该文件是为 32 位 OS/2 设计的
                    eVOS_HI_NT = 0x0004L,       // 该文件是为 Windows NT 设计的
                }High;
            };

        }dwFileOS;      // 为其设计此文件的操作系统

        enum eFileType : DWORD {
            eVFT_UNKNOWN = 0x00000000L,     // 系统不知道文件类型
            eVFT_APP = 0x00000001L,         // 该文件包含一个应用程序
            eVFT_DLL = 0x00000002L,         // 该文件包含一个 DLL
            eVFT_DRV = 0x00000003L,         // 该文件包含设备驱动程序
            eVFT_FONT = 0x00000004L,        // 该文件包含字体
            eVFT_VXD = 0x00000005L,         // 该文件包含一个虚拟设备
            eVFT_STATIC_LIB = 0x00000007L   // 该文件包含一个静态链接库
        }dwFileType;    // 文件的常规类型

        union {
            // 如果 dwFileTypeVFT_FONT， 则 dwFileSubtype 可以是以下值之一。
            enum eFileSubtypeDrv : DWORD {
                eVFT2_DRV_UNKNOWN = 0x00000000L,            //系统未知驱动程序类型。
                eVFT2_DRV_PRINTER = 0x00000001L,            //文件包含打印机驱动程序。
                eVFT2_DRV_KEYBOARD = 0x00000002L,           //文件包含键盘驱动程序。
                eVFT2_DRV_LANGUAGE = 0x00000003L,           //文件包含语言驱动程序。
                eVFT2_DRV_DISPLAY = 0x00000004L,            //文件包含显示驱动程序。
                eVFT2_DRV_MOUSE = 0x00000005L,              //文件包含鼠标驱动程序。
                eVFT2_DRV_NETWORK = 0x00000006L,            //文件包含网络驱动程序。
                eVFT2_DRV_SYSTEM = 0x00000007L,             //文件包含系统驱动程序。
                eVFT2_DRV_INSTALLABLE = 0x00000008L,        //文件包含可安装的驱动程序。
                eVFT2_DRV_SOUND = 0x00000009L,              //该文件包含声音驱动程序。
                eVFT2_DRV_COMM = 0x0000000AL,               //文件包含通信驱动程序。
                eVFT2_DRV_VERSIONED_PRINTER = 0x0000000CL,  //文件包含版本控制打印机驱动程序。
            }dwFileTypeVFT_DRV;     // 驱动文件类型

            // 如果 dwFileTypeVFT_VXD， 则 dwFileSubtype 包含虚拟设备控制块中包含的虚拟设备标识符。
            enum eFileSubtypeFont : DWORD {
                eVFT2_FONT_UNKNOWN = 0x00000000L,   //系统未知字体类型。
                eVFT2_FONT_RASTER = 0x00000001L,    //文件包含光栅字体。
                eVFT2_FONT_TRUETYPE = 0x00000003L,  //文件包含 TrueType 字体。
                eVFT2_FONT_VECTOR = 0x00000002L,    //文件包含矢量字体。
            }dwFileTypeVFT_FONT;    // 字体文件类型

        }dwFileSubtype;     // 文件的子类型

        DWORD dwFileDateMS; // 文件的 64 位二进制创建日期和时间戳中最高有效 32 位。
        DWORD dwFileDateLS; // 文件的 64 位二进制创建日期和时间戳的最低有效 32 位。

        _VS_VER_FIXEDFILEINFO()
        {
            memset(this, 0, sizeof(*this));
        }

    } VS_VER_FIXEDFILEINFO, * PVS_VER_FIXEDFILEINFO;

    // 语言与代码页
    typedef struct _LANGANDCODEPAGE {
        LANGID wLangId;
        WORD wCodePage; // https://learn.microsoft.com/zh-cn/windows/win32/menurc/varfileinfo-block

        _LANGANDCODEPAGE();
        bool operator < (const _LANGANDCODEPAGE& r) const;
        bool operator == (const _LANGANDCODEPAGE& r) const;

    }LANGANDCODEPAGE, * PLANGANDCODEPAGE;

    // PE文件头辅助类
    class CPEHeader
    {
    public:
        CPEHeader();
        CPEHeader(const CPEHeader& r) noexcept;
        CPEHeader(CPEHeader&& r) noexcept;
        CPEHeader& operator = (const CPEHeader& r) noexcept;
        CPEHeader& operator = (CPEHeader&& r) noexcept;
        ~CPEHeader();

        bool LoadFromFile(const _tstring& strFile);
        bool LoadFromModule(HMODULE hModule);
        void CopyFrom(const CPEHeader& r);
        void Release();

        WORD GetMachine();
        DWORD GetSubsystemType();
        bool IsX86();
        bool IsX64();
        bool IsARM32();
        bool IsARM64();
        bool IsWindowsGUI();
        bool IsWindowsCUI();

    public:
        PIMAGE_DOS_HEADER       m_pDosHeader;            // Dos头
        PIMAGE_NT_HEADERS64     m_pNtHeaders64;          // NT头(64位)
        PIMAGE_NT_HEADERS32     m_pNtHeaders32;          // NT头(32位)
        WORD                    m_NtHeadersMagic;        // NT头魔数
    };

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
        VS_VER_FIXEDFILEINFO vsFixedInfo;           // 固定文件信息
        CVersionNumber FileVerNumber;               // 文件版本号
        CVersionNumber ProductVerNumber;            // 产品版本号
        LANGANDCODEPAGE langAndCodePage;            // 语言代码页
        LANGID   wLangId;                           // 语言ID
    }VERSION_INFO;

    // 进程 SID 信息
    typedef struct _PROC_SID_INFO
    {
        DWORD ProcessId;
        DWORD SessionId;
        _tstring ProcessName;
        _tstring DomainName;
        _tstring UserName;

        _PROC_SID_INFO() :SessionId(0), ProcessId(0) {}

    } PROC_SID_INFO, * PPROC_NAME_INFO;

    // 进程组信息
    typedef struct _TOKEN_GROUPS_INFO
    {
        _tstring    SID;
        _tstring    DomainName;
        _tstring    UserName;
        DWORD       Attributes;

        _TOKEN_GROUPS_INFO() : Attributes(0) {}

    } TOKEN_GROUPS_INFO, * PTOKEN_GROUPS_INFO;

    // 模块信息
    typedef struct _MODULE_INFO
    {
        _tstring    strModule;              // 模块名
        _tstring    strExePath;             // 模块路径
        DWORD       th32ProcessID;          // 要检查其模块的进程标识符
        DWORD       GlblcntUsage;           // 模块的负载计数，通常没有意义，通常等于0xFFFF
        LPVOID      modBaseAddr;            // 拥有进程上下文中模块的基址
        DWORD       modBaseSize;            // 模块的大小（以字节为单位）
        HMODULE     hModule;                // 拥有进程上下文中模块的句柄

        _MODULE_INFO();

    }MODULE_INFO;

    // 窗口信息
    typedef struct _WND_INFO
    {
        HWND hWnd;
        DWORD dwPid;
        DWORD dwTid;
        _tstring strClass;
        _tstring strText;

    }WND_INFO;

    // 窗口节点信息
    typedef struct _WND_NODE
    {
        HWND hWnd;                          // 窗口句柄
        DWORD dwPid;                        // 窗口进程ID
        DWORD dwTid;                        // 窗口线程ID
        _tstring strClass;                  // 窗口类名
        _tstring strText;                   // 窗口名
        _tstring strModule;                 // 窗口关联的模块名
        std::vector<_WND_NODE> NodeList;    // 子窗口列表
    }WND_NODE;

    // 语言信息
    typedef struct _LANG_INFO
    {
        LANGID   wLangId;                   // 语言ID
        _tstring strLocaleName;             // 区域名
        _tstring strLangName;               // 语言名

    }LANG_INFO;

    using VERSION_INFO_LIST = std::map<DWORD, VERSION_INFO>;
    using PROC_TOKEN_GROUPS = std::vector<TOKEN_GROUPS_INFO>;

    using PROC_NAME_LIST = std::map<DWORD, _tstring>;

    using PROC_SID_LIST = std::map<DWORD, PROC_SID_INFO>;

    using MODULE_INFO_LIST = std::map<_tstring, MODULE_INFO>;

    using WND_INFO_LIST = std::map<HWND, WND_INFO>;

    using WND_NODE_LIST = std::map<HWND, WND_NODE>;

    using LANG_LIST = std::map<LANGID, LANG_INFO>;

    typedef struct _SYS_PROCESS_INFO
    {
        _tstring    ImageName;                      // 进程的映像名称
        _tstring    FilePath;                       // 文件路径

        DOUBLE      CpuUsage;                       // CPU 使用率
        ULONG       NumberOfThreads;                // 线程数
        uint64_t    WorkingSetPrivateSize;          // 进程的私有工作集的大小 since VISTA
        ULONG       HardFaultCount;                 // 硬错误计数 since WIN7
        ULONG       NumberOfThreadsHighWatermark;   // 进程中线程数量的最高记录值 since WIN7
        uint64_t    CycleTime;                      // 周期时间 since WIN7
        uint64_t    CreateTime;                     // 创建时间
        uint64_t    UserTime;                       // 用户时间
        uint64_t    KernelTime;                     // 内核时间
        uint32_t    BasePriority;                   // 进程的基优先级，即在关联进程中创建的线程的起始优先级
        uint64_t    UniqueProcessId;                // 进程的唯一进程 ID
        uint64_t    UniqueParentProcessId;          // 父进程ID
        uint32_t    HandleCount;                    // 进程正在使用的句柄总数, 改用 GetProcessHandleCount 检索此信息
        uint32_t    SessionId;                      // 会话标识符
        uint64_t    UniqueProcessKey;               // since VISTA (requires SystemExtendedProcessInformation)
        size_t      PeakVirtualSize;                // 虚拟内存的峰值大小(字节)
        size_t      VirtualSize;                    // 虚拟内存的当前大小(字节)
        size_t      PageFaultCount;                 // 页错误计数
        size_t      PeakWorkingSetSize;             // 进程的工作集的峰值大小(KB)
        size_t      WorkingSetSize;                 // 进程的工作集大小(KB)
        size_t      QuotaPeakPagedPoolUsage;        // 分页池使用峰值用量
        size_t      QuotaPagedPoolUsage;            // 分页池使用当前用量
        size_t      QuotaPeakNonPagedPoolUsage;     // 非分页池使用峰值用量
        size_t      QuotaNonPagedPoolUsage;         // 非分页池使用当前用量
        size_t      PagefileUsage;                  // 页面文件存储的字节数
        size_t      PeakPagefileUsage;              // 页面文件存储的最大字节数
        size_t      PrivatePageCount;               // 进程分配的内存页数
        uint64_t    ReadOperationCount;             // 读操作计数
        uint64_t    WriteOperationCount;            // 写操作计数
        uint64_t    OtherOperationCount;            // 其他操作计数
        uint64_t    ReadTransferCount;              // 读取传输量计数
        uint64_t    WriteTransferCount;             // 写入传输量计数
        uint64_t    OtherTransferCount;             // 其他传输量计数

        DWORD       GdiCount;                       // GDI对象计数
        DWORD       UserCount;                      // 用户对象计数
        BOOL        fSuspend;                       // 挂起状态


        CPEHeader           PeHeader;               // PE头信息辅助类
        PROC_SID_INFO       SidInfo;                // SID 信息
        VERSION_INFO_LIST   Versions;               // 文件版本信息
        PROC_TOKEN_GROUPS   Groups;                 // 组信息
        MODULE_INFO_LIST    Modules;                // 模块信息
        VERSION_INFO        FileVersion;            // 版本信息

        _SYS_PROCESS_INFO()
            :
            CpuUsage(0.0f),
            NumberOfThreads(0),
            WorkingSetPrivateSize(0),
            HardFaultCount(0),
            NumberOfThreadsHighWatermark(0),
            CycleTime(0),
            CreateTime(0),
            UserTime(0),
            KernelTime(0),
            BasePriority(0),
            UniqueProcessId(0),
            UniqueParentProcessId(0),
            HandleCount(0),
            SessionId(0),
            UniqueProcessKey(0),
            PeakVirtualSize(0),
            VirtualSize(0),
            PageFaultCount(0),
            PeakWorkingSetSize(0),
            WorkingSetSize(0),
            QuotaPeakPagedPoolUsage(0),
            QuotaPagedPoolUsage(0),
            QuotaPeakNonPagedPoolUsage(0),
            QuotaNonPagedPoolUsage(0),
            PagefileUsage(0),
            PeakPagefileUsage(0),
            PrivatePageCount(0),
            ReadOperationCount(0),
            WriteOperationCount(0),
            OtherOperationCount(0),
            ReadTransferCount(0),
            WriteTransferCount(0),
            OtherTransferCount(0),
            GdiCount(0),
            UserCount(0),
            fSuspend(FALSE)
        {

        }

    } PROCESS_INFO, * PSYS_PROCESS_INFO;

    // 进程节点信息
    typedef struct _PROC_NODE
    {
        PROCESS_INFO   ProcInfo;                // 进程信息
        std::map<DWORD, _PROC_NODE> NodeList;   // 子进程列表
    }PROC_NODE;

    // 进程列表
    using PROCESS_INFO_LIST = std::map<DWORD, PROCESS_INFO>;

    // 路径集
    using PROC_PATH_SET = std::set<_tstring>;

    // 进程结点
    using PROC_NODE_LIST = std::map<DWORD, PROC_NODE>;

    //
    // 进程提权
    // @param: hProcess             进程句柄
    // @param: lpPrivilegesName     特权名
    // @ret: bool                   操作是否成功
    bool EnablePrivileges(HANDLE hProcess, LPCTSTR lpPrivilegesName = SE_DEBUG_NAME);

    //
    // @brief: 创建进程
    // @param: strExePath           可执行文件路径
    // @param: strCommand           命令参数
    // @param: strCurDir            程序的当前目录
    // @param: bShow                是否显示程序
    // @param: bWait                是否等待程序直到其退出
    // @ret: bool                   操作是否成功
    bool CreateProcessNormal(
        const _tstring& strExePath = _T(""),
        const _tstring& strCommand = _T(""),
        const _tstring& strCurDir = _T(""),
        bool bShow = true,
        bool bWait = true
    );

    //
    // @brief: 创建 带有UI 权限的进程
    // @param: strExePath           可执行文件路径
    // @param: strCommand           命令参数
    // @param: strCurDir            程序的当前目录
    DWORD CreateProcessWithUIAccess(
        const _tstring& strExePath,
        const _tstring& strCommand,
        const _tstring& strCurDir = _T("")
    );

    //
    // @brief: 创建不带UI权限的进程
    // @param: strExePath           可执行文件路径
    // @param: strCommand           命令参数
    // @param: strCurDir            程序的当前目录
    DWORD CreateProcessNoUIAccess(
        const _tstring& strExePath,
        const _tstring& strCommand,
        const _tstring& strCurDir = _T("")
    );

    //
    // @brief: 创建 SYSTEM 权限的进程
    // @param: strExePath           可执行文件路径
    // @param: strCommand           命令参数
    // @param: strCurDir            程序的当前目录
    bool CreateProcessWithSystem(
        const _tstring& strExePath,
        const _tstring& strCommand,
        const _tstring& strCurDir = _T("")
    );

    //
    // @brief: 检查指定进程是否有UI权限
    // @param: dwPid                进程ID
    // @param: pdwErr               输出错误码
    // @param: pfUIAccess           输出权限
    // @param: 操作是否成功
    BOOL CheckProcessUIAccess(
        DWORD dwPid,
        DWORD* pdwErr,
        DWORD* pfUIAccess
    );

    //
    // @brief: 获取进程可执行文件路径
    // @param: dwPID                进程ID
    // @param: bNtPath              是否为Win32 样式的 NT 路径
    // @ret: _tstring               进程可执行文件路径
    _tstring GetPath(DWORD dwPID, bool bNtPath = true);

    //
    // @brief: 获取父进程可执行文件路径
    // @param: dwPID                进程ID
    // @param: bNtPath              是否为Win32 样式的 NT 路径
    // @ret: _tstring               进程可执行文件路径
    _tstring GetParentPath(DWORD dwPID, bool bNtPath = true);

    //
    // @brief: 获取进程名列表
    // @ret: PROC_NAME_LIST         进程名列表
    PROC_SID_LIST GetProcessAccountSidList();

    //
    // @brief: 获取指定进程的模块信息(注意: 32位进程不能获取64位进程的模块信息)
    // @param: dwPID                进程ID
    // @ret: MODULE_INFO_LIST       模块信息
    MODULE_INFO_LIST GetModuleList(DWORD dwPID);
    MODULE_INFO_LIST GetModuleListEx(DWORD dwPID);

    //
    // @brief: 从进程列表获取指定进程的进程节点
    // @param: procList             进程列表
    // @param: dwPID                进程ID
    // @ret: PROC_NODE  进程信息
    PROC_NODE GetProcessNode(const PROCESS_INFO_LIST& procList, DWORD dwPID);

    //
    // @brief: 获取指定进程的进程节点
    // @ret: PROC_NODE              进程节点
    PROC_NODE GetProcessNode(DWORD dwPID);

    //
    // @brief: 获取指定进程路径是否存在于指定进程节点内
    // @param: strPath              进程路径
    // @param: dwPID                进程ID
    // @ret: bool                   是否存在于指定进程节点内
    bool IsPathInProcessNode(const _tstring& strPath, DWORD dwPID);

    //
    // @brief: 获取父进程的进程ID
    // @param: dwPID                进程ID
    // @ret: DWORD                  父进程的进程ID
    DWORD GetParentID(DWORD dwPID);

    //
    // @brief: 等待进程结束
    // @param: dwPID                进程ID
    // @param: dwMilliseconds       超时时间
    // @ret: bool                   操作是否成功
    bool WaitForProcess(DWORD dwPID, DWORD dwMilliseconds = INFINITE);

    //
    // @brief: 杀死指定进程
    // @param: strName              进程名
    // @ret: bool                   操作是否成功
    bool KillProcess(const _tstring& strName = _T(""));

    //
    // @brief: 杀死指定进程
    // @param: dwPID                进程ID
    // @ret: bool                   操作是否成功
    bool KillProcess(DWORD dwPID);

    //
    // @brief: 杀死指定进程ID的进程节点
    // @param: dwPID                进程ID
    // @ret: bool                   操作是否成功
    bool KillProcessNode(DWORD dwPID);

    //
    // @brief: DOS路径转Nt路径
    // @param: strPath              路径
    // @ret: _tstring               Win32 样式的 NT 路径
    _tstring DosPathToNtPath(const _tstring& strPath);

    //
    // @brief: Nt路径转DOS路径
    // @param: strPath              路径
    // @ret: _tstring               DOS路径
    _tstring NtPathToDosPath(const _tstring& strPath);

    //
    // @brief: 挂起进程
    // @param: dwPID                进程ID
    // @ret: bool                   检查结果
    bool SuspendProcess(DWORD dwPID);

    //
    // @brief: 挂起进程节点
    // @param: dwPID                进程ID
    // @ret: bool                   检查结果
    bool SuspendProcessNode(DWORD dwPID);

    //
    // @brief: 恢复进程
    // @param: dwPID                进程ID
    // @ret: bool                   检查结果
    bool ResumeProcess(DWORD dwPID);

    //
    // @brief: 恢复进程节点
    // @param: dwPID                进程ID
    // @ret: bool                   检查结果
    bool ResumeProcessNode(DWORD dwPID);

    //
    // @brief: 指定进程是否为 64 位处理器上运行的 32 位应用程序
    // @param: dwPID                进程ID
    // @param: dwMilliseconds       超时时间
    // @ret: bool                   进程是否为 64 位处理器上运行的 32 位应用程序
    bool IsWow64Process(DWORD dwPID);

    // @brief: 指定进程是否 32 位应用程序
    // @param: dwPID                进程ID
    // @param: dwMilliseconds       超时时间
    // @ret: bool                   进程是否 32 位应用程序
    bool IsProcessX86(DWORD dwPID);

    //
    // @brief: 指定进程是否 64 位应用程序
    // @param: dwPID                进程ID
    // @param: dwMilliseconds       超时时间
    // @ret: bool                   进程是否 64 位应用程序
    bool IsProcessX64(DWORD dwPID);

    //
    //
    // @brief: 获取进程组信息
    // @param: dwPID                进程ID
    // @ret: PROCESS_GROUPS_LIST    进程组信息
    PROC_TOKEN_GROUPS GetProcessTokenGroups(DWORD dwPID);

    //
    // @brief: 获取进程中句柄计数
    // @param: dwPID                进程ID
    // @ret: DWORD                  句柄计数
    DWORD GetProcessHandleCount(DWORD dwPID);

    //
    // @brief: 获取进程窗口列表
    // @param: dwPID                进程ID
    // @ret: WND_INFO_LIST          窗口列表
    WND_INFO_LIST GetProcessWindows(DWORD dwPid);

    //
    // @brief: 获取进程窗口节点列表
    // @param: dwPID                进程ID
    // @ret: WND_NODE_LIST          窗口节点列表
    WND_NODE_LIST GetProcessWindowNodes(DWORD dwPid);

    //
    // 创建指定进程的令牌
    // @param: pid                  进程ID
    // @param: strExePath           可执行文件路径
    // @param: strCommand           命令参数
    // @param: strCurDir            程序的当前目录
    // @ret: bool                   是否操作成功
    bool CreateProcessWithPidToken(
        DWORD pid,
        const _tstring& strExePath,
        const _tstring& strCommand,
        const _tstring& strCurDir
    );

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
    // 获取资源信息版本信息
    // @param: hModule              模块句柄
    // @ret: langList               语言列表
    // @ret: VER_LANG_ID_LIST       资源信息版本列表
    VERSION_INFO_LIST GetResourceVersion(HMODULE hModule, const LANG_LIST& langList);

    //
    // @brief: 获取进程信息列表
    // @param: strName              进程名
    // @param: dwPID                进程ID
    // @param: fModule              获取模块(耗时高)
    // @param: fMultilingual        多语言资源信息
    // @param: onlyPathSet          指定获取路径集
    // @ret: PROCESS_INFO_LIST      进程信息列表
    PROCESS_INFO_LIST GetProcessList1(
        const _tstring& strName = _T(""),
        DWORD dwPID = 0,
        bool fModule = false,
        bool fMultilingual = false,
        const PROC_PATH_SET& onlyPathSet = {}
    );

    //
    // @brief: 获取进程信息列表
    // @param: strName              进程名
    // @param: dwPID                进程ID
    // @param: fModule              获取模块(耗时高)
    // @param: fMultilingual        多语言资源信息
    // @param: onlyPathSet          指定获取路径集
    // @ret: PROCESS_INFO_LIST      进程信息列表
    PROCESS_INFO_LIST GetProcessList2(
        const _tstring& strName = _T(""),
        DWORD dwPID = 0,
        bool fModule = false,
        bool fMultilingual = false,
        const PROC_PATH_SET& onlyPathSet = {}
    );

    //
    // @brief: 获取当前 CPU 使用率
    // @ret: double                 CPU 使用率
    double GetCpuUsage();

    //
    // @brief: 获取进程信息列表
    // @param: strName              进程名
    // @param: dwPID                进程ID
    // @param: fModule              获取模块(耗时高)
    // @param: fMultilingual        多语言资源信息
    // @param: onlyPathSet          指定获取路径集
    // @ret: PROCESS_INFO_LIST      进程信息列表
    PROCESS_INFO_LIST GetProcessList(
        const _tstring& strName = _T(""), 
        DWORD dwPID = 0,
        bool fModule = false,
        bool fMultilingual = false,
        const PROC_PATH_SET& onlyPathSet = {}
    );

    //
    // @brief: 获取进程信息
    // @param: dwPID                进程ID
    // @param: fModule              获取模块(耗时高)
    // @param: fMultilingual        多语言资源信息
    // @ret: SYS_PROCESS_INFO       进程信息
    PROCESS_INFO GetProcessInfo(
        DWORD dwPID,
        bool fModule = false,
        bool fMultilingual = false
        );

    //
    // @brief: 获取进程节点
    // @ret: procList        进程列表
    PROC_NODE_LIST GetProcessNodes(const PROCESS_INFO_LIST& procList);

    //
    // @brief: 获取所有进程节点
    // @param: fModule              获取模块(耗时高)
    // @param: fMultilingual        多语言资源信息
    // @ret: PROC_NODE_LIST  进程节点信息
    PROC_NODE_LIST GetProcessNodes(
        bool fModule = false,
        bool fMultilingual = false
    );

    //
    // @brief: 获取所有进程名列表
    // @ret: PROC_NODE_LIST  进程名列表
    PROC_NAME_LIST GetProcessNames();
}
