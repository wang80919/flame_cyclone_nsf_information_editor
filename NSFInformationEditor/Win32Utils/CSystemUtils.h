#pragma once

#include <string>
#include <stdint.h>
#include <windows.h>
#include <tchar.h>

#ifdef _UNICODE
using _tstring = std::wstring;
#else
using _tstring = std::string;
#endif

#define PROCESSOR_FEATURE_MAX 64

typedef struct _KSYSTEM_TIME
{
    ULONG LowPart;
    LONG High1Time;
    LONG High2Time;
} KSYSTEM_TIME, * PKSYSTEM_TIME;

typedef enum _NT_PRODUCT_TYPE
{
    NtProductWinNt = 1,
    NtProductLanManNt = 2,
    NtProductServer = 3
} NT_PRODUCT_TYPE;

typedef enum _ALTERNATIVE_ARCHITECTURE_TYPE {
    StandardDesign,
    NEC98x86,
    EndAlternatives
} ALTERNATIVE_ARCHITECTURE_TYPE;

// https://learn.microsoft.com/zh-cn/windows-hardware/drivers/ddi/ntddk/ns-ntddk-kuser_shared_data
typedef struct _KUSER_SHARED_DATA {
    ULONG                         TickCountLowDeprecated;       //当前低 32 位时钟周期计数
    ULONG                         TickCountMultiplier;          //时钟周期计数乘数
    KSYSTEM_TIME                  InterruptTime;                //当前 64 位中断时间（以 100ns 为单位）
    KSYSTEM_TIME                  SystemTime;                   //当前 64 位系统时间（以 100ns 为单位）
    KSYSTEM_TIME                  TimeZoneBias;                 //当前 64 位时区偏差
    USHORT                        ImageNumberLow;               //主机系统的低图像幻数
    USHORT                        ImageNumberHigh;              //主机系统的高图像幻数
    WCHAR                         NtSystemRoot[260];            //unicode 中系统根目录的副本。 必须通过 RtlGetNtSystemRoot API 访问此字段才能获得准确结果
    ULONG                         MaxStackTraceDepth;           //启用跟踪时的最大堆栈跟踪深度
    ULONG                         CryptoExponent;               //加密指数值
    ULONG                         TimeZoneId;                   //时区 ID
    ULONG                         LargePageMinimum;             //定义 ULONG 成员 LargePageMinimum
    ULONG                         AitSamplingValue;             //此值控制 AIT 采样率
    ULONG                         AppCompatFlag;                //此值控制切换回处理
    ULONGLONG                     RNGSeedVersion;               //当前内核根 RNG 状态种子版本
    ULONG                         GlobalValidationRunlevel;     //此值控制断言失败处理
    LONG                          TimeZoneBiasStamp;            //定义 LONG 成员 TimeZoneBiasStamp
    ULONG                         NtBuildNumber;                //使用 C 或 F 取消修饰的共享集合内部版本号. GetVersionEx 隐藏实数
    NT_PRODUCT_TYPE               NtProductType;                //产品类型。 必须通过 RtlGetNtProductType API 访问此字段才能获得准确结果
    BOOLEAN                       ProductTypeIsValid;           //定义 BOOLEAN 成员 ProductTypeIsValid
    BOOLEAN                       Reserved0[1];                 //保留供将来使用
    USHORT                        NativeProcessorArchitecture;  //定义 USHORT 成员 NativeProcessorArchitecture
    ULONG                         NtMajorVersion;               //NT 主版本。 每个进程都会从其 PEB 中看到一个版本，但如果进程运行的是系统版本的已更改视图，则此字段用于正确标识版本。
    ULONG                         NtMinorVersion;               //NT 次要版本。 每个进程都会从其 PEB 中看到一个版本，但如果进程运行的是系统版本的已更改视图，则此字段用于正确标识版本
    BOOLEAN                       ProcessorFeatures[PROCESSOR_FEATURE_MAX]; //处理器功能
    ULONG                         Reserved1;                    //留待将来使用
    ULONG                         Reserved3;                    //留待将来使用
    ULONG                         TimeSlip;                     //调试器中的时间滑点
    ALTERNATIVE_ARCHITECTURE_TYPE AlternativeArchitecture;      //备用系统体系结构。 例如，x86 上的 NEC PC98xx
    ULONG                         BootId;                       //启动序列，针对 OS 加载程序每次启动尝试递增
    LARGE_INTEGER                 SystemExpirationDate;         //如果系统是评估单元，则以下字段包含评估单元过期的日期和时间。 值为 0 表示没有过期时间。 非零值是系统过期的 UTC 绝对时间
    ULONG                         SuiteMask;                    //套件支持。 必须通过 RtlGetSuiteMask API 访问此字段才能获得准确结果
    BOOLEAN                       KdDebuggerEnabled;            //如果已连接/启用内核调试器，则为 TRUE
    union {
        UCHAR MitigationPolicies;                               //缓解策略
        struct {
            UCHAR NXSupportPolicy : 2;                          //定义 UCHAR 成员 NXSupportPolicy
            UCHAR SEHValidationPolicy : 2;                      //定义 UCHAR 成员 SEHValidationPolicy
            UCHAR CurDirDevicesSkippedForDlls : 2;              //定义 UCHAR 成员 CurDirDevicesSkippedForDlls
            UCHAR Reserved : 2;                                 //保留供将来使用
        };
    };
    USHORT                        CyclesPerYield;               //单个处理器产量的测量持续时间（以周期为单位）。 锁包使用它来确定在阻止之前等待状态更改的旋转次数
    ULONG                         ActiveConsoleId;              //当前控制台会话 ID。非 TS 系统上始终为零。 必须通过 RtlGetActiveConsoleId API 访问此字段才能获得准确结果
    ULONG                         DismountCount;                //强制卸载会导致句柄无效。 客户端可以使用这些序列号来查看是否需要探测句柄，而不是始终探测句柄
    ULONG                         ComPlusPackage;               //此字段指示系统上 64 位 COM+ 包的状态。 它指示中间语言 (IL) COM+ 映像是否需要使用 64 位 COM+ 运行时或 32 位 COM+ 运行时
    ULONG                         LastSystemRITEventTickCount;  //系统范围内所有终端会话中最后一次用户输入的计时周期计数。 对于 MP 性能，它不会一直更新 (例如，每个会话) 每分钟一次。 它用于空闲检测
    ULONG                         NumberOfPhysicalPages;        //系统中的物理页数
    BOOLEAN                       SafeBootMode;                 //如果系统是在安全启动模式下启动的，则为 True
    union {
        UCHAR VirtualizationFlags;                              //虚拟化标志
        struct {
            UCHAR ArchStartedInEl2 : 1;                         //使此位字段与 arc.w 中的位字段保持同步
            UCHAR QcSlIsSupported : 1;                          //使此位字段与 arc.w 中的位字段保持同步
        };
    };
    UCHAR                         Reserved12[2];                //保留供将来使用
    union {
        ULONG SharedDataFlags;
        struct {
            ULONG DbgErrorPortPresent : 1;          //仅适用于调试器。 请勿使用。 请改用位定义
            ULONG DbgElevationEnabled : 1;          //仅适用于调试器。 请勿使用。 请改用位定义
            ULONG DbgVirtEnabled : 1;               //仅适用于调试器。 请勿使用。 请改用位定义
            ULONG DbgInstallerDetectEnabled : 1;    //仅适用于调试器。 请勿使用。 请改用位定义
            ULONG DbgLkgEnabled : 1;                //仅适用于调试器。 请勿使用。 请改用位定义
            ULONG DbgDynProcessorEnabled : 1;       //仅适用于调试器。 请勿使用。 请改用位定义
            ULONG DbgConsoleBrokerEnabled : 1;      //仅适用于调试器。 请勿使用。 请改用位定义
            ULONG DbgSecureBootEnabled : 1;         //仅适用于调试器。 请勿使用。 请改用位定义
            ULONG DbgMultiSessionSku : 1;           //仅适用于调试器。 请勿使用。 请改用位定义
            ULONG DbgMultiUsersInSessionSku : 1;    //仅适用于调试器。 请勿使用。 请改用位定义
            ULONG DbgStateSeparationEnabled : 1;    //仅适用于调试器。 请勿使用。 请改用位定义
            ULONG SpareBits : 21;                   ////仅适用于调试器。 请勿使用。 请改用位定义
        } DUMMYSTRUCTNAME2;
    } DUMMYUNIONNAME2;  ////这是一个打包的位字段，其中包含有关系统状态的各种标志。 必须使用互锁操作来操作它们。 必须通过 RtlIsMultiSessionSku API 访问 DbgMultiSessionSku 才能获得准确结果
    ULONG                         DataFlagsPad[1];              //定义 ULONG 成员 DataFlagsPad
    ULONGLONG                     TestRetInstruction;           //根据处理器的不同，快速系统调用的代码将有所不同。 此字段仅在 32 位系统上使用
    LONGLONG                      QpcFrequency;                 //定义 LONGLONG 成员 QpcFrequency
    ULONG                         SystemCall;                   //在 AMD64 上，如果系统使用系统服务调用机制的已更改视图运行，则此值将初始化为非零值
    ULONG                         Reserved2;                    //留待将来使用
    ULONGLONG                     SystemCallPad[2];             //留待将来使用
    union {
        KSYSTEM_TIME TickCount;                                 //定义 KSYSTEM_TIME 成员 TickCount
        ULONG64      TickCountQuad;                             //定义 ULONG64 成员 TickCountQuad
        struct {                                                //定义 DUMMYSTRUCTNAME
            ULONG ReservedTickCountOverlay[3];                  //定义 ULONG 成员 ReservedTickCountOverlay
            ULONG TickCountPad[1];                              //定义 ULONG 成员 TickCountPad
        } DUMMYSTRUCTNAME;
    } DUMMYUNIONNAME3;      //64 位刻度计数
    ULONG                         Cookie;                       //用于对系统范围内的指针进行编码的 Cookie
    ULONG                         CookiePad[1];                 //保留供将来使用
    LONGLONG                      ConsoleSessionForegroundProcessId;    //焦点位于当前活动控制台会话 ID 中的进程的客户端 ID。必须通过 RtlGetConsoleSessionForegroundProcessId API 访问此字段，才能获得准确结果
    ULONGLONG                     TimeUpdateLock;               //(内部) 时间更新锁的占位符。 此数据用于实现精确的时间服务。 它在 64 字节缓存线边界上对齐，并按典型访问的顺序排列
    ULONGLONG                     BaselineSystemTimeQpc;        //用于建立当前系统时间的性能计数器值
    ULONGLONG                     BaselineInterruptTimeQpc;     //用于计算上次中断时间的性能计数器值
    ULONGLONG                     QpcSystemTimeIncrement;       //此值 (单个性能计数表示的缩放系统时间秒数可能会有所不同，以实现时间同步) 
    ULONGLONG                     QpcInterruptTimeIncrement;    //在启动系统) 后，由单个性能计数表示的缩放中断时间秒数 (此值是恒定的
    UCHAR                         QpcSystemTimeIncrementShift;  //应用于性能计数器系统时间增量的缩放移位计数
    UCHAR                         QpcInterruptTimeIncrementShift;   //应用于性能计数器中断时间增量的缩放移位计数
    USHORT                        UnparkedProcessorCount;       //未停止的处理器计数
    ULONG                         EnclaveFeatureMask[4];        //此系统支持的 enclave 功能的位掩码。 必须通过 RtlIsEnclareFeaturePresent API 访问此字段才能获得准确结果
    ULONG                         TelemetryCoverageRound;       //基于遥测的覆盖范围的当前覆盖轮次
    USHORT                        UserModeGlobalLogger[16];     //以下字段用于 ETW 用户模式全局日志记录 (UMGL) 
    ULONG                         ImageFileExecutionOptions;    //除了原始 HKLM 之外，还可以启用使用 HKCU 中的图像文件执行选项的设置
    ULONG                         LangGenerationCount;          //生成保存系统语言信息的内核结构
    ULONGLONG                     Reserved4;                    //保留供将来使用
    ULONGLONG                     InterruptTimeBias;            //当前 64 位中断时间偏差（以 100ns 为单位）
    ULONGLONG                     QpcBias;                      //当前 64 位性能计数器偏差，在应用移位之前，以性能计数器单位为单位
    ULONG                         ActiveProcessorCount;         //活动处理器数
    UCHAR                         ActiveGroupCount;             //活动组数
    UCHAR                         Reserved9;                    //保留供将来使用
    union {                                                     
        USHORT QpcData; //定义 USHORT 成员 QpcData
        struct {
            UCHAR QpcBypassEnabled; //一个布尔值，指示性能计数器查询是否可以 (绕过系统调用) 直接读取计数器
            UCHAR QpcShift;         //应用于原始计数器值的 Shift 以派生 QPC 计数
        };
    };
    LARGE_INTEGER                 TimeZoneBiasEffectiveStart;   //定义 LARGE_INTEGER 成员 TimeZoneBiasEffectiveStart
    LARGE_INTEGER                 TimeZoneBiasEffectiveEnd;     //定义 LARGE_INTEGER 成员 TimeZoneBiasEffectiveEnd
    XSTATE_CONFIGURATION          XState;                       //扩展处理器状态配置
    KSYSTEM_TIME                  FeatureConfigurationChangeStamp;  //定义 KSYSTEM_TIME 成员 FeatureConfigurationChangeStamp
    ULONG                         Spare;                        //定义 ULONG 成员 备用
    ULONG64                       UserPointerAuthMask;          //定义 ULONG64 成员 UserPointerAuthMask
} KUSER_SHARED_DATA, * PKUSER_SHARED_DATA;

typedef struct _NTVERSIONNUMBERS
{
    DWORD dwMajor;
    DWORD dwMinor;
    DWORD dwBuildNumber;
}NTVERSIONNUMBERS, * PNTVERSIONNUMBERS;

class CSystemUtils
{
public:

    static const PKUSER_SHARED_DATA GetKUserData();
    static NTVERSIONNUMBERS _GetNtVersionNumbers();
    static std::string GetCpuName();
private:

};