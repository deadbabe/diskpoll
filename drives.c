#include "drives.h"

struct drives drives = {
	NULL, NULL, 0, NULL
};

BOOL drives_add_by_letter(WCHAR letter) {
	DWORD tmp;
	WCHAR path[32];

	_snwprintf(path, sizeof(path), L"\\\\.\\%c:", letter);
	HANDLE handle = CreateFile(path, 0, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);

	if (handle == INVALID_HANDLE_VALUE)
		return GetLastError();

	VOLUME_DISK_EXTENTS exts;
	BOOL result = DeviceIoControl(handle, IOCTL_VOLUME_GET_VOLUME_DISK_EXTENTS, NULL, 0, &exts, sizeof(exts), &tmp, NULL);
	CloseHandle(handle);

	if (result == 0)
		return GetLastError();

	PDISK_EXTENT ext = exts.Extents;
	DWORD cnt = exts.NumberOfDiskExtents;
	for (; cnt--; ext++)
	{
		if (drives_has_number(ext->DiskNumber))
			continue;

		_snwprintf(path, sizeof(path), L"\\\\.\\PhysicalDrive%d", ext->DiskNumber);
		HANDLE handle = CreateFile(
			path,
			GENERIC_READ,
			FILE_SHARE_READ | FILE_SHARE_WRITE,
			NULL,
			OPEN_EXISTING,
			FILE_FLAG_NO_BUFFERING | FILE_FLAG_OVERLAPPED,
			NULL
		);
		if (handle == INVALID_HANDLE_VALUE)
			return GetLastError();

		DISK_GEOMETRY_EX geom;
		BOOL result = DeviceIoControl(handle, IOCTL_DISK_GET_DRIVE_GEOMETRY_EX, NULL, 0, &geom, sizeof(geom), &tmp, NULL);
		if (result == 0) {
			CloseHandle(handle);
			return GetLastError();
		}

		struct drive *drive = malloc(sizeof(struct drive));
		if (drive == NULL) {
			CloseHandle(handle);
			return errno;
		}

		drive->next = NULL;
		drive->handle = handle;
		drive->drive_size = geom.DiskSize.QuadPart;
		drive->sector_size = geom.Geometry.BytesPerSector;
		memset(&drive->overlapped, 0, sizeof(drive->overlapped));

		if (drives.last == NULL) {
			drives.last = drives.first = drive;
		} else {
			drives.last = drives.last->next = drive;
		}

		drives_set_has_number(ext->DiskNumber);
	}

	return 0;
}
