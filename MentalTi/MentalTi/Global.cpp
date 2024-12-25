#include "Global.hpp"

Globals* Globals::s_Globals = nullptr;
Globals* g_Global = nullptr;

Globals::Globals() {


	s_Globals = this;
	g_Global = this;

	m_Data = { 0 };

}

Globals& Globals::Get() {
	return *s_Globals;
}

Globals::~Globals() {

	if (m_Data.OutputHandle)
		m_Data.OutputHandle.close();

	if (m_Data.LoggerHandle != 0 && m_Data.LoggerInfo != nullptr) {
		ULONG status = 0;
		if ((status = ::StopTraceW(m_Data.LoggerHandle, m_Data.TraceName, m_Data.LoggerInfo)) != ERROR_SUCCESS) {
			std::wprintf(L"[!] StopTraceW Failed: %d \n", status);
		}
		else {
			std::printf("[i] Tracing Session Stopped \n");
		}
	}

	if (m_Data.DriverHandle)
		::CloseHandle(m_Data.DriverHandle);

	if (m_Data.LoggerHandle) 
		::CloseTrace(m_Data.LoggerHandle);

	if (m_Data.LoggerInfo) {
		::HeapFree(GetProcessHeap(), 0, m_Data.LoggerInfo);
	}
}

GlobalData& Globals::Vars() {
	return m_Data;
}