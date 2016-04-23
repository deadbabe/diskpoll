#ifndef __DISKPOLL_DRIVES_H
#define __DISKPOLL_DRIVES_H

#include "main.h"

#define SECTOR_ALIGNMENT 4096

struct drive {
	struct drive *next;
	HANDLE handle;
	LONGLONG drive_size;
	DWORD sector_size;
	OVERLAPPED overlapped;
};

struct drives {
	struct drive *first, *last;
	DWORD bitmap;
	void *buffer;
};

extern struct drives drives;

static inline DWORD drives_init()
{
	drives.buffer = _aligned_malloc(SECTOR_ALIGNMENT, SECTOR_ALIGNMENT);
	if (drives.buffer == NULL)
		return errno;

	srand(GetTickCount());
	return 0;
}

static inline BOOL drives_has_number(DWORD number) {
	return (drives.bitmap & (1 << number)) != 0;
}

static inline void drives_set_has_number(DWORD number) {
	drives.bitmap |= 1 << number;
}

static inline BOOL drives_has_any() {
	return drives.bitmap != 0;
}

static inline LARGE_INTEGER rand_large() {
	LARGE_INTEGER result;
#ifdef _AMD64_
	result.QuadPart = rand() ^ rand() << 16 ^ (LONGLONG)rand() << 32 ^ (LONGLONG)rand() << 48;
#else
	result.LowPart = rand() ^ rand() << 16;
	result.HighPart = rand() ^ rand() << 16;
#endif
	return result;
}

static inline void drives_poll()
{
	for (struct drive *drive = drives.first; drive; drive = drive->next) {
		LARGE_INTEGER offset = rand_large();
		offset.QuadPart %= drive->drive_size;
		offset.QuadPart &= -(LONGLONG)drive->sector_size;

		drive->overlapped.Offset = offset.LowPart;
		drive->overlapped.OffsetHigh = offset.HighPart;

		ReadFile(drive->handle, drives.buffer, drive->sector_size, NULL, &drive->overlapped);
	}
}

static inline void drives_free()
{
	struct drive *current, *next;

	for (current = drives.first; current; current = next) {
		next = current->next;
		CloseHandle(current->handle);
		free(current);
	}

	_aligned_free(drives.buffer);
}

extern BOOL drives_add_by_letter(WCHAR letter);

#endif // __DISKPOLL_DRIVES_H
