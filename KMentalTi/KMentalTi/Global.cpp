#include "Global.hpp"

Globals* Globals::s_Globals = nullptr;
Globals* g_Global = nullptr;

Globals::Globals() {

	s_Globals = this;
	g_Global = this;

	m_Data.MutexProcInfo.Init();
	m_Data.b24H2 = false;
	m_Data.cOpen = 0;
	m_Data.ulFlags = 0;
}

Globals& Globals::Get() {
	return *s_Globals;
}

Globals::~Globals() {
	
	//Just for the future idk
}

GlobalData& Globals::Vars() {
	return m_Data;
}
