#include "CProcessUtils.h"
#include <tlhelp32.h>
#include <strsafe.h>
#include <stdint.h>
#include <WtsApi32.h>
#include <sddl.h>

#define NT_SUCCESS(Status)                  (((NTSTATUS)(Status)) >= 0)
#define STATUS_SUCCESS                      ((NTSTATUS)0x00000000L)
#define STATUS_INFO_LENGTH_MISMATCH         ((NTSTATUS)0xC0000004L)

#define DPSAPI_VERSION = 1
#pragma comment(lib, "Psapi.lib")
#pragma comment(lib, "Wtsapi32.lib" )
#pragma comment(lib, "Advapi32.lib" )
#pragma comment(lib, "Version.lib")

namespace CProcessUtils
{
    typedef _Return_type_success_(return >= 0) LONG NTSTATUS;

    typedef enum _SYSTEM_INFORMATION_CLASS
    {
        SystemBasicInformation, // q: SYSTEM_BASIC_INFORMATION
        SystemProcessorInformation, // q: SYSTEM_PROCESSOR_INFORMATION
        SystemPerformanceInformation, // q: SYSTEM_PERFORMANCE_INFORMATION
        SystemTimeOfDayInformation, // q: SYSTEM_TIMEOFDAY_INFORMATION
        SystemPathInformation, // not implemented
        SystemProcessInformation, // q: SYSTEM_PROCESS_INFORMATION
        SystemCallCountInformation, // q: SYSTEM_CALL_COUNT_INFORMATION
        SystemDeviceInformation, // q: SYSTEM_DEVICE_INFORMATION
        SystemProcessorPerformanceInformation, // q: SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION (EX in: USHORT ProcessorGroup)
        SystemFlagsInformation, // q: SYSTEM_FLAGS_INFORMATION
        SystemCallTimeInformation, // not implemented // SYSTEM_CALL_TIME_INFORMATION // 10
        SystemModuleInformation, // q: RTL_PROCESS_MODULES
        SystemLocksInformation, // q: RTL_PROCESS_LOCKS
        SystemStackTraceInformation, // q: RTL_PROCESS_BACKTRACES
        SystemPagedPoolInformation, // not implemented
        SystemNonPagedPoolInformation, // not implemented
        SystemHandleInformation, // q: SYSTEM_HANDLE_INFORMATION
        SystemObjectInformation, // q: SYSTEM_OBJECTTYPE_INFORMATION mixed with SYSTEM_OBJECT_INFORMATION
        SystemPageFileInformation, // q: SYSTEM_PAGEFILE_INFORMATION
        SystemVdmInstemulInformation, // q: SYSTEM_VDM_INSTEMUL_INFO
        SystemVdmBopInformation, // not implemented // 20
        SystemFileCacheInformation, // q: SYSTEM_FILECACHE_INFORMATION; s (requires SeIncreaseQuotaPrivilege) (info for WorkingSetTypeSystemCache)
        SystemPoolTagInformation, // q: SYSTEM_POOLTAG_INFORMATION
        SystemInterruptInformation, // q: SYSTEM_INTERRUPT_INFORMATION (EX in: USHORT ProcessorGroup)
        SystemDpcBehaviorInformation, // q: SYSTEM_DPC_BEHAVIOR_INFORMATION; s: SYSTEM_DPC_BEHAVIOR_INFORMATION (requires SeLoadDriverPrivilege)
        SystemFullMemoryInformation, // not implemented // SYSTEM_MEMORY_USAGE_INFORMATION
        SystemLoadGdiDriverInformation, // s (kernel-mode only)
        SystemUnloadGdiDriverInformation, // s (kernel-mode only)
        SystemTimeAdjustmentInformation, // q: SYSTEM_QUERY_TIME_ADJUST_INFORMATION; s: SYSTEM_SET_TIME_ADJUST_INFORMATION (requires SeSystemtimePrivilege)
        SystemSummaryMemoryInformation, // not implemented // SYSTEM_MEMORY_USAGE_INFORMATION
        SystemMirrorMemoryInformation, // s (requires license value "Kernel-MemoryMirroringSupported") (requires SeShutdownPrivilege) // 30
        SystemPerformanceTraceInformation, // q; s: (type depends on EVENT_TRACE_INFORMATION_CLASS)
        SystemObsolete0, // not implemented
        SystemExceptionInformation, // q: SYSTEM_EXCEPTION_INFORMATION
        SystemCrashDumpStateInformation, // s: SYSTEM_CRASH_DUMP_STATE_INFORMATION (requires SeDebugPrivilege)
        SystemKernelDebuggerInformation, // q: SYSTEM_KERNEL_DEBUGGER_INFORMATION
        SystemContextSwitchInformation, // q: SYSTEM_CONTEXT_SWITCH_INFORMATION
        SystemRegistryQuotaInformation, // q: SYSTEM_REGISTRY_QUOTA_INFORMATION; s (requires SeIncreaseQuotaPrivilege)
        SystemExtendServiceTableInformation, // s (requires SeLoadDriverPrivilege) // loads win32k only
        SystemPrioritySeparation, // s (requires SeTcbPrivilege)
        SystemVerifierAddDriverInformation, // s: UNICODE_STRING (requires SeDebugPrivilege) // 40
        SystemVerifierRemoveDriverInformation, // s: UNICODE_STRING (requires SeDebugPrivilege)
        SystemProcessorIdleInformation, // q: SYSTEM_PROCESSOR_IDLE_INFORMATION (EX in: USHORT ProcessorGroup)
        SystemLegacyDriverInformation, // q: SYSTEM_LEGACY_DRIVER_INFORMATION
        SystemCurrentTimeZoneInformation, // q; s: RTL_TIME_ZONE_INFORMATION
        SystemLookasideInformation, // q: SYSTEM_LOOKASIDE_INFORMATION
        SystemTimeSlipNotification, // s: HANDLE (NtCreateEvent) (requires SeSystemtimePrivilege)
        SystemSessionCreate, // not implemented
        SystemSessionDetach, // not implemented
        SystemSessionInformation, // not implemented (SYSTEM_SESSION_INFORMATION)
        SystemRangeStartInformation, // q: SYSTEM_RANGE_START_INFORMATION // 50
        SystemVerifierInformation, // q: SYSTEM_VERIFIER_INFORMATION; s (requires SeDebugPrivilege)
        SystemVerifierThunkExtend, // s (kernel-mode only)
        SystemSessionProcessInformation, // q: SYSTEM_SESSION_PROCESS_INFORMATION
        SystemLoadGdiDriverInSystemSpace, // s: SYSTEM_GDI_DRIVER_INFORMATION (kernel-mode only) (same as SystemLoadGdiDriverInformation)
        SystemNumaProcessorMap, // q: SYSTEM_NUMA_INFORMATION
        SystemPrefetcherInformation, // q; s: PREFETCHER_INFORMATION // PfSnQueryPrefetcherInformation
        SystemExtendedProcessInformation, // q: SYSTEM_PROCESS_INFORMATION
        SystemRecommendedSharedDataAlignment, // q: ULONG // KeGetRecommendedSharedDataAlignment
        SystemComPlusPackage, // q; s: ULONG
        SystemNumaAvailableMemory, // q: SYSTEM_NUMA_INFORMATION // 60
        SystemProcessorPowerInformation, // q: SYSTEM_PROCESSOR_POWER_INFORMATION (EX in: USHORT ProcessorGroup)
        SystemEmulationBasicInformation, // q: SYSTEM_BASIC_INFORMATION
        SystemEmulationProcessorInformation, // q: SYSTEM_PROCESSOR_INFORMATION
        SystemExtendedHandleInformation, // q: SYSTEM_HANDLE_INFORMATION_EX
        SystemLostDelayedWriteInformation, // q: ULONG
        SystemBigPoolInformation, // q: SYSTEM_BIGPOOL_INFORMATION
        SystemSessionPoolTagInformation, // q: SYSTEM_SESSION_POOLTAG_INFORMATION
        SystemSessionMappedViewInformation, // q: SYSTEM_SESSION_MAPPED_VIEW_INFORMATION
        SystemHotpatchInformation, // q; s: SYSTEM_HOTPATCH_CODE_INFORMATION
        SystemObjectSecurityMode, // q: ULONG // 70
        SystemWatchdogTimerHandler, // s: SYSTEM_WATCHDOG_HANDLER_INFORMATION // (kernel-mode only)
        SystemWatchdogTimerInformation, // q: SYSTEM_WATCHDOG_TIMER_INFORMATION // NtQuerySystemInformationEx // (kernel-mode only)
        SystemLogicalProcessorInformation, // q: SYSTEM_LOGICAL_PROCESSOR_INFORMATION (EX in: USHORT ProcessorGroup) // NtQuerySystemInformationEx
        SystemWow64SharedInformationObsolete, // not implemented
        SystemRegisterFirmwareTableInformationHandler, // s: SYSTEM_FIRMWARE_TABLE_HANDLER // (kernel-mode only)
        SystemFirmwareTableInformation, // SYSTEM_FIRMWARE_TABLE_INFORMATION
        SystemModuleInformationEx, // q: RTL_PROCESS_MODULE_INFORMATION_EX // since VISTA
        SystemVerifierTriageInformation, // not implemented
        SystemSuperfetchInformation, // q; s: SUPERFETCH_INFORMATION // PfQuerySuperfetchInformation
        SystemMemoryListInformation, // q: SYSTEM_MEMORY_LIST_INFORMATION; s: SYSTEM_MEMORY_LIST_COMMAND (requires SeProfileSingleProcessPrivilege) // 80
        SystemFileCacheInformationEx, // q: SYSTEM_FILECACHE_INFORMATION; s (requires SeIncreaseQuotaPrivilege) (same as SystemFileCacheInformation)
        SystemThreadPriorityClientIdInformation, // s: SYSTEM_THREAD_CID_PRIORITY_INFORMATION (requires SeIncreaseBasePriorityPrivilege) // NtQuerySystemInformationEx
        SystemProcessorIdleCycleTimeInformation, // q: SYSTEM_PROCESSOR_IDLE_CYCLE_TIME_INFORMATION[] (EX in: USHORT ProcessorGroup) // NtQuerySystemInformationEx
        SystemVerifierCancellationInformation, // SYSTEM_VERIFIER_CANCELLATION_INFORMATION // name:wow64:whNT32QuerySystemVerifierCancellationInformation
        SystemProcessorPowerInformationEx, // not implemented
        SystemRefTraceInformation, // q; s: SYSTEM_REF_TRACE_INFORMATION // ObQueryRefTraceInformation
        SystemSpecialPoolInformation, // q; s: SYSTEM_SPECIAL_POOL_INFORMATION (requires SeDebugPrivilege) // MmSpecialPoolTag, then MmSpecialPoolCatchOverruns != 0
        SystemProcessIdInformation, // q: SYSTEM_PROCESS_ID_INFORMATION
        SystemErrorPortInformation, // s (requires SeTcbPrivilege)
        SystemBootEnvironmentInformation, // q: SYSTEM_BOOT_ENVIRONMENT_INFORMATION // 90
        SystemHypervisorInformation, // q: SYSTEM_HYPERVISOR_QUERY_INFORMATION
        SystemVerifierInformationEx, // q; s: SYSTEM_VERIFIER_INFORMATION_EX
        SystemTimeZoneInformation, // q; s: RTL_TIME_ZONE_INFORMATION (requires SeTimeZonePrivilege)
        SystemImageFileExecutionOptionsInformation, // s: SYSTEM_IMAGE_FILE_EXECUTION_OPTIONS_INFORMATION (requires SeTcbPrivilege)
        SystemCoverageInformation, // q: COVERAGE_MODULES s: COVERAGE_MODULE_REQUEST // ExpCovQueryInformation (requires SeDebugPrivilege)
        SystemPrefetchPatchInformation, // SYSTEM_PREFETCH_PATCH_INFORMATION
        SystemVerifierFaultsInformation, // s: SYSTEM_VERIFIER_FAULTS_INFORMATION (requires SeDebugPrivilege)
        SystemSystemPartitionInformation, // q: SYSTEM_SYSTEM_PARTITION_INFORMATION
        SystemSystemDiskInformation, // q: SYSTEM_SYSTEM_DISK_INFORMATION
        SystemProcessorPerformanceDistribution, // q: SYSTEM_PROCESSOR_PERFORMANCE_DISTRIBUTION (EX in: USHORT ProcessorGroup) // NtQuerySystemInformationEx // 100
        SystemNumaProximityNodeInformation, // q; s: SYSTEM_NUMA_PROXIMITY_MAP
        SystemDynamicTimeZoneInformation, // q; s: RTL_DYNAMIC_TIME_ZONE_INFORMATION (requires SeTimeZonePrivilege)
        SystemCodeIntegrityInformation, // q: SYSTEM_CODEINTEGRITY_INFORMATION // SeCodeIntegrityQueryInformation
        SystemProcessorMicrocodeUpdateInformation, // s: SYSTEM_PROCESSOR_MICROCODE_UPDATE_INFORMATION
        SystemProcessorBrandString, // q: CHAR[] // HaliQuerySystemInformation -> HalpGetProcessorBrandString, info class 23
        SystemVirtualAddressInformation, // q: SYSTEM_VA_LIST_INFORMATION[]; s: SYSTEM_VA_LIST_INFORMATION[] (requires SeIncreaseQuotaPrivilege) // MmQuerySystemVaInformation
        SystemLogicalProcessorAndGroupInformation, // q: SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX (EX in: LOGICAL_PROCESSOR_RELATIONSHIP RelationshipType) // since WIN7 // NtQuerySystemInformationEx // KeQueryLogicalProcessorRelationship
        SystemProcessorCycleTimeInformation, // q: SYSTEM_PROCESSOR_CYCLE_TIME_INFORMATION[] (EX in: USHORT ProcessorGroup) // NtQuerySystemInformationEx
        SystemStoreInformation, // q; s: SYSTEM_STORE_INFORMATION (requires SeProfileSingleProcessPrivilege) // SmQueryStoreInformation
        SystemRegistryAppendString, // s: SYSTEM_REGISTRY_APPEND_STRING_PARAMETERS // 110
        SystemAitSamplingValue, // s: ULONG (requires SeProfileSingleProcessPrivilege)
        SystemVhdBootInformation, // q: SYSTEM_VHD_BOOT_INFORMATION
        SystemCpuQuotaInformation, // q; s: PS_CPU_QUOTA_QUERY_INFORMATION
        SystemNativeBasicInformation, // q: SYSTEM_BASIC_INFORMATION
        SystemErrorPortTimeouts, // SYSTEM_ERROR_PORT_TIMEOUTS
        SystemLowPriorityIoInformation, // q: SYSTEM_LOW_PRIORITY_IO_INFORMATION
        SystemTpmBootEntropyInformation, // q: BOOT_ENTROPY_NT_RESULT // ExQueryBootEntropyInformation
        SystemVerifierCountersInformation, // q: SYSTEM_VERIFIER_COUNTERS_INFORMATION
        SystemPagedPoolInformationEx, // q: SYSTEM_FILECACHE_INFORMATION; s (requires SeIncreaseQuotaPrivilege) (info for WorkingSetTypePagedPool)
        SystemSystemPtesInformationEx, // q: SYSTEM_FILECACHE_INFORMATION; s (requires SeIncreaseQuotaPrivilege) (info for WorkingSetTypeSystemPtes) // 120
        SystemNodeDistanceInformation, // q: USHORT[4*NumaNodes] // (EX in: USHORT NodeNumber) // NtQuerySystemInformationEx
        SystemAcpiAuditInformation, // q: SYSTEM_ACPI_AUDIT_INFORMATION // HaliQuerySystemInformation -> HalpAuditQueryResults, info class 26
        SystemBasicPerformanceInformation, // q: SYSTEM_BASIC_PERFORMANCE_INFORMATION // name:wow64:whNtQuerySystemInformation_SystemBasicPerformanceInformation
        SystemQueryPerformanceCounterInformation, // q: SYSTEM_QUERY_PERFORMANCE_COUNTER_INFORMATION // since WIN7 SP1
        SystemSessionBigPoolInformation, // q: SYSTEM_SESSION_POOLTAG_INFORMATION // since WIN8
        SystemBootGraphicsInformation, // q; s: SYSTEM_BOOT_GRAPHICS_INFORMATION (kernel-mode only)
        SystemScrubPhysicalMemoryInformation, // q; s: MEMORY_SCRUB_INFORMATION
        SystemBadPageInformation, // SYSTEM_BAD_PAGE_INFORMATION
        SystemProcessorProfileControlArea, // q; s: SYSTEM_PROCESSOR_PROFILE_CONTROL_AREA
        SystemCombinePhysicalMemoryInformation, // s: MEMORY_COMBINE_INFORMATION, MEMORY_COMBINE_INFORMATION_EX, MEMORY_COMBINE_INFORMATION_EX2 // 130
        SystemEntropyInterruptTimingInformation, // q; s: SYSTEM_ENTROPY_TIMING_INFORMATION
        SystemConsoleInformation, // q; s: SYSTEM_CONSOLE_INFORMATION
        SystemPlatformBinaryInformation, // q: SYSTEM_PLATFORM_BINARY_INFORMATION (requires SeTcbPrivilege)
        SystemPolicyInformation, // q: SYSTEM_POLICY_INFORMATION (Warbird/Encrypt/Decrypt/Execute)
        SystemHypervisorProcessorCountInformation, // q: SYSTEM_HYPERVISOR_PROCESSOR_COUNT_INFORMATION
        SystemDeviceDataInformation, // q: SYSTEM_DEVICE_DATA_INFORMATION
        SystemDeviceDataEnumerationInformation, // q: SYSTEM_DEVICE_DATA_INFORMATION
        SystemMemoryTopologyInformation, // q: SYSTEM_MEMORY_TOPOLOGY_INFORMATION
        SystemMemoryChannelInformation, // q: SYSTEM_MEMORY_CHANNEL_INFORMATION
        SystemBootLogoInformation, // q: SYSTEM_BOOT_LOGO_INFORMATION // 140
        SystemProcessorPerformanceInformationEx, // q: SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION_EX // (EX in: USHORT ProcessorGroup) // NtQuerySystemInformationEx // since WINBLUE
        SystemCriticalProcessErrorLogInformation, // CRITICAL_PROCESS_EXCEPTION_DATA
        SystemSecureBootPolicyInformation, // q: SYSTEM_SECUREBOOT_POLICY_INFORMATION
        SystemPageFileInformationEx, // q: SYSTEM_PAGEFILE_INFORMATION_EX
        SystemSecureBootInformation, // q: SYSTEM_SECUREBOOT_INFORMATION
        SystemEntropyInterruptTimingRawInformation,
        SystemPortableWorkspaceEfiLauncherInformation, // q: SYSTEM_PORTABLE_WORKSPACE_EFI_LAUNCHER_INFORMATION
        SystemFullProcessInformation, // q: SYSTEM_PROCESS_INFORMATION with SYSTEM_PROCESS_INFORMATION_EXTENSION (requires admin)
        SystemKernelDebuggerInformationEx, // q: SYSTEM_KERNEL_DEBUGGER_INFORMATION_EX
        SystemBootMetadataInformation, // 150 // (requires SeTcbPrivilege)
        SystemSoftRebootInformation, // q: ULONG
        SystemElamCertificateInformation, // s: SYSTEM_ELAM_CERTIFICATE_INFORMATION
        SystemOfflineDumpConfigInformation, // q: OFFLINE_CRASHDUMP_CONFIGURATION_TABLE_V2
        SystemProcessorFeaturesInformation, // q: SYSTEM_PROCESSOR_FEATURES_INFORMATION
        SystemRegistryReconciliationInformation, // s: NULL (requires admin) (flushes registry hives)
        SystemEdidInformation, // q: SYSTEM_EDID_INFORMATION
        SystemManufacturingInformation, // q: SYSTEM_MANUFACTURING_INFORMATION // since THRESHOLD
        SystemEnergyEstimationConfigInformation, // q: SYSTEM_ENERGY_ESTIMATION_CONFIG_INFORMATION
        SystemHypervisorDetailInformation, // q: SYSTEM_HYPERVISOR_DETAIL_INFORMATION
        SystemProcessorCycleStatsInformation, // q: SYSTEM_PROCESSOR_CYCLE_STATS_INFORMATION (EX in: USHORT ProcessorGroup) // NtQuerySystemInformationEx // 160
        SystemVmGenerationCountInformation,
        SystemTrustedPlatformModuleInformation, // q: SYSTEM_TPM_INFORMATION
        SystemKernelDebuggerFlags, // SYSTEM_KERNEL_DEBUGGER_FLAGS
        SystemCodeIntegrityPolicyInformation, // q; s: SYSTEM_CODEINTEGRITYPOLICY_INFORMATION
        SystemIsolatedUserModeInformation, // q: SYSTEM_ISOLATED_USER_MODE_INFORMATION
        SystemHardwareSecurityTestInterfaceResultsInformation,
        SystemSingleModuleInformation, // q: SYSTEM_SINGLE_MODULE_INFORMATION
        SystemAllowedCpuSetsInformation, // s: SYSTEM_WORKLOAD_ALLOWED_CPU_SET_INFORMATION
        SystemVsmProtectionInformation, // q: SYSTEM_VSM_PROTECTION_INFORMATION (previously SystemDmaProtectionInformation)
        SystemInterruptCpuSetsInformation, // q: SYSTEM_INTERRUPT_CPU_SET_INFORMATION // 170
        SystemSecureBootPolicyFullInformation, // q: SYSTEM_SECUREBOOT_POLICY_FULL_INFORMATION
        SystemCodeIntegrityPolicyFullInformation,
        SystemAffinitizedInterruptProcessorInformation, // q: KAFFINITY_EX // (requires SeIncreaseBasePriorityPrivilege)
        SystemRootSiloInformation, // q: SYSTEM_ROOT_SILO_INFORMATION
        SystemCpuSetInformation, // q: SYSTEM_CPU_SET_INFORMATION // since THRESHOLD2
        SystemCpuSetTagInformation, // q: SYSTEM_CPU_SET_TAG_INFORMATION
        SystemWin32WerStartCallout,
        SystemSecureKernelProfileInformation, // q: SYSTEM_SECURE_KERNEL_HYPERGUARD_PROFILE_INFORMATION
        SystemCodeIntegrityPlatformManifestInformation, // q: SYSTEM_SECUREBOOT_PLATFORM_MANIFEST_INFORMATION // NtQuerySystemInformationEx // since REDSTONE
        SystemInterruptSteeringInformation, // q: in: SYSTEM_INTERRUPT_STEERING_INFORMATION_INPUT, out: SYSTEM_INTERRUPT_STEERING_INFORMATION_OUTPUT // NtQuerySystemInformationEx // 180
        SystemSupportedProcessorArchitectures, // p: in opt: HANDLE, out: SYSTEM_SUPPORTED_PROCESSOR_ARCHITECTURES_INFORMATION[] // NtQuerySystemInformationEx
        SystemMemoryUsageInformation, // q: SYSTEM_MEMORY_USAGE_INFORMATION
        SystemCodeIntegrityCertificateInformation, // q: SYSTEM_CODEINTEGRITY_CERTIFICATE_INFORMATION
        SystemPhysicalMemoryInformation, // q: SYSTEM_PHYSICAL_MEMORY_INFORMATION // since REDSTONE2
        SystemControlFlowTransition, // (Warbird/Encrypt/Decrypt/Execute)
        SystemKernelDebuggingAllowed, // s: ULONG
        SystemActivityModerationExeState, // SYSTEM_ACTIVITY_MODERATION_EXE_STATE
        SystemActivityModerationUserSettings, // SYSTEM_ACTIVITY_MODERATION_USER_SETTINGS
        SystemCodeIntegrityPoliciesFullInformation, // NtQuerySystemInformationEx
        SystemCodeIntegrityUnlockInformation, // SYSTEM_CODEINTEGRITY_UNLOCK_INFORMATION // 190
        SystemIntegrityQuotaInformation,
        SystemFlushInformation, // q: SYSTEM_FLUSH_INFORMATION
        SystemProcessorIdleMaskInformation, // q: ULONG_PTR[ActiveGroupCount] // since REDSTONE3
        SystemSecureDumpEncryptionInformation, // NtQuerySystemInformationEx
        SystemWriteConstraintInformation, // SYSTEM_WRITE_CONSTRAINT_INFORMATION
        SystemKernelVaShadowInformation, // SYSTEM_KERNEL_VA_SHADOW_INFORMATION
        SystemHypervisorSharedPageInformation, // SYSTEM_HYPERVISOR_SHARED_PAGE_INFORMATION // since REDSTONE4
        SystemFirmwareBootPerformanceInformation,
        SystemCodeIntegrityVerificationInformation, // SYSTEM_CODEINTEGRITYVERIFICATION_INFORMATION
        SystemFirmwarePartitionInformation, // SYSTEM_FIRMWARE_PARTITION_INFORMATION // 200
        SystemSpeculationControlInformation, // SYSTEM_SPECULATION_CONTROL_INFORMATION // (CVE-2017-5715) REDSTONE3 and above.
        SystemDmaGuardPolicyInformation, // SYSTEM_DMA_GUARD_POLICY_INFORMATION
        SystemEnclaveLaunchControlInformation, // SYSTEM_ENCLAVE_LAUNCH_CONTROL_INFORMATION
        SystemWorkloadAllowedCpuSetsInformation, // SYSTEM_WORKLOAD_ALLOWED_CPU_SET_INFORMATION // since REDSTONE5
        SystemCodeIntegrityUnlockModeInformation, // SYSTEM_CODEINTEGRITY_UNLOCK_INFORMATION
        SystemLeapSecondInformation, // SYSTEM_LEAP_SECOND_INFORMATION
        SystemFlags2Information, // q: SYSTEM_FLAGS_INFORMATION
        SystemSecurityModelInformation, // SYSTEM_SECURITY_MODEL_INFORMATION // since 19H1
        SystemCodeIntegritySyntheticCacheInformation, // NtQuerySystemInformationEx
        SystemFeatureConfigurationInformation, // q: in: SYSTEM_FEATURE_CONFIGURATION_QUERY, out: SYSTEM_FEATURE_CONFIGURATION_INFORMATION; s: SYSTEM_FEATURE_CONFIGURATION_UPDATE // NtQuerySystemInformationEx // since 20H1 // 210
        SystemFeatureConfigurationSectionInformation, // q: in: SYSTEM_FEATURE_CONFIGURATION_SECTIONS_REQUEST, out: SYSTEM_FEATURE_CONFIGURATION_SECTIONS_INFORMATION // NtQuerySystemInformationEx
        SystemFeatureUsageSubscriptionInformation, // q: SYSTEM_FEATURE_USAGE_SUBSCRIPTION_DETAILS; s: SYSTEM_FEATURE_USAGE_SUBSCRIPTION_UPDATE
        SystemSecureSpeculationControlInformation, // SECURE_SPECULATION_CONTROL_INFORMATION
        SystemSpacesBootInformation, // since 20H2
        SystemFwRamdiskInformation, // SYSTEM_FIRMWARE_RAMDISK_INFORMATION
        SystemWheaIpmiHardwareInformation,
        SystemDifSetRuleClassInformation, // SYSTEM_DIF_VOLATILE_INFORMATION
        SystemDifClearRuleClassInformation,
        SystemDifApplyPluginVerificationOnDriver, // SYSTEM_DIF_PLUGIN_DRIVER_INFORMATION
        SystemDifRemovePluginVerificationOnDriver, // SYSTEM_DIF_PLUGIN_DRIVER_INFORMATION // 220
        SystemShadowStackInformation, // SYSTEM_SHADOW_STACK_INFORMATION
        SystemBuildVersionInformation, // q: in: ULONG (LayerNumber), out: SYSTEM_BUILD_VERSION_INFORMATION // NtQuerySystemInformationEx // 222
        SystemPoolLimitInformation, // SYSTEM_POOL_LIMIT_INFORMATION (requires SeIncreaseQuotaPrivilege) // NtQuerySystemInformationEx
        SystemCodeIntegrityAddDynamicStore,
        SystemCodeIntegrityClearDynamicStores,
        SystemDifPoolTrackingInformation,
        SystemPoolZeroingInformation, // q: SYSTEM_POOL_ZEROING_INFORMATION
        SystemDpcWatchdogInformation, // q; s: SYSTEM_DPC_WATCHDOG_CONFIGURATION_INFORMATION
        SystemDpcWatchdogInformation2, // q; s: SYSTEM_DPC_WATCHDOG_CONFIGURATION_INFORMATION_V2
        SystemSupportedProcessorArchitectures2, // q: in opt: HANDLE, out: SYSTEM_SUPPORTED_PROCESSOR_ARCHITECTURES_INFORMATION[] // NtQuerySystemInformationEx // 230
        SystemSingleProcessorRelationshipInformation, // q: SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX // (EX in: PROCESSOR_NUMBER Processor) // NtQuerySystemInformationEx
        SystemXfgCheckFailureInformation, // q: SYSTEM_XFG_FAILURE_INFORMATION
        SystemIommuStateInformation, // SYSTEM_IOMMU_STATE_INFORMATION // since 22H1
        SystemHypervisorMinrootInformation, // SYSTEM_HYPERVISOR_MINROOT_INFORMATION
        SystemHypervisorBootPagesInformation, // SYSTEM_HYPERVISOR_BOOT_PAGES_INFORMATION
        SystemPointerAuthInformation, // SYSTEM_POINTER_AUTH_INFORMATION
        SystemSecureKernelDebuggerInformation, // NtQuerySystemInformationEx
        SystemOriginalImageFeatureInformation, // q: in: SYSTEM_ORIGINAL_IMAGE_FEATURE_INFORMATION_INPUT, out: SYSTEM_ORIGINAL_IMAGE_FEATURE_INFORMATION_OUTPUT // NtQuerySystemInformationEx
        SystemMemoryNumaInformation, // SYSTEM_MEMORY_NUMA_INFORMATION_INPUT, SYSTEM_MEMORY_NUMA_INFORMATION_OUTPUT // NtQuerySystemInformationEx
        SystemMemoryNumaPerformanceInformation, // SYSTEM_MEMORY_NUMA_PERFORMANCE_INFORMATION_INPUTSYSTEM_MEMORY_NUMA_PERFORMANCE_INFORMATION_INPUT, SYSTEM_MEMORY_NUMA_PERFORMANCE_INFORMATION_OUTPUT // since 24H2 // 240
        SystemCodeIntegritySignedPoliciesFullInformation,
        SystemSecureCoreInformation, // SystemSecureSecretsInformation
        SystemTrustedAppsRuntimeInformation, // SYSTEM_TRUSTEDAPPS_RUNTIME_INFORMATION
        SystemBadPageInformationEx, // SYSTEM_BAD_PAGE_INFORMATION
        SystemResourceDeadlockTimeout, // ULONG
        SystemBreakOnContextUnwindFailureInformation, // ULONG (requires SeDebugPrivilege)
        SystemOslRamdiskInformation, // SYSTEM_OSL_RAMDISK_INFORMATION
        MaxSystemInfoClass
    } SYSTEM_INFORMATION_CLASS;

