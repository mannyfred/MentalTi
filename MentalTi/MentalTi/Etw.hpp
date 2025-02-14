#ifndef ETW_HPP
#define ETW_HPP

#include <windows.h>
#include <iostream>
#include <memory>
#include <tdh.h>
#include <bitset>
#include <string>
#include <winternl.h>
#include <functional>
#pragma comment(lib, "tdh.lib")


namespace Etw {

    class EventParser {
    public:

        EventParser(EVENT_RECORD* pEventRecord) : _EventRecord(pEventRecord) {}

        ULONG GetEventId() const;

        ULONG GetProcId() const;

        template<typename T>
        std::unique_ptr<T> ParseEvent(const std::string& member_name) {

            std::wstring w_member_name(member_name.begin(), member_name.end());

            try {
                std::unique_ptr<TRACE_EVENT_INFO> eventInfo = GetEventInfo();

                ULONG                       status = 0;
                PROPERTY_DATA_DESCRIPTOR    data_desc = { 0 };

                for (ULONG i = 0; i < eventInfo->TopLevelPropertyCount; i++) {

                    EVENT_PROPERTY_INFO* pPropInfo = &eventInfo->EventPropertyInfoArray[i];

                    auto current_name = std::wstring((WCHAR*)((BYTE*)eventInfo.get() + pPropInfo->NameOffset));

                    if (current_name == w_member_name) {

                        data_desc.PropertyName = (ULONGLONG)((BYTE*)eventInfo.get() + pPropInfo->NameOffset);
                        data_desc.ArrayIndex = ULONG_MAX;

                        ULONG property_len = pPropInfo->length;

                        if (!property_len) {
                            status = TdhGetPropertySize(_EventRecord, 0, nullptr, 1, &data_desc, &property_len);
                            if (status != ERROR_SUCCESS) {
                                throw std::runtime_error("Failed to retrieve property size - Error: " + std::to_string(status));
                            }
                        }

                        //Stupid little shit
                        if constexpr (std::is_same_v<T, UNICODE_STRING>) {

                            //Get this weak ass size out of here
                            if (property_len <= sizeof(T)) {
                                return nullptr;
                            }

                            auto cancer = std::make_unique<std::byte[]>(property_len);
                            std::memset(cancer.get(), 0, property_len);
                            status = TdhGetProperty(_EventRecord, 0, nullptr, 1, &data_desc, property_len, reinterpret_cast<BYTE*>(cancer.get()));

                            if (status != ERROR_SUCCESS) {
                                throw std::runtime_error("Failed to retrieve property value - Error: " + std::to_string(status));
                            }

                            UNICODE_STRING* res = reinterpret_cast<UNICODE_STRING*>(cancer.get());
                            res->Buffer = reinterpret_cast<PWSTR>(cancer.get() + sizeof(T));

                            return std::unique_ptr<T>(reinterpret_cast<T*>(cancer.release()));
                        }

                        //Balls
                        auto member_value = [&]() -> std::unique_ptr<T> {
                            if constexpr (std::is_same_v<T, SID>) {
                                auto sid = std::make_unique<BYTE[]>(39);
                                std::memset(sid.get(), 0, 39);
                                return std::unique_ptr<T>(reinterpret_cast<T*>(sid.release()));
                            }
                            else {
                                auto other = std::make_unique<T>();
                                std::memset(other.get(), 0, property_len);
                                return other;
                            }
                        }();

                        status = TdhGetProperty(_EventRecord, 0, nullptr, 1, &data_desc, property_len, reinterpret_cast<BYTE*>(member_value.get()));

                        if (status != ERROR_SUCCESS) {
                            throw std::runtime_error("Failed to retrieve property value - Error: " + std::to_string(status));
                        }

                        return member_value;
                    }
                }

                throw std::runtime_error("Not found: " + member_name);
            }
            catch (const std::exception& e) {
                std::cerr << "[!] Exception ParseEvent - " << e.what() << std::endl;
                return nullptr;
            }
        }

    private:

        EVENT_RECORD* _EventRecord;
        std::unique_ptr<TRACE_EVENT_INFO> GetEventInfo();

    };
}

#endif // !ETW_HPP