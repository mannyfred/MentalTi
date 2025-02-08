#ifndef GLOBAL_HPP
#define GLOBAL_HPP

#include <Windows.h>
#include <evntrace.h>
#include <fstream>
#include <map>

struct GlobalData {

	EVENT_TRACE_PROPERTIES* LoggerInfo;
	TRACEHANDLE             LoggerHandle;
	LPWSTR					TraceName;
	std::ofstream			OutputHandle;
	ULONGLONG				Keywords;
	ULONG					TargetProc;
	HANDLE					DriverHandle;
	HANDLE					TargetHandle;
	ULONG					TargetFlags;
	bool					ModifyLoggingAll;
};

class Globals {
public:
	Globals();
	static Globals& Get();
	Globals(Globals const&) = delete;
	Globals& operator=(Globals const&) = delete;
	~Globals();

	GlobalData& Vars();

private:
	static Globals* s_Globals;
	GlobalData m_Data;
};
extern Globals* g_Global;

#endif // !GLOBAL_HPP