    typedef enum _KTHREAD_STATE
    {
        Initialized = 0,
        Ready = 1,
        Running = 2,
        Standby = 3,
        Terminated = 4,
        Waiting = 5,
        Transition = 6,
        DeferredReady = 7,
        GateWait = 8
    } KTHREAD_STATE;

    typedef enum _KWAIT_REASON
    {
        Executive = 0,
        FreePage = 1,
        PageIn = 2,
        PoolAllocation = 3,
        DelayExecution = 4,
        Suspended = 5,
        UserRequest = 6,
        WrExecutive = 7,
        WrFreePage = 8,
        WrPageIn = 9,
        WrPoolAllocation = 10,
        WrDelayExecution = 11,
        WrSuspended = 12,
        WrUserRequest = 13,
        WrEventPair = 14,
        WrQueue = 15,
        WrLpcReceive = 16,
        WrLpcReply = 17,
        WrVirtualMemory = 18,
        WrPageOut = 19,
        WrRendezvous = 20,
        Spare2 = 21,
        Spare3 = 22,
        Spare4 = 23,
        Spare5 = 24,
        WrCalloutStack = 25,
        WrKernel = 26,
        WrResource = 27,
        WrPushLock = 28,
        WrMutex = 29,
        WrQuantumEnd = 30,
        WrDispatchInt = 31,
        WrPreempted = 32,
        WrYieldExecution = 33,
        WrFastMutex = 34,
        WrGuardedMutex = 35,
        WrRundown = 36,
        MaximumWaitReason = 37
    } KWAIT_REASON;

    typedef struct _CLIENT_ID {
        HANDLE UniqueProcess;
        HANDLE UniqueThread;
    } CLIENT_ID;

    typedef struct _UNICODE_STRING {
        USHORT Length;
        USHORT MaximumLength;
        PWSTR  Buffer;
    } UNICODE_STRING;
    typedef UNICODE_STRING* PUNICODE_STRING;
    typedef const UNICODE_STRING* PCUNICODE_STRING;

    typedef LONG KPRIORITY;

    // https://ntdoc.m417z.com/system_thread_information
    typedef struct _SYSTEM_THREAD_INFORMATION
    {
        LARGE_INTEGER KernelTime;
        LARGE_INTEGER UserTime;
        LARGE_INTEGER CreateTime;
        ULONG WaitTime;
        PVOID StartAddress;
        CLIENT_ID ClientId;
        KPRIORITY Priority;
        KPRIORITY BasePriority;
        ULONG ContextSwitches;
        KTHREAD_STATE ThreadState;
        KWAIT_REASON WaitReason;
    } SYSTEM_THREAD_INFORMATION, * PSYSTEM_THREAD_INFORMATION;

    // https://ntdoc.m417z.com/system_process_information
    typedef struct _SYSTEM_PROCESS_INFORMATION
    {
        ULONG NextEntryOffset;
        ULONG NumberOfThreads;
        LARGE_INTEGER WorkingSetPrivateSize; // since VISTA
        ULONG HardFaultCount; // since WIN7
        ULONG NumberOfThreadsHighWatermark; // since WIN7
        ULONGLONG CycleTime; // since WIN7
        LARGE_INTEGER CreateTime;
        LARGE_INTEGER UserTime;
        LARGE_INTEGER KernelTime;
        UNICODE_STRING ImageName;
        KPRIORITY BasePriority;
        HANDLE UniqueProcessId;
        HANDLE UniqueParentProcessId;
        ULONG HandleCount;
        ULONG SessionId;
        ULONG_PTR UniqueProcessKey; // since VISTA (requires SystemExtendedProcessInformation)
        SIZE_T PeakVirtualSize;
        SIZE_T VirtualSize;
        ULONG PageFaultCount;
        SIZE_T PeakWorkingSetSize;
        SIZE_T WorkingSetSize;
        SIZE_T QuotaPeakPagedPoolUsage;
        SIZE_T QuotaPagedPoolUsage;
        SIZE_T QuotaPeakNonPagedPoolUsage;
        SIZE_T QuotaNonPagedPoolUsage;
        SIZE_T PagefileUsage;
        SIZE_T PeakPagefileUsage;
        SIZE_T PrivatePageCount;
        LARGE_INTEGER ReadOperationCount;
        LARGE_INTEGER WriteOperationCount;
        LARGE_INTEGER OtherOperationCount;
        LARGE_INTEGER ReadTransferCount;
        LARGE_INTEGER WriteTransferCount;
        LARGE_INTEGER OtherTransferCount;
        SYSTEM_THREAD_INFORMATION Threads[1]; // SystemProcessInformation
        // SYSTEM_EXTENDED_THREAD_INFORMATION Threads[1]; // SystemExtendedProcessinformation
        // SYSTEM_EXTENDED_THREAD_INFORMATION + SYSTEM_PROCESS_INFORMATION_EXTENSION // SystemFullProcessInformation
    } SYSTEM_PROCESS_INFORMATION, * PSYSTEM_PROCESS_INFORMATION;

