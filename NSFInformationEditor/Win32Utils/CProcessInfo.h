#pragma once
#include <windows.h>
#include <tchar.h>

namespace CProcessInfo
{
    typedef _Return_type_success_(return >= 0) LONG NTSTATUS;

    typedef enum _PROCESSINFOCLASS
    {
        ProcessBasicInformation, // q: PROCESS_BASIC_INFORMATION, PROCESS_EXTENDED_BASIC_INFORMATION
        ProcessQuotaLimits, // qs: QUOTA_LIMITS, QUOTA_LIMITS_EX
        ProcessIoCounters, // q: IO_COUNTERS
        ProcessVmCounters, // q: VM_COUNTERS, VM_COUNTERS_EX, VM_COUNTERS_EX2
        ProcessTimes, // q: KERNEL_USER_TIMES
        ProcessBasePriority, // s: KPRIORITY
        ProcessRaisePriority, // s: ULONG
        ProcessDebugPort, // q: HANDLE
        ProcessExceptionPort, // s: PROCESS_EXCEPTION_PORT (requires SeTcbPrivilege)
        ProcessAccessToken, // s: PROCESS_ACCESS_TOKEN
        ProcessLdtInformation, // qs: PROCESS_LDT_INFORMATION // 10
        ProcessLdtSize, // s: PROCESS_LDT_SIZE
        ProcessDefaultHardErrorMode, // qs: ULONG
        ProcessIoPortHandlers, // (kernel-mode only) // s: PROCESS_IO_PORT_HANDLER_INFORMATION
        ProcessPooledUsageAndLimits, // q: POOLED_USAGE_AND_LIMITS
        ProcessWorkingSetWatch, // q: PROCESS_WS_WATCH_INFORMATION[]; s: void
        ProcessUserModeIOPL, // qs: ULONG (requires SeTcbPrivilege)
        ProcessEnableAlignmentFaultFixup, // s: BOOLEAN
        ProcessPriorityClass, // qs: PROCESS_PRIORITY_CLASS
        ProcessWx86Information, // qs: ULONG (requires SeTcbPrivilege) (VdmAllowed)
        ProcessHandleCount, // q: ULONG, PROCESS_HANDLE_INFORMATION // 20
        ProcessAffinityMask, // (q >WIN7)s: KAFFINITY, qs: GROUP_AFFINITY
        ProcessPriorityBoost, // qs: ULONG
        ProcessDeviceMap, // qs: PROCESS_DEVICEMAP_INFORMATION, PROCESS_DEVICEMAP_INFORMATION_EX
        ProcessSessionInformation, // q: PROCESS_SESSION_INFORMATION
        ProcessForegroundInformation, // s: PROCESS_FOREGROUND_BACKGROUND
        ProcessWow64Information, // q: ULONG_PTR
        ProcessImageFileName, // q: UNICODE_STRING
        ProcessLUIDDeviceMapsEnabled, // q: ULONG
        ProcessBreakOnTermination, // qs: ULONG
        ProcessDebugObjectHandle, // q: HANDLE // 30
        ProcessDebugFlags, // qs: ULONG
        ProcessHandleTracing, // q: PROCESS_HANDLE_TRACING_QUERY; s: PROCESS_HANDLE_TRACING_ENABLE[_EX] or void to disable
        ProcessIoPriority, // qs: IO_PRIORITY_HINT
        ProcessExecuteFlags, // qs: ULONG (MEM_EXECUTE_OPTION_*)
        ProcessTlsInformation, // PROCESS_TLS_INFORMATION // ProcessResourceManagement
        ProcessCookie, // q: ULONG
        ProcessImageInformation, // q: SECTION_IMAGE_INFORMATION
        ProcessCycleTime, // q: PROCESS_CYCLE_TIME_INFORMATION // since VISTA
        ProcessPagePriority, // qs: PAGE_PRIORITY_INFORMATION
        ProcessInstrumentationCallback, // s: PVOID or PROCESS_INSTRUMENTATION_CALLBACK_INFORMATION // 40
        ProcessThreadStackAllocation, // s: PROCESS_STACK_ALLOCATION_INFORMATION, PROCESS_STACK_ALLOCATION_INFORMATION_EX
        ProcessWorkingSetWatchEx, // q: PROCESS_WS_WATCH_INFORMATION_EX[]; s: void
        ProcessImageFileNameWin32, // q: UNICODE_STRING
        ProcessImageFileMapping, // q: HANDLE (input)
        ProcessAffinityUpdateMode, // qs: PROCESS_AFFINITY_UPDATE_MODE
        ProcessMemoryAllocationMode, // qs: PROCESS_MEMORY_ALLOCATION_MODE
        ProcessGroupInformation, // q: USHORT[]
        ProcessTokenVirtualizationEnabled, // s: ULONG
        ProcessConsoleHostProcess, // qs: ULONG_PTR // ProcessOwnerInformation
        ProcessWindowInformation, // q: PROCESS_WINDOW_INFORMATION // 50
        ProcessHandleInformation, // q: PROCESS_HANDLE_SNAPSHOT_INFORMATION // since WIN8
        ProcessMitigationPolicy, // s: PROCESS_MITIGATION_POLICY_INFORMATION
        ProcessDynamicFunctionTableInformation, // s: PROCESS_DYNAMIC_FUNCTION_TABLE_INFORMATION
        ProcessHandleCheckingMode, // qs: ULONG; s: 0 disables, otherwise enables
        ProcessKeepAliveCount, // q: PROCESS_KEEPALIVE_COUNT_INFORMATION
        ProcessRevokeFileHandles, // s: PROCESS_REVOKE_FILE_HANDLES_INFORMATION
        ProcessWorkingSetControl, // s: PROCESS_WORKING_SET_CONTROL (requires SeDebugPrivilege)
        ProcessHandleTable, // q: ULONG[] // since WINBLUE
        ProcessCheckStackExtentsMode, // qs: ULONG // KPROCESS->CheckStackExtents (CFG)
        ProcessCommandLineInformation, // q: UNICODE_STRING // 60
        ProcessProtectionInformation, // q: PS_PROTECTION
        ProcessMemoryExhaustion, // s: PROCESS_MEMORY_EXHAUSTION_INFO // since THRESHOLD
        ProcessFaultInformation, // s: PROCESS_FAULT_INFORMATION
        ProcessTelemetryIdInformation, // q: PROCESS_TELEMETRY_ID_INFORMATION
        ProcessCommitReleaseInformation, // qs: PROCESS_COMMIT_RELEASE_INFORMATION
        ProcessDefaultCpuSetsInformation, // qs: SYSTEM_CPU_SET_INFORMATION[5]
        ProcessAllowedCpuSetsInformation, // qs: SYSTEM_CPU_SET_INFORMATION[5]
        ProcessSubsystemProcess,
        ProcessJobMemoryInformation, // q: PROCESS_JOB_MEMORY_INFO
        ProcessInPrivate, // q: BOOLEAN; s: void // ETW // since THRESHOLD2 // 70
        ProcessRaiseUMExceptionOnInvalidHandleClose, // qs: ULONG; s: 0 disables, otherwise enables
        ProcessIumChallengeResponse,
        ProcessChildProcessInformation, // q: PROCESS_CHILD_PROCESS_INFORMATION
        ProcessHighGraphicsPriorityInformation, // qs: BOOLEAN (requires SeTcbPrivilege)
        ProcessSubsystemInformation, // q: SUBSYSTEM_INFORMATION_TYPE // since REDSTONE2
        ProcessEnergyValues, // q: PROCESS_ENERGY_VALUES, PROCESS_EXTENDED_ENERGY_VALUES
        ProcessPowerThrottlingState, // qs: POWER_THROTTLING_PROCESS_STATE
        ProcessReserved3Information, // ProcessActivityThrottlePolicy // PROCESS_ACTIVITY_THROTTLE_POLICY
        ProcessWin32kSyscallFilterInformation, // q: WIN32K_SYSCALL_FILTER
        ProcessDisableSystemAllowedCpuSets, // s: BOOLEAN // 80
        ProcessWakeInformation, // q: PROCESS_WAKE_INFORMATION
        ProcessEnergyTrackingState, // qs: PROCESS_ENERGY_TRACKING_STATE
        ProcessManageWritesToExecutableMemory, // MANAGE_WRITES_TO_EXECUTABLE_MEMORY // since REDSTONE3
        ProcessCaptureTrustletLiveDump,
        ProcessTelemetryCoverage, // q: TELEMETRY_COVERAGE_HEADER; s: TELEMETRY_COVERAGE_POINT
        ProcessEnclaveInformation,
        ProcessEnableReadWriteVmLogging, // qs: PROCESS_READWRITEVM_LOGGING_INFORMATION
        ProcessUptimeInformation, // q: PROCESS_UPTIME_INFORMATION
        ProcessImageSection, // q: HANDLE
        ProcessDebugAuthInformation, // since REDSTONE4 // 90
        ProcessSystemResourceManagement, // s: PROCESS_SYSTEM_RESOURCE_MANAGEMENT
        ProcessSequenceNumber, // q: ULONGLONG
        ProcessLoaderDetour, // since REDSTONE5
        ProcessSecurityDomainInformation, // q: PROCESS_SECURITY_DOMAIN_INFORMATION
        ProcessCombineSecurityDomainsInformation, // s: PROCESS_COMBINE_SECURITY_DOMAINS_INFORMATION
        ProcessEnableLogging, // qs: PROCESS_LOGGING_INFORMATION
        ProcessLeapSecondInformation, // qs: PROCESS_LEAP_SECOND_INFORMATION
        ProcessFiberShadowStackAllocation, // s: PROCESS_FIBER_SHADOW_STACK_ALLOCATION_INFORMATION // since 19H1
        ProcessFreeFiberShadowStackAllocation, // s: PROCESS_FREE_FIBER_SHADOW_STACK_ALLOCATION_INFORMATION
        ProcessAltSystemCallInformation, // s: PROCESS_SYSCALL_PROVIDER_INFORMATION // since 20H1 // 100
        ProcessDynamicEHContinuationTargets, // s: PROCESS_DYNAMIC_EH_CONTINUATION_TARGETS_INFORMATION
        ProcessDynamicEnforcedCetCompatibleRanges, // s: PROCESS_DYNAMIC_ENFORCED_ADDRESS_RANGE_INFORMATION // since 20H2
        ProcessCreateStateChange, // since WIN11
        ProcessApplyStateChange,
        ProcessEnableOptionalXStateFeatures, // s: ULONG64 // optional XState feature bitmask
        ProcessAltPrefetchParam, // qs: OVERRIDE_PREFETCH_PARAMETER // App Launch Prefetch (ALPF) // since 22H1
        ProcessAssignCpuPartitions, // HANDLE
        ProcessPriorityClassEx, // s: PROCESS_PRIORITY_CLASS_EX
        ProcessMembershipInformation, // q: PROCESS_MEMBERSHIP_INFORMATION
        ProcessEffectiveIoPriority, // q: IO_PRIORITY_HINT // 110
        ProcessEffectivePagePriority, // q: ULONG
        ProcessSchedulerSharedData, // SCHEDULER_SHARED_DATA_SLOT_INFORMATION // since 24H2
        ProcessSlistRollbackInformation,
        ProcessNetworkIoCounters, // q: PROCESS_NETWORK_COUNTERS
        ProcessFindFirstThreadByTebValue, // PROCESS_TEB_VALUE_INFORMATION
        MaxProcessInfoClass
    } PROCESSINFOCLASS;

