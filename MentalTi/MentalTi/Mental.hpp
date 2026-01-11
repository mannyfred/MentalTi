#ifndef MENTAL_HPP
#define MENTAL_HPP

#include <typeindex>
#include <sstream>
#include <format>
#include <variant>
#include <any>
#include <bitset>
#include <sddl.h>
#include "external/LilETW/LilETW.hpp"
#include "external/json/single_include/nlohmann/json.hpp"
#include "EtwTi.hpp"
#include "Symbols.hpp"
#include "Utils.hpp"

//If mental illness was a header

struct EventMetadata {
    ULONG Id;
    std::function<void()> InputHandler;
    std::function<void(LilETW::EventParser&)> Parser;
};

std::unordered_map<std::type_index, void*> wrappers;

template <typename T>
using ParserReturn  = std::conditional_t<LilETW::EtwSpecialType<T>, std::unique_ptr<T>, std::optional<T>>;
using ReturnVariant = std::variant<UCHAR, ULONG, PVOID, FILETIME, ULONG64, USHORT, std::unique_ptr<UNICODE_STRING>, std::unique_ptr<SID>, std::unique_ptr<char>>;
using MemberReturn  = std::optional<ReturnVariant>;

template <typename T>
constexpr MemberReturn GetMember(LilETW::EventParser& parser, const std::wstring& name) {
    auto res = parser.ParseMember<T>(name);
    if constexpr (LilETW::EtwSpecialType<T>) {
        if (!res) return std::nullopt;
        return ReturnVariant(std::move(res));
    }
    else {
        if (!res) return std::nullopt;
        return ReturnVariant(*res);
    }
}

using MemberFunc = MemberReturn(*)(LilETW::EventParser&, const std::wstring&);

#define X(member, type) { L## #member, &GetMember<type> },

template <typename T>
std::map<std::wstring, MemberFunc> FieldMap();