    typedef NTSTATUS(NTAPI* pFnNtQuerySystemInformation)(
        IN SYSTEM_INFORMATION_CLASS SystemInformationClass,
        OUT PVOID SystemInformation,
        IN ULONG SystemInformationLength,
        OUT PULONG ReturnLength OPTIONAL
        );

    pFnNtQuerySystemInformation _NtQuerySystemInformation;

    static PROC_TOKEN_GROUPS _GetProcessTokenGroups(HANDLE hProcess);

    NTSTATUS NTAPI NtQuerySystemInformation(
        IN SYSTEM_INFORMATION_CLASS SystemInformationClass,
        OUT PVOID SystemInformation,
        IN ULONG SystemInformationLength,
        OUT PULONG ReturnLength OPTIONAL
    )
    {
        if (!_NtQuerySystemInformation)
        {
            do
            {
                HMODULE hModule = ::GetModuleHandle(_T("Ntdll.dll"));
                if (!hModule)
                {
                    break;
                }

                _NtQuerySystemInformation = (pFnNtQuerySystemInformation)::GetProcAddress(
                    hModule,
                    "NtQuerySystemInformation"
                );
                if (!_NtQuerySystemInformation)
                {
                    break;
                }

            } while (false);
        }

        if (!_NtQuerySystemInformation)
        {
            return 0;
        }

        return _NtQuerySystemInformation(
            SystemInformationClass,
            SystemInformation,
            SystemInformationLength,
            ReturnLength
        );
    }

    std::string _WStrToMultiStr(UINT CodePage, const std::wstring& str)
    {
        //计算缓冲区所需的字节长度
        int cbMultiByte = ::WideCharToMultiByte(CodePage, 0, str.c_str(), -1, NULL, 0, NULL, NULL);
        std::string strResult(cbMultiByte, 0);

        //成功则返回写入到指示的缓冲区的字节数
        size_t nConverted = ::WideCharToMultiByte(CodePage, 0, str.c_str(), (int)str.size(), &strResult[0], (int)strResult.size(), NULL, NULL);

        //调整内容长度
        strResult.resize(nConverted);
        return strResult;
    }

    std::wstring _MultiStrToWStr(UINT CodePage, const std::string& str)
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

    std::string WStrToAStr(const std::wstring& str)
    {
        return _WStrToMultiStr(CP_ACP, str);
    }

    std::string WStrToU8Str(const std::wstring& str)
    {
        return _WStrToMultiStr(CP_UTF8, str);
    }

    _tstring WStrToTStr(const std::wstring& str)
    {
#ifdef _UNICODE
        return str;
#else
        return _WStrToMultiStr(CP_ACP, str);
#endif
    }

    std::wstring AStrToWStr(const std::string& str)
    {
        return _MultiStrToWStr(CP_ACP, str);
    }

    std::string AStrToU8Str(const std::string& str)
    {
        return WStrToU8Str(AStrToWStr(str));
    }

    _tstring AStrToTStr(const std::string& str)
    {
#ifdef _UNICODE
        return _MultiStrToWStr(CP_ACP, str);
#else
        return str;
#endif
    }

    std::wstring U8StrToWStr(const std::string& str)
    {
        return _MultiStrToWStr(CP_UTF8, str);
    }

    std::string U8StrToAStr(const std::string& str)
    {
        return WStrToAStr(U8StrToWStr(str));
    }

    _tstring U8StrToTStr(const std::string& str)
    {
#ifdef _UNICODE
        return _MultiStrToWStr(CP_UTF8, str);
#else
        return WStrToAStr(U8StrToWStr(str));
#endif
    }

    std::string TStrToAStr(const _tstring& str)
    {
#ifdef _UNICODE
        return _WStrToMultiStr(CP_ACP, str);
#else
        return str;
#endif
    }

    std::wstring TStrToWStr(const _tstring& str)
    {
#ifdef _UNICODE
        return str;
#else
        return AStrToWStr(str);
#endif
    }

    std::string TStrToU8Str(const _tstring& str)
    {
#ifdef _UNICODE
        return WStrToU8Str(str);
#else
        return WStrToU8Str(AStrToWStr(str));
#endif
    }

    _MODULE_INFO::_MODULE_INFO() :
        th32ProcessID(0),
        GlblcntUsage(0),
        modBaseAddr(0),
        modBaseSize(0),
        hModule(NULL)
    {

    }

    _LANGANDCODEPAGE::_LANGANDCODEPAGE() : wLangId(0), wCodePage(0)
    {

    }

    bool _LANGANDCODEPAGE::operator < (const _LANGANDCODEPAGE& r) const
    {
        if (this->wLangId < r.wLangId)
        {
            return true;
        }

        if (this->wCodePage < r.wCodePage)
        {
            return true;
        }

        return false;
    }

    bool _LANGANDCODEPAGE::operator == (const _LANGANDCODEPAGE& r) const
    {
        return this->wLangId == r.wLangId && this->wCodePage == r.wCodePage;
    }

    CPEHeader::CPEHeader()
        :
        m_pDosHeader(nullptr),
        m_pNtHeaders32(nullptr),
        m_pNtHeaders64(nullptr),
        m_NtHeadersMagic(0)
    {

    }

    CPEHeader::CPEHeader(const CPEHeader& r) noexcept
        :
        m_pDosHeader(nullptr),
        m_pNtHeaders32(nullptr),
        m_pNtHeaders64(nullptr),
        m_NtHeadersMagic(0)
    {
        CopyFrom(r);
    }

    CPEHeader::CPEHeader(CPEHeader&& r) noexcept
        :
        m_pDosHeader(nullptr),
        m_pNtHeaders32(nullptr),
        m_pNtHeaders64(nullptr),
        m_NtHeadersMagic(0)
    {
        CopyFrom(r);
        r.Release();
    }

    CPEHeader& CPEHeader::operator = (const CPEHeader& r) noexcept
    {
        if (&r != this)
        {
            CopyFrom(r);
        }

        return *this;
    }

    CPEHeader& CPEHeader::operator = (CPEHeader&& r) noexcept
    {
        if (&r != this)
        {
            Release();

            m_NtHeadersMagic = r.m_NtHeadersMagic;
            m_pDosHeader = r.m_pDosHeader;
            m_pNtHeaders32 = r.m_pNtHeaders32;
            m_pNtHeaders64 = r.m_pNtHeaders64;

            r.Release();
        }

        return *this;
    }

    CPEHeader::~CPEHeader()
    {
        Release();
    }

    void CPEHeader::CopyFrom(const CPEHeader& r)
    {
        if (&r == this)
        {
            return;
        }

        Release();

        m_NtHeadersMagic = r.m_NtHeadersMagic;

        if (r.m_pDosHeader)
        {
            m_pDosHeader = new (std::nothrow) IMAGE_DOS_HEADER(*r.m_pDosHeader);
        }

        if (r.m_pNtHeaders32)
        {
            m_pNtHeaders32 = new (std::nothrow) IMAGE_NT_HEADERS32(*r.m_pNtHeaders32);
        }

        if (r.m_pNtHeaders64)
        {
            m_pNtHeaders64 = new (std::nothrow) IMAGE_NT_HEADERS64(*r.m_pNtHeaders64);
        }
    }

    void CPEHeader::Release()
    {
        if (m_pDosHeader)
        {
            delete m_pDosHeader;
            m_pDosHeader = nullptr;
        }

        if (m_pNtHeaders32)
        {
            delete m_pNtHeaders32;
            m_pNtHeaders32 = nullptr;
        }

        if (m_pNtHeaders64)
        {
            delete m_pNtHeaders64;
            m_pNtHeaders64 = nullptr;
        }
    }

    bool CPEHeader::LoadFromFile(const _tstring& strFile)
    {
        Release();

        HMODULE hModule = NULL;
        LPVOID OldValue = NULL;
        BOOL isDisableWow64Fs = ::Wow64DisableWow64FsRedirection(&OldValue);
        bool fSuccess = false;

        do
        {
            DWORD dwFlags = LOAD_LIBRARY_AS_DATAFILE | LOAD_LIBRARY_AS_IMAGE_RESOURCE;
            hModule = ::LoadLibraryEx(
                strFile.c_str(),
                0,
                dwFlags
            );

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

            PIMAGE_NT_HEADERS pNtHeader = (IMAGE_NT_HEADERS*)((BYTE*)(pDosHeader)+(DWORD)(pDosHeader->e_lfanew));
            if (IMAGE_NT_SIGNATURE != pNtHeader->Signature)
            {
                break;
            }

            // 检查 是否为 32位程序可选头
            if (IMAGE_NT_OPTIONAL_HDR32_MAGIC == pNtHeader->OptionalHeader.Magic)
            {
                m_NtHeadersMagic = IMAGE_NT_OPTIONAL_HDR32_MAGIC;
                m_pNtHeaders32 = new (std::nothrow) IMAGE_NT_HEADERS32;
                if (m_pNtHeaders32)
                {
                    *m_pNtHeaders32 = *(PIMAGE_NT_HEADERS32)pNtHeader;
                }
            }
            // 检查 是否为 64位程序可选头
            else if (IMAGE_NT_OPTIONAL_HDR64_MAGIC == pNtHeader->OptionalHeader.Magic)
            {
                m_NtHeadersMagic = IMAGE_NT_OPTIONAL_HDR64_MAGIC;

                m_pNtHeaders64 = new (std::nothrow) IMAGE_NT_HEADERS64;
                if (m_pNtHeaders64)
                {
                    *m_pNtHeaders64 = *(PIMAGE_NT_HEADERS64)pNtHeader;
                }
            }
            else
            {
                break;
            }

            m_pDosHeader = new (std::nothrow) IMAGE_DOS_HEADER;
            if (m_pDosHeader)
            {
                *m_pDosHeader = *(PIMAGE_DOS_HEADER)pImageData;
            }

            fSuccess = true;

        } while (false);

        if (isDisableWow64Fs)
        {
            ::Wow64RevertWow64FsRedirection(OldValue);
        }

        if (NULL != hModule)
        {
            ::FreeLibrary(hModule);
        }

        return fSuccess;
    }

    bool CPEHeader::LoadFromModule(HMODULE hModule)
    {
        Release();

        bool fSuccess = false;

        do
        {
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

            PIMAGE_NT_HEADERS pNtHeader = (IMAGE_NT_HEADERS*)((BYTE*)(pDosHeader)+(DWORD)(pDosHeader->e_lfanew));
            if (IMAGE_NT_SIGNATURE != pNtHeader->Signature)
            {
                break;
            }

            // 检查 是否为 32位程序可选头
            if (IMAGE_NT_OPTIONAL_HDR32_MAGIC == pNtHeader->OptionalHeader.Magic)
            {
                m_NtHeadersMagic = IMAGE_NT_OPTIONAL_HDR32_MAGIC;
                m_pNtHeaders32 = new (std::nothrow) IMAGE_NT_HEADERS32;
                if (m_pNtHeaders32)
                {
                    *m_pNtHeaders32 = *(PIMAGE_NT_HEADERS32)pNtHeader;
                }
            }
            // 检查 是否为 64位程序可选头
            else if (IMAGE_NT_OPTIONAL_HDR64_MAGIC == pNtHeader->OptionalHeader.Magic)
            {
                m_NtHeadersMagic = IMAGE_NT_OPTIONAL_HDR64_MAGIC;

                m_pNtHeaders64 = new (std::nothrow) IMAGE_NT_HEADERS64;
                if (m_pNtHeaders64)
                {
                    *m_pNtHeaders64 = *(PIMAGE_NT_HEADERS64)pNtHeader;
                }
            }
            else
            {
                break;
            }

            m_pDosHeader = new (std::nothrow) IMAGE_DOS_HEADER;
            if (m_pDosHeader)
            {
                *m_pDosHeader = *(PIMAGE_DOS_HEADER)pImageData;
            }

            fSuccess = true;

        } while (false);

        return fSuccess;
    }

    WORD CPEHeader::GetMachine()
    {
        if (IMAGE_NT_OPTIONAL_HDR32_MAGIC == m_NtHeadersMagic && m_pNtHeaders32)
        {
            return m_pNtHeaders32->FileHeader.Machine;
        }

        if (IMAGE_NT_OPTIONAL_HDR64_MAGIC == m_NtHeadersMagic && m_pNtHeaders64)
        {
            return m_pNtHeaders64->FileHeader.Machine;
        }

        return 0;
    }

    DWORD CPEHeader::GetSubsystemType()
    {
        if (IMAGE_NT_OPTIONAL_HDR32_MAGIC == m_NtHeadersMagic && m_pNtHeaders32)
        {
            return m_pNtHeaders32->OptionalHeader.Subsystem;
        }

        if (IMAGE_NT_OPTIONAL_HDR64_MAGIC == m_NtHeadersMagic && m_pNtHeaders64)
        {
            return m_pNtHeaders64->OptionalHeader.Subsystem;
        }

        return 0;
    }

    bool CPEHeader::IsX86()
    {
        return IMAGE_FILE_MACHINE_I386 == GetMachine();
    }

    bool CPEHeader::IsX64()
    {
        return IMAGE_FILE_MACHINE_AMD64 == GetMachine();
    }

    bool CPEHeader::IsARM32()
    {
        WORD wMachine = GetMachine();
        return wMachine >= IMAGE_FILE_MACHINE_ARM && wMachine <= IMAGE_FILE_MACHINE_ARMNT;
    }

    bool CPEHeader::IsARM64()
    {
        return IMAGE_FILE_MACHINE_ARM64 == GetMachine();
    }

    bool CPEHeader::IsWindowsGUI()
    {
        return IMAGE_SUBSYSTEM_WINDOWS_GUI == GetSubsystemType();
    }

    bool CPEHeader::IsWindowsCUI()
    {
        return IMAGE_SUBSYSTEM_WINDOWS_CUI == GetSubsystemType();
    }

    CVersionNumber::CVersionNumber()
        :m_nVer{ 0 }
    {
    };

    CVersionNumber::CVersionNumber(const _tstring& strVer)
        :
        m_nVer{ 0 }
    {
        _stscanf_s(strVer.c_str(), _T("%hd.%hd.%hd.%hd"), &m_nVer[0], &m_nVer[1], &m_nVer[2], &m_nVer[3]);
    }

    CVersionNumber::CVersionNumber(WORD v1, WORD v2, WORD v3, WORD v4)
        :m_nVer{ v1, v2, v3, v4 }
    {
    }

    CVersionNumber& CVersionNumber::operator = (const _tstring& strVer)
    {
        _stscanf_s(strVer.c_str(), _T("%hd.%hd.%hd.%hd"), &m_nVer[0], &m_nVer[1], &m_nVer[2], &m_nVer[3]);
        return *this;
    }

    int CVersionNumber::_Compare(const CVersionNumber& ref) const
    {
        for (int i = 0; i < _countof(m_nVer); i++)
        {
            if (m_nVer[i] != ref.m_nVer[i])
            {
                return (m_nVer[i] > ref.m_nVer[i] ? 1 : -1);
            }
        }

        return 0;
    }

    _tstring CVersionNumber::GetString() const
    {
        TCHAR szBuf[MAX_PATH] = { 0 };
        (void)::StringCchPrintf(szBuf, _countof(szBuf), _T("%hd.%hd.%hd.%hd"),
            m_nVer[0],
            m_nVer[1],
            m_nVer[2],
            m_nVer[3]
        );

        return szBuf;
    }

    bool CVersionNumber::IsEmpty() const
    {
        for (const auto& item : m_nVer)
        {
            if (0 != item)
            {
                return false;
            }
        }

        return true;
    }

    void CVersionNumber::Clear()
    {
        for (auto& item : m_nVer)
        {
            item = 0;
        }
    }

    bool CVersionNumber::operator == (const CVersionNumber& ref)
    {
        return _Compare(ref) == 0;
    }

    bool CVersionNumber::operator != (const CVersionNumber& ref)
    {
        return _Compare(ref) != 0;
    }

    bool CVersionNumber::operator < (const CVersionNumber& ref)
    {
        return _Compare(ref) < 0;
    }

    bool CVersionNumber::operator <= (const CVersionNumber& ref)
    {
        return _Compare(ref) <= 0;
    }

    bool CVersionNumber::operator > (const CVersionNumber& ref)
    {
        return _Compare(ref) > 0;
    }

    bool CVersionNumber::operator >= (const CVersionNumber& ref)
    {
        return _Compare(ref) >= 0;
    }

    //
    // @brief: 杀死进程节点
    // @param: procNode             进程节点
    // @ret: bool                   操作是否成功
    bool _KillProcessNode(const PROC_NODE& procNode);

    //
    // @brief: 挂起进程节点
    // @param: procNode             进程节点
    // @ret: bool                   检查结果
    bool _SuspendProcessNode(const PROC_NODE& procNode);

    //
    // @brief: 恢复进程节点
    // @param: procNode             进程节点
    // @ret: bool                   检查结果
    bool _ResumeProcessNode(const PROC_NODE& procNode);

