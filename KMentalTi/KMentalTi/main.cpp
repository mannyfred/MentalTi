#include <ntifs.h>
#include "defs.hpp"
#include "Avl.hpp"
#include "Memory.hpp"

UNICODE_STRING device_name = RTL_CONSTANT_STRING(L"\\Device\\KMentalTi");
UNICODE_STRING symlink = RTL_CONSTANT_STRING(L"\\??\\KMentalTi");

NTSTATUS GetProcs();
VOID UnLoader(DRIVER_OBJECT* DriverObject);
NTSTATUS Open(DEVICE_OBJECT* DeviceObject, IRP* Irp);
NTSTATUS Close(DEVICE_OBJECT* DeviceObject, IRP* Irp);
NTSTATUS DeviceControl(DEVICE_OBJECT* DeviceObject, IRP* Irp);
VOID OnProcessNotify(PEPROCESS Process, HANDLE ProcessId, PS_CREATE_NOTIFY_INFO* CreateInfo);
NTSTATUS ModifyLogging(bool bRevert, ULONG pid, ULONG flags);


extern "C" NTSTATUS DriverEntry(DRIVER_OBJECT* DriverObject, UNICODE_STRING* RegistryPath) {

	UNREFERENCED_PARAMETER(RegistryPath);

	NTSTATUS			STATUS			= STATUS_SUCCESS;
	DEVICE_OBJECT*		device_object	= nullptr;
	bool				symlinkyes		= false;

	if (SharedUserData->NtMajorVersion != 10) {
		return STATUS_INCOMPATIBLE_DRIVER_BLOCKED;
	}
		
	do {

		g_Global = new (PoolType::NonPaged) Globals;
		if (!g_Global) {
			return STATUS_INSUFFICIENT_RESOURCES;
		}

		if (SharedUserData->NtBuildNumber >= 26100) {
			g_Global->Vars().b24H2 = true;
		}

		STATUS = IoCreateDevice(DriverObject, 0, &device_name, FILE_DEVICE_UNKNOWN, 0, false, &device_object);
		if (!NT_SUCCESS(STATUS)) {
			DbgPrint("[!] IoCreateDevice failed\n");
			break;
		}

		STATUS = IoCreateSymbolicLink(&symlink, &device_name);
		if (!NT_SUCCESS(STATUS)) {
			DbgPrint("[!] IoCreateSymbolicLink failed\n");
			break;
		}
		symlinkyes = true;

	} while (false);

	if (!NT_SUCCESS(STATUS)) {

		delete g_Global;

		if (symlinkyes)
			IoDeleteSymbolicLink(&symlink);

		if (device_object)
			IoDeleteDevice(device_object);

		return STATUS;
	}

	DriverObject->DriverUnload = UnLoader;
	DriverObject->MajorFunction[IRP_MJ_CLOSE] = Close;
	DriverObject->MajorFunction[IRP_MJ_CREATE] = Open;
	DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = DeviceControl;

	return STATUS;
}

VOID OnProcessNotify(PEPROCESS Process, HANDLE ProcessId, PS_CREATE_NOTIFY_INFO* CreateInfo) {
	
	UNREFERENCED_PARAMETER(Process);
	NTSTATUS STATUS = STATUS_SUCCESS;

	if (CreateInfo) {

		STATUS = ModifyLogging(false, HandleToULong(ProcessId), 0);
		if (!NT_SUCCESS(STATUS)) {
			DbgPrint("[!] ModifyLogging: 0x%0.8X\n", STATUS);
		}
	}
	else {
		AvlDelete(HandleToULong(ProcessId));
	}

	return;
}

NTSTATUS DeviceControl(DEVICE_OBJECT* DeviceObject, IRP* Irp) {

	UNREFERENCED_PARAMETER(DeviceObject);

	NTSTATUS			STATUS		= STATUS_SUCCESS;
	PS_PROTECTION*		pPsProtect	= nullptr;

	PEPROCESS			Process	= IoGetRequestorProcess(Irp);
	IO_STACK_LOCATION*	pStack	= IoGetCurrentIrpStackLocation(Irp);
	ULONG				ioctl	= pStack->Parameters.DeviceIoControl.IoControlCode;

	if (ioctl == MENTALTI_OPEN) {

		if (!Process) {
			STATUS = STATUS_NOT_FOUND;
			goto _End;
		}

		if (g_Global->Vars().b24H2) {
			pPsProtect = (PS_PROTECTION*)((ULONG_PTR)Process + 0x5fa);
		}
		else {
			pPsProtect = (PS_PROTECTION*)((ULONG_PTR)Process + 0x87a);
		}

		pPsProtect->Level = 0x31;
		pPsProtect->Type = 0x1;
		pPsProtect->Audit = 0;
		pPsProtect->Signer = 0x3;
	}
	else if (ioctl == MENTALTI_ALL) {

		g_Global->Vars().ulFlags = (ULONG)pStack->Parameters.DeviceIoControl.InputBufferLength;

		STATUS = GetProcs();
		if (!NT_SUCCESS(STATUS)) {
			goto _End;
		}

		STATUS = PsSetCreateProcessNotifyRoutineEx(OnProcessNotify, false);
		if (!NT_SUCCESS(STATUS)) {
			goto _End;
		}
	}
	else {
		STATUS = STATUS_INVALID_PARAMETER;
	}

_End:

	Irp->IoStatus.Status = STATUS;
	Irp->IoStatus.Information = 0;

	IoCompleteRequest(Irp, IO_NO_INCREMENT);

	return Irp->IoStatus.Status;
}

VOID UnLoader(DRIVER_OBJECT* DriverObject) {

	//PVOID p = nullptr;

	//PsSetCreateProcessNotifyRoutineEx(OnProcessNotify, true);

	//auto t = &g_Global->Vars().AvlProcInfo;

	//for (p = RtlEnumerateGenericTableAvl(t, 1);
	//	p != nullptr;
	//	p = RtlEnumerateGenericTableAvl(t, 0)) {
	//	//PROCS_FLAGS* entry = (PROCS_FLAGS*)p;
	//	//DbgPrint("[exit] Proc: %lu - Flags: %lu\n", entry->Pid, entry->Flags);
	//	RtlDeleteElementGenericTableAvl(t, p);
	//}

	delete g_Global;
	IoDeleteSymbolicLink(&symlink);
	IoDeleteDevice(DriverObject->DeviceObject);
	DbgPrint("[+] Driver Unloaded\n");
}

NTSTATUS Close(DEVICE_OBJECT* DeviceObject, IRP* Irp) {

	UNREFERENCED_PARAMETER(DeviceObject);

	InterlockedExchange8(&g_Global->Vars().cOpen, 0);

	PsSetCreateProcessNotifyRoutineEx(OnProcessNotify, true);

	AvlNuke();

	Irp->IoStatus.Status = STATUS_SUCCESS;
	Irp->IoStatus.Information = 0;

	IoCompleteRequest(Irp, IO_NO_INCREMENT);

	return Irp->IoStatus.Status;
}

NTSTATUS Open(DEVICE_OBJECT* DeviceObject, IRP* Irp) {

	UNREFERENCED_PARAMETER(DeviceObject);

	if (InterlockedExchange8(&g_Global->Vars().cOpen, 1) == 1) {
		Irp->IoStatus.Status = STATUS_ACCESS_DENIED;
	}
	else {
		Irp->IoStatus.Status = STATUS_SUCCESS;
	}

	Irp->IoStatus.Information = 0;
	IoCompleteRequest(Irp, IO_NO_INCREMENT);

	return Irp->IoStatus.Status;
}