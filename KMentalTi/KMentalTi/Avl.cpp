#include "Avl.hpp"
#include "defs.hpp"

NTSTATUS ModifyLogging(bool bRevert, ULONG pid, ULONG flags);

RTL_GENERIC_COMPARE_RESULTS AvlCompare(RTL_AVL_TABLE* Table, PVOID First, PVOID Second) {

	UNREFERENCED_PARAMETER(Table);

	PROCS_FLAGS* entry1 = (PROCS_FLAGS*)First;
	PROCS_FLAGS* entry2 = (PROCS_FLAGS*)Second;

	if (entry1->Pid == entry2->Pid)
		return GenericEqual;

	return entry1->Pid > entry2->Pid ? GenericGreaterThan : GenericLessThan;
}

VOID AvlFree(RTL_AVL_TABLE* Table, PVOID Buffer) {

	UNREFERENCED_PARAMETER(Table);
	return ExFreePoolWithTag(Buffer, MY_TAG);
}

PVOID AvlAlloc(RTL_AVL_TABLE* Table, CLONG Bytes) {

	UNREFERENCED_PARAMETER(Table);
	return ExAllocatePool2(POOL_FLAG_PAGED, Bytes, MY_TAG);
}

VOID AvlInsert(ULONG Key, ULONG Value) {

	PROCS_FLAGS info = { 0 };
	info.Pid = Key;
	info.Flags = Value;

	AutoLock<FastMutex> locker(g_Global->Vars().MutexProcInfo);
	RtlInsertElementGenericTableAvl(&g_Global->Vars().AvlProcInfo, &info, sizeof(info), nullptr);

	return;
}

bool AvlDelete(ULONG Key) {

	PROCS_FLAGS info = { 0 };
	info.Pid = Key;

	AutoLock<FastMutex> locker(g_Global->Vars().MutexProcInfo);
	return RtlDeleteElementGenericTableAvl(&g_Global->Vars().AvlProcInfo, &info);
}

void AvlNuke() {

	PVOID p = nullptr;
	NTSTATUS STATUS = 0x00;

	AutoLock<FastMutex> locker(g_Global->Vars().MutexProcInfo);
	auto t = &g_Global->Vars().AvlProcInfo;

	for (p = RtlEnumerateGenericTableAvl(t, 1);
		p != nullptr;
		p = RtlEnumerateGenericTableAvl(t, 0)) {

		PROCS_FLAGS* info = (PROCS_FLAGS*)p;
		STATUS = ModifyLogging(true, info->Pid, info->Flags);
		if (!NT_SUCCESS(STATUS)) {
			DbgPrint("[!] Error reverting flags for proc: %lu - 0x%0.8X\n", info->Pid, STATUS);
		}

		RtlDeleteElementGenericTableAvl(t, p);
	}
}