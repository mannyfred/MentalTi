#pragma once
#include <windows.h>
#include <winternl.h>


//There are some V2 representations on WIN11 (WRITEVM_REMOTE, PROTECTVM, READVM)
//Define this if you want to try out this tool on WIN11, will give access to more members (aka more juicy data)

//#define V2

namespace EtwTi {

    //Event ID 11 and 13 etc
#ifdef V2
    
#define ETWTI_READVM_LOCALREMOTE_FIELDS             \
    X(OperationStatus, ULONG)                       \
    X(CallingProcessId, ULONG)                      \
    X(CallingProcessCreateTime, FILETIME)           \
    X(CallingProcessStartKey, ULONG64)              \
    X(CallingProcessSignatureLevel, UCHAR)          \
    X(CallingProcessSectionSignatureLevel, UCHAR)   \
    X(CallingProcessProtection, UCHAR)              \
    X(CallingThreadId, ULONG)                       \
    X(CallingThreadCreateTime, FILETIME)            \
    X(TargetProcessId, ULONG)                       \
    X(TargetProcessCreateTime, FILETIME)            \
    X(TargetProcessStartKey, ULONG64)               \
    X(TargetProcessSignatureLevel, UCHAR)           \
    X(TargetProcessSectionSignatureLevel, UCHAR)    \
    X(TargetProcessProtection, UCHAR)               \
    X(BaseAddress, PVOID)                           \
    X(BytesCopied, PVOID)                           \
    X(VaVadQueryResult, ULONG)                      \
    X(VaVadAllocationBase, PVOID)                   \
    X(VaVadAllocationProtect, ULONG)                \
    X(VaVadRegionType, ULONG)                       \
    X(VaVadRegionSize, PVOID)                       \
    X(VaVadCommitSize, PVOID)                       \
    X(VaVadMmfName, UNICODE_STRING)                 \

#else

#define ETWTI_READVM_LOCALREMOTE_FIELDS             \
    X(OperationStatus, ULONG)                       \
    X(CallingProcessId, ULONG)                      \
    X(CallingProcessCreateTime, FILETIME)           \
    X(CallingProcessStartKey, ULONG64)              \
    X(CallingProcessSignatureLevel, UCHAR)          \
    X(CallingProcessSectionSignatureLevel, UCHAR)   \
    X(CallingProcessProtection, UCHAR)              \
    X(CallingThreadId, ULONG)                       \
    X(CallingThreadCreateTime, FILETIME)            \
    X(TargetProcessId, ULONG)                       \
    X(TargetProcessCreateTime, FILETIME)            \
    X(TargetProcessStartKey, ULONG64)               \
    X(TargetProcessSignatureLevel, UCHAR)           \
    X(TargetProcessSectionSignatureLevel, UCHAR)    \
    X(TargetProcessProtection, UCHAR)               \
    X(BaseAddress, PVOID)                           \
    X(BytesCopied, PVOID)                           \

#endif

    //event ID 15 and 16
#define ETWTI_SUSPENDRESUME_THREAD_FIELDS           \
    X(OperationStatus, ULONG)                       \
    X(CallingProcessId, ULONG)                      \
    X(CallingProcessCreateTime, FILETIME)           \
    X(CallingProcessStartKey, ULONG64)              \
    X(CallingProcessSignatureLevel, UCHAR)          \
    X(CallingProcessSectionSignatureLevel, UCHAR)   \
    X(CallingProcessProtection, UCHAR)              \
    X(CallingThreadId, ULONG)                       \
    X(CallingThreadCreateTime, FILETIME)            \
    X(TargetProcessId, ULONG)                       \
    X(TargetProcessCreateTime, FILETIME)            \
    X(TargetProcessStartKey, ULONG64)               \
    X(TargetProcessSignatureLevel, UCHAR)           \
    X(TargetProcessSectionSignatureLevel, UCHAR)    \
    X(TargetProcessProtection, UCHAR)               \
    X(TargetThreadId, ULONG)                        \
    X(TargetThreadCreateTime, FILETIME)             \


