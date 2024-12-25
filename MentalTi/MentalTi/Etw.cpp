#include "Etw.hpp"

namespace Etw {

    ULONG EventParser::GetEventId() const {
        return _EventRecord->EventHeader.EventDescriptor.Id;
    }

    ULONG EventParser::GetProcId() const {
        return _EventRecord->EventHeader.ProcessId;
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