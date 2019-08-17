#include "File.h"

BOOL File::MapFile() {
	DWORD bytes = 0;
	wchar_t FilePath[MAX_PATH];
	ExpandEnvironmentStringsW(L"%localappdata%", FilePath, MAX_PATH);
	wcscat_s(FilePath, MAX_PATH, L"\\DeadByDaylight\\Saved\\Logs\\DeadByDaylight.log");
	FileHandle = CreateFileW(FilePath, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE |
		FILE_SHARE_DELETE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (FileHandle == INVALID_HANDLE_VALUE) {
		return FALSE;
	}
	FileSz = GetFileSize(FileHandle, NULL);
	if (FileSz == 0) {
		return FALSE;
	}
	FileBuffer = VirtualAlloc(NULL, FileSz, MEM_COMMIT, PAGE_READWRITE);
	if (FileBuffer == NULL) {
		return FALSE;
	}
	if (!ReadFile(FileHandle, FileBuffer, FileSz, &bytes, NULL)) {
		VirtualFree(FileBuffer, 0, MEM_RELEASE);
		return FALSE;
	}
	return TRUE;
}

std::wstring File::GetLastID64() {
	if (!FileBuffer) {
	     return L"";
	}

	DWORD ret = Remap();

	std::string s((char*)FileBuffer, ret);
	std::string line = s;
	size_t pos = 0;

	while ((pos = s.find("Sending hello")) != std::wstring::npos) {
		line = s.substr(pos);
		pos = line.find(":");
		++pos; ++pos;
		line = line.substr(pos);
		pos = line.find(":");
		line = line.substr(0, pos);
		
		pos = s.find("Sending hello");
		s = s.substr(++pos);
	}

	if (line.size() != 17) {
		return L"";
	}

	return to_utf16(line);
}

DWORD File::Remap() {
	Sleep(1000);
	DWORD bytes = 0;
	DWORD TmpBufSz = 0;
	DWORD Current = FileSize();
	if (Current == 0) {
	      Current = FileSize();
	}
	if (FileSz < Current) {
		if (FileBuffer) {
			VirtualFree(FileBuffer, 0, MEM_RELEASE);
			FileBuffer = NULL;
		}
		TmpBufSz = Current - FileSz;
		FileBuffer = VirtualAlloc(NULL, TmpBufSz, MEM_COMMIT, PAGE_READWRITE);
		if (!FileBuffer)
			return 0;
		if (!ReadFile(FileHandle, FileBuffer, TmpBufSz, &bytes, NULL)) {
			VirtualFree(FileBuffer, 0, MEM_RELEASE);
			FileBuffer = NULL;
			return 0;
		}
		if (TmpBufSz != bytes) {
			return 0;
		}

		FileSz = Current;
		return TmpBufSz;
	}
	else if (FileSz > Current) {
		SetFilePointer(FileHandle, 0, NULL, FILE_BEGIN);
		if (FileBuffer) {
			VirtualFree(FileBuffer, 0, MEM_RELEASE);
			FileBuffer = NULL;
		}
		FileBuffer = VirtualAlloc(NULL, Current, MEM_COMMIT, PAGE_READWRITE);
		if (!FileBuffer)
			return 0;
		if (!ReadFile(FileHandle, FileBuffer, Current, &bytes, NULL)) {
			VirtualFree(FileBuffer, 0, MEM_RELEASE);
			FileBuffer = NULL;
			return 0;
		}
		if (Current != bytes) {
			return 0;
		}
		FileSz = Current;
		return Current;
	}
	return Current;
}

BOOL File::CloseFile() {
	if (FileHandle) {
		CloseHandle(FileHandle);
		FileHandle = INVALID_HANDLE_VALUE;
		return TRUE;
	}
	return FALSE;
}

File::~File() {
	if (FileBuffer) {
		VirtualFree(FileBuffer, 0, MEM_RELEASE);
	}
	if (FileHandle) {
		CloseHandle(FileHandle);
	}
}
