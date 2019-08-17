#ifndef FILE_H
#define FILE_H

#include <string>
#include <Windows.h>
#include "Str.h"

class File {
public:
	File() = default;
	~File();
	BOOL MapFile();
	DWORD FileSize() {
		if(FileHandle != INVALID_HANDLE_VALUE)
			return GetFileSize(FileHandle, NULL);
		return 0;
	}
	DWORD OldFileSz() {
		return FileSz;
	}
	BOOL IsHandleInvalid() {
		return (FileHandle == INVALID_HANDLE_VALUE);
	}
	std::wstring GetLastID64();
	DWORD Remap();
	BOOL CloseFile();
private:
	HANDLE FileHandle = INVALID_HANDLE_VALUE;
	PVOID FileBuffer = NULL;
	DWORD FileSz = 0;
};

#endif