    //event ID 17, 18, 19 and 20
#define ETWTI_SUSPENDRESUME_PROCESS_FIELDS          \
    X(OperationStatus, ULONG)                       \
    X(CallingProcessId, ULONG)                      \
    X(CallingProcessCreateTime, FILETIME)           \
    X(CallingProcessStartKey, ULONG64)              \
    X(CallingProcessSignatureLevel, UCHAR)          \
    X(CallingProcessSectionSignatureLevel, UCHAR)   \
    X(CallingProcessProtection, UCHAR)              \
    X(CallingThreadId, ULONG)                       \
    X(CallingThreadCreateTime, FILETIME)            \
    X(TargetProcessId, ULONG)                       \
    X(TargetProcessCreateTime, FILETIME)            \
    X(TargetProcessStartKey, ULONG64)               \
    X(TargetProcessSignatureLevel, UCHAR)           \
    X(TargetProcessSectionSignatureLevel, UCHAR)    \
    X(TargetProcessProtection, UCHAR)               \
    

    //event ID 2 and 7 etc
#ifdef V2

#define ETWTI_PROTECTVM_LOCALREMOTE_FIELDS          \
    X(CallingProcessId, ULONG)                      \
    X(CallingProcessCreateTime, FILETIME)           \
    X(CallingProcessStartKey, ULONG64)              \
    X(CallingProcessSignatureLevel, UCHAR)          \
    X(CallingProcessSectionSignatureLevel, UCHAR)   \
    X(CallingProcessProtection, UCHAR)              \
    X(CallingThreadId, ULONG)                       \
    X(CallingThreadCreateTime, FILETIME)            \
    X(TargetProcessId, ULONG)                       \
    X(TargetProcessCreateTime, FILETIME)            \
    X(TargetProcessStartKey, ULONG64)               \
    X(TargetProcessSignatureLevel, UCHAR)           \
    X(TargetProcessSectionSignatureLevel, UCHAR)    \
    X(TargetProcessProtection, UCHAR)               \
    X(OriginalProcessId, ULONG)                     \
    X(OriginalProcessCreateTime, FILETIME)          \
    X(OriginalProcessStartKey, ULONG64)             \
    X(OriginalProcessSignatureLevel, UCHAR)         \
    X(OriginalProcessSectionSignatureLevel, UCHAR)  \
    X(OriginalProcessProtection, UCHAR)             \
    X(BaseAddress, PVOID)                           \
    X(RegionSize, PVOID)                            \
    X(ProtectionMask, ULONG)                        \
    X(LastProtectionMask, ULONG)                    \
    X(VaVadQueryResult, ULONG)                      \
    X(VaVadAllocationBase, PVOID)                   \
    X(VaVadAllocationProtect, ULONG)                \
    X(VaVadRegionType, ULONG)                       \
    X(VaVadRegionSize, PVOID)                       \
    X(VaVadCommitSize, PVOID)                       \
    X(VaVadMmfName, UNICODE_STRING)                 \

#else

#define ETWTI_PROTECTVM_LOCALREMOTE_FIELDS          \
    X(CallingProcessId, ULONG)                      \
    X(CallingProcessCreateTime, FILETIME)           \
    X(CallingProcessStartKey, ULONG64)              \
    X(CallingProcessSignatureLevel, UCHAR)          \
    X(CallingProcessSectionSignatureLevel, UCHAR)   \
    X(CallingProcessProtection, UCHAR)              \
    X(CallingThreadId, ULONG)                       \
    X(CallingThreadCreateTime, FILETIME)            \
    X(TargetProcessId, ULONG)                       \
    X(TargetProcessCreateTime, FILETIME)            \
    X(TargetProcessStartKey, ULONG64)               \
    X(TargetProcessSignatureLevel, UCHAR)           \
    X(TargetProcessSectionSignatureLevel, UCHAR)    \
    X(TargetProcessProtection, UCHAR)               \
    X(OriginalProcessId, ULONG)                     \
    X(OriginalProcessCreateTime, FILETIME)          \
    X(OriginalProcessStartKey, ULONG64)             \
    X(OriginalProcessSignatureLevel, UCHAR)         \
    X(OriginalProcessSectionSignatureLevel, UCHAR)  \
    X(OriginalProcessProtection, UCHAR)             \
    X(BaseAddress, PVOID)                           \
    X(RegionSize, PVOID)                            \
    X(ProtectionMask, ULONG)                        \
    X(LastProtectionMask, ULONG)                    \

#endif


