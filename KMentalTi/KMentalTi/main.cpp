#include <ntifs.h>

#define KMENTALTI	    0x8000
#define MENTALTI_OPEN   CTL_CODE(KMENTALTI, 0x800, METHOD_NEITHER, FILE_ANY_ACCESS)

typedef struct _PS_PROTECTION {
	UCHAR	Level;
	UCHAR	Type : 3;
	UCHAR	Audit : 1;
	UCHAR	Signer : 4;
}PS_PROTECTION, * PPS_PROTECTION;

UNICODE_STRING device_name = RTL_CONSTANT_STRING(L"\\Device\\KMentalTi");
UNICODE_STRING symlink = RTL_CONSTANT_STRING(L"\\??\\KMentalTi");

volatile char isOpened = false;

VOID UnLoader(DRIVER_OBJECT* DriverObject);
NTSTATUS Open(DEVICE_OBJECT* DeviceObject, IRP* Irp);
NTSTATUS Close(DEVICE_OBJECT* DeviceObject, IRP* Irp);
NTSTATUS DeviceControl(DEVICE_OBJECT* DeviceObject, IRP* Irp);


extern "C" NTSTATUS
DriverEntry(DRIVER_OBJECT* DriverObject, UNICODE_STRING* RegistryPath) {

	UNREFERENCED_PARAMETER(RegistryPath);

	NTSTATUS			STATUS = STATUS_SUCCESS;
	DEVICE_OBJECT*		device_object;
	bool				symlinkyes = false;

	do {

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

NTSTATUS DeviceControl(DEVICE_OBJECT* DeviceObject, IRP* Irp) {

	UNREFERENCED_PARAMETER(DeviceObject);

	NTSTATUS STATUS = STATUS_SUCCESS;

	PEPROCESS pProc = IoGetRequestorProcess(Irp);

	if (!pProc) {
		STATUS = STATUS_NOT_FOUND;
		goto _End;
	}

	PS_PROTECTION* pPsProtect = (PS_PROTECTION*)((ULONG_PTR)pProc + 0x87a);

	pPsProtect->Level = 0x31;
	pPsProtect->Type = 0x1;
	pPsProtect->Audit = 0;
	pPsProtect->Signer = 0x3;

_End:

	Irp->IoStatus.Status = STATUS;
	Irp->IoStatus.Information = 0;

	IoCompleteRequest(Irp, IO_NO_INCREMENT);

	return Irp->IoStatus.Status;
}

VOID UnLoader(DRIVER_OBJECT* DriverObject) {

	IoDeleteSymbolicLink(&symlink);
	IoDeleteDevice(DriverObject->DeviceObject);
	DbgPrint("[+] Driver Unloaded\n");
}

NTSTATUS Close(DEVICE_OBJECT* DeviceObject, IRP* Irp) {

	UNREFERENCED_PARAMETER(DeviceObject);

	InterlockedExchange8(&isOpened, 0);

	Irp->IoStatus.Status = STATUS_SUCCESS;
	Irp->IoStatus.Information = 0;

	IoCompleteRequest(Irp, IO_NO_INCREMENT);

	return Irp->IoStatus.Status;
}

NTSTATUS Open(DEVICE_OBJECT* DeviceObject, IRP* Irp) {

	UNREFERENCED_PARAMETER(DeviceObject);

	if (InterlockedExchange8(&isOpened, 1) == 1) {
		Irp->IoStatus.Status = STATUS_ACCESS_DENIED;
	}
	else {
		Irp->IoStatus.Status = STATUS_SUCCESS;
	}

	Irp->IoStatus.Information = 0;
	IoCompleteRequest(Irp, IO_NO_INCREMENT);

	return Irp->IoStatus.Status;
}