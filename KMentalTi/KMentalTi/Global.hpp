#pragma once
#include <ntifs.h>


template<typename T>
struct AutoLock {
	AutoLock(T& lock) : _lock(lock) {
		_lock.Lock();
	}

	~AutoLock() {
		_lock.Unlock();
	}

private:
	T& _lock;
};

class FastMutex {
public:
	void FastMutex::Init() {
		ExInitializeFastMutex(&_mutex);
	}

	void FastMutex::Lock() {
		ExAcquireFastMutex(&_mutex);
	}

	void FastMutex::Unlock() {
		ExReleaseFastMutex(&_mutex);
	}

private:
	FAST_MUTEX _mutex;
};

struct GlobalData {

	bool		b24H2;
	volatile	ULONG ulFlags;
	volatile	char	cOpen;

	RTL_AVL_TABLE AvlProcInfo;
	FastMutex	MutexProcInfo;
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