    typedef struct _UNICODE_STRING
    {
        USHORT Length;
        USHORT MaximumLength;
        _Field_size_bytes_part_opt_(MaximumLength, Length) PWCH Buffer;
    } UNICODE_STRING, * PUNICODE_STRING;

    // PEB->ApiSetMap
    typedef struct _API_SET_NAMESPACE
    {
        ULONG Version;
        ULONG Size;
        ULONG Flags;
        ULONG Count;
        ULONG EntryOffset;
        ULONG HashOffset;
        ULONG HashFactor;
    } API_SET_NAMESPACE, * PAPI_SET_NAMESPACE;

    typedef struct _PEB_LDR_DATA
    {
        ULONG Length;
        BOOLEAN Initialized;
        HANDLE SsHandle;
        LIST_ENTRY InLoadOrderModuleList;
        LIST_ENTRY InMemoryOrderModuleList;
        LIST_ENTRY InInitializationOrderModuleList;
        PVOID EntryInProgress;
        BOOLEAN ShutdownInProgress;
        HANDLE ShutdownThreadId;
    } PEB_LDR_DATA, * PPEB_LDR_DATA;

    typedef struct _RTL_USER_PROCESS_PARAMETERS* PRTL_USER_PROCESS_PARAMETERS;
    typedef struct _RTL_CRITICAL_SECTION* PRTL_CRITICAL_SECTION;
    typedef struct _SILO_USER_SHARED_DATA* PSILO_USER_SHARED_DATA;
    typedef struct _LEAP_SECOND_DATA* PLEAP_SECOND_DATA;


