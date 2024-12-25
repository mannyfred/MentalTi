#include "Utils.hpp"

#define KMENTALTI	    0x8000
#define MENTALTI_OPEN   CTL_CODE(KMENTALTI, 0x800, METHOD_NEITHER, FILE_ANY_ACCESS)

extern void ParseKeywords(ULONG arg);

namespace Utils {

    ULONG GetUlong(const std::string& arg) {

        if (arg.substr(0, 2) == "0x" || arg.substr(0, 2) == "0X") {
            try {
                return std::stoul(arg, nullptr, 16);
            }
            catch (const std::invalid_argument&) {
                throw std::invalid_argument("Invalid string: " + arg);
            }
            catch (const std::out_of_range&) {
                throw std::out_of_range("Out of range: " + arg);
            }
        }
        else {
            try {
                return std::stoul(arg);
            }
            catch (const std::invalid_argument&) {
                throw std::invalid_argument("Invalid string: " + arg);
            }
            catch (const std::out_of_range&) {
                throw std::out_of_range("Out of range: " + arg);
            }
        }
    }

    bool CreateOpenOutputFile(const std::string& filename) {

        g_Global->Vars().OutputHandle.open(filename, std::ios::app);

        if (!g_Global->Vars().OutputHandle.is_open()) {
            std::printf("[!] Error opening/creating output file \n");
            return false;
        }

        return true;
    }

    bool ParseUserKeywords(const std::string& input) {

        ULONG hex = 0;
        ULONG combined = 0;
        std::stringstream ss(input);
        std::string piece;

        while (std::getline(ss, piece, '|')) {

            size_t start = piece.find_first_not_of(' ');
            size_t end = piece.find_last_not_of(' ');

            if (start != std::string::npos && end != std::string::npos) {
                piece = piece.substr(start, end - start + 1);
            }

            try {
                hex = GetUlong(piece);
                ParseKeywords(hex);
                combined |= hex;
            }
            catch (const std::exception& e) {
                std::cerr << "Error with keyword: " << piece << " - " << e.what() << std::endl;
                return false;
            }
        }

        g_Global->Vars().Keywords = combined;
        return true;
    }

    bool ParseUserInput(const int argc, char** argv) {

        if (std::string(argv[1]) != "-proc") {
            std::printf("[!] Invalid arg \n");
            return false;
        }

        std::string pid_string = argv[2];
        if (pid_string == "all") {
            g_Global->Vars().TargetProc = 0;
        }
        else if (!pid_string.empty()) {
            try {
                ULONG pid = GetUlong(pid_string);
                g_Global->Vars().TargetProc = pid;
            }
            catch (const std::exception& e) {
                std::cerr << "Error - " << e.what() << std::endl;
                return false;
            }
        }

        std::string keywords = argv[3];
        try {
            ParseUserKeywords(keywords);
        }
        catch (const std::exception& e) {
            std::cerr << "Error - " << e.what() << std::endl;
            return false;
        }

        std::string out_file = argv[4];
        if (out_file.empty()) {
            std::printf("[!] Invalid arg \n");
            return false;
        }

        if (!CreateOpenOutputFile(out_file)) {
            std::printf("[!] Invalid arg \n");
            return false;
        }

        return true;
    }

    void PrintHelp() {

        std::printf("\nExample: MentalTi.exe -proc <PID>/all \"0x10 | 0x80 | 0x2000\" .\\out.json \n\n");
        std::printf("Arg1 & Arg2 - Specify PID to monitor a specific process or \"all\" to capture from all processes: \n\t -proc <PID> \n\t -proc all\n\n");
        std::printf("Arg3 - Keywords for specific events: \n");
        std::printf(" 0x1\t\t- ALLOCVM_LOCAL\n 0x2\t\t- ALLOCVM_LOCAL_KERNEL\n 0x4\t\t- ALLOCVM_REMOTE\n 0x8\t\t- ALLOCVM_REMOTE_KERNEL\n 0x10\t\t- PROTECTVM_LOCAL\n 0x20\t\t- PROTECTVM_LOCAL_KERNEL\n");
        std::printf(" 0x40\t\t- PROTECTVM_REMOTE\n 0x80\t\t- PROTECTVM_REMOTE_KERNEL\n 0x100\t\t- MAPVIEW_LOCAL\n 0x200\t\t- MAPVIEW_LOCAL_KERNEL\n 0x400\t\t- MAPVIEW_REMOTE\n");
        std::printf(" 0x800\t\t- MAPVIEW_REMOTE_KERNEL\n 0x1000\t\t- QUEUE_USER_APC_REMOTE\n 0x2000\t\t- QUEUE_USER_APC_REMOTE_KERNEL\n 0x4000\t\t- SETTHREADCONTEXT_REMOTE\n");
        std::printf(" 0x8000\t\t- SETTHREADCONTEXT_REMOTE_KERNEL\n 0x10000\t- READVM_LOCAL\n 0x20000\t- READVM_REMOTE\n 0x40000\t- WRITEVM_LOCAL\n 0x80000\t- WRITEVM_REMOTE\n");
        std::printf(" 0x100000\t- SUSPEND_THREAD\n 0x200000\t- RESUME_THREAD\n 0x400000\t- SUSPEND_PROCESS\n 0x800000\t- RESUME_PROCESS\n 0x1000000\t- FREEZE_PROCESS\n");
        std::printf(" 0x2000000\t- THAW_PROCESS\n 0x40000000\t- DRIVER_EVENT\n 0x80000000\t- DEVICE_EVENT\n\n");
        std::printf("Arg4 - Output file:\n\tC:\\Users\\someone\\log.json\n\tlog.json\n");
    }

    bool EnablePPL() {

        g_Global->Vars().DriverHandle = ::CreateFileW(L"\\\\.\\KMentalTi", GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);

        if (g_Global->Vars().DriverHandle == nullptr) {
            std::printf("[!] Error getting driver handle: %ld\n", GetLastError());
            return false;
        }

        if (!::DeviceIoControl(g_Global->Vars().DriverHandle, MENTALTI_OPEN, nullptr, 0, nullptr, 0, nullptr, nullptr)) {
            std::printf("[!] Error with driver: %ld\n", GetLastError());
            return false;
        }

        return true;
    }

    bool CtrlHandler(DWORD fdwCtrlType) {

        switch (fdwCtrlType) {

        case CTRL_C_EVENT: {

            delete g_Global;

            ::ExitProcess(0);
        }

        default:
            return false;
        }
    }

}


