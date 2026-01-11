#include "Symbols.hpp"
#include "Utils.hpp"
#include <tlhelp32.h>
#include <array>
#include <shared_mutex>

namespace Symbols {

    std::shared_mutex ProcessStoreMutex;
    std::unordered_map<ULONG, ProcessInfo> ProcessStore;

    void LoadProcessInfo(ULONG pid, const char* imagename) {

        HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, pid);

        DEFER({
            if (hSnap != INVALID_HANDLE_VALUE)
                ::CloseHandle(hSnap);
        });

        if (hSnap == INVALID_HANDLE_VALUE) {
            return;
        }

        std::string proc_name = imagename ? imagename : "<null>";

        MODULEENTRY32 mod = { .dwSize = sizeof(MODULEENTRY32) };

        if (Module32First(hSnap, &mod)) {

            uintptr_t base = reinterpret_cast<uintptr_t>(mod.modBaseAddr);
            uintptr_t end  = base + mod.modBaseSize;

            ProcessInfo pInfo = { base, end, proc_name };
            std::unique_lock lock(ProcessStoreMutex);
            ProcessStore[pid] = std::move(pInfo);
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
}
