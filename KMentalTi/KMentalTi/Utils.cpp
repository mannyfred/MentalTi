#include "Global.hpp"
#include "Avl.hpp"
#include "defs.hpp"

NTSTATUS ModifyLogging(bool bRevert, ULONG pid, ULONG flags) {

	ULONG		ulRet		= 0;
	NTSTATUS	STATUS		= STATUS_SUCCESS;
	PEPROCESS	Process		= nullptr;
	HANDLE		hProcess	= nullptr;
	PROCESS_LOGGING_INFORMATION pli = { 0 };

	STATUS = PsLookupProcessByProcessId((HANDLE)pid, &Process);
	if (!NT_SUCCESS(STATUS)) {
		DbgPrint("[!] PsLookupProcessByProcessId: 0x%0.8X\n", STATUS);
		return STATUS;
	}

	STATUS = ObOpenObjectByPointer(Process, OBJ_KERNEL_HANDLE, nullptr, GENERIC_READ, nullptr, KernelMode, &hProcess);
	if (!NT_SUCCESS(STATUS)) {
		DbgPrint("[!] ObOpenObjectByPointer: 0x%0.8X\n", STATUS);
		ObDereferenceObject(Process);
		return STATUS;
	}

	if (!bRevert) {

		STATUS = ZwQueryInformationProcess(hProcess, 96, &pli, sizeof(pli), &ulRet);
		if (!NT_SUCCESS(STATUS)) {
			DbgPrint("[!] ZwQueryInformationProcess: 0x%0.8X\n", STATUS);
			ZwClose(hProcess);
			ObDereferenceObject(Process);
			return STATUS;
		}

		AvlInsert(pid, pli.Flags);
		pli.Flags |= g_Global->Vars().ulFlags;
	}
	else {
		pli.Flags = flags;
	}

	STATUS = ZwSetInformationProcess(hProcess, 96, &pli, sizeof(pli));
	if (!NT_SUCCESS(STATUS)) {
		DbgPrint("[!] ZwSetInformationProcess: 0x%0.8X\n", STATUS);
	}

	ZwClose(hProcess);
	ObDereferenceObject(Process);

	return STATUS;
}

NTSTATUS GetProcs() {

	NTSTATUS	STATUS = STATUS_SUCCESS;
	ULONG		ulRet = 0;
	PVOID		pFree = nullptr;

	ZwQuerySystemInformation(SystemProcessInformation, nullptr, 0, &ulRet);
	ulRet += 1 << 12;

	SYSTEM_PROCESSES* pSysProcBuffer = (SYSTEM_PROCESSES*)ExAllocatePool2(POOL_FLAG_PAGED, ulRet, MY_TAG);

	if (!pSysProcBuffer) {
		goto _End;
	}

	STATUS = ZwQuerySystemInformation(SystemProcessInformation, pSysProcBuffer, ulRet, nullptr);

	if (!NT_SUCCESS(STATUS)) {
		goto _End;
	}

	pFree = pSysProcBuffer;

	pSysProcBuffer = (SYSTEM_PROCESSES*)((unsigned char*)pSysProcBuffer + pSysProcBuffer->NextEntryDelta);

	for (;;) {

		STATUS = ModifyLogging(false, (ULONG)pSysProcBuffer->ProcessId, 0);
		if (!NT_SUCCESS(STATUS)) {
			DbgPrint("[!] ModifyLogging: 0x%0.8X\n", STATUS);
			goto _End;
		}

		if (!pSysProcBuffer->NextEntryDelta)
			break;

		pSysProcBuffer = (SYSTEM_PROCESSES*)((unsigned char*)pSysProcBuffer + pSysProcBuffer->NextEntryDelta);
	}

_End:

	if (pFree) {
		ExFreePoolWithTag(pFree, MY_TAG);
		pFree = nullptr;
	}

	return STATUS;
}