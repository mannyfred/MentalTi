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
#define STAT_INTERVAL_MS    3500       

static const GUID   g_EtwTiProviderGuid = { 0xF4E1897C, 0xBB5D, 0x5668, { 0xF1, 0xD8, 0x04, 0x0f, 0x4d, 0x8d, 0xd3, 0x44 } };

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


void WINAPI EtwTiCallback(EVENT_RECORD* pEventRecord) {


    // APC/SETTHREADXONTEXT always have PID as 4 in the event header, last checks to make sure we include them when targeting a specific proc 
    if (g_Global->Vars().TargetProc == 0 || g_Global->Vars().TargetProc == pEventRecord->EventHeader.ProcessId || pEventRecord->EventHeader.EventDescriptor.Id == 4 || pEventRecord->EventHeader.EventDescriptor.Id == 5) {

        g_Stats.Increment(pEventRecord->EventHeader.EventDescriptor.Id);

        Etw::EventParser event_data(pEventRecord);

        for (const auto& [keyword, metadata] : Keywords) {

            if (pEventRecord->EventHeader.EventDescriptor.Id == metadata.Id) {

                metadata.Parser(event_data);
                return;
            }
        }
    }

    return;
}


void QueryStats() {

    DWORD nuke = 0;
    DWORD nuked = 0;
    ULONG status;

    COORD                       start   = { 0, 1 };
    EVENT_TRACE_PROPERTIES      trace   = { 0 };
    CONSOLE_SCREEN_BUFFER_INFO  csbi    = { 0 };

    trace.Wnode.BufferSize = sizeof(EVENT_TRACE_PROPERTIES) + sizeof(ETW_SESSION_NAME);
    trace.Wnode.Flags = WNODE_FLAG_TRACED_GUID;
    trace.Wnode.ClientContext = 1;
    trace.LogFileMode = EVENT_TRACE_REAL_TIME_MODE;
    trace.LoggerNameOffset = sizeof(EVENT_TRACE_PROPERTIES);

    HANDLE hConsole = ::GetStdHandle(STD_OUTPUT_HANDLE);

    Sleep(2000);

    while (true) {

        ::GetConsoleScreenBufferInfo(hConsole, &csbi);
        
        nuke = csbi.dwSize.X * (csbi.dwSize.Y - 1);
        
        ::FillConsoleOutputCharacter(hConsole, ' ', nuke, start, &nuked);
        ::FillConsoleOutputAttribute(hConsole, csbi.wAttributes, nuke, start, &nuked);
        ::SetConsoleCursorPosition(hConsole, start);

        status = ::ControlTraceW(0, (LPCWSTR)ETW_SESSION_NAME, &trace, EVENT_TRACE_CONTROL_QUERY);
        if (status == ERROR_SUCCESS) {
            std::printf("[Events lost: %lu]\n", trace.EventsLost);
        }
        else {
            std::printf("[Events lost: -]\n");
        }

        g_Stats.Display();

        ::Sleep(STAT_INTERVAL_MS);
    }

    return;
}


bool StartEtwTi() {

    ULONG                   status;
    bool                    bSuccess    = false;
    EVENT_TRACE_LOGFILEW    trace       = { 0 };
    
    ULONG                   bufferSize  = sizeof(EVENT_TRACE_PROPERTIES) + sizeof(ETW_SESSION_NAME);

    g_Global->Vars().TraceName  = (LPWSTR)ETW_SESSION_NAME;
    g_Global->Vars().LoggerInfo = (EVENT_TRACE_PROPERTIES*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, bufferSize);

    if (!g_Global->Vars().LoggerInfo) {
        goto _EndOfFunc;
    }

    g_Global->Vars().LoggerInfo->Wnode.BufferSize       = bufferSize;
    g_Global->Vars().LoggerInfo->BufferSize             = 0x1000 * 8;
    g_Global->Vars().LoggerInfo->MinimumBuffers         = 0;
    g_Global->Vars().LoggerInfo->Wnode.Flags            = WNODE_FLAG_TRACED_GUID;
    g_Global->Vars().LoggerInfo->Wnode.ClientContext    = 1;
    g_Global->Vars().LoggerInfo->LogFileMode            = EVENT_TRACE_REAL_TIME_MODE;
    g_Global->Vars().LoggerInfo->LogFileNameOffset      = 0;
    g_Global->Vars().LoggerInfo->LoggerNameOffset       = sizeof(EVENT_TRACE_PROPERTIES);

    status = ::StartTraceW(&g_Global->Vars().LoggerHandle, (LPCWSTR)ETW_SESSION_NAME, g_Global->Vars().LoggerInfo);
    if (status != ERROR_SUCCESS) {
        std::printf("[!] StartTraceW: %lu\n", status);
        goto _EndOfFunc;
    }

    status = ::EnableTraceEx2(g_Global->Vars().LoggerHandle, &g_EtwTiProviderGuid, EVENT_CONTROL_CODE_ENABLE_PROVIDER, TRACE_LEVEL_VERBOSE, g_Global->Vars().Keywords, 0, 0, nullptr);
    if (status != ERROR_SUCCESS) {
        std::printf("[!] EnableTraceEx2: %lu\n", status);
        goto _EndOfFunc;
    }
    
    trace.LogFileName = nullptr;
    trace.LoggerName = (LPWSTR)ETW_SESSION_NAME;
    trace.ProcessTraceMode = PROCESS_TRACE_MODE_REAL_TIME | PROCESS_TRACE_MODE_EVENT_RECORD;
    trace.EventRecordCallback = (PEVENT_RECORD_CALLBACK)EtwTiCallback;

    g_Global->Vars().LoggerHandle = ::OpenTraceW(&trace);
    if (g_Global->Vars().LoggerHandle == INVALID_PROCESSTRACE_HANDLE) {
        std::printf("[!] OpenTrace: %lu\n", status);
        goto _EndOfFunc;
    }

    status = ::ProcessTrace(&g_Global->Vars().LoggerHandle, 1, nullptr, nullptr);
    if (status != ERROR_SUCCESS) {
        std::printf("[!] ProcessTrace: %lu\n", status);
        goto _EndOfFunc;
    }

    bSuccess = true;

_EndOfFunc:

    if (!bSuccess) {
        delete g_Global;
        return false;
    }

    return true;
}


int main(int argc, char** argv) {

    if (argc != 5) {
        Utils::PrintHelp();
        return -1;
    }

    g_Global = new Globals;
    if (!g_Global)
        return -1;

    if (!Utils::SendIOCTL(MENTALTI_OPEN, 0, 0)) {
        delete g_Global;
        return -1;
    }

    if (!Utils::ParseUserInput(argc, argv)) {
        delete g_Global;
        return -1;
    }

    if (!Symbols::InitSymbols()) {
        delete g_Global;
        return -1;
    }

    if (::SetConsoleCtrlHandler((PHANDLER_ROUTINE)Utils::CtrlHandler, true))
        printf("\t\t\t\t\t\t >> Press <CTRL+C> To Quit <<\n");

    HANDLE  hThread1 = ::CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)StartEtwTi, nullptr, CREATE_SUSPENDED, nullptr);
    HANDLE  hThread2 = ::CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)QueryStats, nullptr, 0, nullptr);

    if (!hThread1 || !hThread2) {
        delete g_Global;
        return -1;
    }

    ::SetThreadPriority(hThread1, 1);

    ::ResumeThread(hThread1);

    ::WaitForSingleObject(hThread1, INFINITE);

    return 0;
}