    typedef struct _TEB* PTEB;
    typedef struct _COUNTED_REASON_CONTEXT* PCOUNTED_REASON_CONTEXT;
    typedef struct _FILE_IO_COMPLETION_INFORMATION* PFILE_IO_COMPLETION_INFORMATION;
    typedef struct _PORT_MESSAGE* PPORT_MESSAGE;
    typedef struct _IMAGE_EXPORT_DIRECTORY* PIMAGE_EXPORT_DIRECTORY;
    typedef struct _FILE_OBJECT* PFILE_OBJECT;
    typedef struct _DEVICE_OBJECT* PDEVICE_OBJECT;
    typedef struct _IRP* PIRP;
    typedef struct _RTL_BITMAP* PRTL_BITMAP;

#define GDI_HANDLE_BUFFER_SIZE32 34
#define GDI_HANDLE_BUFFER_SIZE64 60

#ifndef _WIN64
#define GDI_HANDLE_BUFFER_SIZE GDI_HANDLE_BUFFER_SIZE32
#else
#define GDI_HANDLE_BUFFER_SIZE GDI_HANDLE_BUFFER_SIZE64
#endif

    typedef ULONG GDI_HANDLE_BUFFER[GDI_HANDLE_BUFFER_SIZE];

    typedef ULONG GDI_HANDLE_BUFFER32[GDI_HANDLE_BUFFER_SIZE32];
    typedef ULONG GDI_HANDLE_BUFFER64[GDI_HANDLE_BUFFER_SIZE64];

