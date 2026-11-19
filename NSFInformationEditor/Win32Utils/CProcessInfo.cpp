#include "CProcessInfo.h"
#include <ntstatus.h>
#include <stdint.h>

#define NT_SUCCESS(Status) (((NTSTATUS)(Status)) >= 0)
#define NT_INFORMATION(Status) ((((ULONG)(Status)) >> 30) == 1)
#define NT_WARNING(Status) ((((ULONG)(Status)) >> 30) == 2)
#define NT_ERROR(Status) ((((ULONG)(Status)) >> 30) == 3)

namespace CProcessInfo
{
    typedef NTSTATUS(NTAPI* pFnNtQueryInformationProcess)(
        IN            HANDLE           ProcessHandle,
        IN            PROCESSINFOCLASS ProcessInformationClass,
        OUT           PVOID            ProcessInformation,
        IN            ULONG            ProcessInformationLength,
        OUT OPTIONAL  PULONG           ReturnLength
        );

    typedef NTSTATUS (NTAPI* pFnNtReadVirtualMemory)(
        _In_        HANDLE  ProcessHandle,
        _In_opt_    PVOID   BaseAddress,
        _Out_writes_bytes_to_(NumberOfBytesToRead, *NumberOfBytesRead) PVOID Buffer,
        _In_        SIZE_T NumberOfBytesToRead,
        _Out_opt_   PSIZE_T NumberOfBytesRead
    );

    typedef NTSTATUS(NTAPI* pFnRtlGetVersion)(
        _Out_ PRTL_OSVERSIONINFOEXW VersionInformation // PRTL_OSVERSIONINFOW
    );

    pFnNtQueryInformationProcess _NtQueryInformationProcess = nullptr;
    pFnNtReadVirtualMemory _NtReadVirtualMemory = nullptr;
    pFnRtlGetVersion _RtlGetVersion = nullptr;

    NTSTATUS NTAPI NtQueryInformationProcess(
        IN            HANDLE           ProcessHandle,
        IN            PROCESSINFOCLASS ProcessInformationClass,
        OUT           PVOID            ProcessInformation,
        IN            ULONG            ProcessInformationLength,
        OUT OPTIONAL  PULONG           ReturnLength
    )
    {
        if (!_NtQueryInformationProcess)
        {
            do
            {
                HMODULE hModule = ::GetModuleHandle(_T("Ntdll.dll"));
                if (!hModule)
                {
                    break;
                }

                _NtQueryInformationProcess = (pFnNtQueryInformationProcess)::GetProcAddress(
                    hModule,
                    "NtQueryInformationProcess"
                );
                if (!_NtQueryInformationProcess)
                {
                    break;
                }

            } while (false);
        }

        if (!_NtQueryInformationProcess)
        {
            return 0;
        }

        return _NtQueryInformationProcess(
            ProcessHandle,
            ProcessInformationClass,
            ProcessInformation,
            ProcessInformationLength,
            ReturnLength
        );
    }

    NTSTATUS NTAPI NtReadVirtualMemory(
        _In_ HANDLE ProcessHandle,
        _In_opt_ PVOID BaseAddress,
        _Out_writes_bytes_to_(NumberOfBytesToRead, *NumberOfBytesRead) PVOID Buffer,
        _In_ SIZE_T NumberOfBytesToRead,
        _Out_opt_ PSIZE_T NumberOfBytesRead
    )
    {
        if (!_NtReadVirtualMemory)
        {
            do
            {
                HMODULE hModule = ::GetModuleHandle(_T("Ntdll.dll"));
                if (!hModule)
                {
                    break;
                }

                _NtReadVirtualMemory = (pFnNtReadVirtualMemory)::GetProcAddress(
                    hModule,
                    "NtReadVirtualMemory"
                );
                if (!_NtReadVirtualMemory)
                {
                    break;
                }

            } while (false);
        }

        if (!_NtReadVirtualMemory)
        {
            return 0;
        }

        return _NtReadVirtualMemory(
            ProcessHandle,
            BaseAddress,
            Buffer,
            NumberOfBytesToRead,
            NumberOfBytesRead
        );
    }

    NTSTATUS NTAPI RtlGetVersion(
        _Out_ PRTL_OSVERSIONINFOEXW VersionInformation // PRTL_OSVERSIONINFOW
        )
    {
        if (!_RtlGetVersion)
        {
            do
            {
                HMODULE hModule = ::GetModuleHandle(_T("Ntdll.dll"));
                if (!hModule)
                {
                    break;
                }

                _RtlGetVersion = (pFnRtlGetVersion)::GetProcAddress(
                    hModule,
                    "RtlGetVersion"
                );
                if (!_RtlGetVersion)
                {
                    break;
                }

            } while (false);
        }

        if (!_RtlGetVersion)
        {
            return 0;
        }

        return _RtlGetVersion(VersionInformation);
    }

