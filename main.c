#include "drives.h"

static const WCHAR service_name[] = L"DiskPoll";
static SERVICE_STATUS service_status;
static SERVICE_STATUS_HANDLE service_handle;
static HANDLE service_stop;
static DWORD poll_internal = 500;

static void WINAPI service_handler(DWORD control)
{
	if (control == SERVICE_CONTROL_STOP) {
		SetEvent(service_stop);
		service_status.dwCurrentState = SERVICE_STOP_PENDING;
		SetServiceStatus(service_handle, &service_status);
	}
}

static DWORD service_set_args(unsigned int argc, WCHAR **argv)
{
	DWORD rc;
	for (; argc-- > 0; argv++) {
		WCHAR *a = *argv;
		if ((a[0] >= L'A' && a[0] <= L'Z' || a[0] >= L'a' && a[0] <= L'z')
			&& (a[1] == L':' && a[2] == L'\0'))
		{
			if (rc = drives_add_by_letter(a[0]))
				return rc;
		} else {
			WCHAR *pend;
			DWORD i = wcstoul(a, &pend, 0);
			if (i > 0 && *pend == L'\0') {
				poll_internal = i;
			}
		}
	}

	if (!drives_has_any()) {
		if (rc = drives_add_by_letter(L'C'))
			return rc;
	}

	return 0;
}

static void WINAPI service_main(DWORD argc, WCHAR **argv)
{
	service_handle = RegisterServiceCtrlHandler(service_name, service_handler);
	if (service_handle == 0)
		return;

	service_status.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
	service_status.dwControlsAccepted = SERVICE_ACCEPT_STOP;
	service_status.dwWaitHint =
	service_status.dwCheckPoint =
	service_status.dwServiceSpecificExitCode =
	service_status.dwWin32ExitCode = 0;

	DWORD rc;
	do {
		service_stop = CreateEvent(NULL, FALSE, FALSE, NULL);
		if (service_stop == NULL) {
			rc = GetLastError();
			break;
		}

		service_status.dwCurrentState = SERVICE_RUNNING;
		SetServiceStatus(service_handle, &service_status);

		do {
			drives_poll();
			rc = WaitForSingleObject(service_stop, poll_internal);
		} while (rc == WAIT_TIMEOUT);

		rc = rc == WAIT_FAILED ? GetLastError() : 0;
		CloseHandle(service_stop);
	} while (0);

	service_status.dwWin32ExitCode = rc;
	service_status.dwCurrentState = SERVICE_STOPPED;
	SetServiceStatus(service_handle, &service_status);
}

int __cdecl wmain(unsigned int argc, WCHAR **argv)
{
	DWORD rc;
	do {
		if (rc = drives_init())
			break;

		if (rc = service_set_args(argc, argv))
			break;

		static const SERVICE_TABLE_ENTRY service_table[] = {
			{ (LPWSTR)service_name, service_main },
			{ NULL, NULL }
		};

		rc = StartServiceCtrlDispatcher(service_table) == 0 ? GetLastError() : 0;
	} while (0);

	drives_free();

	return rc;
}

// disable VS2015 telemetry
void __cdecl __vcrt_initialize_telemetry_provider() {}
void __cdecl __telemetry_main_invoke_trigger() {}
void __cdecl __telemetry_main_return_trigger() {}
void __cdecl __vcrt_uninitialize_telemetry_provider() {}
