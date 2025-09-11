#include "Etw.hpp"

namespace Etw {

    ULONG EventParser::GetEventId() const {
        return _EventRecord->EventHeader.EventDescriptor.Id;
    }

    ULONG EventParser::GetProcId() const {
        return _EventRecord->EventHeader.ProcessId;
    }

    bool EventParser::StackTrace64Present() const {
        return _EventRecord->ExtendedData && _EventRecord->ExtendedData[0].ExtType == EVENT_HEADER_EXT_TYPE_STACK_TRACE64;
    }

    ULONG EventParser::StackFrameCount() const {
        return _EventRecord->ExtendedData[0].DataSize / sizeof(ULONG64);
    }

    ULONG64* EventParser::StackFrames() const {
        return (ULONG64*)_EventRecord->ExtendedData[0].DataPtr;
    }

    std::unique_ptr<TRACE_EVENT_INFO> EventParser::GetEventInfo() {

        ULONG buffer_size = 0;
        ULONG status = TdhGetEventInformation(_EventRecord, 0, nullptr, nullptr, &buffer_size);

        if (status != ERROR_INSUFFICIENT_BUFFER) {
            throw std::runtime_error("Event error idk");
        }

        auto buffer = std::make_unique<BYTE[]>(buffer_size);
        TRACE_EVENT_INFO* event_info = reinterpret_cast<TRACE_EVENT_INFO*>(buffer.get());

        status = TdhGetEventInformation(_EventRecord, 0, nullptr, event_info, &buffer_size);

        if (status != ERROR_SUCCESS) {
            throw std::runtime_error("Event error again");
        }

        return std::unique_ptr<TRACE_EVENT_INFO>(reinterpret_cast<TRACE_EVENT_INFO*>(buffer.release()));
    }
}