template <>
std::map<std::wstring, MemberFunc> FieldMap<EtwTi::ETWTI_ALLOCVM_LOCAL>() {
    return { ETWTI_ALLOCVM_LOCALREMOTE_FIELDS };
}
template <>
std::map<std::wstring, MemberFunc> FieldMap<EtwTi::ETWTI_ALLOCVM_LOCAL_KERNEL>() {
    return { ETWTI_ALLOCVM_LOCALREMOTE_FIELDS };
}
template <>
std::map<std::wstring, MemberFunc> FieldMap<EtwTi::ETWTI_ALLOCVM_REMOTE>() {
    return { ETWTI_ALLOCVM_LOCALREMOTE_FIELDS };
}
template <>
std::map<std::wstring, MemberFunc> FieldMap<EtwTi::ETWTI_ALLOCVM_REMOTE_KERNEL>() {
    return { ETWTI_ALLOCVM_LOCALREMOTE_FIELDS };
}
template <>
std::map<std::wstring, MemberFunc> FieldMap<EtwTi::ETWTI_PROTECTVM_LOCAL>() {
    return { ETWTI_PROTECTVM_LOCALREMOTE_FIELDS };
}
template <>
std::map<std::wstring, MemberFunc> FieldMap<EtwTi::ETWTI_PROTECTVM_LOCAL_KERNEL>() {
    return { ETWTI_PROTECTVM_LOCALREMOTE_FIELDS };
}
template <>
std::map<std::wstring, MemberFunc> FieldMap<EtwTi::ETWTI_PROTECTVM_REMOTE>() {
    return { ETWTI_PROTECTVM_LOCALREMOTE_FIELDS };
}
template <>
std::map<std::wstring, MemberFunc> FieldMap<EtwTi::ETWTI_PROTECTVM_REMOTE_KERNEL>() {
    return { ETWTI_PROTECTVM_LOCALREMOTE_FIELDS };
}
template <>
std::map<std::wstring, MemberFunc> FieldMap<EtwTi::ETWTI_MAPVIEW_LOCAL>() {
    return { ETWTI_MAPVIEW_LOCALREMOTE_FIELDS };
}
template <>
std::map<std::wstring, MemberFunc> FieldMap<EtwTi::ETWTI_MAPVIEW_LOCAL_KERNEL>() {
    return { ETWTI_MAPVIEW_LOCALREMOTE_FIELDS };
}
template <>
std::map<std::wstring, MemberFunc> FieldMap<EtwTi::ETWTI_MAPVIEW_REMOTE>() {
    return { ETWTI_MAPVIEW_LOCALREMOTE_FIELDS };
}
template <>
std::map<std::wstring, MemberFunc> FieldMap<EtwTi::ETWTI_MAPVIEW_REMOTE_KERNEL>() {
    return { ETWTI_MAPVIEW_LOCALREMOTE_FIELDS };
}
template <>
std::map<std::wstring, MemberFunc> FieldMap<EtwTi::ETWTI_QUEUEUSERAPC_REMOTE>() {
    return { ETWTI_QUEUE_APC_REMOTE_FIELDS };
}
template <>
std::map<std::wstring, MemberFunc> FieldMap<EtwTi::ETWTI_QUEUEUSERAPC_REMOTE_KERNEL>() {
    return { ETWTI_QUEUE_APC_REMOTE_FIELDS };
}
template <>
std::map<std::wstring, MemberFunc> FieldMap<EtwTi::ETWTI_SETTHREADCONTEXT_REMOTE>() {
    return { ETWTI_SETTHREADCONTEXT_REMOTE_FIELDS };
}
template <>
std::map<std::wstring, MemberFunc> FieldMap<EtwTi::ETWTI_SETTHREADCONTEXT_REMOTE_KERNEL>() {
    return { ETWTI_SETTHREADCONTEXT_REMOTE_FIELDS };
}
template <>
std::map<std::wstring, MemberFunc> FieldMap<EtwTi::ETWTI_READVM_LOCAL>() {
    return { ETWTI_READVM_LOCALREMOTE_FIELDS };
}
template <>
std::map<std::wstring, MemberFunc> FieldMap<EtwTi::ETWTI_READVM_REMOTE>() {
    return { ETWTI_READVM_LOCALREMOTE_FIELDS };
}
template <>
std::map<std::wstring, MemberFunc> FieldMap<EtwTi::ETWTI_WRITEVM_LOCAL>() {
    return { ETWTI_WRITEVM_LOCALREMOTE_FIELDS };
}
template <>
std::map<std::wstring, MemberFunc> FieldMap<EtwTi::ETWTI_WRITEVM_REMOTE>() {
    return { ETWTI_WRITEVM_LOCALREMOTE_FIELDS };
}
template <>
std::map<std::wstring, MemberFunc> FieldMap<EtwTi::ETWTI_SUSPEND_THREAD>() {
    return { ETWTI_SUSPENDRESUME_THREAD_FIELDS };
}
template <>
std::map<std::wstring, MemberFunc> FieldMap<EtwTi::ETWTI_RESUME_THREAD>() {
    return { ETWTI_SUSPENDRESUME_THREAD_FIELDS };
}
template <>
std::map<std::wstring, MemberFunc> FieldMap<EtwTi::ETWTI_SUSPEND_PROCESS>() {
    return { ETWTI_SUSPENDRESUME_PROCESS_FIELDS };
}
template <>
std::map<std::wstring, MemberFunc> FieldMap<EtwTi::ETWTI_RESUME_PROCESS>() {
    return { ETWTI_SUSPENDRESUME_PROCESS_FIELDS };
}
template <>
std::map<std::wstring, MemberFunc> FieldMap<EtwTi::ETWTI_FREEZE_PROCESS>() {
    return { ETWTI_SUSPENDRESUME_PROCESS_FIELDS };
}
template <>
std::map<std::wstring, MemberFunc> FieldMap<EtwTi::ETWTI_THAW_PROCESS>() {
    return { ETWTI_SUSPENDRESUME_PROCESS_FIELDS };
}
template <>
std::map<std::wstring, MemberFunc> FieldMap<EtwTi::ETWTI_IMPERSONATION_UP>() {
    return { ETWTI_IMPERSONATE_UPDOWN_FIELDS };
}
template <>
std::map<std::wstring, MemberFunc> FieldMap<EtwTi::ETWTI_IMPERSONATION_REVERT>() {
    return { ETWTI_REVERT_TO_SELF_FIELDS };
}
template <>
std::map<std::wstring, MemberFunc> FieldMap<EtwTi::ETWTI_SYSCALL_EVENT>() {
    return { ETWTI_SYSCALL_EVENT_FIELDS };
}
template <>
std::map<std::wstring, MemberFunc> FieldMap<EtwTi::ETWTI_IMPERSONATION_DOWN>() {
    return { ETWTI_IMPERSONATE_UPDOWN_FIELDS };
}

#undef X


template <typename T>
class EventWrapper {
public:

    T data{};
    std::bitset<48> active_fields;

    EventWrapper() : active_fields(0) {}

    void ToggleMember(size_t idx) {
        if (idx < active_fields.size()) {
            active_fields.flip(idx);
        }
    }

    bool IsMemberActive(size_t idx) const {
        return idx < active_fields.size() && active_fields.test(idx);
    }
};


template <typename T>
void GetUserOptions(EventWrapper<T>& wrapper, const std::vector<std::wstring>& field_names) {

    int current_index   = 0;

    DWORD events        = 0;
    DWORD written       = 0;
    DWORD console_size  = 0;
    DWORD old_con_mode  = 0;
    DWORD new_con_mode  = 0;

    HANDLE hConsole     = ::GetStdHandle(STD_OUTPUT_HANDLE);
    HANDLE hInput       = ::GetStdHandle(STD_INPUT_HANDLE);

    INPUT_RECORD                input       = { 0 };
    COORD                       cursor_pos  = { 0, 0 };
    CONSOLE_SCREEN_BUFFER_INFO  csbi        = { 0 };

    ::GetConsoleMode(hInput, &old_con_mode);
    new_con_mode = old_con_mode | ENABLE_PROCESSED_INPUT | ENABLE_EXTENDED_FLAGS;
    ::SetConsoleMode(hInput, new_con_mode);

    while (true) {

        if (::GetConsoleScreenBufferInfo(hConsole, &csbi)) {
            
            console_size = csbi.dwSize.X * csbi.dwSize.Y;
            ::FillConsoleOutputCharacter(hConsole, ' ', console_size, { 0, 0 }, &written);
            ::FillConsoleOutputAttribute(hConsole, csbi.wAttributes, console_size, { 0, 0 }, &written);
            ::SetConsoleCursorPosition(hConsole, { 0, 0 });
        }

        std::wcout << "Selection for: " << typeid(T).name() << std::endl;
        for (int i = 0; i < field_names.size(); i++) {
            std::wcout << (i == current_index ? "> " : "  ");
            std::wcout << field_names[i] << " [" << (wrapper.IsMemberActive(i) ? "+" : "-") << "]" << std::endl;
        }

        while (true) {

            ::ReadConsoleInput(hInput, &input, 1, &events);

            if (input.EventType == KEY_EVENT && input.Event.KeyEvent.bKeyDown) {

                int key = input.Event.KeyEvent.wVirtualKeyCode;

                if (key == VK_RETURN) {
                    return;
                }
                else if (key == VK_SPACE) {
                    wrapper.ToggleMember(current_index);
                    break;
                }
                else if (key == VK_UP && current_index > 0) {
                    --current_index;
                    break;
                }
                else if (key == VK_DOWN && current_index < field_names.size() - 1) {
                    ++current_index;
                    break;
                }
            }
        }
    }
}


template <typename T>
EventWrapper<T>& GetWrapper() {

    auto idx = std::type_index(typeid(T));

    if (wrappers.find(idx) == wrappers.end()) {
        wrappers[idx] = new EventWrapper<T>();
    }

    return *reinterpret_cast<EventWrapper<T>*>(wrappers[idx]);
}


