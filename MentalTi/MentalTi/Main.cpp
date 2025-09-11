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
    if (Globals::Get().Vars().TargetProc == 0 || Globals::Get().Vars().TargetProc == pEventRecord->EventHeader.ProcessId || pEventRecord->EventHeader.EventDescriptor.Id == 4 || pEventRecord->EventHeader.EventDescriptor.Id == 5) {

        Etw::EventParser event_data(pEventRecord);

        for (const auto& [keyword, metadata] : Keywords) {

            if (pEventRecord->EventHeader.EventDescriptor.Id == metadata.Id) {

                metadata.Parser(event_data);

                g_Stats.Increment(pEventRecord->EventHeader.EventDescriptor.Id);
                return;
            }
        }
    }
}


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


bool StartEtwTi() {

    ULONG                       status;
    bool                        bSuccess    = false;
    EVENT_TRACE_LOGFILEW        trace       = { 0 };
    ENABLE_TRACE_PARAMETERS*    params      = nullptr;
    
    Globals::Get().Vars().TraceName  = (LPWSTR)ETW_SESSION_NAME;
    Globals::Get().Vars().LoggerInfo = (EVENT_TRACE_PROPERTIES*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(EVENT_TRACE_PROPERTIES) + sizeof(ETW_SESSION_NAME));

    if (!Globals::Get().Vars().LoggerInfo) {
        goto _EndOfFunc;
    }

    Globals::Get().Vars().LoggerInfo->Wnode.BufferSize       = sizeof(EVENT_TRACE_PROPERTIES) + sizeof(ETW_SESSION_NAME);
    Globals::Get().Vars().LoggerInfo->BufferSize             = 0x1000 * 8;
    Globals::Get().Vars().LoggerInfo->MinimumBuffers         = 0;
    Globals::Get().Vars().LoggerInfo->Wnode.Flags            = WNODE_FLAG_TRACED_GUID;
    Globals::Get().Vars().LoggerInfo->Wnode.ClientContext    = 1;
    Globals::Get().Vars().LoggerInfo->LogFileMode            = EVENT_TRACE_REAL_TIME_MODE;
    Globals::Get().Vars().LoggerInfo->LogFileNameOffset      = 0;
    Globals::Get().Vars().LoggerInfo->LoggerNameOffset       = sizeof(EVENT_TRACE_PROPERTIES);

    trace.LogFileName           = nullptr;
    trace.LoggerName            = (LPWSTR)ETW_SESSION_NAME;
    trace.ProcessTraceMode      = PROCESS_TRACE_MODE_REAL_TIME | PROCESS_TRACE_MODE_EVENT_RECORD;
    trace.EventRecordCallback   = (PEVENT_RECORD_CALLBACK)EtwTiCallback;

    if ((status = ::StartTraceW(&Globals::Get().Vars().LoggerHandle, (LPCWSTR)ETW_SESSION_NAME, Globals::Get().Vars().LoggerInfo)) != ERROR_SUCCESS) {
        std::printf("[!] StartTraceW: %lu\n", status);
        goto _EndOfFunc;
    }

    // For testing
    if (Globals::Get().Vars().StackTrace) {

        ENABLE_TRACE_PARAMETERS	trace_params    = { 0 };
        EVENT_FILTER_DESCRIPTOR	filter_desc     = { 0 };
        EVENT_FILTER_EVENT_ID	filter_event    = { 0 };

        filter_event.FilterIn   = true;
        filter_event.Count      = 1;
        filter_event.Events[0]  = 2;

        filter_desc.Ptr     = (ULONGLONG)&filter_event;
        filter_desc.Size    = sizeof(filter_event);
        filter_desc.Type    = EVENT_FILTER_TYPE_STACKWALK;

        trace_params.Version             = ENABLE_TRACE_PARAMETERS_VERSION_2;
        trace_params.EnableFilterDesc    = &filter_desc;
        trace_params.FilterDescCount     = 1;
        trace_params.EnableProperty      = EVENT_ENABLE_PROPERTY_STACK_TRACE;

        params = &trace_params;
    }

    if ((status = ::EnableTraceEx2(Globals::Get().Vars().LoggerHandle, &g_EtwTiProviderGuid, EVENT_CONTROL_CODE_ENABLE_PROVIDER, TRACE_LEVEL_VERBOSE, Globals::Get().Vars().Keywords, 0, 0, params)) != ERROR_SUCCESS) {
        std::printf("[!] EnableTraceEx2: %lu\n", status);
        goto _EndOfFunc;
    }
    
    if ((Globals::Get().Vars().LoggerHandle = ::OpenTraceW(&trace)) == INVALID_PROCESSTRACE_HANDLE) {
        std::printf("[!] OpenTrace: %lu\n", status);
        goto _EndOfFunc;
    }

    if ((status = ::ProcessTrace(&Globals::Get().Vars().LoggerHandle, 1, nullptr, nullptr)) != ERROR_SUCCESS) {
        std::printf("[!] ProcessTrace: %lu\n", status);
        goto _EndOfFunc;
    }

    bSuccess = true;

_EndOfFunc:

    return bSuccess;
}


int main(int argc, char** argv) {

    if (argc != 5) {
        Utils::PrintHelp();
        return -1;
    }

    if (!Utils::SendIOCTL(MENTALTI_OPEN, 0, 0)) {
        return -1;
    }

    if (!Utils::ParseUserInput(argc, argv)) {
        return -1;
    }

    if (!Symbols::InitSymbols()) {
        return -1;
    }

    if (::SetConsoleCtrlHandler((PHANDLER_ROUTINE)Utils::CtrlHandler, true))
        printf("\t\t\t\t\t\t >> Press <CTRL+C> To Quit <<\n");

    HANDLE  hThread1 = ::CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)StartEtwTi, nullptr, CREATE_SUSPENDED, nullptr);
    HANDLE  hThread2 = ::CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)QueryStats, nullptr, 0, nullptr);

    if (!hThread1 || !hThread2) {
        return -1;
    }

    ::SetThreadPriority(hThread1, 1);

    ::ResumeThread(hThread1);

    ::WaitForSingleObject(hThread1, INFINITE);

    return 0;
}