    NTSTATUS GetProcessBasicInformation(
        _In_ HANDLE ProcessHandle,
        _Out_ PPROCESS_BASIC_INFORMATION BasicInformation
    )
    {
        return NtQueryInformationProcess(
            ProcessHandle,
            ProcessBasicInformation,
            BasicInformation,
            sizeof(PROCESS_BASIC_INFORMATION),
            NULL
        );
    }
    
    NTSTATUS PhGetProcessPeb(
        _In_ HANDLE ProcessHandle,
        _Out_ PVOID* PebBaseAddress
    )
    {
        NTSTATUS status;
        PROCESS_BASIC_INFORMATION basicInfo;

        status = GetProcessBasicInformation(ProcessHandle, &basicInfo);

        if (NT_SUCCESS(status))
        {
            // No PEB for System, Minimal or Pico processes. (dmex)
            if (!basicInfo.PebBaseAddress)
                return STATUS_UNSUCCESSFUL;

            *PebBaseAddress = (PVOID)basicInfo.PebBaseAddress;
        }

        return status;
    }

    typedef BOOLEAN(NTAPI* PLDR_INIT_ROUTINE)(
        _In_ PVOID DllHandle,
        _In_ ULONG Reason,
        _In_opt_ PVOID Context
        );

    typedef struct _ACTIVATION_CONTEXT* PACTIVATION_CONTEXT;
    typedef struct _LDRP_LOAD_CONTEXT* PLDRP_LOAD_CONTEXT;

    // symbols
    typedef struct _LDR_SERVICE_TAG_RECORD
    {
        struct _LDR_SERVICE_TAG_RECORD* Next;
        ULONG ServiceTag;
    } LDR_SERVICE_TAG_RECORD, * PLDR_SERVICE_TAG_RECORD;

    // symbols
    typedef struct _LDRP_CSLIST
    {
        PSINGLE_LIST_ENTRY Tail;
    } LDRP_CSLIST, * PLDRP_CSLIST;

    // symbols
    typedef enum _LDR_DDAG_STATE
    {
        LdrModulesMerged = -5,
        LdrModulesInitError = -4,
        LdrModulesSnapError = -3,
        LdrModulesUnloaded = -2,
        LdrModulesUnloading = -1,
        LdrModulesPlaceHolder = 0,
        LdrModulesMapping = 1,
        LdrModulesMapped = 2,
        LdrModulesWaitingForDependencies = 3,
        LdrModulesSnapping = 4,
        LdrModulesSnapped = 5,
        LdrModulesCondensed = 6,
        LdrModulesReadyToInit = 7,
        LdrModulesInitializing = 8,
        LdrModulesReadyToRun = 9
    } LDR_DDAG_STATE;

    // symbols
    typedef struct _LDR_DDAG_NODE
    {
        LIST_ENTRY Modules;
        PLDR_SERVICE_TAG_RECORD ServiceTagList;
        ULONG LoadCount;
        ULONG LoadWhileUnloadingCount;
        ULONG LowestLink;
        union
        {
            LDRP_CSLIST Dependencies;
            SINGLE_LIST_ENTRY RemovalLink;
        };
        LDRP_CSLIST IncomingDependencies;
        LDR_DDAG_STATE State;
        SINGLE_LIST_ENTRY CondenseLink;
        ULONG PreorderNumber;
    } LDR_DDAG_NODE, * PLDR_DDAG_NODE;

    typedef struct _RTL_BALANCED_NODE
    {
        union
        {
            struct _RTL_BALANCED_NODE* Children[2];
            struct
            {
                struct _RTL_BALANCED_NODE* Left;
                struct _RTL_BALANCED_NODE* Right;
            };
        };
        union
        {
            UCHAR Red : 1;
            UCHAR Balance : 2;
            ULONG_PTR ParentValue;
        };
    } RTL_BALANCED_NODE, * PRTL_BALANCED_NODE;

    // symbols
    typedef enum _LDR_DLL_LOAD_REASON
    {
        LoadReasonStaticDependency,
        LoadReasonStaticForwarderDependency,
        LoadReasonDynamicForwarderDependency,
        LoadReasonDelayloadDependency,
        LoadReasonDynamicLoad,
        LoadReasonAsImageLoad,
        LoadReasonAsDataLoad,
        LoadReasonEnclavePrimary, // since REDSTONE3
        LoadReasonEnclaveDependency,
        LoadReasonPatchImage, // since WIN11
        LoadReasonUnknown = -1
    } LDR_DLL_LOAD_REASON, * PLDR_DLL_LOAD_REASON;