template<typename T>
void ParserWrapper(const EventWrapper<T>& wrapper, LilETW::EventParser& parser) {

    const auto& field_map = FieldMap<T>();
    size_t idx = 0;

    nlohmann::json json_header;
    nlohmann::json json_data;

    ULONG id1 = parser.GetEventId();
    ULONG id2 = parser.GetProcessId();

    json_header["Metadata"] = {
        {"EventId", id1 },
        {"ProcessId", id2 }
    };

    json_header["Metadata"]["Exe"] = Symbols::ReturnProcessExecutable(id2);

    for (const auto& [name, field_parser] : field_map) {

        if (wrapper.IsMemberActive(idx)) {

            auto value_any = field_parser(parser, name);

            if (!value_any) {
                idx++; continue;
            }

            std::string key = Utils::ToUtf8(name);

            std::visit([&](auto&& value) {

                using T = std::decay_t<decltype(value)>;

                if constexpr (std::is_same_v<T, UCHAR> || std::is_same_v<T, ULONG> || std::is_same_v<T, ULONG64> || std::is_same_v<T, USHORT>) {
                    json_data[key] = value;
                }
                else if constexpr (std::is_same_v<T, PVOID>) {

                    uintptr_t addr = reinterpret_cast<uintptr_t>(value);

                    json_data[key] = nlohmann::json::array({ std::format("0x{:x}", addr) });

                    if ((addr & 0xFFF000000000) == 0x7FF000000000) {

                        std::string symbol = parser.ResolveSymbol(addr);

                        if (!symbol.empty()) {
                            json_data[key].push_back(symbol);
                        }
                    }
                }
                else if constexpr (std::is_same_v<T, FILETIME>) {

                    SYSTEMTIME st;
                    ::FileTimeToSystemTime(std::addressof(value), &st);

                    std::string formatted_time = (st.wHour < 10 ? "0" : "") + std::to_string(st.wHour) + ":" 
                        + (st.wMinute < 10 ? "0" : "") + std::to_string(st.wMinute) + ":" + (st.wSecond < 10 ? "0" : "") 
                        + std::to_string(st.wSecond) + "." + (st.wMilliseconds < 10 ? "00" : (st.wMilliseconds < 100 ? "0" : "")) 
                        + std::to_string(st.wMilliseconds);

                    json_data[key] = formatted_time;
                }
                else if constexpr (std::is_same_v<T, std::unique_ptr<UNICODE_STRING>>) {
                    if (value) { json_data[key] = value->Buffer; }
                }
                else if constexpr (std::is_same_v<T, std::unique_ptr<SID>>) {

                    if (value) {
                        LPSTR lSid;
                        if (::ConvertSidToStringSidA(value.get(), &lSid)) {
                            json_data[key] = lSid;
                            ::LocalFree(lSid);
                        }
                    }
                }
                else if constexpr (std::is_same_v<T, std::unique_ptr<char>>) {
                    if (value) { json_data[key] = std::string(value.get()); }
                }

            }, *value_any);
        }
        idx++;
    }

    auto frames = parser.GetStackFrames64();

    if (!frames.empty()) {

        auto& json_stack = json_header["Metadata"]["Stack"] = nlohmann::json::array();

        for (auto frame : frames) {

            if (frame >= 0xFFFF000000000000 || frame == 0x0) {
                continue;
            }

            auto symbol = parser.ResolveSymbol(frame);

            if (symbol.empty()) {
                //symbol = Symbols::ResolveProcessSymbol(id2, frame);
            }

            json_stack.push_back(symbol.empty() ? std::format("0x{:x}", frame) : symbol);
        }
    }

    json_header["Data"] = json_data;

    std::string json_str = json_header.dump(-1);
    Globals::Get().Vars().OutputHandle.write(json_str.c_str(), json_str.size());
}


template <typename T>
void Input(EventWrapper<T>& wrapper) {

    const auto& field_map = FieldMap<T>();
    std::vector<std::wstring> field_names;

    for (const auto& [name, index] : field_map) {
        field_names.push_back(name);
    }

    GetUserOptions(wrapper, field_names);
}   


