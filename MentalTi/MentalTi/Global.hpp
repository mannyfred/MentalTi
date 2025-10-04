#ifndef GLOBAL_HPP
#define GLOBAL_HPP

#include <Windows.h>
#include <evntrace.h>
#include <fstream>
#include <map>
#include <vector>

struct GlobalData {

	EVENT_TRACE_PROPERTIES* LoggerInfo;
	TRACEHANDLE             LoggerHandle;
	LPWSTR					TraceName;
	std::ofstream			OutputHandle;
	ULONGLONG				Keywords;
	ULONG					TargetProc;
	HANDLE					DriverHandle;
	bool					ModifyLoggingAll;
	bool					StackTrace;
	std::vector<USHORT>		StackTracedEvents;
};

class Globals {
public:
	Globals();
	~Globals();

	Globals(Globals const&) = delete;
	Globals& operator=(Globals const&) = delete;

	static Globals& Get();
	GlobalData& Vars();

private:
	GlobalData m_Data;
};

#endif // !GLOBAL_HPP