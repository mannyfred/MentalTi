#ifndef SYMBOLS_HPP
#define SYMBOLS_HPP

#include <windows.h>
#include <unordered_map>
#include <string>
#include <iostream>
#include <filesystem>


struct ProcessInfo {
    uintptr_t base;
    uintptr_t end;
    std::string proc_name;
};

namespace Symbols {

    void LoadProcessInfo(ULONG pid, const char* imagename);
    std::string ResolveProcessSymbol(ULONG pid, uintptr_t address);
    void NukeProcessInfo(ULONG pid);
    std::string ReturnProcessExecutable(ULONG pid);
}

#endif // !SYMBOLS_HPP