const std::unordered_map<ULONGLONG, EventMetadata> Keywords {
    { 0x1,              { 6,    [] { Input(GetWrapper<EtwTi::ETWTI_ALLOCVM_LOCAL>());                   }, [](LilETW::EventParser& parser) { ParserWrapper(GetWrapper<EtwTi::ETWTI_ALLOCVM_LOCAL>(), parser); }}},
    { 0x2,              { 26,   [] { Input(GetWrapper<EtwTi::ETWTI_ALLOCVM_LOCAL_KERNEL>());            }, [](LilETW::EventParser& parser) { ParserWrapper(GetWrapper<EtwTi::ETWTI_ALLOCVM_LOCAL_KERNEL>(), parser); }}},
    { 0x4,              { 1,    [] { Input(GetWrapper<EtwTi::ETWTI_ALLOCVM_REMOTE>());                  }, [](LilETW::EventParser& parser) { ParserWrapper(GetWrapper<EtwTi::ETWTI_ALLOCVM_REMOTE>(), parser); }}},
    { 0x8,              { 21,   [] { Input(GetWrapper<EtwTi::ETWTI_ALLOCVM_REMOTE_KERNEL>());           }, [](LilETW::EventParser& parser) { ParserWrapper(GetWrapper<EtwTi::ETWTI_ALLOCVM_REMOTE_KERNEL>(), parser); }} },
    { 0x10,             { 7,    [] { Input(GetWrapper<EtwTi::ETWTI_PROTECTVM_LOCAL>());                 }, [](LilETW::EventParser& parser) { ParserWrapper(GetWrapper<EtwTi::ETWTI_PROTECTVM_LOCAL>(), parser); }}},
    { 0x20,             { 27,   [] { Input(GetWrapper<EtwTi::ETWTI_PROTECTVM_LOCAL_KERNEL>());          }, [](LilETW::EventParser& parser) { ParserWrapper(GetWrapper<EtwTi::ETWTI_PROTECTVM_LOCAL_KERNEL>(), parser); }}},
    { 0x40,             { 2,    [] { Input(GetWrapper<EtwTi::ETWTI_PROTECTVM_REMOTE>());                }, [](LilETW::EventParser& parser) { ParserWrapper(GetWrapper<EtwTi::ETWTI_PROTECTVM_REMOTE>(), parser); }}},
    { 0x80,             { 22,   [] { Input(GetWrapper<EtwTi::ETWTI_PROTECTVM_REMOTE_KERNEL>());         }, [](LilETW::EventParser& parser) { ParserWrapper(GetWrapper<EtwTi::ETWTI_PROTECTVM_REMOTE_KERNEL>(), parser); }}},
    { 0x100,            { 8,    [] { Input(GetWrapper<EtwTi::ETWTI_MAPVIEW_LOCAL>());                   }, [](LilETW::EventParser& parser) { ParserWrapper(GetWrapper<EtwTi::ETWTI_MAPVIEW_LOCAL>(), parser); }}},
    { 0x200,            { 28,   [] { Input(GetWrapper<EtwTi::ETWTI_MAPVIEW_LOCAL_KERNEL>());            }, [](LilETW::EventParser& parser) { ParserWrapper(GetWrapper<EtwTi::ETWTI_MAPVIEW_LOCAL_KERNEL>(), parser); }}},
    { 0x400,            { 3,    [] { Input(GetWrapper<EtwTi::ETWTI_MAPVIEW_REMOTE>());                  }, [](LilETW::EventParser& parser) { ParserWrapper(GetWrapper<EtwTi::ETWTI_MAPVIEW_REMOTE>(), parser); }}},
    { 0x800,            { 23,   [] { Input(GetWrapper<EtwTi::ETWTI_MAPVIEW_REMOTE_KERNEL>());           }, [](LilETW::EventParser& parser) { ParserWrapper(GetWrapper<EtwTi::ETWTI_MAPVIEW_REMOTE_KERNEL>(), parser); }}},
    { 0x1000,           { 4,    [] { Input(GetWrapper<EtwTi::ETWTI_QUEUEUSERAPC_REMOTE>());             }, [](LilETW::EventParser& parser) { ParserWrapper(GetWrapper<EtwTi::ETWTI_QUEUEUSERAPC_REMOTE>(), parser); }}},
    { 0x2000,           { 24,   [] { Input(GetWrapper<EtwTi::ETWTI_QUEUEUSERAPC_REMOTE_KERNEL>());      }, [](LilETW::EventParser& parser) { ParserWrapper(GetWrapper<EtwTi::ETWTI_QUEUEUSERAPC_REMOTE_KERNEL>(), parser);}}},
    { 0x4000,           { 5,    [] { Input(GetWrapper<EtwTi::ETWTI_SETTHREADCONTEXT_REMOTE>());         }, [](LilETW::EventParser& parser) { ParserWrapper(GetWrapper<EtwTi::ETWTI_SETTHREADCONTEXT_REMOTE>(), parser); }}},
    { 0x8000,           { 25,   [] { Input(GetWrapper<EtwTi::ETWTI_SETTHREADCONTEXT_REMOTE_KERNEL>());  }, [](LilETW::EventParser& parser) { ParserWrapper(GetWrapper<EtwTi::ETWTI_SETTHREADCONTEXT_REMOTE_KERNEL>(), parser);}}},
    { 0x10000,          { 11,   [] { Input(GetWrapper<EtwTi::ETWTI_READVM_LOCAL>());                    }, [](LilETW::EventParser& parser) { ParserWrapper(GetWrapper<EtwTi::ETWTI_READVM_LOCAL>(), parser); }}},
    { 0x20000,          { 13,   [] { Input(GetWrapper<EtwTi::ETWTI_READVM_REMOTE>());                   }, [](LilETW::EventParser& parser) { ParserWrapper(GetWrapper<EtwTi::ETWTI_READVM_REMOTE>(), parser); }}},
    { 0x40000,          { 12,   [] { Input(GetWrapper<EtwTi::ETWTI_WRITEVM_LOCAL>());                   }, [](LilETW::EventParser& parser) { ParserWrapper(GetWrapper<EtwTi::ETWTI_WRITEVM_LOCAL>(), parser); }}},
    { 0x80000,          { 14,   [] { Input(GetWrapper<EtwTi::ETWTI_WRITEVM_REMOTE>());                  }, [](LilETW::EventParser& parser) { ParserWrapper(GetWrapper<EtwTi::ETWTI_WRITEVM_REMOTE>(), parser); }}},
    { 0x100000,         { 15,   [] { Input(GetWrapper<EtwTi::ETWTI_SUSPEND_THREAD>());                  }, [](LilETW::EventParser& parser) { ParserWrapper(GetWrapper<EtwTi::ETWTI_SUSPEND_THREAD>(), parser); }}},
    { 0x200000,         { 16,   [] { Input(GetWrapper<EtwTi::ETWTI_RESUME_THREAD>());                   }, [](LilETW::EventParser& parser) { ParserWrapper(GetWrapper<EtwTi::ETWTI_RESUME_THREAD>(), parser); }}},
    { 0x400000,         { 17,   [] { Input(GetWrapper<EtwTi::ETWTI_SUSPEND_PROCESS>());                 }, [](LilETW::EventParser& parser) { ParserWrapper(GetWrapper<EtwTi::ETWTI_SUSPEND_PROCESS>(), parser); }}},
    { 0x800000,         { 18,   [] { Input(GetWrapper<EtwTi::ETWTI_RESUME_PROCESS>());                  }, [](LilETW::EventParser& parser) { ParserWrapper(GetWrapper<EtwTi::ETWTI_RESUME_PROCESS>(), parser); }}},
    { 0x1000000,        { 19,   [] { Input(GetWrapper<EtwTi::ETWTI_FREEZE_PROCESS>());                  }, [](LilETW::EventParser& parser) { ParserWrapper(GetWrapper<EtwTi::ETWTI_FREEZE_PROCESS>(), parser); }}},
    { 0x2000000,        { 20,   [] { Input(GetWrapper<EtwTi::ETWTI_THAW_PROCESS>());                    }, [](LilETW::EventParser& parser) { ParserWrapper(GetWrapper<EtwTi::ETWTI_THAW_PROCESS>(), parser); }}},
    { 0x4000000000,     { 33,   [] { Input(GetWrapper<EtwTi::ETWTI_IMPERSONATION_UP>());                }, [](LilETW::EventParser& parser) { ParserWrapper(GetWrapper<EtwTi::ETWTI_IMPERSONATION_UP>(), parser); }}},
    { 0x8000000000,     { 34,   [] { Input(GetWrapper<EtwTi::ETWTI_IMPERSONATION_REVERT>());            }, [](LilETW::EventParser& parser) { ParserWrapper(GetWrapper<EtwTi::ETWTI_IMPERSONATION_REVERT>(), parser); }}},
    { 0x10000000000,    { 35,   [] { Input(GetWrapper<EtwTi::ETWTI_SYSCALL_EVENT>());                   }, [](LilETW::EventParser& parser) { ParserWrapper(GetWrapper<EtwTi::ETWTI_SYSCALL_EVENT>(), parser); }}},
    { 0x40000000000,    { 36,   [] { Input(GetWrapper<EtwTi::ETWTI_IMPERSONATION_DOWN>());              }, [](LilETW::EventParser& parser) { ParserWrapper(GetWrapper<EtwTi::ETWTI_IMPERSONATION_DOWN>(), parser); }}}
};


void ParseKeywords(ULONGLONG arg) {

    for (const auto& [keyword, metadata] : Keywords) {

        if ((arg & keyword) == keyword) {

            metadata.InputHandler();
        }
    }

    CONSOLE_SCREEN_BUFFER_INFO csbi = { 0 };

    HANDLE hConsole = ::GetStdHandle(STD_OUTPUT_HANDLE);

    if (::GetConsoleScreenBufferInfo(hConsole, &csbi)) {
        DWORD written;
        DWORD console_size = csbi.dwSize.X * csbi.dwSize.Y;
        ::FillConsoleOutputCharacter(hConsole, ' ', console_size, { 0, 0 }, &written);
        ::FillConsoleOutputAttribute(hConsole, csbi.wAttributes, console_size, { 0, 0 }, &written);
        ::SetConsoleCursorPosition(hConsole, { 0, 0 });
    }
}

#endif // !MENTAL_HPP