    typedef enum _LDR_HOT_PATCH_STATE
    {
        LdrHotPatchBaseImage,
        LdrHotPatchNotApplied,
        LdrHotPatchAppliedReverse,
        LdrHotPatchAppliedForward,
        LdrHotPatchFailedToPatch,
        LdrHotPatchStateMax,
    } LDR_HOT_PATCH_STATE, * PLDR_HOT_PATCH_STATE;

    // symbols
    typedef struct _LDR_DATA_TABLE_ENTRY
    {
        LIST_ENTRY InLoadOrderLinks;
        LIST_ENTRY InMemoryOrderLinks;
        LIST_ENTRY InInitializationOrderLinks;
        PVOID DllBase;
        PLDR_INIT_ROUTINE EntryPoint;
        ULONG SizeOfImage;
        UNICODE_STRING FullDllName;
        UNICODE_STRING BaseDllName;
        union
        {
            UCHAR FlagGroup[4];
            ULONG Flags;
            struct
            {
                ULONG PackagedBinary : 1;
                ULONG MarkedForRemoval : 1;
                ULONG ImageDll : 1;
                ULONG LoadNotificationsSent : 1;
                ULONG TelemetryEntryProcessed : 1;
                ULONG ProcessStaticImport : 1;
                ULONG InLegacyLists : 1;
                ULONG InIndexes : 1;
                ULONG ShimDll : 1;
                ULONG InExceptionTable : 1;
                ULONG ReservedFlags1 : 2;
                ULONG LoadInProgress : 1;
                ULONG LoadConfigProcessed : 1;
                ULONG EntryProcessed : 1;
                ULONG ProtectDelayLoad : 1;
                ULONG ReservedFlags3 : 2;
                ULONG DontCallForThreads : 1;
                ULONG ProcessAttachCalled : 1;
                ULONG ProcessAttachFailed : 1;
                ULONG CorDeferredValidate : 1;
                ULONG CorImage : 1;
                ULONG DontRelocate : 1;
                ULONG CorILOnly : 1;
                ULONG ChpeImage : 1;
                ULONG ChpeEmulatorImage : 1;
                ULONG ReservedFlags5 : 1;
                ULONG Redirected : 1;
                ULONG ReservedFlags6 : 2;
                ULONG CompatDatabaseProcessed : 1;
            };
        };
        USHORT ObsoleteLoadCount;
        USHORT TlsIndex;
        LIST_ENTRY HashLinks;
        ULONG TimeDateStamp;
        PACTIVATION_CONTEXT EntryPointActivationContext;
        PVOID Lock; // RtlAcquireSRWLockExclusive
        PLDR_DDAG_NODE DdagNode;
        LIST_ENTRY NodeModuleLink;
        PLDRP_LOAD_CONTEXT LoadContext;
        PVOID ParentDllBase;
        PVOID SwitchBackContext;
        RTL_BALANCED_NODE BaseAddressIndexNode;
        RTL_BALANCED_NODE MappingInfoIndexNode;
        ULONG_PTR OriginalBase;
        LARGE_INTEGER LoadTime;
        ULONG BaseNameHashValue;
        LDR_DLL_LOAD_REASON LoadReason; // since WIN8
        ULONG ImplicitPathOptions;
        ULONG ReferenceCount; // since WIN10
        ULONG DependentLoadFlags;
        UCHAR SigningLevel; // since REDSTONE2
        ULONG CheckSum; // since 22H1
        PVOID ActivePatchImageBase;
        LDR_HOT_PATCH_STATE HotPatchState;
    } LDR_DATA_TABLE_ENTRY, * PLDR_DATA_TABLE_ENTRY;