    //event ID 1 and 6 etc
#define ETWTI_ALLOCVM_LOCALREMOTE_FIELDS            \
    X(CallingProcessId, ULONG)                      \
    X(CallingProcessCreateTime, FILETIME)           \
    X(CallingProcessStartKey, ULONG64)              \
    X(CallingProcessSignatureLevel, UCHAR)          \
    X(CallingProcessSectionSignatureLevel, UCHAR)   \
    X(CallingProcessProtection, UCHAR)              \
    X(CallingThreadId, ULONG)                       \
    X(CallingThreadCreateTime, FILETIME)            \
    X(TargetProcessId, ULONG)                       \
    X(TargetProcessCreateTime, FILETIME)            \
    X(TargetProcessStartKey, ULONG64)               \
    X(TargetProcessSignatureLevel, UCHAR)           \
    X(TargetProcessSectionSignatureLevel, UCHAR)    \
    X(TargetProcessProtection, UCHAR)               \
    X(OriginalProcessId, ULONG)                     \
    X(OriginalProcessCreateTime, FILETIME)          \
    X(OriginalProcessStartKey, ULONG64)             \
    X(OriginalProcessSignatureLevel, UCHAR)         \
    X(OriginalProcessSectionSignatureLevel, UCHAR)  \
    X(OriginalProcessProtection, UCHAR)             \
    X(BaseAddress, PVOID)                           \
    X(RegionSize, PVOID)                            \
    X(AllocationType, ULONG)                        \
    X(ProtectionMask, ULONG)                        \


