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
#define MENTALTI_SINGLE	CTL_CODE(KMENTALTI, 0x802, METHOD_NEITHER, FILE_ANY_ACCESS)

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

namespace Utils {

	bool ParseUserKeywords(const std::string& input);
	bool ParseUserInput(const int argc, char** argv);
	bool CreateOpenOutputFile(const std::string& filename);
	void PrintHelp();
	bool SendIOCTL(ULONG ioctl, ULONG flags, ULONG pid);
	bool CtrlHandler(DWORD fdwCtrlType);
}

#endif // !UTILS_HPP