    //
    // @brief: 获取指定进程的进程节点
    // @param: procInfos            进程信息
    // @param: dwPID                进程ID
    // @ret: PROC_LIST  进程信息
    PROC_NODE _GetProcessNode(
        const PROC_INFO_LIST& procInfos,
        DWORD dwPID
    );

    //
    // @brief: 复制指定进程名的令牌
    // @param: strName              进程名
    // @param: dwSessionId          会话ID
    // @param: dwDesiredAccess      权限
    // @param: phToken              输出令牌缓冲
    // @ret: DWORD                  操作结果错误码
    DWORD _DuplicateProcessToken(
        const _tstring& strName,
        DWORD dwSessionId,
        DWORD dwDesiredAccess,
        PHANDLE phToken
    );

    // @brief: 创建带有UI权限的进程
    // @param: strExePath           可执行文件路径
    // @param: strCommand           命令参数
    // @param: strCurDir            程序的当前目录
    // @param: fUIAccess            是否带有UI权限
    // @ret: DWORD                  操作结果错误码
    DWORD _CreateProcessWithUIAccess(
        const _tstring& strExePath,
        const _tstring& strCommand,
        const _tstring& strCurDir = _T(""),
        bool fUIAccess = true
    );

    // 枚举进程窗口
    void _EnumProcessWindows(WND_INFO_LIST& infos, HWND hWnd, DWORD dwPid);

    // 获取子窗口节点
    WND_NODE _GetWindowNode(HWND hWnd, MODULE_INFO_LIST& modules);

    bool EnablePrivileges(
        HANDLE hProcess,
        LPCTSTR lpPrivilegesName
    )
    {
        HANDLE hToken = NULL;
        LUID luidValue = { 0 };
        TOKEN_PRIVILEGES tokenPrivileges = { 0 };
        BOOL isSuccess = FALSE;

        do
        {
            if (!::OpenProcessToken(hProcess, TOKEN_ADJUST_PRIVILEGES, &hToken))
            {
                break;
            }

            if (!::LookupPrivilegeValue(NULL, lpPrivilegesName, &luidValue))
            {
                break;
            }

            tokenPrivileges.PrivilegeCount = 1;
            tokenPrivileges.Privileges[0].Luid = luidValue;
            tokenPrivileges.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

            if (!::AdjustTokenPrivileges(hToken, FALSE, &tokenPrivileges, 0, NULL, NULL))
            {
                break;
            }

            isSuccess = true;

        } while (false);

        if (hToken)
        {
            ::CloseHandle(hToken);
        }

        return isSuccess;
    }

    // 操作系统 版本号
    // Windows 11               10.0*
    // Windows 10               10.0*
    // Windows Server 2022      10.0*
    // Windows Server 2019      10.0*
    // Windows Server 2016      10.0*
    // Windows 8.1              6.3*
    // Windows Server 2012 R2   6.3*
    // Windows 8                6.2
    // Windows Server 2012      6.2
    // Windows 7                6.1
    // Windows Server 2008 R2   6.1
    // Windows Server 2008      6.0
    // Windows Vista            6.0
    // Windows Server 2003 R2   5.2
    // Windows Server 2003      5.2
    // Windows XP 64 位版本     5.2
    // Windows XP               5.1
    // Windows 2000             5.0
    static bool _GetNtVersionNumbers(DWORD* pMajor, DWORD* pMinor, DWORD* pBuild)
    {
        typedef VOID(NTAPI* NTPROC)(DWORD* dwMajor, DWORD* dwMinor, DWORD* dwBuild);
        HMODULE hModule = NULL;
        static NTPROC ProcAddress = NULL;
        bool fResult = false;

        if (NULL == ProcAddress)
        {
            hModule = ::LoadLibrary(_T("ntdll.dll"));
            if (NULL != hModule)
            {
                ProcAddress = (NTPROC)::GetProcAddress(hModule, "RtlGetNtVersionNumbers");
            }
        }

        if (NULL != ProcAddress)
        {
            fResult = true;

            ProcAddress(pMajor, pMinor, pBuild);
            *pBuild &= 0xffff;
        }

        if (NULL != hModule)
        {
            ::FreeLibrary(hModule);
        }

        return fResult;
    }

    static bool IsWindowsXPOrServer2003()
    {
        DWORD dwMajor = 0;
        DWORD dwMinor = 0;
        DWORD dwBuild = 0;

        if (!_GetNtVersionNumbers(&dwMajor, &dwMinor, &dwBuild))
        {
            return false;
        }

        if (dwMajor == 5 && dwMinor >= 1)
        {
            return true;
        }

        return false;
    }

    static DWORD GetQueryAccess()
    {
        static DWORD dwDesiredAccess = PROCESS_QUERY_LIMITED_INFORMATION;
        if (0 != dwDesiredAccess)
        {
            return dwDesiredAccess;
        }

        if (IsWindowsXPOrServer2003())
        {
            dwDesiredAccess = PROCESS_QUERY_INFORMATION;
        }

        return dwDesiredAccess;
    }

    bool CreateProcessNormal(
        const _tstring& strExePath/* = _T("")*/,
        const _tstring& strCommand/* = _T("")*/,
        const _tstring& strCurDir/* = _T("")*/,
        bool bShow,
        bool bWait
    )
    {
        SECURITY_ATTRIBUTES   sa = { 0 };
        STARTUPINFO si = { 0 };
        PROCESS_INFORMATION pi = { 0 };

        sa.bInheritHandle = TRUE;
        sa.lpSecurityDescriptor = NULL;
        sa.nLength = sizeof(sa);

        si.cb = sizeof(STARTUPINFO);
        si.dwFlags = STARTF_USESHOWWINDOW;
        si.wShowWindow = bShow ? SW_SHOW : SW_HIDE;
        si.hStdInput = NULL;
        si.hStdOutput = NULL;
        si.hStdError = NULL;

        _tstring exePath = strExePath;
        _tstring exeCommand = strCommand;
        _tstring exeCurDir = strCurDir;

        LPCTSTR lpApplicationName = NULL;
        LPTSTR lpCommandLine = NULL;
        LPCTSTR lpCurrentDir = NULL;

        if (!exePath.empty())
        {
            lpApplicationName = exePath.c_str();
        }

        if (!exeCommand.empty())
        {
            lpCommandLine = (LPTSTR)exeCommand.c_str();
        }

        if (!exeCurDir.empty())
        {
            lpCurrentDir = exeCurDir.c_str();
        }

        if (!::CreateProcess(lpApplicationName, lpCommandLine, NULL, NULL, TRUE, NORMAL_PRIORITY_CLASS, NULL, lpCurrentDir, &si, &pi))
        {
            return false;
        }

        if (bWait)
        {
            ::WaitForSingleObject(pi.hProcess, INFINITE);
        }

        ::CloseHandle(pi.hProcess);
        ::CloseHandle(pi.hThread);

        return true;
    }

    _tstring GetPath(DWORD dwPID, bool bNtPath)
    {
        _tstring strPath;
        HANDLE hProcess = NULL;
        TCHAR szBuffer[MAX_PATH] = { 0 };
        DWORD dwSize = _countof(szBuffer);

        do
        {
            hProcess = ::OpenProcess(GetQueryAccess(), FALSE, dwPID);
            if (NULL == hProcess)
            {
                break;
            }

            // 句柄必须具有 PROCESS_QUERY_INFORMATION 或 PROCESS_QUERY_LIMITED_INFORMATION 访问权限。
            // Windows Server 2003 和 Windows XP： 句柄必须具有 PROCESS_QUERY_INFORMATION 访问权限。
            if (0 == ::GetProcessImageFileName(hProcess, szBuffer, dwSize))
            {
                break;
            }

            if (bNtPath)
            {
                strPath = DosPathToNtPath(szBuffer);
            }
            else
            {
                strPath = szBuffer;
            }

        } while (false);

        if (NULL != hProcess)
        {
            ::CloseHandle(hProcess);
        }

        return strPath;
    }

    _tstring GetPathEx(DWORD dwPID, bool bNtPath)
    {
        _tstring strPath;
        HANDLE hProcess = NULL;
        TCHAR szBuffer[MAX_PATH] = { 0 };
        DWORD dwSize = _countof(szBuffer);

        do
        {
            hProcess = ::OpenProcess(GetQueryAccess(), FALSE, dwPID);
            if (NULL == hProcess)
            {
                break;
            }

            // 最低受支持的客户端: Windows Vista [仅限桌面应用]
            // 最低受支持的服务器: Windows Server 2008 [仅限桌面应用]
            // 进程句柄权限: PROCESS_QUERY_INFORMATION或PROCESS_QUERY_LIMITED_INFORMATION
            if (!::QueryFullProcessImageName(hProcess, bNtPath ? 0 : PROCESS_NAME_NATIVE, szBuffer, &dwSize))
            {
                break;
            }

            strPath = szBuffer;

        } while (false);

        if (NULL != hProcess)
        {
            ::CloseHandle(hProcess);
        }

        return strPath;
    }

    _tstring GetParentPath(DWORD dwPID, bool bNtPath)
    {
        _tstring strPath;
        HANDLE hProcessSnap = NULL;
        PROCESSENTRY32 pe32 = { 0 };
        pe32.dwSize = sizeof(pe32);

        do
        {
            hProcessSnap = ::CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
            if (INVALID_HANDLE_VALUE == hProcessSnap)
            {
                break;
            }

            if (!::Process32First(hProcessSnap, &pe32))
            {
                break;
            }

            do
            {
                if (pe32.th32ProcessID == dwPID)
                {
                    strPath = GetPath(pe32.th32ParentProcessID, bNtPath);
                    break;
                }

            } while (::Process32Next(hProcessSnap, &pe32));

        } while (false);

        if (INVALID_HANDLE_VALUE != hProcessSnap)
        {
            ::CloseHandle(hProcessSnap);
        }

        return strPath;
    }

    DWORD GetParentID(DWORD dwPID)
    {
        HANDLE hProcessSnap = NULL;
        PROCESSENTRY32 pe32 = { 0 };
        pe32.dwSize = sizeof(pe32);
        DWORD dwParentProcessId = 0;

        do
        {
            hProcessSnap = ::CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
            if (INVALID_HANDLE_VALUE == hProcessSnap)
            {
                break;
            }

            if (!::Process32First(hProcessSnap, &pe32))
            {
                break;
            }

            do
            {
                if (pe32.th32ProcessID == dwPID)
                {
                    dwParentProcessId = pe32.th32ParentProcessID;
                    break;
                }

            } while (::Process32Next(hProcessSnap, &pe32));

        } while (false);

        if (INVALID_HANDLE_VALUE != hProcessSnap)
        {
            ::CloseHandle(hProcessSnap);
        }

        return dwParentProcessId;
    }

    std::vector<_tstring> GetModules(DWORD dwPID)
    {
        std::vector<_tstring> modulesList;
        TCHAR szModName[MAX_PATH] = { 0 };
        HMODULE* phMods = NULL;
        HANDLE hProcess = NULL;
        DWORD cbNeeded = 0;
        DWORD dwModSize = 0;

        do
        {
            hProcess = ::OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, dwPID);
            if (NULL == hProcess)
            {
                break;
            }

            if (!::EnumProcessModulesEx(hProcess, NULL, NULL, &cbNeeded, LIST_MODULES_ALL))
            {
                break;
            }

            dwModSize = cbNeeded;

            phMods = (HMODULE*)::HeapAlloc(::GetProcessHeap(), HEAP_ZERO_MEMORY, dwModSize);
            if (NULL == phMods)
            {
                break;
            }

            if (!::EnumProcessModulesEx(hProcess, phMods, dwModSize, &cbNeeded, LIST_MODULES_ALL))
            {
                break;
            }

            size_t nModuleCnt = (cbNeeded / sizeof(HMODULE));
            for (size_t i = 0; i < nModuleCnt; i++)
            {
                if (::GetModuleFileNameEx(hProcess, phMods[i], szModName, _countof(szModName)))
                {
                    modulesList.push_back(szModName);
                }
            }

        } while (false);

        if (NULL != hProcess)
        {
            ::CloseHandle(hProcess);
        }

        if (phMods)
        {
            ::HeapFree(::GetProcessHeap(), 0, phMods);
        }

