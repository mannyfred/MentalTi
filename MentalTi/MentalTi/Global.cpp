#include "Global.hpp"
#include <DbgHelp.h>


Globals::Globals() {

	m_Data = { 0 };

	m_Data.Keywords		= 0;
	m_Data.TargetProc	= 0;
	m_Data.StackTrace	= false;
	m_Data.ModifyLoggingAll = false;
}

Globals& Globals::Get() {
	static std::unique_ptr<Globals> g_Global = std::make_unique<Globals>();
	return *g_Global;
}

Globals::~Globals() {

	if (m_Data.LoggerHandle != 0 && m_Data.LoggerInfo != nullptr) {

		ULONG status = 0;

		if ((status = ::ControlTraceW(0, m_Data.TraceName, m_Data.LoggerInfo, EVENT_TRACE_CONTROL_STOP)) == ERROR_SUCCESS) {
			std::printf("[i] Tracing Session Stopped \n");
		}
	}

	if (m_Data.OutputHandle) {
		m_Data.OutputHandle.close();
	}

	if (m_Data.DriverHandle) {
		::CloseHandle(m_Data.DriverHandle);
	}

	if (m_Data.LoggerHandle) {
		::CloseTrace(m_Data.LoggerHandle);
	}

	if (m_Data.LoggerInfo) {
		::HeapFree(::GetProcessHeap(), 0, m_Data.LoggerInfo);
	}

	::SymCleanup((HANDLE)-1);
	std::printf("Bye");
}

GlobalData& Globals::Vars() {
	return m_Data;
}