    _Function_class_(PHP_ENUM_PROCESS_MODULES_CALLBACK)
        typedef BOOLEAN(NTAPI PHP_ENUM_PROCESS_MODULES_CALLBACK)(
            _In_ HANDLE ProcessHandle,
            _In_ PLDR_DATA_TABLE_ENTRY Entry,
            _In_ PVOID AddressOfEntry,
            _In_opt_ PVOID Context1,
            _In_opt_ PVOID Context2
            );
    typedef PHP_ENUM_PROCESS_MODULES_CALLBACK* PPHP_ENUM_PROCESS_MODULES_CALLBACK;

#define WINDOWS_ANCIENT 0
#define WINDOWS_XP 51 // August, 2001
#define WINDOWS_SERVER_2003 52 // April, 2003
#define WINDOWS_VISTA 60 // November, 2006
#define WINDOWS_7 61 // July, 2009
#define WINDOWS_8 62 // August, 2012
#define WINDOWS_8_1 63 // August, 2013
#define WINDOWS_10 100 // TH1 // July, 2015
#define WINDOWS_10_TH2 101 // November, 2015
#define WINDOWS_10_RS1 102 // August, 2016
#define WINDOWS_10_RS2 103 // April, 2017
#define WINDOWS_10_RS3 104 // October, 2017
#define WINDOWS_10_RS4 105 // April, 2018
#define WINDOWS_10_RS5 106 // November, 2018
#define WINDOWS_10_19H1 107 // May, 2019
#define WINDOWS_10_19H2 108 // November, 2019
#define WINDOWS_10_20H1 109 // May, 2020
#define WINDOWS_10_20H2 110 // October, 2020
#define WINDOWS_10_21H1 111 // May, 2021
#define WINDOWS_10_21H2 112 // November, 2021
#define WINDOWS_10_22H2 113 // October, 2022
#define WINDOWS_11 114 // October, 2021
#define WINDOWS_11_22H2 115 // September, 2022
#define WINDOWS_11_23H2 116 // October, 2023
#define WINDOWS_11_24H2 117 // TBA
#define WINDOWS_NEW ULONG_MAX
#define PH_ENUM_PROCESS_MODULES_LIMIT 0x800
#define PTR_ADD_OFFSET(Pointer, Offset) ((PVOID)((ULONG_PTR)(Pointer) + (ULONG_PTR)(Offset)))
#define PTR_SUB_OFFSET(Pointer, Offset) ((PVOID)((ULONG_PTR)(Pointer) - (ULONG_PTR)(Offset)))

    RTL_OSVERSIONINFOEXW PhOsVersion = { 0 };

    ULONG WindowsVersion = WINDOWS_NEW;
    VOID PhInitializeWindowsVersion(VOID)
    {
        RTL_OSVERSIONINFOEXW versionInfo;
        ULONG majorVersion;
        ULONG minorVersion;
        ULONG buildVersion;

        memset(&versionInfo, 0, sizeof(RTL_OSVERSIONINFOEXW));
        versionInfo.dwOSVersionInfoSize = sizeof(RTL_OSVERSIONINFOEXW);

        if (!NT_SUCCESS(RtlGetVersion(&versionInfo)))
        {
            WindowsVersion = WINDOWS_ANCIENT;
            return;
        }

        memcpy(&PhOsVersion, &versionInfo, sizeof(RTL_OSVERSIONINFOEXW));
        majorVersion = versionInfo.dwMajorVersion;
        minorVersion = versionInfo.dwMinorVersion;
        buildVersion = versionInfo.dwBuildNumber;

        if (majorVersion == 6 && minorVersion < 1 || majorVersion < 6)
        {
            WindowsVersion = WINDOWS_ANCIENT;
        }
        // Windows 7, Windows Server 2008 R2
        else if (majorVersion == 6 && minorVersion == 1)
        {
            WindowsVersion = WINDOWS_7;
        }
        // Windows 8, Windows Server 2012
        else if (majorVersion == 6 && minorVersion == 2)
        {
            WindowsVersion = WINDOWS_8;
        }
        // Windows 8.1, Windows Server 2012 R2
        else if (majorVersion == 6 && minorVersion == 3)
        {
            WindowsVersion = WINDOWS_8_1;
        }
        // Windows 10, Windows Server 2016
        else if (majorVersion == 10 && minorVersion == 0)
        {
            if (buildVersion > 26100)
            {
                WindowsVersion = WINDOWS_NEW;
            }
            else if (buildVersion >= 26100)
            {
                WindowsVersion = WINDOWS_11_24H2;
            }
            else if (buildVersion >= 22631)
            {
                WindowsVersion = WINDOWS_11_23H2;
            }
            else if (buildVersion >= 22621)
            {
                WindowsVersion = WINDOWS_11_22H2;
            }
            else if (buildVersion >= 22000)
            {
                WindowsVersion = WINDOWS_11;
            }
            else if (buildVersion >= 19045)
            {
                WindowsVersion = WINDOWS_10_22H2;
            }
            else if (buildVersion >= 19044)
            {
                WindowsVersion = WINDOWS_10_21H2;
            }
            else if (buildVersion >= 19043)
            {
                WindowsVersion = WINDOWS_10_21H1;
            }
            else if (buildVersion >= 19042)
            {
                WindowsVersion = WINDOWS_10_20H2;
            }
            else if (buildVersion >= 19041)
            {
                WindowsVersion = WINDOWS_10_20H1;
            }
            else if (buildVersion >= 18363)
            {
                WindowsVersion = WINDOWS_10_19H2;
            }
            else if (buildVersion >= 18362)
            {
                WindowsVersion = WINDOWS_10_19H1;
            }
            else if (buildVersion >= 17763)
            {
                WindowsVersion = WINDOWS_10_RS5;
            }
            else if (buildVersion >= 17134)
            {
                WindowsVersion = WINDOWS_10_RS4;
            }
            else if (buildVersion >= 16299)
            {
                WindowsVersion = WINDOWS_10_RS3;
            }
            else if (buildVersion >= 15063)
            {
                WindowsVersion = WINDOWS_10_RS2;
            }
            else if (buildVersion >= 14393)
            {
                WindowsVersion = WINDOWS_10_RS1;
            }
            else if (buildVersion >= 10586)
            {
                WindowsVersion = WINDOWS_10_TH2;
            }
            else if (buildVersion >= 10240)
            {
                WindowsVersion = WINDOWS_10;
            }
            else
            {
                WindowsVersion = WINDOWS_10;
            }
        }
        else
        {
            WindowsVersion = WINDOWS_NEW;
        }
    }

#define LDR_DATA_TABLE_ENTRY_SIZE_WINXP_32 FIELD_OFFSET(LDR_DATA_TABLE_ENTRY32, DdagNode)
#define LDR_DATA_TABLE_ENTRY_SIZE_WIN7_32 FIELD_OFFSET(LDR_DATA_TABLE_ENTRY32, BaseNameHashValue)
#define LDR_DATA_TABLE_ENTRY_SIZE_WIN8_32 FIELD_OFFSET(LDR_DATA_TABLE_ENTRY32, ImplicitPathOptions)
#define LDR_DATA_TABLE_ENTRY_SIZE_WIN10_32 FIELD_OFFSET(LDR_DATA_TABLE_ENTRY32, SigningLevel)
#define LDR_DATA_TABLE_ENTRY_SIZE_WIN11_32 sizeof(LDR_DATA_TABLE_ENTRY32)

#define LDR_DATA_TABLE_ENTRY_SIZE_WINXP FIELD_OFFSET(LDR_DATA_TABLE_ENTRY, DdagNode)
#define LDR_DATA_TABLE_ENTRY_SIZE_WIN7 FIELD_OFFSET(LDR_DATA_TABLE_ENTRY, BaseNameHashValue)
#define LDR_DATA_TABLE_ENTRY_SIZE_WIN8 FIELD_OFFSET(LDR_DATA_TABLE_ENTRY, ImplicitPathOptions)
#define LDR_DATA_TABLE_ENTRY_SIZE_WIN10 FIELD_OFFSET(LDR_DATA_TABLE_ENTRY, SigningLevel)
#define LDR_DATA_TABLE_ENTRY_SIZE_WIN11 sizeof(LDR_DATA_TABLE_ENTRY)

#define WOW64_POINTER(Type) ULONG