        return modulesList;
    }

    bool WaitForProcess(
        DWORD dwPID,
        DWORD dwMilliseconds/* = INFINITE*/
    )
    {
        HANDLE hProcess = NULL;
        bool fResult = false;

        hProcess = ::OpenProcess(SYNCHRONIZE, FALSE, dwPID);
        if (NULL == hProcess)
        {
            return false;
        }

        fResult = (WAIT_OBJECT_0 == ::WaitForSingleObject(hProcess, dwMilliseconds));
        ::CloseHandle(hProcess);

        return fResult;
    }

    PROC_SID_LIST GetProcessAccountSidList()
    {
        PROC_SID_LIST mapResult;
        PWTS_PROCESS_INFO pPi = NULL;
        DWORD dwCount = 0;
        do
        {
            if (!::WTSEnumerateProcesses(NULL, 0, 1, &pPi, &dwCount))
            {
                break;
            }

            for (DWORD i = 0; i < dwCount; i++)
            {
                PSID pUserSid = pPi[i].pUserSid;
                TCHAR szName[MAX_PATH] = { 0 };
                DWORD cchName = _countof(szName);
                TCHAR szDomainName[MAX_PATH] = { 0 };
                DWORD cchDomainName = _countof(szDomainName);
                SID_NAME_USE sidNameUse = SidTypeUser;

                if (::LookupAccountSid(NULL, pUserSid, szName, &cchName, szDomainName, &cchDomainName, &sidNameUse))
                {
                    PROC_SID_INFO info;
                    info.SessionId = pPi[i].SessionId;
                    info.ProcessName = pPi[i].pProcessName;
                    info.ProcessId = pPi[i].ProcessId;
                    info.UserName = szName;
                    info.DomainName = szDomainName;

                    mapResult.emplace(pPi[i].ProcessId, info);
                }
            }

        } while (false);

        if (pPi)
        {
            ::WTSFreeMemory(pPi);
        }

        return mapResult;
    }

    PROC_INFO_LIST GetProcessList(const _tstring& strName/* = _T("")*/, DWORD dwPID/* = 0*/, bool fMultilingual/* = true*/)
    {
        PROC_INFO_LIST infos;
        HANDLE hProcessSnap = NULL;
        PROCESSENTRY32 pe32 = { 0 };
        pe32.dwSize = sizeof(pe32);
        DWORD dwQueryAccess = GetQueryAccess();
        LANGID LangId = ::GetThreadUILanguage();

        do
        {
            hProcessSnap = ::CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
            if (INVALID_HANDLE_VALUE == hProcessSnap)
            {
                break;
            }

            if (!::Process32First(hProcessSnap, &pe32))
            {
                break;
            }

            do
            {
                PROC_INFO info;
                info.dwParentPID = pe32.th32ParentProcessID;
                info.dwPID = pe32.th32ProcessID;
                info.strExeFile = pe32.szExeFile;
                info.cntThreads = pe32.cntThreads;
                info.nPriClassBase = pe32.pcPriClassBase;

                HANDLE hProcess = NULL;

                // 获取句柄数量, GUI对象数量, 用户对象数量
                do
                {
                    // 打开进程
                    hProcess = ::OpenProcess(dwQueryAccess, FALSE, info.dwPID);
                    if (NULL == hProcess)
                    {
                        break;
                    }

                    ::GetProcessHandleCount(hProcess, &info.GuiResource.dwHandleCount);
                    info.GuiResource.dwGdiCount = GetGuiResources(hProcess, GR_GDIOBJECTS);
                    info.GuiResource.dwUserCount = GetGuiResources(hProcess, GR_USEROBJECTS);

                    ::GetProcessTimes(hProcess,
                        &info.TimeInfo.CreationTime,
                        &info.TimeInfo.ExitTime,
                        &info.TimeInfo.KernelTime,
                        &info.TimeInfo.UserTime
                    );

                    // 获取进程内存信息
                    ::GetProcessMemoryInfo(hProcess, &info.Memory, sizeof(PROCESS_MEMORY_COUNTERS));

                    // 获取进程 I/O 操作信息
                    ::GetProcessIoCounters(hProcess, &info.IoCounters);

                    // 获取进程组信息, 耗时比较高(已通过缓存加速)
                    if (GET_PROCESS_GROUP_INFO)
                    {
                        info.Groups = _GetProcessTokenGroups(hProcess);
                    }

                } while (false);

                if (NULL != hProcess)
                {
                    ::CloseHandle(hProcess);
                }

                // 进程 ID 筛选
                if ((0 == dwPID || dwPID == info.dwPID) &&
                    (strName.empty() || 0 == _tcsicmp(strName.c_str(), pe32.szExeFile))
                    )
                {
                    info.strFilePath = GetPath(info.dwPID);

                    if (!info.strFilePath.empty())
                    {
                        info.PeHeader.LoadFromFile(info.strFilePath);
                    }

                    info.VersionList = GetFileVersionLangNameList(info.strFilePath, fMultilingual);

                    for (const auto& item : info.VersionList)
                    {
                        if (!fMultilingual)
                        {
                            info.strDesc = item.second.strFileDescription;
                            break;
                        }
                        else
                        {
                            if (LangId == item.second.wLangId)
                            {
                                info.strDesc = item.second.strFileDescription;
                            }
                        }
                    }

                    if (info.strDesc.empty())
                    {
                        info.strDesc = info.strExeFile;
                    }

                    infos.emplace(info.dwPID, info);
                }

            } while (::Process32Next(hProcessSnap, &pe32));

        } while (false);

        if (INVALID_HANDLE_VALUE != hProcessSnap)
        {
            ::CloseHandle(hProcessSnap);
        }

        // 获取进程用户名信息
        PROC_SID_LIST mapResult = GetProcessAccountSidList();

        for (auto& item : infos)
        {
            auto itFind = mapResult.find(item.first);
            if (mapResult.end() != itFind)
            {
                item.second.SidInfo = itFind->second;
            }
        }

        return infos;
    }

    PROC_INFO GetProcessInfo(DWORD dwPID)
    {
        PROC_INFO info;
        PROC_INFO_LIST infos = GetProcessList(_T(""), dwPID);

        auto itFind = infos.find(dwPID);
        if (itFind != infos.end())
        {
            info = itFind->second;
        }

        return info;
    }

    PROC_NODE_LIST GetProcessNodes(const PROC_INFO_LIST& procList)
    {
        PROC_NODE_LIST mapResult;

        for (const auto& item : procList)
        {
            if (mapResult.end() != mapResult.find(item.first))
            {
                continue;
            }

            if (0 == item.second.dwParentPID ||
                procList.end() == procList.find(item.second.dwParentPID)
                )
            {
                PROC_NODE node = _GetProcessNode(procList, item.first);
                mapResult.insert(std::make_pair(item.first, node));
            }
        }

        return mapResult;
    }

    PROC_NODE_LIST GetProcessNodes()
    {
        PROC_NODE_LIST mapResult;
        PROC_INFO_LIST infos = GetProcessList();
        mapResult = GetProcessNodes(infos);
        return mapResult;
    }

    MODULE_INFO_LIST GetModuleInfos(DWORD dwPID)
    {
        MODULE_INFO_LIST vModules;
        HANDLE hModuleSnap = NULL;
        MODULEENTRY32 me32 = { 0 };
        me32.dwSize = sizeof(me32);

        do
        {
            hModuleSnap = ::CreateToolhelp32Snapshot(
                TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32,
                dwPID
            );

            if (INVALID_HANDLE_VALUE == hModuleSnap)
            {
                break;
            }

            if (!::Module32First(hModuleSnap, &me32))
            {
                break;
            }

            do
            {
                MODULE_INFO info;
                info.th32ProcessID = me32.th32ProcessID;
                info.GlblcntUsage = me32.GlblcntUsage;
                info.modBaseAddr = me32.modBaseAddr;
                info.modBaseSize = me32.modBaseSize;
                info.hModule = me32.hModule;
                info.strExePath = me32.szExePath;
                info.strModule = me32.szModule;

                vModules.push_back(info);

            } while (::Module32Next(hModuleSnap, &me32));

        } while (false);

        if (INVALID_HANDLE_VALUE != hModuleSnap)
        {
            ::CloseHandle(hModuleSnap);
        }

        return vModules;
    }

    PROC_NODE GetProcessNode(const PROC_INFO_LIST& procList, DWORD dwPID)
    {
        return _GetProcessNode(procList, dwPID);
    }

    PROC_NODE GetProcessNode(DWORD dwPID)
    {
        PROC_INFO_LIST procInfos = GetProcessList();
        return _GetProcessNode(procInfos, dwPID);
    }

    PROC_NODE _GetProcessNode(
        const PROC_INFO_LIST& procInfos,
        DWORD dwPID
    )
    {
        PROC_NODE procNode;
        for (const auto& item : procInfos)
        {
            const PROC_INFO& info = item.second;
            if (item.first == dwPID)
            {
                procNode.ProcInfo = info;
            }
            else if (info.dwParentPID == dwPID)
            {
                PROC_NODE subNode = _GetProcessNode(procInfos, info.dwPID);
                procNode.NodeList.push_back(subNode);
            }
        }

        return procNode;
    }

    bool KillProcess(const _tstring& strName/* = _T("")*/)
    {
        bool fResult = true;
        PROC_INFO_LIST infos = GetProcessList(strName, 0);
        for (const auto& item : infos)
        {
            if (!KillProcess(item.first))
            {
                fResult = false;
            }
        }

        return fResult;
    }

    bool KillProcess(DWORD dwPID)
    {
        HANDLE hProcess = NULL;
        BOOL fSuccess = false;

        // 杀死进程
        hProcess = ::OpenProcess(PROCESS_TERMINATE, FALSE, dwPID);
        if (NULL == hProcess)
        {
            return false;
        }

        fSuccess = ::TerminateProcess(hProcess, 0);
        ::CloseHandle(hProcess);

        return fSuccess;
    }

    bool _KillProcessNode(const PROC_NODE& procNode)
    {
        HANDLE hProcess = NULL;
        BOOL fSuccess = false;

        // 杀死子进程
        for (auto item : procNode.NodeList)
        {
            _KillProcessNode(item);
        }

        // 杀死进程
        hProcess = ::OpenProcess(PROCESS_TERMINATE, FALSE, procNode.ProcInfo.dwPID);
        if (NULL == hProcess)
        {
            return false;
        }

        fSuccess = ::TerminateProcess(hProcess, 0);
        ::CloseHandle(hProcess);

        return fSuccess;
    }

    bool KillProcessNode(DWORD dwPID)
    {
        return _KillProcessNode(GetProcessNode(dwPID));
    }

    using DRIVE_DOS_LIST = std::map<_tstring, _tstring>;
    DRIVE_DOS_LIST _GetDriveDosPathList()
    {
        DRIVE_DOS_LIST pathList;

        TCHAR szDriveStrings[MAX_PATH] = { 0 };
        TCHAR szDosBuf[MAX_PATH] = { 0 };
        LPTSTR pDriveStr = NULL;

        // 获取盘符名到缓冲
        if (::GetLogicalDriveStrings(_countof(szDriveStrings), szDriveStrings))
        {
            // 遍历盘符名
            for (int i = 0; i < _countof(szDriveStrings); i += 4)
            {
                pDriveStr = &szDriveStrings[i];
                pDriveStr[2] = _T('\0');

                // 查询盘符对应的DOS设备名称
                if (!::QueryDosDevice(pDriveStr, szDosBuf, _countof(szDosBuf)))
                {
                    break;
                }

                pathList.emplace(pDriveStr, szDosBuf);
            }
        }

        return pathList;
    }

    _tstring DosPathToNtPath(const _tstring& strPath)
    {
        _tstring strResultPath;
        TCHAR szDriveStrings[MAX_PATH] = { 0 };
        TCHAR szDosBuf[MAX_PATH] = { 0 };
        LPTSTR pDriveStr = NULL;

        // 获取盘符名到缓冲
        if (::GetLogicalDriveStrings(_countof(szDriveStrings), szDriveStrings))
        {
            // 遍历盘符名
            for (int i = 0; i < _countof(szDriveStrings); i += 4)
            {
                pDriveStr = &szDriveStrings[i];
                pDriveStr[2] = _T('\0');

                // 查询盘符对应的DOS设备名称
                if (!::QueryDosDevice(pDriveStr, szDosBuf, _countof(szDosBuf)))
                {
                    break;
                }

                // 对比路径前缀
                size_t nLen = _tcslen(szDosBuf);

                if (strPath.size() < nLen)
                {
                    break;
                }

                if (_T('\\') == strPath[nLen] && 0 == _tcsncmp(strPath.c_str(), szDosBuf, nLen))
                {
                    strResultPath = pDriveStr;
                    strResultPath += &strPath[nLen];
                    break;
                }
            }
        }

        return strResultPath;
    }

    _tstring NtPathToDosPath(const _tstring& strPath)
    {
        _tstring strResultPath;
        TCHAR szDriveStrings[MAX_PATH] = { 0 };
        TCHAR szDosBuf[MAX_PATH] = { 0 };
        TCHAR szResultBuf[MAX_PATH] = { 0 };
        LPTSTR pDriveStr = NULL;

        // 获取盘符名到缓冲
        if (::GetLogicalDriveStrings(_countof(szDriveStrings), szDriveStrings))
        {
            // 遍历盘符名
            for (int i = 0; i < _countof(szDriveStrings); i += 4)
            {
                pDriveStr = &szDriveStrings[i];
                pDriveStr[2] = _T('\0');

                // 查询盘符对应的DOS设备名称
                if (!::QueryDosDevice(pDriveStr, szDosBuf, _countof(szDosBuf)))
                {
                    break;
                }

                // 对比路径前缀
                size_t nLen = _tcslen(pDriveStr);
                if (0 == _tcsncmp(strPath.c_str(), pDriveStr, nLen))
                {
                    lstrcpy(szResultBuf, szDosBuf);
                    lstrcat(szResultBuf, strPath.c_str() + nLen);
                    strResultPath = szResultBuf;
                    break;
                }
            }
        }

        return strResultPath;
    }

    bool SuspendProcess(DWORD dwPID)
    {
        typedef NTSTATUS(*_NtSuspendProcess)(HANDLE hProcess);
        _NtSuspendProcess NtSuspendProcess = NULL;
        HANDLE hProcess = NULL;
        HMODULE hModule = NULL;
        bool fResult = false;

        do
        {
            hModule = ::LoadLibrary(_T("ntdll.dll"));
            if (NULL == hModule)
            {
                break;
            }

            hProcess = ::OpenProcess(PROCESS_SUSPEND_RESUME, FALSE, dwPID);
            if (NULL == hProcess)
            {
                break;
            }

            NtSuspendProcess = (_NtSuspendProcess)::GetProcAddress(hModule, "NtSuspendProcess");
            if (NULL == NtSuspendProcess)
            {
                break;
            }

            fResult = (STATUS_SUCCESS == NtSuspendProcess(hProcess));

        } while (false);

        if (hProcess)
        {
            ::CloseHandle(hProcess);
        }

        if (hModule)
        {
            ::FreeLibrary(hModule);
        }

        return fResult;
    }

    bool ResumeProcess(DWORD dwPID)
    {
        typedef NTSTATUS(*_NtResumeProcess)(HANDLE hProcess);
        _NtResumeProcess NtResumeProcess = NULL;
        HANDLE hProcess = NULL;
        HMODULE hModule = NULL;
        bool fResult = false;

        do
        {
            hModule = ::LoadLibrary(_T("ntdll.dll"));
            if (NULL == hModule)
            {
                break;
            }

            hProcess = ::OpenProcess(PROCESS_SUSPEND_RESUME, FALSE, dwPID);
            if (NULL == hProcess)
            {
                break;
            }

            NtResumeProcess = (_NtResumeProcess)::GetProcAddress(hModule, "NtResumeProcess");
            if (NULL == NtResumeProcess)
            {
                break;
            }

            fResult = (STATUS_SUCCESS == NtResumeProcess(hProcess));

        } while (false);

        if (hProcess)
        {
            ::CloseHandle(hProcess);
        }

        if (hModule)
        {
            ::FreeLibrary(hModule);
        }

        return fResult;
    }

    bool SuspendProcessNode(DWORD dwPID)
    {
        return _SuspendProcessNode(GetProcessNode(dwPID));
    }

    bool ResumeProcessNode(DWORD dwPID)
    {
        return _ResumeProcessNode(GetProcessNode(dwPID));
    }

    bool _SuspendProcessNode(const PROC_NODE& procNode)
    {
        bool fResult = false;

        do
        {
            // 暂停主进程
            if (!SuspendProcess(procNode.ProcInfo.dwPID))
            {
                break;
            }

            // 暂停子进程
            for (const auto& item : procNode.NodeList)
            {
                if (!_SuspendProcessNode(item))
                {
                    break;
                }
            }

            fResult = true;

        } while (false);

        return fResult;
    }

    bool _ResumeProcessNode(const PROC_NODE& procNode)
    {
        bool fResult = false;

        do
        {
            // 恢复主进程
            if (!ResumeProcess(procNode.ProcInfo.dwPID))
            {
                break;
            }

            // 恢复子进程
            for (const auto& item : procNode.NodeList)
            {
                if (!_ResumeProcessNode(item))
                {
                    break;
                }
            }

            fResult = true;

        } while (false);

        return fResult;
    }

    bool IsWow64Process(DWORD dwPID)
    {
        HANDLE hProcess = NULL;
        bool fWow64 = false;

        do
        {
            hProcess = ::OpenProcess(GetQueryAccess(), FALSE, dwPID);
            if (NULL == hProcess)
            {
                break;
            }

            BOOL Wow64Process = FALSE;
            if (!::IsWow64Process(hProcess, &Wow64Process))
            {
                break;
            }

            fWow64 = Wow64Process;

        } while (false);

        if (NULL != hProcess)
        {
            ::CloseHandle(hProcess);
        }

        return fWow64;
    }

    bool IsProcessX86(DWORD dwPID)
    {
        return true == IsWow64Process(dwPID);
    }

    bool IsProcessX64(DWORD dwPID)
    {
        return false == IsWow64Process(dwPID);
    }

    DWORD GetProcessHandleCount(DWORD dwPID)
    {
        HANDLE hProcess = NULL;
        DWORD dwHandleCount = 0;
        DWORD dwGdiCount = 0;
        DWORD dwUserCount = 0;

        do
        {
            // 打开进程
            hProcess = ::OpenProcess(GetQueryAccess(), FALSE, dwPID);
            if (NULL == hProcess)
            {
                break;
            }

            ::GetProcessHandleCount(hProcess, &dwHandleCount);
            dwGdiCount = GetGuiResources(hProcess, GR_GDIOBJECTS);
            dwUserCount = GetGuiResources(hProcess, GR_USEROBJECTS);

        } while (false);


        if (NULL != hProcess)
        {
            ::CloseHandle(hProcess);
        }

        return dwHandleCount;
    }

    PROC_TOKEN_GROUPS _GetProcessTokenGroups(HANDLE hProcess)
    {
        PROC_TOKEN_GROUPS vResult;
        HANDLE hToken = NULL;
        PTOKEN_GROUPS pGroupInfo = NULL;
        SID_NAME_USE SidType = SidTypeUser;
        TCHAR szName[MAX_PATH] = { 0 };
        TCHAR szDomain[MAX_PATH] = { 0 };
        PSID pSID = NULL;
        SID_IDENTIFIER_AUTHORITY SIDAuth = SECURITY_NT_AUTHORITY;
        DWORD dwSize = 0;

        do
        {
            // 打开进程令牌
            if (!::OpenProcessToken(hProcess, TOKEN_QUERY, &hToken))
            {
                break;
            }

            // 获取令牌信息所需缓冲大小
            if (!::GetTokenInformation(hToken, TokenGroups, NULL, dwSize, &dwSize))
            {
                if (ERROR_INSUFFICIENT_BUFFER != ::GetLastError())
                {
                    break;
                }
            }

            // 分配缓冲
            pGroupInfo = (PTOKEN_GROUPS)::GlobalAlloc(GPTR, dwSize);
            if (NULL == pGroupInfo)
            {
                break;
            }

            // 获取令牌组信息
            if (!::GetTokenInformation(hToken, TokenGroups, pGroupInfo, dwSize, &dwSize))
            {
                break;
            }

            // 分配和初始化一个安全标识符
            if (!::AllocateAndInitializeSid(&SIDAuth, 2, SECURITY_BUILTIN_DOMAIN_RID, DOMAIN_ALIAS_RID_ADMINS,
                0, 0, 0, 0, 0, 0,
                &pSID))
            {
                break;
            }

            // 检索SID信息
            for (DWORD i = 0; i < pGroupInfo->GroupCount; i++)
            {
                PROC_TOKEN_GROUPS_INFO info;

                dwSize = _countof(szName);
                if (!::LookupAccountSid(NULL, pGroupInfo->Groups[i].Sid, szName, &dwSize, szDomain, &dwSize, &SidType))
                {
                    if (ERROR_NONE_MAPPED != ::GetLastError())
                    {
                        break;
                    }

                    _tcscpy_s(szName, dwSize, _T("NONE_MAPPED"));
                }

                LPTSTR pStringSid = NULL;
                if (ConvertSidToStringSid(pGroupInfo->Groups[i].Sid, &pStringSid))
                {
                    info.SID = pStringSid;
                    ::LocalFree(pStringSid);
                }

                info.DomainName = szDomain;
                info.UserName = szName;
                info.Attributes = pGroupInfo->Groups[i].Attributes;
                vResult.push_back(info);
            }

        } while (false);

        if (pSID)
        {
            ::FreeSid(pSID);
        }

        if (pGroupInfo)
        {
            ::GlobalFree(pGroupInfo);
        }

        return vResult;
    }

    PROC_TOKEN_GROUPS GetProcessTokenGroups(DWORD dwPID)
    {
        PROC_TOKEN_GROUPS vResult;
        HANDLE hProcess = NULL;

        do
        {
            // 打开进程
            hProcess = ::OpenProcess(GetQueryAccess(), FALSE, dwPID);
            if (NULL == hProcess)
            {
                break;
            }

            vResult = _GetProcessTokenGroups(hProcess);

        } while (false);

        if (NULL != hProcess)
        {
            ::CloseHandle(hProcess);
        }

        return vResult;
    }

    DWORD _DuplicateProcessToken(
        const _tstring& strName,
        DWORD dwSessionId,
        DWORD dwDesiredAccess,
        PHANDLE phToken
    )
    {
        PROCESSENTRY32 pe = { 0 };
        PRIVILEGE_SET ps = { 0 };
        HANDLE hSnapshot = INVALID_HANDLE_VALUE;
        BOOL fResult = FALSE;
        DWORD dwErr = ERROR_SUCCESS;
        BOOL fFound = FALSE;

        ps.PrivilegeCount = 1;
        ps.Control = PRIVILEGE_SET_ALL_NECESSARY;

        do
        {
            if (!::LookupPrivilegeValue(NULL, SE_TCB_NAME, &ps.Privilege[0].Luid))
            {
                break;
            }

            hSnapshot = ::CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
            if (INVALID_HANDLE_VALUE == hSnapshot)
            {
                break;
            }

            pe.dwSize = sizeof(pe);
            if (!::Process32First(hSnapshot, &pe))
            {
                break;
            }

            do
            {
                HANDLE hProcess = NULL;
                HANDLE hToken = NULL;
                DWORD dwRetLen = 0;
                DWORD sid = 0;

                if (0 != _tcsicmp(pe.szExeFile, strName.c_str()))
                {
                    continue;
                }

                hProcess = ::OpenProcess(GetQueryAccess(), FALSE, pe.th32ProcessID);
                if (!hProcess)
                {
                    break;
                }

                do
                {
                    BOOL fTcb = FALSE;

                    if (!::OpenProcessToken(hProcess, TOKEN_QUERY | TOKEN_DUPLICATE, &hToken))
                    {
                        break;
                    }

                    if (!::PrivilegeCheck(hToken, &ps, &fTcb) && fTcb)
                    {
                        break;
                    }

                    if (!::GetTokenInformation(hToken, TokenSessionId, &sid, sizeof(sid), &dwRetLen) && sid == dwSessionId)
                    {
                        break;
                    }

                    if (!::DuplicateTokenEx(hToken, dwDesiredAccess, NULL, SecurityImpersonation, TokenImpersonation, phToken))
                    {
                        break;
                    }

                    fFound = true;
                    dwErr = ERROR_SUCCESS;

                } while (false);

                if (hToken)
                {
                    ::CloseHandle(hToken);
                }

                if (hProcess)
                {
                    ::CloseHandle(hProcess);
                }

                if (fFound)
                {
                    break;
                }

            } while (::Process32Next(hSnapshot, &pe));

            if (!fFound)
            {
                dwErr = ERROR_NOT_FOUND;
                break;
            }

            fResult = true;

        } while (false);

        if (!fResult)
        {
            dwErr = ::GetLastError();
            if (!fFound)
            {
                dwErr = ERROR_NOT_FOUND;
            }
        }

        if (INVALID_HANDLE_VALUE != hSnapshot)
        {
            ::CloseHandle(hSnapshot);
        }

        return dwErr;
    }

    DWORD _CreateProcessWithUIAccess(
        const _tstring& strExePath,
        const _tstring& strCommand,
        const _tstring& strCurDir/* = _T("")*/,
        bool fUIAccess/* = true*/
    )
    {
        HANDLE hTokenUIAccess = NULL;
        DWORD dwErr = ERROR_SUCCESS;
        BOOL fResult = FALSE;
        _tstring exePath = strExePath;
        _tstring exeCommand = strCommand;
        _tstring exeCurDir = strCurDir;

        LPCTSTR lpApplicationName = NULL;
        LPTSTR lpCommandLine = NULL;
        LPCTSTR lpCurrentDir = NULL;

        if (!strExePath.empty())
        {
            lpApplicationName = exePath.c_str();
        }

        if (!strCommand.empty())
        {
            lpCommandLine = (LPTSTR)exeCommand.c_str();
        }

        if (!exeCurDir.empty())
        {
            lpCurrentDir = exeCurDir.c_str();
        }

        do
        {
            STARTUPINFO si = { 0 };
            PROCESS_INFORMATION pi = { 0 };

            dwErr = _CreateUIAccessToken(&hTokenUIAccess);
            if (ERROR_SUCCESS != dwErr)
            {
                break;
            }

            if (!fUIAccess)
            {
                BOOL fTokenInformation = FALSE;
                ::SetTokenInformation(hTokenUIAccess, TokenUIAccess, &fTokenInformation, sizeof(fTokenInformation));
            }

            ::GetStartupInfo(&si);
            if (!::CreateProcessAsUser(hTokenUIAccess, lpApplicationName, lpCommandLine, NULL, NULL, FALSE, 0, NULL, lpCurrentDir, &si, &pi))
            {
                break;
            }

            ::CloseHandle(pi.hProcess);
            ::CloseHandle(pi.hThread);

        } while (false);

        if (!fResult)
        {
            dwErr = ::GetLastError();
        }

        if (hTokenUIAccess)
        {
            ::CloseHandle(hTokenUIAccess);
        }

        return dwErr;
    }

    bool CreateProcessWithPidToken(
        DWORD pid,
        const _tstring& strExePath,
        const _tstring& strCommand,
        const _tstring& strCurDir
    )
    {
        STARTUPINFOW si = { 0 };
        PROCESS_INFORMATION pi = { 0 };
        HANDLE hProcess = NULL;
        HANDLE hProcessToken = NULL;
        HANDLE hDuplicateToken = NULL;
        bool fResult = false;

        si.cb = sizeof(STARTUPINFO);

        std::wstring exePath = TStrToWStr(strExePath);
        std::wstring exeCommand = TStrToWStr(strCommand);
        std::wstring exeCurDir = TStrToWStr(strCurDir);

        LPCWSTR lpApplicationName = NULL;
        LPWSTR lpCommandLine = NULL;
        LPCWSTR lpCurrentDir = NULL;

        if (!exePath.empty())
        {
            lpApplicationName = exePath.c_str();
        }

        if (!exeCommand.empty())
        {
            lpCommandLine = (LPWSTR)exeCommand.c_str();
        }

        if (!exeCurDir.empty())
        {
            lpCurrentDir = exeCurDir.c_str();
        }

        do
        {
            // 打开进程句柄
            hProcess = ::OpenProcess(GetQueryAccess(), TRUE, pid);
            if (NULL == hProcess)
            {
                break;
            }

            // 打开进程令牌
            DWORD dwOpenDesiredAccess = TOKEN_DUPLICATE | TOKEN_ASSIGN_PRIMARY | TOKEN_QUERY;
            if (!::OpenProcessToken(hProcess, dwOpenDesiredAccess, &hProcessToken))
            {
                break;
            }

            // 模拟登录用户
            if (!::ImpersonateLoggedOnUser(hProcessToken))
            {
                break;
            }

            // 终止客户端应用程序的模拟
            ::RevertToSelf();

            // 创建一个复制现有令牌的新访问令牌
            DWORD dwDuplicateDesiredAccess = TOKEN_ADJUST_DEFAULT | TOKEN_ADJUST_SESSIONID | TOKEN_QUERY | TOKEN_DUPLICATE | TOKEN_ASSIGN_PRIMARY;
            if (!::DuplicateTokenEx(hProcessToken, dwDuplicateDesiredAccess, NULL, SecurityImpersonation, TokenPrimary, &hDuplicateToken))
            {
                break;
            }

            // 使用令牌创建进程
            // 调用 CreateProcessWithTokenW 的进程必须具有SE_IMPERSONATE_NAME特权。
            if (!::CreateProcessWithTokenW(hDuplicateToken, LOGON_WITH_PROFILE, lpApplicationName, lpCommandLine, 0, NULL, lpCurrentDir, &si, &pi))
            {
                break;
            }

            fResult = true;

            ::CloseHandle(pi.hProcess);
            ::CloseHandle(pi.hThread);

        } while (false);

        if (hDuplicateToken)
        {
            ::CloseHandle(hDuplicateToken);
        }

        if (hProcessToken)
        {
            ::CloseHandle(hProcessToken);
        }

        if (hProcess)
        {
            ::CloseHandle(hProcess);
        }

        return fResult;
    }

    DWORD _CreateUIAccessToken(
        PHANDLE phToken
    )
    {
        DWORD dwErr = ERROR_SUCCESS;
        HANDLE hTokenSelf = NULL;
        HANDLE hWinlogonToken = NULL;
        BOOL fResult = FALSE;

        do
        {
            DWORD dwDesiredAccess = TOKEN_QUERY | TOKEN_DUPLICATE | TOKEN_ASSIGN_PRIMARY | TOKEN_ADJUST_DEFAULT;
            DWORD dwSessionId = 0;
            DWORD dwRetLen = 0;
            BOOL bUIAccess = TRUE;

            if (!::OpenProcessToken(::GetCurrentProcess(), TOKEN_QUERY | TOKEN_DUPLICATE, &hTokenSelf))
            {
                break;
            }

            if (!::GetTokenInformation(hTokenSelf, TokenSessionId, &dwSessionId, sizeof(dwSessionId), &dwRetLen))
            {
                break;
            }

            dwErr = _DuplicateProcessToken(_T("winlogon.exe"), dwSessionId, TOKEN_IMPERSONATE, &hWinlogonToken);
            if (ERROR_SUCCESS != dwErr)
            {
                break;
            }

            if (!::SetThreadToken(NULL, hWinlogonToken))
            {
                break;
            }

            if (!::DuplicateTokenEx(hTokenSelf, dwDesiredAccess, NULL, SecurityAnonymous, TokenPrimary, phToken))
            {
                break;
            }

            if (!::SetTokenInformation(*phToken, TokenUIAccess, &bUIAccess, sizeof(bUIAccess)))
            {
                break;
            }

            ::RevertToSelf();

            fResult = TRUE;

        } while (false);

        if (!fResult)
        {
            if (*phToken)
            {
                ::CloseHandle(*phToken);
                *phToken = NULL;
            }

            dwErr = ::GetLastError();
        }

        if (hWinlogonToken)
        {
            ::CloseHandle(hWinlogonToken);
        }

        if (hTokenSelf)
        {
            ::CloseHandle(hTokenSelf);
        }

        return dwErr;
    }

    DWORD CreateProcessWithUIAccess(
        const _tstring& strExePath,
        const _tstring& strCommand,
        const _tstring& strCurDir/* = _T("")*/
    )
    {
        return _CreateProcessWithUIAccess(strExePath, strCommand, strCurDir, true);
    }

    DWORD CreateProcessNoUIAccess(
        const _tstring& strExePath,
        const _tstring& strCommand,
        const _tstring& strCurDir/* = _T("")*/
    )
    {
        return _CreateProcessWithUIAccess(strExePath, strCommand, strCurDir, false);
    }

    bool CreateProcessWithSystem(
        const _tstring& strExePath,
        const _tstring& strCommand,
        const _tstring& strCurDir/* = _T("")*/
    )
    {
        PROC_INFO_LIST infos = GetProcessList(_T("winlogon.exe"), 0);
        if (infos.empty())
        {
            return false;
        }

        return CreateProcessWithPidToken(infos.begin()->first, strExePath, strCommand, strCurDir);
    }

    BOOL CheckProcessUIAccess(
        DWORD dwPid,
        DWORD* pdwErr,
        DWORD* pfUIAccess
    )
    {
        HANDLE hProcess = NULL;
        HANDLE hToken = NULL;
        DWORD dwRetLen = 0;
        BOOL result = FALSE;

        do
        {
            hProcess = ::OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, dwPid);
            if (!hProcess)
            {
                break;
            }

            if (!::OpenProcessToken(hProcess, TOKEN_QUERY, &hToken))
            {
                break;
            }

            if (!::GetTokenInformation(hToken, TokenUIAccess, pfUIAccess, sizeof(*pfUIAccess), &dwRetLen))
            {
                break;
            }

            result = TRUE;

        } while (false);

        if (!result)
        {
            *pdwErr = ::GetLastError();
        }

        if (hProcess)
        {
            ::CloseHandle(hProcess);
        }

        if (hToken)
        {
            ::CloseHandle(hToken);
        }

        return result;
    }

    void _EnumProcessWindows(WND_INFO_LIST& infos, HWND hWnd, DWORD dwPid)
    {
        HWND childWindow = NULL;
        ULONG nMaxTimes = 0x4000;

        //使用 FindWindowEx 函数搜索子窗口
        while (nMaxTimes > 0 && (childWindow = ::FindWindowEx(hWnd, childWindow, NULL, NULL)))
        {
            ULONG processId = 0;
            ULONG threadId = 0;

            threadId = ::GetWindowThreadProcessId(childWindow, &processId);

            if (dwPid == processId || 0 == dwPid)
            {
                TCHAR szClassBuf[MAX_PATH] = { 0 };
                TCHAR szTextBuf[MAX_PATH] = { 0 };
                ::GetClassName(childWindow, szClassBuf, _countof(szClassBuf));
                ::GetWindowText(childWindow, szTextBuf, _countof(szTextBuf));

                WND_INFO info;
                info.hWnd = childWindow;
                info.dwPid = processId;
                info.dwTid = threadId;
                info.strClass = szClassBuf;
                info.strText = szTextBuf;

                infos.insert(std::make_pair(childWindow, info));
            }

            nMaxTimes--;
        }
    }

    WND_INFO_LIST GetProcessWindows(DWORD dwPid)
    {
        WND_INFO_LIST infos;

        //枚举 桌面窗口 的子窗口
        _EnumProcessWindows(infos, ::GetDesktopWindow(), dwPid);

        //枚举 仅消息窗口 的子窗口
        _EnumProcessWindows(infos, HWND_MESSAGE, dwPid);
        return infos;
    }

    WND_NODE _GetWindowNode(HWND hWnd, MODULE_INFO_LIST& modules)
    {
        TCHAR szClassBuf[MAX_PATH] = { 0 };
        TCHAR szTextBuf[MAX_PATH] = { 0 };
        ::GetClassName(hWnd, szClassBuf, _countof(szClassBuf));
        ::GetWindowText(hWnd, szTextBuf, _countof(szTextBuf));

        WND_NODE node;
        node.hWnd = hWnd;
        node.strClass = szClassBuf;
        node.strText = szTextBuf;
        node.dwTid = ::GetWindowThreadProcessId(hWnd, &node.dwPid);

        HMODULE hModule = (HMODULE)::GetWindowLongPtr(hWnd, GWLP_HINSTANCE);

        for (const auto& item : modules)
        {
            if (hModule == item.hModule)
            {
                node.strModule = item.strModule;
            }
        }

        HWND childWindow = NULL;
        ULONG nMaxTimes = 0x4000;

        //使用 FindWindowEx 函数搜索子窗口
        while (nMaxTimes > 0 && (childWindow = ::FindWindowEx(hWnd, childWindow, NULL, NULL)))
        {
            node.NodeList.push_back(_GetWindowNode(childWindow, modules));

            nMaxTimes--;
        }

        return node;
    }

    WND_NODE_LIST GetProcessWindowNodes(DWORD dwPid)
    {
        WND_NODE_LIST nodes;
        WND_INFO_LIST infos;
        MODULE_INFO_LIST moduleInfos = GetModuleInfos(dwPid);

        //枚举 桌面窗口 的子窗口
        _EnumProcessWindows(infos, ::GetDesktopWindow(), dwPid);

        //枚举 仅消息窗口 的子窗口
        _EnumProcessWindows(infos, HWND_MESSAGE, dwPid);

        for (const auto& item : infos)
        {
            WND_NODE node = _GetWindowNode(item.first, moduleInfos);
            nodes.insert(std::make_pair(item.first, node));
        }

        return nodes;
    }

