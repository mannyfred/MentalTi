#include "Symbols.hpp"
#include "Utils.hpp"

namespace Symbols {

    using SymbolMap = std::unordered_map<DWORD64, SymbolInfo>;
    std::unordered_map<ModuleInfo, SymbolMap> SymbolStore;

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

    std::string ResolveSymbol(uintptr_t address) {

        for (const auto& [mod_info, symbols] : SymbolStore) {

            if (address >= mod_info.base && address < mod_info.end) {

                if (address == mod_info.base) {
                    return mod_info.mod_name;
                }

                auto it = symbols.find(address);

                if (it != symbols.end()) {
                    return mod_info.mod_name + "!" + it->second.name;
                }
                else {

                    for (const auto& [sym_addr, sym_info] : symbols) {

                        if (address >= sym_addr && address < sym_addr + sym_info.size) {

                            uintptr_t offset = address - sym_addr;
                            return mod_info.mod_name + "!" + sym_info.name + "+" + std::format("0x{:x}", offset);
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