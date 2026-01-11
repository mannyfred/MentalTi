#include "Global.hpp"
#include <DbgHelp.h>


Globals::Globals() {
	m_Data.DriverHandle			= nullptr;
	m_Data.Keywords				= 0;
	m_Data.TargetProc			= 0;
	m_Data.StackTracedEvents	= {};
	m_Data.ModifyLoggingAll		= false;
}

Globals& Globals::Get() {
	static std::unique_ptr<Globals> g_Global = std::make_unique<Globals>();
	return *g_Global;
}

Globals::~Globals() {

	if (m_Data.OutputHandle) {
		m_Data.OutputHandle.close();
	}

	if (m_Data.DriverHandle) {
		::CloseHandle(m_Data.DriverHandle);
	}

	std::printf("Bye");
}

GlobalData& Globals::Vars() {
	return m_Data;
}