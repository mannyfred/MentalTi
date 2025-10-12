#include "Symbols.hpp"
#include "Utils.hpp"
#include <tlhelp32.h>
#include <array>
#include <shared_mutex>

namespace Symbols {

    using SymbolMap = std::unordered_map<DWORD64, SymbolInfo>;
    std::unordered_map<ModuleInfo, SymbolMap> SymbolStore;

    std::shared_mutex ProcessStoreMutex;
    std::unordered_map<ULONG, ProcessInfo> ProcessStore;

    BOOL EnumSymbolsCallback(SYMBOL_INFO* pSymInfo, ULONG SymbolSize, PVOID UserContext) {

        SymbolMap* symbol_map = static_cast<SymbolMap*>(UserContext);
        SymbolInfo symbol_info = { pSymInfo->Address, SymbolSize, pSymInfo->Name };
        (*symbol_map)[pSymInfo->Address] = symbol_info;

        return TRUE;
    }

    void LoadSymbolsForModule(HMODULE hModule, PWSTR name) {

        MODULEINFO module = { 0 };
        ::GetModuleInformation((HANDLE)-1, hModule, &module, sizeof(module));

        uintptr_t base_addr = reinterpret_cast<uintptr_t>(module.lpBaseOfDll);
        uintptr_t end_addr = base_addr + module.SizeOfImage;

        std::wstring wstr(name);

        ModuleInfo module_info = { base_addr, end_addr, std::filesystem::path(wstr).stem().string() };
        SymbolMap symbol_map;

        ::SymEnumSymbols((HANDLE)-1, base_addr, nullptr, EnumSymbolsCallback, &symbol_map);

        SymbolStore[module_info] = std::move(symbol_map);
    }

    void LoadProcessInfo(std::unique_ptr<ULONG>& pid, std::unique_ptr<std::array<char, 420>>& imagename) {

        HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, *pid);

        DEFER({
            if (hSnap != INVALID_HANDLE_VALUE)
                ::CloseHandle(hSnap);
        });

        if (hSnap == INVALID_HANDLE_VALUE) {
            return;
        }

        std::string proc_name(imagename ? imagename->data() : "<null>");

        MODULEENTRY32 mod = { .dwSize = sizeof(MODULEENTRY32) };

        if (Module32First(hSnap, &mod)) {

            uintptr_t base = reinterpret_cast<uintptr_t>(mod.modBaseAddr);
            uintptr_t end  = base + mod.modBaseSize;

            ProcessInfo pInfo = { base, end, proc_name };
            std::unique_lock lock(ProcessStoreMutex);
            ProcessStore[*pid] = std::move(pInfo);
        }
    }

    std::string ResolveProcessSymbol(ULONG pid, uintptr_t address) {

        std::shared_lock lock(ProcessStoreMutex);

        auto it = ProcessStore.find(pid);
        if (it == ProcessStore.end()) {
            return {};
        }

        ProcessInfo& info = it->second;

        if (address >= info.base && address <= info.end) {
            return std::format("{}+0x{:x}", info.proc_name, address - info.base);
        }

        return {};
    }

    void NukeProcessInfo(ULONG pid) {
        std::unique_lock lock(ProcessStoreMutex);
        ProcessStore.erase(pid);
    }

    std::string ReturnProcessExecutable(ULONG pid) {

        std::shared_lock lock(ProcessStoreMutex);

        auto it = ProcessStore.find(pid);
        if (it == ProcessStore.end()) {
            return {};
        }

        ProcessInfo& info = it->second;
        return info.proc_name;
    }

    std::string ResolveSymbol(uintptr_t address) {

        for (const auto& [mod_info, symbols] : SymbolStore) {

            if (address >= mod_info.base && address < mod_info.end) {

                if (address == mod_info.base) {
                    return mod_info.mod_name;
                }

                auto it = symbols.find(address);

                if (it != symbols.end()) {
                    return std::format("{}!{}", mod_info.mod_name, it->second.name);
                }
                else {

                    for (const auto& [sym_addr, sym_info] : symbols) {

                        if (address >= sym_addr && address < sym_addr + sym_info.size) {
                            return std::format("{}!{}+0x{:x}", mod_info.mod_name, sym_info.name, address - sym_addr);
                        }
                    }
                }
            }
        }

        return {};
    }

    bool InitSymbols() {

        UNICODE_STRING      us           = { 0 };
        OBJECT_ATTRIBUTES   oa           = { 0 };
        NTSTATUS            STATUS       = 0x00;
        ULONG               uRetLen      = 0x00,
                            uContext     = 0x00;
        HANDLE              hDirectory   = nullptr;
        HMODULE             hModule      = nullptr;
        OBJECT_DIRECTORY_INFORMATION* pInfo = nullptr;

        DEFER({
            if (hDirectory)
                ::CloseHandle(hDirectory);

            if (pInfo)
                ::HeapFree(::GetProcessHeap(), 0, pInfo);
        });

        HMODULE hNtdll = ::GetModuleHandle(TEXT("NTDLL.DLL"));
        static WCHAR sKnownDLLs[] = L"\\KnownDlls";

        fnNtOpenDirectoryObject     pNtOpenDirectoryObject = reinterpret_cast<fnNtOpenDirectoryObject>(::GetProcAddress(hNtdll, "NtOpenDirectoryObject"));
        fnNtQueryDirectoryObject    pNtQueryDirectoryObject = reinterpret_cast<fnNtQueryDirectoryObject>(::GetProcAddress(hNtdll, "NtQueryDirectoryObject"));

        if (!pNtOpenDirectoryObject || !pNtQueryDirectoryObject)
            return false;

        us.Buffer = sKnownDLLs;
        us.Length = static_cast<USHORT>(std::wcslen(sKnownDLLs) * sizeof(WCHAR));
        us.MaximumLength = us.Length + sizeof(WCHAR);

        InitializeObjectAttributes(&oa, &us, OBJ_CASE_INSENSITIVE, nullptr, nullptr);

        if ((STATUS = pNtOpenDirectoryObject(&hDirectory, DIRECTORY_QUERY | DIRECTORY_TRAVERSE, &oa)) != 0x00) {
            std::printf("[!] NtOpenDirectoryObject: 0x%0.8X\n", STATUS);
            return false;
        }

        if (!::SymInitialize((HANDLE)-1, nullptr, true)) {
            std::printf("[!] SymInitialize: %ld\n", ::GetLastError());
            return false;
        }

        pInfo = reinterpret_cast<OBJECT_DIRECTORY_INFORMATION*>(::HeapAlloc(::GetProcessHeap(), HEAP_ZERO_MEMORY, 1024));

        for (;;) {

            if ((STATUS = pNtQueryDirectoryObject(hDirectory, pInfo, 1024, true, false, &uContext, &uRetLen)) == STATUS_NO_MORE_ENTRIES)
                break;

            ::CharLowerW(pInfo->Name.Buffer);

            if ((hModule = ::GetModuleHandleW(pInfo->Name.Buffer)) == nullptr) {
                hModule = ::LoadLibraryW(pInfo->Name.Buffer);
            }

            LoadSymbolsForModule(hModule, pInfo->Name.Buffer);
        }

        return true;
    }
}