    typedef struct _RTL_BALANCED_NODE32
    {
        union
        {
            WOW64_POINTER(struct _RTL_BALANCED_NODE*) Children[2];
            struct
            {
                WOW64_POINTER(struct _RTL_BALANCED_NODE*) Left;
                WOW64_POINTER(struct _RTL_BALANCED_NODE*) Right;
            };
        };
        union
        {
            WOW64_POINTER(UCHAR) Red : 1;
            WOW64_POINTER(UCHAR) Balance : 2;
            WOW64_POINTER(ULONG_PTR) ParentValue;
        };
    } RTL_BALANCED_NODE32, * PRTL_BALANCED_NODE32;

    typedef struct _STRING32
    {
        USHORT Length;
        USHORT MaximumLength;
        ULONG Buffer;
    } STRING32, * PSTRING32;

    typedef STRING32 UNICODE_STRING32, * PUNICODE_STRING32;
    typedef STRING32 ANSI_STRING32, * PANSI_STRING32;

    typedef struct _STRING64
    {
        USHORT Length;
        USHORT MaximumLength;
        ULONGLONG Buffer;
    } STRING64, * PSTRING64;

    typedef STRING64 UNICODE_STRING64, * PUNICODE_STRING64;
    typedef STRING64 ANSI_STRING64, * PANSI_STRING64;

