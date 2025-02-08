#ifndef UTILS_HPP
#define UTILS_HPP

#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include "Global.hpp"

#define KMENTALTI	    0x8000
#define MENTALTI_OPEN   CTL_CODE(KMENTALTI, 0x800, METHOD_NEITHER, FILE_ANY_ACCESS)
#define MENTALTI_ALL    CTL_CODE(KMENTALTI, 0x801, METHOD_NEITHER, FILE_ANY_ACCESS)

#define PROTECTVM_LOCAL         0x10
#define PROTECTVM_REMOTE        0x40
#define READVM_LOCAL            0x10000
#define READVM_REMOTE           0x20000
#define WRITEVM_LOCAL           0x40000
#define WRITEVM_REMOTE          0x80000
#define THREAD_SUSPEND          0x100000
#define THREAD_RESUMED          0x200000
#define PROCESS_SUSPEND         0x400000
#define PROCESS_RESUME          0x800000
#define PROCESS_FREEZE          0x1000000
#define PROCESS_THAW            0x2000000
#define IMPERSONATION_UP        0x4000000000
#define IMPERSONATION_REVERT    0x8000000000
#define IMPERSONATION_DOWN      0x40000000000

typedef enum PROCESS_INFO_CLASS {
    ProcessEnableLogging = 96
} PROCESS_INFO_CLASS;

typedef union _PROCESS_LOGGING_INFORMATION {
    ULONG Flags;
    struct {
        ULONG EnableReadVmLogging : 1;
        ULONG EnableWriteVmLogging : 1;
        ULONG EnableProcessSuspendResumeLogging : 1;
        ULONG EnableThreadSuspendResumeLogging : 1;
        ULONG EnableLocalExecProtectVmLogging : 1;
        ULONG EnableRemoteExecProtectVmLogging : 1;
        ULONG EnableImpersonationLogging : 1;
        ULONG Reserved : 25;
    };
} PROCESS_LOGGING_INFORMATION, * PPROCESS_LOGGING_INFORMATION;

typedef NTSTATUS(NTAPI* fnNtQueryInformationProcess)(
    _In_ HANDLE ProcessHandle,
    _In_ PROCESS_INFO_CLASS ProcessInformationClass,
    _Out_writes_bytes_(ProcessInformationLength) PVOID ProcessInformation,
    _In_ ULONG ProcessInformationLength,
    _Out_opt_ PULONG ReturnLength
);

typedef NTSTATUS(NTAPI* fnNtSetInformationProcess)(
    _In_ HANDLE ProcessHandle,
    _In_ PROCESS_INFO_CLASS ProcessInformationClass,
    _In_reads_bytes_(ProcessInformationLength) PVOID ProcessInformation,
    _In_ ULONG ProcessInformationLength
    );

namespace Utils {

    bool ModifyLogging(bool bRevert, ULONG flags);
	bool ParseUserKeywords(const std::string& input);
	bool ParseUserInput(const int argc, char** argv);
	bool CreateOpenOutputFile(const std::string& filename);
	void PrintHelp();
	bool SendIOCTL(ULONG ioctl, ULONG flags);
	bool CtrlHandler(DWORD fdwCtrlType);
}

#endif // !UTILS_HPP