#pragma pack(push)
#pragma pack(1)
    // 包含文件的版本信息。 此信息与语言和代码页无关
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
    } VS_VERSIONINFO, * PVS_VERSIONINFO;

#pragma pack(pop)

    static VER_LANG_NAME_LIST _GetFileVersionLangNameList(const _tstring& strFile, bool fMultilingual)
    {
        VER_LANG_NAME_LIST infoResult;
        PVOID pFsRedirectionOldValue = NULL;
        bool isDisableWow64Fs = false;
        UINT cbTranslate = 0;
        DWORD dwVerSize = 0;

        // 加载文件
        if (strFile.empty())
        {
            return infoResult;
        }

        // 禁用文件重定向
        isDisableWow64Fs = ::Wow64DisableWow64FsRedirection(&pFsRedirectionOldValue);

        LANG_LIST langList = GetLangList();
        LANGID LangId = ::GetThreadUILanguage();

        // 多语言获取信息
        for (const auto& item : langList)
        {
            if (fMultilingual)
            {
                ::SetThreadUILanguage(item.first);
            }
            else
            {
                ::SetThreadUILanguage(LangId);
            }

            LPVOID lpVerData = NULL;

            do
            {
                // 获取版本信息数据大小
                dwVerSize = ::GetFileVersionInfoSize(strFile.c_str(), NULL);
                if (0 == dwVerSize)
                {
                    break;
                }

                // 分配版本信息缓冲
                lpVerData = ::HeapAlloc(::GetProcessHeap(), HEAP_ZERO_MEMORY, dwVerSize);
                if (!lpVerData)
                {
                    break;
                }

                // 获取版本信息
                BOOL fResult = FALSE;
                fResult = ::GetFileVersionInfo(strFile.c_str(), NULL, dwVerSize, lpVerData);
                if (!fResult)
                {
                    break;
                }

                // 获取语言代码
                LANGANDCODEPAGE* lpTranslate = NULL;
                if (!::VerQueryValue(lpVerData, _T("\\VarFileInfo\\Translation"), (LPVOID*)&lpTranslate, &cbTranslate))
                {
                    break;
                }

                DWORD dwCount = cbTranslate / sizeof(LANGANDCODEPAGE);
                for (DWORD i = 0; i < dwCount; i++)
                {
                    LANGANDCODEPAGE langCodePage = lpTranslate[i];
                    VERSION_INFO versionInfo;

                    // 获取 VS_FIXEDFILEINFO 信息
                    PVS_VERSIONINFO lpVersion = (PVS_VERSIONINFO)lpVerData;
                    if (0 != lpVersion->wValueLength)
                    {
                        VS_FIXEDFILEINFO* pFixedFileInfo = (VS_FIXEDFILEINFO*)((LPBYTE)lpVersion + sizeof(VS_VERSIONINFO));

                        DWORD dwAlign = 4;
                        DWORD_PTR dwPadding = ((DWORD_PTR)pFixedFileInfo % dwAlign);
                        if (0 != dwPadding)
                        {
                            pFixedFileInfo = (VS_FIXEDFILEINFO*)((LPBYTE)pFixedFileInfo + (dwAlign - dwPadding));
                        }

                        versionInfo.vsFixedInfo = *((PVS_VER_FIXEDFILEINFO)pFixedFileInfo);
                    }

                    // 查询信息回调
                    auto _QueryInfo = [](const _tstring& strName, const LPVOID lpData, const LANGANDCODEPAGE& code) {

                        TCHAR strQuery[MAX_PATH] = { 0 };
                        LPCTSTR lpQueryRes = NULL;
                        UINT uQueryCchSize;
                        (void)::StringCchPrintf(strQuery, _countof(strQuery),
                            _T("\\StringFileInfo\\%04x%04x\\%s"),
                            code.wLangId, code.wCodePage, strName.c_str());
                        if (::VerQueryValue(lpData, strQuery, (LPVOID*)&lpQueryRes, &uQueryCchSize))
                        {
                            return lpQueryRes;
                        }

                        return _T("");

                        };

                    // 获取版本字符串回调
                    auto _GetVersionStr = [](VERSON_NUMBER hi, VERSON_NUMBER lo) -> _tstring {
                        TCHAR szBuf[MAX_PATH] = { 0 };
                        (void)::StringCchPrintf(szBuf, _countof(szBuf), _T("%hd.%hd.%hd.%hd"),
                            hi.Version.wHigh,
                            hi.Version.wLow,
                            lo.Version.wHigh,
                            lo.Version.wLow
                        );

                        return szBuf;

                        };

                    versionInfo.langAndCodePage = langCodePage;
                    versionInfo.strComments = _QueryInfo(_T("Comments"), lpVerData, langCodePage);
                    versionInfo.strInternalName = _QueryInfo(_T("InternalName"), lpVerData, langCodePage);
                    versionInfo.strProductName = _QueryInfo(_T("ProductName"), lpVerData, langCodePage);
                    versionInfo.strCompanyName = _QueryInfo(_T("CompanyName"), lpVerData, langCodePage);
                    versionInfo.strLegalCopyright = _QueryInfo(_T("LegalCopyright"), lpVerData, langCodePage);
                    versionInfo.strProductVersion = _QueryInfo(_T("ProductVersion"), lpVerData, langCodePage);
                    versionInfo.strFileDescription = _QueryInfo(_T("FileDescription"), lpVerData, langCodePage);
                    versionInfo.strLegalTrademarks = _QueryInfo(_T("LegalTrademarks"), lpVerData, langCodePage);
                    versionInfo.strPrivateBuild = _QueryInfo(_T("PrivateBuild"), lpVerData, langCodePage);
                    versionInfo.strFileVersion = _QueryInfo(_T("FileVersion"), lpVerData, langCodePage);
                    versionInfo.strOriginalFilename = _QueryInfo(_T("OriginalFilename"), lpVerData, langCodePage);
                    versionInfo.strSpecialBuild = _QueryInfo(_T("SpecialBuild"), lpVerData, langCodePage);

                    VS_VER_FIXEDFILEINFO& vsFixedInfo = versionInfo.vsFixedInfo;
                    versionInfo.strFileVersionEx = _GetVersionStr(vsFixedInfo.dwFileVersionMS, vsFixedInfo.dwFileVersionLS);
                    versionInfo.strProductVersionEx = _GetVersionStr(vsFixedInfo.dwProductVersionMS, vsFixedInfo.dwProductVersionLS);

                    versionInfo.wLangId = item.second.wLangId;
                    versionInfo.strLocaleName = item.second.strLocaleName;
                    versionInfo.strLangName = item.second.strLangName;

                    versionInfo.FileVerNumber = versionInfo.strFileVersionEx.c_str();
                    versionInfo.ProductVerNumber = versionInfo.strProductVersionEx.c_str();

                    infoResult.emplace(versionInfo.strLocaleName, versionInfo);
                }

            } while (false);

            if (lpVerData)
            {
                ::HeapFree(::GetProcessHeap(), 0, lpVerData);
                lpVerData = NULL;
            }

            if (!fMultilingual)
            {
                break;
            }
        }

        if (fMultilingual)
        {
            ::SetThreadUILanguage(LangId);
        }

        // 恢复文件重定向
        if (isDisableWow64Fs)
        {
            ::Wow64RevertWow64FsRedirection(pFsRedirectionOldValue);
        }

        return infoResult;
    }

    static VER_LANG_ID_LIST _GetFileVersionLangIdList(const _tstring& strFile, const LANG_LIST& langList)
    {
        VER_LANG_ID_LIST infoResult;
        PVOID pFsRedirectionOldValue = NULL;
        bool isDisableWow64Fs = false;
        DWORD dwVerSize = 0;

        // 加载文件
        if (strFile.empty())
        {
            return infoResult;
        }

        // 禁用文件重定向
        isDisableWow64Fs = ::Wow64DisableWow64FsRedirection(&pFsRedirectionOldValue);

        // 备份线程语言
        LANGID LangId = ::GetThreadUILanguage();

        // 多语言获取信息
        for (const auto& item : langList)
        {
            ::SetThreadUILanguage(item.first);

            LPVOID lpVerData = NULL;

            do
            {
                // 获取版本信息数据大小
                dwVerSize = ::GetFileVersionInfoSize(strFile.c_str(), NULL);
                if (0 == dwVerSize)
                {
                    break;
                }

                // 分配版本信息缓冲
                lpVerData = ::HeapAlloc(::GetProcessHeap(), 0, dwVerSize);
                if (!lpVerData)
                {
                    break;
                }

                // 获取版本信息
                BOOL fResult = FALSE;
                fResult = ::GetFileVersionInfo(strFile.c_str(), NULL, dwVerSize, lpVerData);
                if (!fResult)
                {
                    break;
                }

                // 获取语言代码
                LANGANDCODEPAGE* lpTranslate = NULL;
                UINT cbTranslate = 0;
                if (!::VerQueryValue(lpVerData, _T("\\VarFileInfo\\Translation"), (LPVOID*)&lpTranslate, &cbTranslate))
                {
                    break;
                }

                DWORD dwCount = cbTranslate / sizeof(LANGANDCODEPAGE);
                for (DWORD i = 0; i < dwCount; i++)
                {
                    LANGANDCODEPAGE langCodePage = lpTranslate[i];
                    VERSION_INFO versionInfo;

                    versionInfo.langAndCodePage = langCodePage;

                    // 获取 VS_FIXEDFILEINFO 信息
                    PVS_VERSIONINFO lpVersion = (PVS_VERSIONINFO)lpVerData;
                    if (0 != lpVersion->wValueLength)
                    {
                        VS_FIXEDFILEINFO* pFixedFileInfo = (VS_FIXEDFILEINFO*)((LPBYTE)lpVersion + sizeof(VS_VERSIONINFO));

                        DWORD dwAlign = 4;
                        DWORD_PTR dwPadding = ((DWORD_PTR)pFixedFileInfo % dwAlign);
                        if (0 != dwPadding)
                        {
                            pFixedFileInfo = (VS_FIXEDFILEINFO*)((LPBYTE)pFixedFileInfo + (dwAlign - dwPadding));
                        }

                        versionInfo.vsFixedInfo = *((PVS_VER_FIXEDFILEINFO)pFixedFileInfo);
                    }

                    // 查询信息回调
                    auto _QueryInfo = [](const _tstring& strName, const LPVOID lpData, const LANGANDCODEPAGE& code) {

                        TCHAR strQuery[MAX_PATH] = { 0 };
                        LPCTSTR lpQueryRes = NULL;
                        UINT uQueryCchSize;
                        (void)::StringCchPrintf(strQuery, _countof(strQuery),
                            _T("\\StringFileInfo\\%04x%04x\\%s"),
                            code.wLangId, code.wCodePage, strName.c_str());
                        if (::VerQueryValue(lpData, strQuery, (LPVOID*)&lpQueryRes, &uQueryCchSize))
                        {
                            return lpQueryRes;
                        }

                        return _T("");

                        };

                    versionInfo.strComments = _QueryInfo(_T("Comments"), lpVerData, langCodePage);
                    versionInfo.strInternalName = _QueryInfo(_T("InternalName"), lpVerData, langCodePage);
                    versionInfo.strProductName = _QueryInfo(_T("ProductName"), lpVerData, langCodePage);
                    versionInfo.strCompanyName = _QueryInfo(_T("CompanyName"), lpVerData, langCodePage);
                    versionInfo.strLegalCopyright = _QueryInfo(_T("LegalCopyright"), lpVerData, langCodePage);
                    versionInfo.strProductVersion = _QueryInfo(_T("ProductVersion"), lpVerData, langCodePage);
                    versionInfo.strFileDescription = _QueryInfo(_T("FileDescription"), lpVerData, langCodePage);
                    versionInfo.strLegalTrademarks = _QueryInfo(_T("LegalTrademarks"), lpVerData, langCodePage);
                    versionInfo.strPrivateBuild = _QueryInfo(_T("PrivateBuild"), lpVerData, langCodePage);
                    versionInfo.strFileVersion = _QueryInfo(_T("FileVersion"), lpVerData, langCodePage);
                    versionInfo.strOriginalFilename = _QueryInfo(_T("OriginalFilename"), lpVerData, langCodePage);
                    versionInfo.strSpecialBuild = _QueryInfo(_T("SpecialBuild"), lpVerData, langCodePage);

                    // 获取版本字符串回调
                    auto _GetVersionStr = [](VERSON_NUMBER hi, VERSON_NUMBER lo) -> _tstring {
                        TCHAR szBuf[MAX_PATH] = { 0 };
                        (void)::StringCchPrintf(szBuf, _countof(szBuf), _T("%hd.%hd.%hd.%hd"),
                            hi.Version.wHigh,
                            hi.Version.wLow,
                            lo.Version.wHigh,
                            lo.Version.wLow
                        );

                        return szBuf;

                        };

                    VS_VER_FIXEDFILEINFO& vsFixedInfo = versionInfo.vsFixedInfo;
                    versionInfo.strFileVersionEx = _GetVersionStr(vsFixedInfo.dwFileVersionMS, vsFixedInfo.dwFileVersionLS);
                    versionInfo.strProductVersionEx = _GetVersionStr(vsFixedInfo.dwProductVersionMS, vsFixedInfo.dwProductVersionLS);

                    versionInfo.wLangId = item.second.wLangId;
                    versionInfo.strLocaleName = item.second.strLocaleName;
                    versionInfo.strLangName = item.second.strLangName;

                    versionInfo.FileVerNumber = versionInfo.strFileVersionEx.c_str();
                    versionInfo.ProductVerNumber = versionInfo.strProductVersionEx.c_str();

                    infoResult.emplace(versionInfo.wLangId, versionInfo);
                }

            } while (false);

            if (lpVerData)
            {
                ::HeapFree(::GetProcessHeap(), 0, lpVerData);
                lpVerData = NULL;
            }
        }

        // 恢复线程语言
        ::SetThreadUILanguage(LangId);

        // 恢复文件重定向
        if (isDisableWow64Fs)
        {
            ::Wow64RevertWow64FsRedirection(pFsRedirectionOldValue);
        }

        return infoResult;
    }

    VER_LANG_NAME_LIST GetFileVersionLangNameList(const _tstring& strFile, bool fMultilingual/* = true*/)
    {
        VER_LANG_NAME_LIST fileInfos = _GetFileVersionLangNameList(strFile, fMultilingual);
        return fileInfos;
    }

    VER_LANG_ID_LIST GetFileVersionLangIdList(const _tstring& strFile, const LANG_LIST& langList)
    {
        VER_LANG_ID_LIST fileInfos = _GetFileVersionLangIdList(strFile, langList);
        return fileInfos;
    }

    VERSION_INFO GetFileVersion(const _tstring& strFile, bool fMultilingual/* = true*/)
    {
        VERSION_INFO info;
        VER_LANG_NAME_LIST fileInfos = _GetFileVersionLangNameList(strFile, fMultilingual);
        if (!fileInfos.empty())
        {
            info = fileInfos.begin()->second;
        }

        return info;
    }

    LANG_INFO GetLanguageInfo(LANGID dwLang)
    {
        TCHAR szBuf[MAX_PATH] = { 0 };
        LANG_INFO langInfo;

        //获取当前线程 语言ID
        LANGID LangId = ::GetThreadUILanguage();

        ::GetLocaleInfo(dwLang, LOCALE_SNAME, szBuf, _countof(szBuf));

        langInfo.wLangId = dwLang;
        langInfo.strLocaleName = szBuf;

        ::SetThreadUILanguage(dwLang);
        ::VerLanguageName(dwLang, szBuf, _countof(szBuf));

        langInfo.strLangName = szBuf;

        // 恢复当前线程 语言ID
        ::SetThreadUILanguage(LangId);

        return langInfo;
    }

    LANG_LIST GetLangList()
    {
        LANG_LIST langList;
        LANGID LangId = ::GetThreadUILanguage();

        ::EnumUILanguages([](LPTSTR  name, LONG_PTR lparam) -> BOOL {
            LANG_LIST* pList = (LANG_LIST*)lparam;
            LANGID LangId = (LANGID)_tcstoul(name, nullptr, 16);

            LANG_INFO langInfo = GetLanguageInfo(LangId);
            pList->emplace(LangId, langInfo);

            return TRUE;

            },
            MUI_LANGUAGE_ID,
            (LONG_PTR)&langList
        );

        ::SetThreadUILanguage(LangId);

        return langList;
    }

    std::wstring WStrFromUnicodeString(PUNICODE_STRING pString)
    {
        if (!pString->Buffer)
        {
            return std::wstring();
        }

        return std::wstring(pString->Buffer, pString->Length / sizeof(wchar_t));
    }

    // https://learn.microsoft.com/zh-cn/windows/win32/menurc/newheader
    typedef struct {
        WORD Reserved;                //保留;必须为零
        WORD ResType;                //资源类型 1: RES_ICON    2: RES_CURSOR
        WORD ResCount;                //资源组中的图标或游标组件数
    } ICON_GROUP_HEADER, * LPICON_GROUP_HEADER;

    // https://learn.microsoft.com/zh-cn/windows/win32/menurc/resdir
    // https://learn.microsoft.com/zh-cn/windows/win32/menurc/iconresdir
    typedef struct {
        BYTE Width;                    //图标的宽度（以像素为单位）。 可接受的值为 16、32 和 64
        BYTE Height;                //图标的高度（以像素为单位）。 可接受的值为 16、32 和 64
        BYTE ColorCount;            //图标中的颜色数。 可接受的值为 2、8 和 16。
        BYTE reserved;                //保留;必须设置为与图标文件标头中保留字段的值相同的值
        WORD Planes;                //图标或光标位图中的颜色平面数
        WORD BitCount;                //图标或光标位图中每像素的位数
        DWORD BytesInRes;            //资源的大小（以字节为单位）
        WORD IconId;                //具有唯一序号标识符的图标或光标
    } ICON_ENTRY, * LPICON_ENTRY;

    typedef struct {
        ICON_GROUP_HEADER Header;    //图标组头部
        ICON_ENTRY    IconEntry[1];    //单个图标信息
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

#pragma pack(pop)

    inline LPBYTE _GetAlignAddr(
        LPCVOID lpAddr,
        DWORD dwAlign
    )
    {
        DWORD_PTR dwPadding = ((DWORD_PTR)lpAddr % dwAlign);
        return dwPadding ? (LPBYTE)lpAddr + (dwAlign - dwPadding) : (LPBYTE)lpAddr;
    }

    using VER_STRING_INFO = std::map<_tstring, _tstring>;

    typedef struct _PE_FILE_VERSION_INFO
    {
        _tstring                strLocaleName;              // 区域名
        _tstring                strLangName;                // 语言名
        LANGANDCODEPAGE         LangAndCode;                // 语言与区域码
        VS_FIXEDFILEINFO        FixedFileInfo;              // 固定文件信息
        VER_STRING_INFO         VerString;                  // 版本信息
    }PE_FILE_VERSION_INFO;

    using VER_LANG_ID_INFO = std::map<DWORD, PE_FILE_VERSION_INFO>;

    bool LoadResourceVersion(HMODULE hModule, VER_LANG_ID_INFO& verLangInfo, const LANG_LIST& langList)
    {
        std::vector<LPTSTR> vResourceIds;
        // 枚举指定索引的资源
        ::EnumResourceNames(hModule, RT_VERSION, [](
            _In_opt_ HMODULE hModule,
            _In_ LPCTSTR lpType,
            _In_ LPTSTR lpName,
            _In_ LONG_PTR lParam
            ) -> BOOL {

                std::vector<LPTSTR>* pOut = (std::vector<LPTSTR>*)lParam;
                pOut->push_back(lpName);
                return TRUE;

            }, (LPARAM)&vResourceIds);

        for (const auto& resId : vResourceIds)
        {
            //从指定语言列表遍历
            for (const auto& item : langList)
            {
                HRSRC hRsrc = ::FindResourceEx(hModule, RT_VERSION, MAKEINTRESOURCE(resId), item.first);
                if (!hRsrc)
                {
                    continue;
                }

                HGLOBAL hGlobal = ::LoadResource(hModule, hRsrc);
                if (!hGlobal)
                {
                    continue;
                }

                //DWORD dwResourceSize = ::SizeofResource(hModule, hRsrc);
                LPVOID lpResourceData = ::LockResource(hGlobal);
                if (lpResourceData)
                {
                    VS_FIXEDFILEINFO FixedFileInfo = { 0 };
                    PVS_VERSIONINFO lpVersion = (PVS_VERSIONINFO)lpResourceData;
                    LPBYTE lpVerBeg = (LPBYTE)lpVersion;

                    // 存在 VS_FIXEDFILEINFO 信息
                    if (0 != lpVersion->wValueLength)
                    {
                        VS_FIXEDFILEINFO* pFixedFileInfo = (VS_FIXEDFILEINFO*)((LPBYTE)lpVersion + sizeof(VS_VERSIONINFO));
                        pFixedFileInfo = (VS_FIXEDFILEINFO*)_GetAlignAddr(pFixedFileInfo, sizeof(DWORD));
                        FixedFileInfo = *pFixedFileInfo;
                        lpVerBeg = (LPBYTE)lpVerBeg + sizeof(VS_VERSIONINFO) + lpVersion->wValueLength;
                    }

                    lpVerBeg = _GetAlignAddr((LPBYTE)lpVerBeg, sizeof(DWORD));
                    LPBYTE lpVerEnd = (LPBYTE)lpVersion + lpVersion->wLength;

                    // 解析数据
                    while (lpVerBeg < lpVerEnd)
                    {
                        StringFileInfo* pStringFileInfo = (StringFileInfo*)lpVerBeg;
                        pStringFileInfo = (StringFileInfo*)_GetAlignAddr(pStringFileInfo, sizeof(DWORD));

                        // 解析 "StringFileInfo" 块
                        if (0 == _wcsnicmp(L"StringFileInfo", pStringFileInfo->szKey, 14))
                        {
                            LPBYTE lpStringFileInfoBeg = (LPBYTE)pStringFileInfo;
                            LPBYTE lpStringFileInfoEnd = (LPBYTE)pStringFileInfo + pStringFileInfo->wLength;
                            lpStringFileInfoBeg = (LPBYTE)(pStringFileInfo->Children);

                            while (lpStringFileInfoBeg < lpStringFileInfoEnd)
                            {
                                StringTable* pStringTable = (StringTable*)lpStringFileInfoBeg;
                                _tstring strLangCode = WStrToTStr(std::wstring(pStringTable->szKey, _countof(pStringTable->szKey)));

                                DWORD dwValue = 0;
                                ::_stscanf_s(strLangCode.c_str(), _T("%08X"), &dwValue);

                                PE_FILE_VERSION_INFO verInfo;
                                verInfo.FixedFileInfo = FixedFileInfo;

                                // 语言
                                verInfo.LangAndCode.wCodePage = LOWORD(dwValue);
                                verInfo.LangAndCode.wLangId = HIWORD(dwValue);

                                // 语言名
                                LANG_INFO langInfo = GetLanguageInfo(verInfo.LangAndCode.wLangId);
                                verInfo.strLocaleName = langInfo.strLocaleName;
                                verInfo.strLangName = langInfo.strLangName;

                                // 解析字符串
                                LPBYTE lpStringBeg = (LPBYTE)pStringTable->Children;
                                LPBYTE lpStringEnd = (LPBYTE)pStringTable + pStringTable->wLength;
                                while (lpStringBeg < lpStringEnd)
                                {
                                    const String* pString = (String*)lpStringBeg;

                                    //值名
                                    std::wstring strKey = pString->szKey;

                                    //值数据
                                    LPCWSTR lpStrValue = pString->szKey + strKey.size() + 1;

                                    //值数据位置
                                    lpStrValue = (LPCWSTR)_GetAlignAddr(lpStrValue, sizeof(DWORD));

                                    //字符串类型
                                    if (1 == pString->wType)
                                    {
                                        verInfo.VerString.emplace(WStrToTStr(pString->szKey), WStrToTStr(std::wstring(lpStrValue, pString->wValueLength)));
                                    }

                                    lpStringBeg = _GetAlignAddr(lpStringBeg + pString->wLength, sizeof(DWORD));
                                }

                                if (langList.empty() || langList.end() != langList.find(verInfo.LangAndCode.wLangId))
                                {
                                    verLangInfo.emplace(verInfo.LangAndCode.wLangId, verInfo);
                                }

                                lpStringFileInfoBeg = _GetAlignAddr((LPBYTE)pStringTable + pStringTable->wLength, sizeof(DWORD));
                            }
                        }
                        // 解析 "VarFileInfo" 块
                        else if (0 == _wcsnicmp(L"VarFileInfo", pStringFileInfo->szKey, 11))
                        {
                            VarFileInfo* pVerFileInfo = (VarFileInfo*)_GetAlignAddr(pStringFileInfo, sizeof(DWORD));
                            Var* pVar = (Var*)pVerFileInfo->Children;
                            if (0 == _wcsnicmp(L"Translation", pVar->szKey, 11))
                            {
                                for (DWORD i = 0; i < pVar->wValueLength / sizeof(DWORD); i++)
                                {
                                    LANGANDCODEPAGE transInfo;
                                    transInfo.wCodePage = pVar->Value[i].CodePageID;
                                    transInfo.wLangId = pVar->Value[i].LanguageID;
                                }
                            }
                        }

                        lpVerBeg = _GetAlignAddr(lpVerBeg + pStringFileInfo->wLength, sizeof(DWORD));
                    }
                }
            }
        }

        return true;
    }

    VER_LANG_ID_LIST GetResourceVersion(HMODULE hModule, const LANG_LIST& langList)
    {
        VER_LANG_ID_LIST infoResult;
        VER_LANG_ID_INFO fileInfo;

        LoadResourceVersion(hModule, fileInfo, langList);

        for (const auto& item : fileInfo)
        {
            VERSION_INFO verInfo;

            auto _QueryInfo = [](const VER_STRING_INFO& StrList, const _tstring name) -> _tstring {

                auto itFind = StrList.find(name);
                if (StrList.end() != itFind)
                {
                    return itFind->second;
                }

                return _tstring();

                };

            const VER_STRING_INFO& VerString = item.second.VerString;

            verInfo.strComments = _QueryInfo(VerString, _T("Comments"));
            verInfo.strInternalName = _QueryInfo(VerString, _T("InternalName"));
            verInfo.strProductName = _QueryInfo(VerString, _T("ProductName"));
            verInfo.strCompanyName = _QueryInfo(VerString, _T("CompanyName"));
            verInfo.strLegalCopyright = _QueryInfo(VerString, _T("LegalCopyright"));
            verInfo.strProductVersion = _QueryInfo(VerString, _T("ProductVersion"));
            verInfo.strFileDescription = _QueryInfo(VerString, _T("FileDescription"));
            verInfo.strLegalTrademarks = _QueryInfo(VerString, _T("LegalTrademarks"));
            verInfo.strPrivateBuild = _QueryInfo(VerString, _T("PrivateBuild"));
            verInfo.strFileVersion = _QueryInfo(VerString, _T("FileVersion"));
            verInfo.strOriginalFilename = _QueryInfo(VerString, _T("OriginalFilename"));
            verInfo.strSpecialBuild = _QueryInfo(VerString, _T("SpecialBuild"));

            verInfo.vsFixedInfo = *(VS_VER_FIXEDFILEINFO*)(&item.second.LangAndCode);
            verInfo.langAndCodePage = item.second.LangAndCode;

            const VS_VER_FIXEDFILEINFO& vsFixedInfo = verInfo.vsFixedInfo;

            // 获取版本字符串回调
            auto _GetVersionStr = [](VERSON_NUMBER hi, VERSON_NUMBER lo) -> _tstring {
                TCHAR szBuf[MAX_PATH] = { 0 };
                (void)::StringCchPrintf(szBuf, _countof(szBuf), _T("%hd.%hd.%hd.%hd"),
                    hi.Version.wHigh,
                    hi.Version.wLow,
                    lo.Version.wHigh,
                    lo.Version.wLow
                );

                return szBuf;

                };

            verInfo.strFileVersionEx = _GetVersionStr(vsFixedInfo.dwFileVersionMS, vsFixedInfo.dwFileVersionLS);
            verInfo.strProductVersionEx = _GetVersionStr(vsFixedInfo.dwProductVersionMS, vsFixedInfo.dwProductVersionLS);

            verInfo.wLangId = item.first;

            auto itLang = langList.find(item.first);
            if (itLang != langList.end())
            {
                verInfo.strLocaleName = itLang->second.strLocaleName;
                verInfo.strLangName = itLang->second.strLangName;
            }

            verInfo.FileVerNumber = verInfo.strFileVersionEx.c_str();
            verInfo.ProductVerNumber = verInfo.strProductVersionEx.c_str();

            infoResult.emplace(item.first, verInfo);
        }

        return infoResult;
    }

    SYS_PROCESS_INFO_LIST GetProcessListEx(const _tstring& strName/* = _T("")*/, DWORD dwPID/* = 0*/, bool fMultilingual/* = true*/)
    {
        SYS_PROCESS_INFO_LIST infoList;
        PSYSTEM_PROCESS_INFORMATION pSpi = NULL;
        NTSTATUS ntStatus = STATUS_SUCCESS;
        ULONG uDataLength = 0;
        ULONG uReturnLength = 0;
        bool fResult = false;
        DWORD dwQueryAccess = GetQueryAccess();

        do
        {
            ntStatus = NtQuerySystemInformation(
                SystemProcessInformation,
                NULL,
                0,
                &uReturnLength
            );
            if (STATUS_INFO_LENGTH_MISMATCH != ntStatus)
            {
                break;
            }

            uDataLength = uReturnLength * 2;

            pSpi = (PSYSTEM_PROCESS_INFORMATION)::HeapAlloc(
                ::GetProcessHeap(),
                HEAP_ZERO_MEMORY,
                uDataLength
            );
            if (!pSpi)
            {
                break;
            }

            ntStatus = NtQuerySystemInformation(
                SystemProcessInformation,
                pSpi,
                uDataLength,
                &uReturnLength
            );
            if (STATUS_SUCCESS != ntStatus)
            {
                break;
            }

            DRIVE_DOS_LIST DriveDosPathList = _GetDriveDosPathList();
            LANGID LangId = ::GetThreadUILanguage();
            LANG_LIST langTempList = GetLangList();
            LANG_LIST langList;

            if (!fMultilingual)
            {
                auto itFind = langTempList.find(LangId);
                if (langTempList.end() != itFind)
                {
                    langList.emplace(*itFind);
                }
            }
            else
            {
                langList = std::move(langTempList);
            }

            PSYSTEM_PROCESS_INFORMATION pSpiTemp = pSpi;
            while (pSpiTemp->NextEntryOffset)
            {
                _tstring strImageName = WStrToTStr(WStrFromUnicodeString(&pSpiTemp->ImageName));
                DWORD dwProcessId = (DWORD)pSpiTemp->UniqueProcessId;

                if ((0 == dwPID || dwPID == dwProcessId) &&
                    (strName.empty() || (0 == _tcsicmp(strName.c_str(), strImageName.c_str())))
                    )
                {
                    PSYSTEM_THREAD_INFORMATION pStiTemp = pSpiTemp->Threads;
                    bool fSuspend = pSpiTemp->NumberOfThreads ? true : false;

                    for (ULONG i = 0; i < pSpiTemp->NumberOfThreads; i++)
                    {
                        if (KWAIT_REASON::Suspended != pStiTemp->WaitReason)
                        {
                            fSuspend = false;
                            break;
                        }

                        pStiTemp++;
                    }

                    SYS_PROCESS_INFO info;

                    info.fSuspend = fSuspend;

                    // 标准信息
                    info.NumberOfThreads = pSpiTemp->NumberOfThreads;
                    info.WorkingSetPrivateSize = pSpiTemp->WorkingSetPrivateSize.QuadPart;
                    info.HardFaultCount = pSpiTemp->HardFaultCount;
                    info.NumberOfThreadsHighWatermark = pSpiTemp->NumberOfThreadsHighWatermark;
                    info.CycleTime = pSpiTemp->CycleTime;
                    info.CreateTime = pSpiTemp->CreateTime.QuadPart;
                    info.UserTime = pSpiTemp->UserTime.QuadPart;
                    info.KernelTime = pSpiTemp->KernelTime.QuadPart;
                    info.ImageName = strImageName;
                    info.BasePriority = pSpiTemp->BasePriority;
                    info.UniqueProcessId = (uint64_t)pSpiTemp->UniqueProcessId;
                    info.UniqueParentProcessId = (uint64_t)pSpiTemp->UniqueParentProcessId;
                    info.HandleCount = pSpiTemp->HandleCount;
                    info.SessionId = pSpiTemp->SessionId;
                    info.UniqueProcessKey = pSpiTemp->UniqueProcessKey;
                    info.PeakVirtualSize = pSpiTemp->PeakVirtualSize;
                    info.VirtualSize = pSpiTemp->VirtualSize;
                    info.PageFaultCount = pSpiTemp->PageFaultCount;
                    info.PeakWorkingSetSize = pSpiTemp->PeakWorkingSetSize;
                    info.WorkingSetSize = pSpiTemp->WorkingSetSize;
                    info.QuotaPeakPagedPoolUsage = pSpiTemp->QuotaPeakPagedPoolUsage;
                    info.QuotaPagedPoolUsage = pSpiTemp->QuotaPagedPoolUsage;
                    info.QuotaPeakNonPagedPoolUsage = pSpiTemp->QuotaPeakNonPagedPoolUsage;
                    info.QuotaNonPagedPoolUsage = pSpiTemp->QuotaNonPagedPoolUsage;
                    info.PagefileUsage = pSpiTemp->PagefileUsage;
                    info.PeakPagefileUsage = pSpiTemp->PeakPagefileUsage;
                    info.PrivatePageCount = pSpiTemp->PrivatePageCount;
                    info.ReadOperationCount = pSpiTemp->ReadOperationCount.QuadPart;
                    info.WriteOperationCount = pSpiTemp->WriteOperationCount.QuadPart;
                    info.OtherOperationCount = pSpiTemp->OtherOperationCount.QuadPart;
                    info.ReadTransferCount = pSpiTemp->ReadTransferCount.QuadPart;
                    info.WriteTransferCount = pSpiTemp->WriteTransferCount.QuadPart;
                    info.OtherTransferCount = pSpiTemp->OtherTransferCount.QuadPart;

                    HANDLE hProcess = NULL;
                    do
                    {
                        TCHAR szBuffer[MAX_PATH] = { 0 };

                        // 打开进程
                        hProcess = ::OpenProcess(dwQueryAccess, FALSE, info.UniqueProcessId);
                        if (NULL == hProcess)
                        {
                            break;
                        }

                        // 进程句柄信息
                        ::GetProcessHandleCount(hProcess, (LPDWORD)&info.HandleCount);
                        info.GdiCount = ::GetGuiResources(hProcess, GR_GDIOBJECTS);
                        info.UserCount = ::GetGuiResources(hProcess, GR_USEROBJECTS);

                        // 句柄必须具有 PROCESS_QUERY_INFORMATION 或 PROCESS_QUERY_LIMITED_INFORMATION 访问权限。
                        // Windows Server 2003 和 Windows XP： 句柄必须具有 PROCESS_QUERY_INFORMATION 访问权限。
                        if (0 != ::GetProcessImageFileName(hProcess, szBuffer, _countof(szBuffer)))
                        {
                            for (const auto& item : DriveDosPathList)
                            {
                                if (_T('\\') != szBuffer[item.second.size()])
                                {
                                    continue;
                                }

                                if (0 == _tcsncmp(item.second.c_str(), szBuffer, item.second.size()))
                                {
                                    info.FilePath = item.first + &szBuffer[item.second.size()];
                                    break;
                                }
                            }
                        }

                        if (!info.FilePath.empty())
                        {
                            PVOID pFsRedirectionOldValue = NULL;
                            bool isDisableWow64Fs = false;

                            // 禁用文件重定向
                            isDisableWow64Fs = ::Wow64DisableWow64FsRedirection(&pFsRedirectionOldValue);

                            // 以数据资源方式加载PE文件
                            HMODULE hModule = NULL;

                            do
                            {
                                hModule = ::LoadLibraryEx(
                                    info.FilePath.c_str(),
                                    0, LOAD_LIBRARY_AS_DATAFILE | LOAD_LIBRARY_AS_IMAGE_RESOURCE
                                );

                                if (!hModule)
                                {
                                    break;
                                }

                                // 获取PE文件头信息
                                info.PeHeader.LoadFromModule(hModule);

                                // 获取版本信息
                                info.Versions = GetResourceVersion(hModule, langList);


                            } while (false);

                            if (hModule)
                            {
                                ::FreeLibrary(hModule);
                            }

                            // 恢复文件重定向
                            if (isDisableWow64Fs)
                            {
                                ::Wow64RevertWow64FsRedirection(pFsRedirectionOldValue);
                            }
                        }

                        if (!info.Versions.empty())
                        {
                            auto itFind = info.Versions.find(LangId);
                            if (info.Versions.end() != itFind)
                            {
                                info.FileDesc = itFind->second.strFileDescription;
                            }
                        }

                        if (0 == info.UniqueProcessId)
                        {
                            info.ImageName = _T("[System Process]");
                        }

                        if (info.FileDesc.empty())
                        {
                            info.FileDesc = info.ImageName;
                        }

                        // 获取进程组信息, 耗时比较高
                        if (GET_PROCESS_GROUP_INFO)
                        {
                            info.Groups = _GetProcessTokenGroups(hProcess);
                        }

                    } while (false);

                    if (NULL != hProcess)
                    {
                        ::CloseHandle(hProcess);
                    }

                    infoList.emplace((DWORD)pSpiTemp->UniqueProcessId, info);
                }

                pSpiTemp = (PSYSTEM_PROCESS_INFORMATION)((uint8_t*)pSpiTemp + pSpiTemp->NextEntryOffset);
            }

            fResult = true;

        } while (false);

        if (pSpi)
        {
            ::HeapFree(::GetProcessHeap(), 0, pSpi);
        }

        // 获取进程用户名信息
        PROC_SID_LIST mapResult = GetProcessAccountSidList();

        for (auto& item : infoList)
        {
            auto itFind = mapResult.find(item.first);
            if (mapResult.end() != itFind)
            {
                item.second.SidInfo = itFind->second;
            }
        }

        return infoList;
    }
}