    typedef struct _LDR_DATA_TABLE_ENTRY32
    {
        LIST_ENTRY32 InLoadOrderLinks;
        LIST_ENTRY32 InMemoryOrderLinks;
        union
        {
            LIST_ENTRY32 InInitializationOrderLinks;
            LIST_ENTRY32 InProgressLinks;
        };
        WOW64_POINTER(PVOID) DllBase;
        WOW64_POINTER(PVOID) EntryPoint;
        ULONG SizeOfImage;
        UNICODE_STRING32 FullDllName;
        UNICODE_STRING32 BaseDllName;
        union
        {
            UCHAR FlagGroup[4];
            ULONG Flags;
            struct
            {
                ULONG PackagedBinary : 1;
                ULONG MarkedForRemoval : 1;
                ULONG ImageDll : 1;
                ULONG LoadNotificationsSent : 1;
                ULONG TelemetryEntryProcessed : 1;
                ULONG ProcessStaticImport : 1;
                ULONG InLegacyLists : 1;
                ULONG InIndexes : 1;
                ULONG ShimDll : 1;
                ULONG InExceptionTable : 1;
                ULONG ReservedFlags1 : 2;
                ULONG LoadInProgress : 1;
                ULONG LoadConfigProcessed : 1;
                ULONG EntryProcessed : 1;
                ULONG ProtectDelayLoad : 1;
                ULONG ReservedFlags3 : 2;
                ULONG DontCallForThreads : 1;
                ULONG ProcessAttachCalled : 1;
                ULONG ProcessAttachFailed : 1;
                ULONG CorDeferredValidate : 1;
                ULONG CorImage : 1;
                ULONG DontRelocate : 1;
                ULONG CorILOnly : 1;
                ULONG ChpeImage : 1;
                ULONG ReservedFlags5 : 2;
                ULONG Redirected : 1;
                ULONG ReservedFlags6 : 2;
                ULONG CompatDatabaseProcessed : 1;
            };
        };
        USHORT ObsoleteLoadCount;
        USHORT TlsIndex;
        LIST_ENTRY32 HashLinks;
        ULONG TimeDateStamp;
        WOW64_POINTER(struct _ACTIVATION_CONTEXT*) EntryPointActivationContext;
        WOW64_POINTER(PVOID) Lock;
        WOW64_POINTER(PLDR_DDAG_NODE) DdagNode;
        LIST_ENTRY32 NodeModuleLink;
        WOW64_POINTER(struct _LDRP_LOAD_CONTEXT*) LoadContext;
        WOW64_POINTER(PVOID) ParentDllBase;
        WOW64_POINTER(PVOID) SwitchBackContext;
        RTL_BALANCED_NODE32 BaseAddressIndexNode;
        RTL_BALANCED_NODE32 MappingInfoIndexNode;
        WOW64_POINTER(ULONG_PTR) OriginalBase;
        LARGE_INTEGER LoadTime;
        ULONG BaseNameHashValue;
        LDR_DLL_LOAD_REASON LoadReason;
        ULONG ImplicitPathOptions;
        ULONG ReferenceCount;
        ULONG DependentLoadFlags;
        UCHAR SigningLevel; // since REDSTONE2
        ULONG CheckSum; // since 22H1
        WOW64_POINTER(PVOID) ActivePatchImageBase;
        LDR_HOT_PATCH_STATE HotPatchState;
    } LDR_DATA_TABLE_ENTRY32, * PLDR_DATA_TABLE_ENTRY32;

    typedef BOOLEAN(NTAPI* PPH_ENUM_PROCESS_MODULES_CALLBACK)(
        _In_ PLDR_DATA_TABLE_ENTRY Module,
        _In_opt_ PVOID Context
        );

    typedef struct _PH_ENUM_PROCESS_MODULES_PARAMETERS
    {
        PPH_ENUM_PROCESS_MODULES_CALLBACK Callback;
        PVOID Context;
        ULONG Flags;
    } PH_ENUM_PROCESS_MODULES_PARAMETERS, * PPH_ENUM_PROCESS_MODULES_PARAMETERS;

    typedef struct _PH_STRINGREF
    {
        /** The length, in bytes, of the string. */
        SIZE_T Length;
        /** The buffer containing the contents of the string. */
        PWCH Buffer;
    } PH_STRINGREF, * PPH_STRINGREF;

    typedef struct _PH_STRING
    {
        // Header
        union
        {
            PH_STRINGREF sr;
            struct
            {
                /** The length, in bytes, of the string. */
                SIZE_T Length;
                /** The buffer containing the contents of the string. */
                PWCH Buffer;
            };
        };

        // Data
        union
        {
            WCHAR Data[1];
            struct
            {
                /** Reserved. */
                ULONG AllocationFlags;
                /** Reserved. */
                PVOID Allocation;
            };
        };
    } PH_STRING, * PPH_STRING;

#define PH_ENUM_PROCESS_MODULES_DONT_RESOLVE_WOW64_FS 0x1
#define PH_ENUM_PROCESS_MODULES_TRY_MAPPED_FILE_NAME 0x2

