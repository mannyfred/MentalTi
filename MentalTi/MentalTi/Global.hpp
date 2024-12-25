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
	ULONG					Keywords;
	ULONG					TargetProc;
	HANDLE					DriverHandle;
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