    //Event ID 4 and 24
#define ETWTI_QUEUE_APC_REMOTE_FIELDS               \
    X(CallingProcessId, ULONG)                      \
    X(CallingProcessCreateTime, FILETIME)           \
    X(CallingProcessStartKey, ULONG64)              \
    X(CallingProcessSignatureLevel, UCHAR)          \
    X(CallingProcessSectionSignatureLevel, UCHAR)   \
    X(CallingProcessProtection, UCHAR)              \
    X(CallingThreadId, ULONG)                       \
    X(CallingThreadCreateTime, FILETIME)            \
    X(TargetProcessId, ULONG)                       \
    X(TargetProcessCreateTime, FILETIME)            \
    X(TargetProcessStartKey, ULONG64)               \
    X(TargetProcessSignatureLevel, UCHAR)           \
    X(TargetProcessSectionSignatureLevel, UCHAR)    \
    X(TargetProcessProtection, UCHAR)               \
    X(TargetThreadId, ULONG)                        \
    X(TargetThreadCreateTime, FILETIME)             \
    X(OriginalProcessId, ULONG)                     \
    X(OriginalProcessCreateTime, FILETIME)          \
    X(OriginalProcessStartKey, ULONG64)             \
    X(OriginalProcessSignatureLevel, UCHAR)         \
    X(OriginalProcessSectionSignatureLevel, UCHAR)  \
    X(OriginalProcessProtection, UCHAR)             \
    X(TargetThreadAlertable, UCHAR)                 \
    X(ApcRoutine, PVOID)                            \
    X(ApcArgument1, PVOID)                          \
    X(ApcArgument2, PVOID)                          \
    X(ApcArgument3, PVOID)                          \
    X(RealEventTime, FILETIME)                      \
    X(ApcRoutineVadQueryResult, ULONG)              \
    X(ApcRoutineVadAllocationBase, PVOID)           \
    X(ApcRoutineVadAllocationProtect, ULONG)        \
    X(ApcRoutineVadRegionType, ULONG)               \
    X(ApcRoutineVadRegionSize, PVOID)               \
    X(ApcRoutineVadCommitSize, PVOID)               \
    X(ApcRoutineVadMmfName, UNICODE_STRING)         \
    X(ApcArgument1VadQueryResult, ULONG)            \
    X(ApcArgument1VadAllocationBase, PVOID)         \
    X(ApcArgument1VadAllocationProtect, ULONG)      \
    X(ApcArgument1VadRegionType, ULONG)             \
    X(ApcArgument1VadRegionSize, PVOID)             \
    X(ApcArgument1VadCommitSize, PVOID)             \
    X(ApcArgument1VadMmfName, UNICODE_STRING)       \


//event ID 12 and 14 etc
#ifdef V2

#define ETWTI_WRITEVM_LOCALREMOTE_FIELDS            \
    X(OperationStatus, ULONG)                       \
    X(CallingProcessId, ULONG)                      \
    X(CallingProcessCreateTime, FILETIME)           \
    X(CallingProcessStartKey, ULONG64)              \
    X(CallingProcessSignatureLevel, UCHAR)          \
    X(CallingProcessSectionSignatureLevel, UCHAR)   \
    X(CallingProcessProtection, UCHAR)              \
    X(CallingThreadId, ULONG)                       \
    X(CallingThreadCreateTime, FILETIME)            \
    X(TargetProcessId, ULONG)                       \
    X(TargetProcessCreateTime, FILETIME)            \
    X(TargetProcessStartKey, ULONG64)               \
    X(TargetProcessSignatureLevel, UCHAR)           \
    X(TargetProcessSectionSignatureLevel, UCHAR)    \
    X(TargetProcessProtection, UCHAR)               \
    X(BaseAddress, PVOID)                           \
    X(BytesCopied, PVOID)                           \
    X(VaVadQueryResult, ULONG)                      \
    X(VaVadAllocationBase, PVOID)                   \
    X(VaVadAllocationProtect, ULONG)                \
    X(VaVadRegionType, ULONG)                       \
    X(VaVadRegionSize, PVOID)                       \
    X(VaVadCommitSize, PVOID)                       \
    X(VaVadMmfName, UNICODE_STRING)                 \

#else

#define ETWTI_WRITEVM_LOCALREMOTE_FIELDS            \
    X(OperationStatus, ULONG)                       \
    X(CallingProcessId, ULONG)                      \
    X(CallingProcessCreateTime, FILETIME)           \
    X(CallingProcessStartKey, ULONG64)              \
    X(CallingProcessSignatureLevel, UCHAR)          \
    X(CallingProcessSectionSignatureLevel, UCHAR)   \
    X(CallingProcessProtection, UCHAR)              \
    X(CallingThreadId, ULONG)                       \
    X(CallingThreadCreateTime, FILETIME)            \
    X(TargetProcessId, ULONG)                       \
    X(TargetProcessCreateTime, FILETIME)            \
    X(TargetProcessStartKey, ULONG64)               \
    X(TargetProcessSignatureLevel, UCHAR)           \
    X(TargetProcessSectionSignatureLevel, UCHAR)    \
    X(TargetProcessProtectione, UCHAR)              \
    X(BaseAddress, PVOID)                           \
    X(BytesCopied, PVOID)                           \

#endif