    NTSTATUS PhGetProcessMappedFileName(
        _In_ HANDLE ProcessHandle,
        _In_ PVOID BaseAddress,
        _Out_ PPH_STRING* FileName
    )
    {
        NTSTATUS status;
        SIZE_T bufferSize;
        SIZE_T returnLength;
        PUNICODE_STRING buffer;

        returnLength = 0;
        bufferSize = 0x100;

        status = NtQueryVirtualMemory(
            ProcessHandle,
            BaseAddress,
            MemoryMappedFilenameInformation,
            buffer,
            bufferSize,
            &returnLength
        );

        if (status == STATUS_BUFFER_OVERFLOW && returnLength > 0) // returnLength > 0 required for MemoryMappedFilename on Windows 7 SP1 (dmex)
        {
            bufferSize = returnLength;
            buffer = PhAllocate(bufferSize);

            status = NtQueryVirtualMemory(
                ProcessHandle,
                BaseAddress,
                MemoryMappedFilenameInformation,
                buffer,
                bufferSize,
                &returnLength
            );
        }

        if (!NT_SUCCESS(status))
        {
            return status;
        }

        *FileName = PhCreateStringFromUnicodeString(buffer);

        return status;
    }

    BOOLEAN NTAPI PhpEnumProcessModulesCallback(
        _In_ HANDLE ProcessHandle,
        _In_ PLDR_DATA_TABLE_ENTRY Entry,
        _In_ PVOID AddressOfEntry,
        _In_ PVOID Context1,
        _In_ PVOID Context2
    )
    {
        PPH_ENUM_PROCESS_MODULES_PARAMETERS parameters = (PPH_ENUM_PROCESS_MODULES_PARAMETERS)Context1;
        NTSTATUS status;
        BOOLEAN result;
        PPH_STRING mappedFileName = NULL;
        PWSTR fullDllNameOriginal;
        PWSTR fullDllNameBuffer = NULL;
        PWSTR baseDllNameOriginal;
        PWSTR baseDllNameBuffer = NULL;

        if (parameters->Flags & PH_ENUM_PROCESS_MODULES_TRY_MAPPED_FILE_NAME)
        {
            PhGetProcessMappedFileName(ProcessHandle, Entry->DllBase, &mappedFileName);
        }

        if (mappedFileName)
        {
            ULONG_PTR indexOfLastBackslash;

            //PhStringRefToUnicodeString(&mappedFileName->sr, &Entry->FullDllName);
            //indexOfLastBackslash = PhFindLastCharInString(mappedFileName, 0, OBJ_NAME_PATH_SEPARATOR);

            if (indexOfLastBackslash != SIZE_MAX)
            {
                Entry->BaseDllName.Buffer = Entry->FullDllName.Buffer + indexOfLastBackslash + 1;
                Entry->BaseDllName.Length = Entry->FullDllName.Length - (USHORT)indexOfLastBackslash * sizeof(WCHAR) - sizeof(UNICODE_NULL);
                Entry->BaseDllName.MaximumLength = Entry->BaseDllName.Length + sizeof(UNICODE_NULL);
            }
            else
            {
                Entry->BaseDllName = Entry->FullDllName;
            }
        }
        else
        {
            // Read the full DLL name string and add a null terminator.

            fullDllNameOriginal = Entry->FullDllName.Buffer;
            fullDllNameBuffer = (PWCHAR) new uint8_t[(Entry->FullDllName.Length + sizeof(UNICODE_NULL))];
            Entry->FullDllName.Buffer = fullDllNameBuffer;

            if (NT_SUCCESS(status = NtReadVirtualMemory(
                ProcessHandle,
                fullDllNameOriginal,
                fullDllNameBuffer,
                Entry->FullDllName.Length,
                NULL
            )))
            {
                fullDllNameBuffer[Entry->FullDllName.Length / sizeof(WCHAR)] = UNICODE_NULL;
            }
            else
            {
                fullDllNameBuffer[0] = UNICODE_NULL;
                Entry->FullDllName.Length = 0;
            }

            baseDllNameOriginal = Entry->BaseDllName.Buffer;

            // Try to use the buffer we just read in.
            if (
                NT_SUCCESS(status) &&
                (ULONG_PTR)baseDllNameOriginal >= (ULONG_PTR)fullDllNameOriginal &&
                (ULONG_PTR)PTR_ADD_OFFSET(baseDllNameOriginal, Entry->BaseDllName.Length) >= (ULONG_PTR)baseDllNameOriginal &&
                (ULONG_PTR)PTR_ADD_OFFSET(baseDllNameOriginal, Entry->BaseDllName.Length) <= (ULONG_PTR)PTR_ADD_OFFSET(fullDllNameOriginal, Entry->FullDllName.Length)
                )
            {
                baseDllNameBuffer = NULL;

                Entry->BaseDllName.Buffer = (PWCH)PTR_ADD_OFFSET(Entry->FullDllName.Buffer, (baseDllNameOriginal - fullDllNameOriginal));
            }
            else
            {
                // Read the base DLL name string and add a null terminator.

                baseDllNameBuffer = (PWSTR) new uint8_t[Entry->BaseDllName.Length + sizeof(UNICODE_NULL)];
                Entry->BaseDllName.Buffer = baseDllNameBuffer;

                if (NT_SUCCESS(NtReadVirtualMemory(
                    ProcessHandle,
                    baseDllNameOriginal,
                    baseDllNameBuffer,
                    Entry->BaseDllName.Length,
                    NULL
                )))
                {
                    baseDllNameBuffer[Entry->BaseDllName.Length / sizeof(WCHAR)] = UNICODE_NULL;
                }
                else
                {
                    baseDllNameBuffer[0] = UNICODE_NULL;
                    Entry->BaseDllName.Length = 0;
                }
            }
        }

        if (WindowsVersion >= WINDOWS_8 && Entry->DdagNode)
        {
            LDR_DDAG_NODE ldrDagNode;

            memset(&ldrDagNode, 0, sizeof(LDR_DDAG_NODE));

            if (NT_SUCCESS(NtReadVirtualMemory(
                ProcessHandle,
                Entry->DdagNode,
                &ldrDagNode,
                sizeof(LDR_DDAG_NODE),
                NULL
            )))
            {
                // Fixup the module load count. (dmex)
                Entry->ObsoleteLoadCount = (USHORT)ldrDagNode.LoadCount;
            }
        }

        // Execute the callback.
        result = parameters->Callback(Entry, parameters->Context);

        if (mappedFileName)
        {
            //PhDereferenceObject(mappedFileName);
        }
        else
        {
            if (fullDllNameBuffer)
                delete [] fullDllNameBuffer;
            if (baseDllNameBuffer)
                delete[] baseDllNameBuffer;
        }

        return result;
    }

