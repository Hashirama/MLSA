#include "Steam.h"

SteamUser::SteamUser() {
	if (!file.MapFile()) {
		id64 = L"";
		return;
	}
	id64 = file.GetLastID64();

	if (id64 == L"") {
		return;
	}
}

BOOL SteamUser::LoadText(HWND E) {
	if (id64.empty())
		return FALSE;
	SetWindowTextW(E, L"");

	std::wstring p = L"Users\\" + GetID64();
	HANDLE hFile = CreateFileW(p.c_str(), GENERIC_READ, 0, NULL, OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE && GetLastError() == ERROR_FILE_NOT_FOUND) {
		return FALSE;
	}

	DWORD FileSize = GetFileSize(hFile, NULL);
	DWORD bytes = 0;
	char buffer[2048];
	if (!ReadFile(hFile, buffer, FileSize, &bytes, NULL)) {
		CloseHandle(hFile);
		return FALSE;
	}
	if (bytes != FileSize) {
		CloseHandle(hFile);
		return FALSE;
	}
	CloseHandle(hFile);
	buffer[bytes] = '\0';
	std::string utf8(buffer);
	std::wstring utf16 = to_utf16(utf8);
	if (bytes > 0) {
		SetWindowTextW(E, utf16.c_str());
	}
	return TRUE;
}

BOOL SteamUser::FetchImage() {
	int ret = 0;

	if (!IsConnected()) {
		//MessageBoxW(NULL, L"Check Internet connection", L"Error", MB_ICONERROR);
		return FALSE;
	}

	char* buffer = NULL;
	DWORD read = 1;
	HINTERNET hInternet, hFile;
	hInternet = InternetOpenW(L"", INTERNET_OPEN_TYPE_DIRECT,
		NULL, NULL, 0);
	if (hInternet == NULL) {
		return FALSE;
	}
	hFile = InternetOpenUrlW(hInternet, imgurl.c_str(), NULL, 0L, 0, 0);
	if (hFile == NULL) {
		InternetCloseHandle(hInternet);
		return FALSE;
	}

	WCHAR szBuf[256] = {0};
	DWORD Bufsz = sizeof(wchar_t) * 256;
	DWORD content_length = 0;
	DWORD total;
	if (!HttpQueryInfoW(hFile, HTTP_QUERY_CONTENT_LENGTH, szBuf, &Bufsz, NULL)) {
		buffer = (char*)VirtualAlloc(NULL, BUFSZ, MEM_COMMIT, PAGE_READWRITE);
		total = BUFSZ - 1;
	}
	else {
		content_length = _wtol(szBuf) + 1;
		buffer = (char*)VirtualAlloc(NULL, content_length, MEM_COMMIT, PAGE_READWRITE);
		total = content_length - 1;
	}
	if (!buffer)
		return FALSE;

	while (read > 0) {
		if (!InternetReadFile(hFile, buffer, total, &read)) {
			InternetCloseHandle(hInternet);
			InternetCloseHandle(hFile);
			VirtualFree(buffer, 0, MEM_RELEASE);
			return FALSE;
		}
		if (read > 0 && buffer) {
			SetBufSz(buffer, read);
		}
	}

	ret = !ret;

	return ret;
}

BOOL SteamUser::FetchUser() {
	std::wstring domain = L"https://steamcommunity.com";
	std::wstring path = L"/profiles/";
	std::wstring utf16;
	int ret = 0;

	if (!IsConnected()) {
		//MessageBoxW(NULL, L"Check Internet connection", L"Error", MB_ICONERROR);
		return FALSE;
	}

	char buf[1001];
	DWORD read = 1;
	std::wstring url = domain + path + id64;
	HINTERNET hInternet, hFile;
	std::string utf8;
	hInternet = InternetOpenW(L"", INTERNET_OPEN_TYPE_DIRECT,
		NULL, NULL, 0);
	if (hInternet == NULL) {
		return FALSE;
	}
	hFile = InternetOpenUrlW(hInternet, url.c_str(), NULL, 0L, 0, 0);
	if (hFile == NULL) {
		InternetCloseHandle(hInternet);
		return FALSE;
	}

	while (read > 0) {

		if (!InternetReadFile(hFile, buf, 1000, &read)) {
			InternetCloseHandle(hInternet);
			InternetCloseHandle(hFile);
			return FALSE;
		}
		buf[read] = '\0';
		utf8 += buf;
	}
	utf16 = to_utf16(utf8);
	size_t pos = 0;

	if ((pos = utf16.find(L"actual_persona_name")) != std::string::npos) {
		profilename = utf16.substr(pos);
		pos = profilename.find(L">");
		profilename = profilename.substr(++pos);
		pos = profilename.find(L"<");
		profilename = profilename.substr(0, pos);
	}
	if ((pos = utf16.find(L"header_real_name ellipsis")) != std::string::npos) {
		realname = utf16.substr(pos);
		pos = realname.find(L">");
		realname = realname.substr(++pos);
		pos = realname.find(L">");
		realname = realname.substr(++pos);
		pos = realname.find(L"<");
		realname = realname.substr(0, pos);
	}
	if ((pos = utf16.find(L"profile_flag")) != std::string::npos) {
		country = utf16.substr(++pos);
		pos = country.find(L">");
		country = country.substr(pos);

		auto it = country.begin(); ++it;
		while (iswspace(*it))
			++it;
		auto end = it;
		while (iswalpha(*end))
			++end;
		while (*end != L'<')
			++end;
		std::wstring tmp(it, end);

		country = tmp;
	}

	if ((pos = utf16.find(L"profile_private_info")) != std::string::npos) {
		isprivate = L"This profile is private.";
	}
	else {
		isprivate = L"";
	}

	if ((pos = utf16.find(L"playerAvatarAutoSizeInner")) != std::string::npos) {
		imgurl = utf16.substr(pos);
		pos = imgurl.find(L"=");
		imgurl = imgurl.substr(pos + 2);
		pos = imgurl.find(L"\"");
		imgurl = imgurl.substr(0, pos);
	}

	InternetCloseHandle(hInternet);
	InternetCloseHandle(hFile);

	ret = !ret;

	return ret;
}

BOOL SteamUser::Refresh() {
	if (file.IsHandleInvalid()) {
		if (!file.MapFile()) {
			return FALSE;
		}
	}
	else {
		if (file.OldFileSz() == file.FileSize() && file.OldFileSz() > 0)
			return FALSE;
	}
	std::wstring tmp;
	if ((tmp = file.GetLastID64()) != L"") {
		if (id64 == tmp && ImageBuffer) {
			return -1;
		}
		id64 = tmp; profilename.clear();
		imgurl.clear(); realname.clear();
		isprivate.clear(); country.clear();
		return TRUE;
	}
	return FALSE;
}

SteamUser::~SteamUser() {
	if(this->ImageBuffer)
		VirtualFree(this->ImageBuffer, 0, MEM_RELEASE);
}
