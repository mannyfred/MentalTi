#pragma once
#include <Windows.h>
#include <stdio.h>
#include <conio.h>
#include <wmistr.h>
#include <evntrace.h>
#include <wbemidl.h>
#include <Evntcons.h>
#include "Mental.hpp"
#include "Utils.hpp"

#define	ETW_SESSION_NAME    L"MentalTi"
#define ETW_HELPER_SESSION  L"MentalProc"
#define STAT_INTERVAL_MS    3500       

static const GUID   g_EtwTiProviderGuid         = { 0xF4E1897C, 0xBB5D, 0x5668, { 0xF1, 0xD8, 0x04, 0x0f, 0x4d, 0x8d, 0xd3, 0x44 } };
static const GUID   g_SysProcessProviderGuid    = { 0x151f55dc, 0x467d, 0x471f, { 0x83, 0xb5, 0x5f, 0x88, 0x9d, 0x46, 0xff, 0x66 } };


struct Stats {

    std::unordered_map<int, size_t> Counts;

    void Increment(int Id) {
        Counts[Id]++;
    }

    void Display() const {

        for (const auto& [event, count] : Counts) {
            if (count > 0) {
                std::printf("[Event ID: %lu --- Count: %llu]\n", event, count);
            }
        }
    }
};


Stats g_Stats;


void QueryStats() {

    DWORD nuke = 0;
    DWORD nuked = 0;
    ULONG status;

    COORD                       start   = { 0, 1 };
    EVENT_TRACE_PROPERTIES      trace   = { 0 };
    CONSOLE_SCREEN_BUFFER_INFO  csbi    = { 0 };

    trace.Wnode.BufferSize      = sizeof(EVENT_TRACE_PROPERTIES) + sizeof(ETW_SESSION_NAME);
    trace.Wnode.Flags           = WNODE_FLAG_TRACED_GUID;
    trace.Wnode.ClientContext   = 1;
    trace.LogFileMode           = EVENT_TRACE_REAL_TIME_MODE;
    trace.LoggerNameOffset      = sizeof(EVENT_TRACE_PROPERTIES);

    HANDLE hConsole = ::GetStdHandle(STD_OUTPUT_HANDLE);

    Sleep(2000);

    while (true) {

        ::GetConsoleScreenBufferInfo(hConsole, &csbi);
        
        nuke = csbi.dwSize.X * (csbi.dwSize.Y - 1);
        
        ::FillConsoleOutputCharacter(hConsole, ' ', nuke, start, &nuked);
        ::FillConsoleOutputAttribute(hConsole, csbi.wAttributes, nuke, start, &nuked);
        ::SetConsoleCursorPosition(hConsole, start);

        if ((status = ::ControlTraceW(0, (LPCWSTR)ETW_SESSION_NAME, &trace, EVENT_TRACE_CONTROL_QUERY)) == ERROR_SUCCESS) {
            std::printf("[Events lost: %lu]\n", trace.EventsLost);
        }
        else {
            std::printf("[Events lost: -]\n");
        }

        g_Stats.Display();

        ::Sleep(STAT_INTERVAL_MS);
    }
}


void WINAPI EtwTiCallback(EVENT_RECORD* pEventRecord) {

    // APC/SETTHREADXONTEXT always have PID as 4 in the event header, last checks to make sure we include them when targeting a specific proc 
    if (Globals::Get().Vars().TargetProc == 0 || Globals::Get().Vars().TargetProc == pEventRecord->EventHeader.ProcessId || pEventRecord->EventHeader.EventDescriptor.Id == 4 || pEventRecord->EventHeader.EventDescriptor.Id == 5) {

        LilETW::EventParser event_data(pEventRecord);

        for (const auto& [keyword, metadata] : Keywords) {

            if (pEventRecord->EventHeader.EventDescriptor.Id == metadata.Id) {

                metadata.Parser(event_data);

                g_Stats.Increment(pEventRecord->EventHeader.EventDescriptor.Id);
                return;
            }
        }
    }
}


void WINAPI ProcessEventCallback(EVENT_RECORD* pEventRecord) {

    if (pEventRecord->EventHeader.EventDescriptor.Opcode <= 3) {

        LilETW::EventParser event_parser(pEventRecord);

        auto pid    = event_parser.ParseMember<ULONG>(L"ProcessId");
        auto image  = event_parser.ParseMember<char>(L"ImageFileName");

        BOOL    bIs32bit = false;
        HANDLE  hProcess = ::OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, false, pid.value());

        if (hProcess) {

            ::IsWow64Process(hProcess, &bIs32bit);
            ::CloseHandle(hProcess);

            if (bIs32bit) {
                return;
            }
        }

        switch (pEventRecord->EventHeader.EventDescriptor.Opcode) {
        case 1:
        case 3: {
            Symbols::LoadProcessInfo(pid.value(), image.get());
            break;
        }
        case 2: {
            Symbols::NukeProcessInfo(pid.value());
            break;
        }
        default:
            break;
        }
    }
}


void StartParsing(PROCESSTRACE_HANDLE lpParam) {

    ULONG status;

    if ((status = ::ProcessTrace(&lpParam, 1, nullptr, nullptr)) != ERROR_SUCCESS) {
        std::printf("[!] ProcessTrace: %lu\n", status);
    }
}


int main(int argc, char** argv) {

    HANDLE hThread1 = nullptr;
    HANDLE hThread2 = nullptr;
    HANDLE hThread3 = nullptr;

    if (argc < 5) {
        Utils::PrintHelp();
        return -1;
    }

    if (!Utils::SendIOCTL(MENTALTI_OPEN, 0, 0)) {
        return -1;
    }

    if (!Utils::ParseUserInput(argc, argv)) {
        return -1;
    }

    LilETW::Symbols::Syms();

    auto process_trace  = LilETW::TraceSessions::Get().StartTraceSession(g_SysProcessProviderGuid, ETW_HELPER_SESSION, ProcessEventCallback, 2, true, TRACE_LEVEL_INFORMATION, SYSTEM_PROCESS_KW_GENERAL, {}, {});
    auto threat_trace   = LilETW::TraceSessions::Get().StartTraceSession(g_EtwTiProviderGuid, ETW_SESSION_NAME, EtwTiCallback, 8, false, TRACE_LEVEL_VERBOSE, Globals::Get().Vars().Keywords, {}, Globals::Get().Vars().StackTracedEvents);

    if (::SetConsoleCtrlHandler((PHANDLER_ROUTINE)Utils::CtrlHandler, true))
        printf("\t\t\t\t\t\t >> Press <CTRL+C> To Quit <<\n");

    hThread1 = ::CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)QueryStats, nullptr, 0, nullptr);
    hThread2 = ::CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)StartParsing, (LPVOID)process_trace, 0, nullptr);
    hThread3 = ::CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)StartParsing, (LPVOID)threat_trace, CREATE_SUSPENDED, nullptr);

    if (!hThread1 || !hThread2 || !hThread3) {
        std::printf("[!] CreateThread: %lu\n", GetLastError());
        return -1;
    }

    ::SetThreadPriority(hThread3, 1);

    ::ResumeThread(hThread3);

    ::WaitForSingleObject(hThread3, INFINITE);

    return 0;
}