    // symbols
    typedef struct _PEB
    {
        BOOLEAN InheritedAddressSpace;
        BOOLEAN ReadImageFileExecOptions;
        BOOLEAN BeingDebugged;
        union
        {
            BOOLEAN BitField;
            struct
            {
                BOOLEAN ImageUsesLargePages : 1;
                BOOLEAN IsProtectedProcess : 1;
                BOOLEAN IsImageDynamicallyRelocated : 1;
                BOOLEAN SkipPatchingUser32Forwarders : 1;
                BOOLEAN IsPackagedProcess : 1;
                BOOLEAN IsAppContainer : 1;
                BOOLEAN IsProtectedProcessLight : 1;
                BOOLEAN IsLongPathAwareProcess : 1;
            };
        };
        HANDLE Mutant;

        PVOID ImageBaseAddress;
        PPEB_LDR_DATA Ldr;
        PRTL_USER_PROCESS_PARAMETERS ProcessParameters;
        PVOID SubSystemData;
        PVOID ProcessHeap;
        PRTL_CRITICAL_SECTION FastPebLock;
        PSLIST_HEADER AtlThunkSListPtr;
        PVOID IFEOKey;

        union
        {
            ULONG CrossProcessFlags;
            struct
            {
                ULONG ProcessInJob : 1;
                ULONG ProcessInitializing : 1;
                ULONG ProcessUsingVEH : 1;
                ULONG ProcessUsingVCH : 1;
                ULONG ProcessUsingFTH : 1;
                ULONG ProcessPreviouslyThrottled : 1;
                ULONG ProcessCurrentlyThrottled : 1;
                ULONG ProcessImagesHotPatched : 1; // REDSTONE5
                ULONG ReservedBits0 : 24;
            };
        };
        union
        {
            PVOID KernelCallbackTable;
            PVOID UserSharedInfoPtr;
        };

        ULONG SystemReserved;
        ULONG AtlThunkSListPtr32;
        PAPI_SET_NAMESPACE ApiSetMap;
        ULONG TlsExpansionCounter;
        PRTL_BITMAP TlsBitmap;
        ULONG TlsBitmapBits[2]; // TLS_MINIMUM_AVAILABLE

        PVOID ReadOnlySharedMemoryBase;
        PSILO_USER_SHARED_DATA SharedData; // HotpatchInformation
        PVOID* ReadOnlyStaticServerData;

        PVOID AnsiCodePageData; // PCPTABLEINFO
        PVOID OemCodePageData; // PCPTABLEINFO
        PVOID UnicodeCaseTableData; // PNLSTABLEINFO

        // Information for LdrpInitialize
        ULONG NumberOfProcessors;
        ULONG NtGlobalFlag;

        // Passed up from MmCreatePeb from Session Manager registry key
        LARGE_INTEGER CriticalSectionTimeout;
        SIZE_T HeapSegmentReserve;
        SIZE_T HeapSegmentCommit;
        SIZE_T HeapDeCommitTotalFreeThreshold;
        SIZE_T HeapDeCommitFreeBlockThreshold;

        //
        // Where heap manager keeps track of all heaps created for a process
        // Fields initialized by MmCreatePeb.  ProcessHeaps is initialized
        // to point to the first free byte after the PEB and MaximumNumberOfHeaps
        // is computed from the page size used to hold the PEB, less the fixed
        // size of this data structure.
        //
        ULONG NumberOfHeaps;
        ULONG MaximumNumberOfHeaps;
        PVOID* ProcessHeaps; // PHEAP

        PVOID GdiSharedHandleTable; // PGDI_SHARED_MEMORY
        PVOID ProcessStarterHelper;
        ULONG GdiDCAttributeList;

        PRTL_CRITICAL_SECTION LoaderLock;

        //
        // Following fields filled in by MmCreatePeb from system values and/or
        // image header.
        //
        ULONG OSMajorVersion;
        ULONG OSMinorVersion;
        USHORT OSBuildNumber;
        USHORT OSCSDVersion;
        ULONG OSPlatformId;
        ULONG ImageSubsystem;
        ULONG ImageSubsystemMajorVersion;
        ULONG ImageSubsystemMinorVersion;
        KAFFINITY ActiveProcessAffinityMask;
        GDI_HANDLE_BUFFER GdiHandleBuffer;
        PVOID PostProcessInitRoutine;

        PRTL_BITMAP TlsExpansionBitmap;
        ULONG TlsExpansionBitmapBits[32]; // TLS_EXPANSION_SLOTS

        ULONG SessionId;

        ULARGE_INTEGER AppCompatFlags; // KACF_*
        ULARGE_INTEGER AppCompatFlagsUser;
        PVOID pShimData;
        PVOID AppCompatInfo; // APPCOMPAT_EXE_DATA

        UNICODE_STRING CSDVersion;

        PVOID ActivationContextData;
        PVOID ProcessAssemblyStorageMap;
        PVOID SystemDefaultActivationContextData;
        PVOID SystemAssemblyStorageMap;

        SIZE_T MinimumStackCommit;

        PVOID SparePointers[2]; // 19H1 (previously FlsCallback to FlsHighIndex)
        PVOID PatchLoaderData;
        PVOID ChpeV2ProcessInfo; // _CHPEV2_PROCESS_INFO

        ULONG AppModelFeatureState;
        ULONG SpareUlongs[2];

        USHORT ActiveCodePage;
        USHORT OemCodePage;
        USHORT UseCaseMapping;
        USHORT UnusedNlsField;

        PVOID WerRegistrationData;
        PVOID WerShipAssertPtr;

        union
        {
            PVOID pContextData; // WIN7
            PVOID pUnused; // WIN10
            PVOID EcCodeBitMap; // WIN11
        };

        PVOID pImageHeaderHash;
        union
        {
            ULONG TracingFlags;
            struct
            {
                ULONG HeapTracingEnabled : 1;
                ULONG CritSecTracingEnabled : 1;
                ULONG LibLoaderTracingEnabled : 1;
                ULONG SpareTracingBits : 29;
            };
        };
        ULONGLONG CsrServerReadOnlySharedMemoryBase;
        PRTL_CRITICAL_SECTION TppWorkerpListLock;
        LIST_ENTRY TppWorkerpList;
        PVOID WaitOnAddressHashTable[128];
        PVOID TelemetryCoverageHeader; // REDSTONE3
        ULONG CloudFileFlags;
        ULONG CloudFileDiagFlags; // REDSTONE4
        CHAR PlaceholderCompatibilityMode;
        CHAR PlaceholderCompatibilityModeReserved[7];
        PVOID LeapSecondData; // REDSTONE5
        union
        {
            ULONG LeapSecondFlags;
            struct
            {
                ULONG SixtySecondEnabled : 1;
                ULONG Reserved : 31;
            };
        };
        ULONG NtGlobalFlag2;
        ULONGLONG ExtendedFeatureDisableMask; // since WIN11
    } PEB, * PPEB;

