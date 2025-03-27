#ifndef SYMBOLS_HPP
#define SYMBOLS_HPP

#include <windows.h>
#include <dbghelp.h>
#include <psapi.h>
#include <unordered_map>
#include <string>
#include <iostream>
#include <vector>
#include <filesystem>
#include <winternl.h>

#pragma comment(lib, "dbghelp.lib")

#define DIRECTORY_QUERY         0x1
#define DIRECTORY_TRAVERSE      0x2
#define STATUS_NO_MORE_ENTRIES  0x8000001A

typedef struct _OBJECT_DIRECTORY_INFORMATION {
    UNICODE_STRING Name;
    UNICODE_STRING TypeName;
} OBJECT_DIRECTORY_INFORMATION, * POBJECT_DIRECTORY_INFORMATION;

typedef NTSTATUS(NTAPI* fnNtQueryDirectoryObject)(
    _In_ HANDLE DirectoryHandle,
    _Out_writes_bytes_opt_(Length) PVOID Buffer,
    _In_ ULONG Length,
    _In_ BOOLEAN ReturnSingleEntry,
    _In_ BOOLEAN RestartScan,
    _Inout_ PULONG Context,
    _Out_opt_ PULONG ReturnLength
    );

typedef NTSTATUS(NTAPI* fnNtOpenDirectoryObject)(
    _Out_ PHANDLE DirectoryHandle,
    _In_ ACCESS_MASK DesiredAccess,
    _In_ POBJECT_ATTRIBUTES ObjectAttributes
    );

struct SymbolInfo {
    DWORD64 address;
    DWORD64 size;
    std::string name;
};

struct ModuleInfo {
    uintptr_t base;
    uintptr_t end;
    std::string mod_name;

    bool operator==(const ModuleInfo& rnd) const {
        return base == rnd.base && end == rnd.end;
    }
};

template <>
struct std::hash<ModuleInfo> {
    std::size_t operator()(const ModuleInfo& k) const {
        return hash<uintptr_t>()(k.base) ^ hash<uintptr_t>()(k.end);
    }
};

namespace Symbols {

    BOOL EnumSymbolsCallback(SYMBOL_INFO* pSymInfo, ULONG SymbolSize, PVOID UserContext);
    void LoadSymbolsForModule(HMODULE hModule, PWSTR name);
    std::string ResolveSymbol(uintptr_t address);
    bool InitSymbols();
}

#endif // !SYMBOLS_HPP