    //event ID 3 and 8 etc
#define ETWTI_MAPVIEW_LOCALREMOTE_FIELDS            \
    X(CallingProcessId, ULONG)                      \
    X(CallingProcessCreateTime, FILETIME)           \
    X(CallingProcessStartKey, ULONG64)              \
    X(CallingProcessSignatureLevel, UCHAR)          \
    X(CallingProcessSectionSignatureLevel, UCHAR)   \
    X(CallingProcessProtection, UCHAR)              \
    X(CallingThreadId, ULONG)                       \
    X(CallingThreadCreateTime, FILETIME)            \
    X(TargetProcessId, ULONG)                       \
    X(TargetProcessCreateTime, FILETIME)            \
    X(TargetProcessStartKey, ULONG64)               \
    X(TargetProcessSignatureLevel, UCHAR)           \
    X(TargetProcessSectionSignatureLevel, UCHAR)    \
    X(TargetProcessProtectione, UCHAR)              \
    X(BaseAddress, PVOID)                           \
    X(ViewSize, PVOID)                              \
    X(AllocationType, ULONG)                        \
    X(ProtectionMask, ULONG)                        \


    //event ID 5 and 25
#define ETWTI_SETTHREADCONTEXT_REMOTE_FIELDS        \
    X(CallingProcessId, ULONG)                      \
    X(CallingProcessCreateTime, FILETIME)           \
    X(CallingProcessStartKey, ULONG64)              \
    X(CallingProcessSignatureLevel, UCHAR)          \
    X(CallingProcessSectionSignatureLevel, UCHAR)   \
    X(CallingProcessProtection, UCHAR)              \
    X(CallingThreadId, ULONG)                       \
    X(CallingThreadCreateTime, FILETIME)            \
    X(TargetProcessId, ULONG)                       \
    X(TargetProcessCreateTime, FILETIME)            \
    X(TargetProcessStartKey, ULONG64)               \
    X(TargetProcessSignatureLevel, UCHAR)           \
    X(TargetProcessSectionSignatureLevel, UCHAR)    \
    X(TargetProcessProtection, UCHAR)              \
    X(TargetThreadId, ULONG)                        \
    X(TargetThreadCreateTime, FILETIME)             \
    X(ContextFlags, ULONG)                          \
    X(ContextMask, USHORT)                          \
    X(Pc, PVOID)                                    \
    X(Sp, PVOID)                                    \
    X(Lr, PVOID)                                    \
    X(Fp, PVOID)                                    \
    X(Reg0, PVOID)                                  \
    X(Reg1, PVOID)                                  \
    X(Reg2, PVOID)                                  \
    X(Reg3, PVOID)                                  \
    X(Reg4, PVOID)                                  \
    X(Reg5, PVOID)                                  \
    X(Reg6, PVOID)                                  \
    X(Reg7, PVOID)                                  \
    X(RealEventTime, FILETIME)                      \
    X(PcVadQueryResult, ULONG)                      \
    X(PcAllocationBase, PVOID)                      \
    X(PcVadAllocationProtect, ULONG)                \
    X(PcVadRegionType, ULONG)                       \
    X(PcVadRegionSize, PVOID)                       \
    X(PcVadCommitSize, PVOID)                       \
    X(PcVadMmfName, UNICODE_STRING)                 \
    

#define ETWTI_DRIVER_EVENT_FIELDS                   \
    X(DriverNameLength, USHORT)                     \
    X(DriverName, UNICODE_STRING)                   \
    X(CodeIntegrityOption, ULONG)                   \


#define ETWTI_DEVICE_EVENT_FIELDS                   \
    X(DriverNameLength, USHORT)                     \
    X(DriverName, UNICODE_STRING)                   \
    X(device_nameLength, USHORT)                     \
    X(device_name, UNICODE_STRING)                   \


#define X(member, type) type member;

    struct ETWTI_DRIVER_EVENT {
        ETWTI_DRIVER_EVENT_FIELDS;
    };

    struct ETWTI_DEVICE_EVENT {
        ETWTI_DEVICE_EVENT_FIELDS;
    };