    typedef LONG KPRIORITY, * PKPRIORITY;

    typedef struct _PROCESS_BASIC_INFORMATION
    {
        NTSTATUS ExitStatus;
        PPEB PebBaseAddress;
        KAFFINITY AffinityMask;
        KPRIORITY BasePriority;
        HANDLE UniqueProcessId;
        HANDLE InheritedFromUniqueProcessId;
    } PROCESS_BASIC_INFORMATION, * PPROCESS_BASIC_INFORMATION;

    NTSTATUS NTAPI NtReadVirtualMemory(
        _In_ HANDLE ProcessHandle,
        _In_opt_ PVOID BaseAddress,
        _Out_writes_bytes_to_(NumberOfBytesToRead, *NumberOfBytesRead) PVOID Buffer,
        _In_ SIZE_T NumberOfBytesToRead,
        _Out_opt_ PSIZE_T NumberOfBytesRead
    );

    NTSTATUS NTAPI NtQueryInformationProcess(
        IN            HANDLE           ProcessHandle,
        IN            PROCESSINFOCLASS ProcessInformationClass,
        OUT           PVOID            ProcessInformation,
        IN            ULONG            ProcessInformationLength,
        OUT OPTIONAL  PULONG           ReturnLength
    );

    NTSTATUS GetProcessBasicInformation(
        _In_ HANDLE ProcessHandle,
        _Out_ PPROCESS_BASIC_INFORMATION BasicInformation
    );

    NTSTATUS PhGetProcessPeb(
        _In_ HANDLE ProcessHandle,
        _Out_ PVOID* PebBaseAddress
    );

    VOID PhInitializeWindowsVersion(VOID);
}