    NTSTATUS PhpEnumProcessModules(
        _In_ HANDLE ProcessHandle,
        _In_ PPHP_ENUM_PROCESS_MODULES_CALLBACK Callback,
        _In_opt_ PVOID Context1,
        _In_opt_ PVOID Context2
    )
    {
        NTSTATUS status;
        PPEB peb;
        PPEB_LDR_DATA ldr;
        PEB_LDR_DATA pebLdrData;
        PLIST_ENTRY startLink;
        PLIST_ENTRY currentLink;
        ULONG dataTableEntrySize;
        LDR_DATA_TABLE_ENTRY currentEntry;
        ULONG i;

        // Get the PEB address.
        status = PhGetProcessPeb(ProcessHandle, (PVOID*)&peb);

        if (!NT_SUCCESS(status))
            return status;

        // Read the address of the loader data.
        status = NtReadVirtualMemory(
            ProcessHandle,
            PTR_ADD_OFFSET(peb, FIELD_OFFSET(PEB, Ldr)),
            &ldr,
            sizeof(PVOID),
            NULL
        );

        if (!NT_SUCCESS(status))
            return status;

        // Check the process has initialized (dmex)
        if (!ldr)
            return STATUS_UNSUCCESSFUL;

        // Read the loader data.
        status = NtReadVirtualMemory(
            ProcessHandle,
            ldr,
            &pebLdrData,
            sizeof(PEB_LDR_DATA),
            NULL
        );

        if (!NT_SUCCESS(status))
            return status;

        if (!pebLdrData.Initialized)
            return STATUS_UNSUCCESSFUL;

        if (WindowsVersion >= WINDOWS_11)
            dataTableEntrySize = LDR_DATA_TABLE_ENTRY_SIZE_WIN11;
        else if (WindowsVersion >= WINDOWS_8)
            dataTableEntrySize = LDR_DATA_TABLE_ENTRY_SIZE_WIN8;
        else
            dataTableEntrySize = LDR_DATA_TABLE_ENTRY_SIZE_WIN7;

        // Traverse the linked list (in load order).

        i = 0;
        startLink = (PLIST_ENTRY)PTR_ADD_OFFSET(ldr, FIELD_OFFSET(PEB_LDR_DATA, InLoadOrderModuleList));
        currentLink = pebLdrData.InLoadOrderModuleList.Flink;

        while (
            currentLink != startLink &&
            i <= PH_ENUM_PROCESS_MODULES_LIMIT
            )
        {
            PVOID addressOfEntry;

            addressOfEntry = CONTAINING_RECORD(currentLink, LDR_DATA_TABLE_ENTRY, InLoadOrderLinks);
            status = NtReadVirtualMemory(
                ProcessHandle,
                addressOfEntry,
                &currentEntry,
                dataTableEntrySize,
                NULL
            );

            if (!NT_SUCCESS(status))
                return status;

            // Make sure the entry is valid.
            if (currentEntry.DllBase)
            {
                // Execute the callback.
                if (!Callback(
                    ProcessHandle,
                    &currentEntry,
                    addressOfEntry,
                    Context1,
                    Context2
                ))
                    break;
            }

            currentLink = currentEntry.InLoadOrderLinks.Flink;
            i++;
        }

        return status;
    }
};