    struct ETWTI_QUEUEUSERAPC_REMOTE {
        ETWTI_QUEUE_APC_REMOTE_FIELDS
    };

    struct ETWTI_QUEUEUSERAPC_REMOTE_KERNEL {
        ETWTI_QUEUE_APC_REMOTE_FIELDS
    };

    struct ETWTI_MAPVIEW_LOCAL {
        ETWTI_MAPVIEW_LOCALREMOTE_FIELDS
    };

    struct ETWTI_MAPVIEW_REMOTE {
        ETWTI_MAPVIEW_LOCALREMOTE_FIELDS
    };

    struct ETWTI_MAPVIEW_LOCAL_KERNEL {
        ETWTI_MAPVIEW_LOCALREMOTE_FIELDS
    };

    struct ETWTI_MAPVIEW_REMOTE_KERNEL {
        ETWTI_MAPVIEW_LOCALREMOTE_FIELDS
    };

    struct ETWTI_ALLOCVM_LOCAL {
        ETWTI_ALLOCVM_LOCALREMOTE_FIELDS;
    };

    struct ETWTI_ALLOCVM_REMOTE {
        ETWTI_ALLOCVM_LOCALREMOTE_FIELDS;
    };

    struct ETWTI_ALLOCVM_LOCAL_KERNEL {
        ETWTI_ALLOCVM_LOCALREMOTE_FIELDS;
    };

    struct ETWTI_ALLOCVM_REMOTE_KERNEL {
        ETWTI_ALLOCVM_LOCALREMOTE_FIELDS;
    };

    struct ETWTI_SETTHREADCONTEXT_REMOTE {
        ETWTI_SETTHREADCONTEXT_REMOTE_FIELDS;
    };

    struct ETWTI_SETTHREADCONTEXT_REMOTE_KERNEL {
        ETWTI_SETTHREADCONTEXT_REMOTE_FIELDS;
    };

    struct ETWTI_SUSPEND_THREAD {
        ETWTI_SUSPENDRESUME_THREAD_FIELDS;
    };

    struct ETWTI_RESUME_THREAD {
        ETWTI_SUSPENDRESUME_THREAD_FIELDS;
    };

    struct ETWTI_PROTECTVM_LOCAL {
        ETWTI_PROTECTVM_LOCALREMOTE_FIELDS;
    };

    struct ETWTI_PROTECTVM_LOCAL_KERNEL {
        ETWTI_PROTECTVM_LOCALREMOTE_FIELDS;
    };

    struct ETWTI_PROTECTVM_REMOTE_KERNEL {
        ETWTI_PROTECTVM_LOCALREMOTE_FIELDS;
    };

    struct ETWTI_PROTECTVM_REMOTE {
        ETWTI_PROTECTVM_LOCALREMOTE_FIELDS;
    };

    struct ETWTI_SUSPEND_PROCESS {
        ETWTI_SUSPENDRESUME_PROCESS_FIELDS;
    };

    struct ETWTI_RESUME_PROCESS {
        ETWTI_SUSPENDRESUME_PROCESS_FIELDS
    };

    struct ETWTI_THAW_PROCESS {
        ETWTI_SUSPENDRESUME_PROCESS_FIELDS
    };

    struct ETWTI_FREEZE_PROCESS {
        ETWTI_SUSPENDRESUME_PROCESS_FIELDS
    };

    struct ETWTI_READVM_LOCAL {
        ETWTI_READVM_LOCALREMOTE_FIELDS
    };

    struct ETWTI_READVM_REMOTE {
        ETWTI_READVM_LOCALREMOTE_FIELDS
    };

    struct ETWTI_WRITEVM_LOCAL {
        ETWTI_WRITEVM_LOCALREMOTE_FIELDS
    };

    struct ETWTI_WRITEVM_REMOTE {
        ETWTI_WRITEVM_LOCALREMOTE_FIELDS
    };

#undef X

}