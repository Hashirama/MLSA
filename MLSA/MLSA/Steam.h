#ifndef STEAM_H
#define STEAM_H
#define BUFSZ 40000

#include "File.h"
#include "Str.h"
#include <string>
#include <wininet.h>
#include <vector>

class SteamUser {
public:
	SteamUser();
	~SteamUser();
	BOOL FetchUser();
	BOOL FetchImage();
	char* ReturnBuffer() { return ImageBuffer; }
	int ReturnBufferSize() { return ImageSize; }
	VOID SetBufSz(char* buf, int sz) { ImageBuffer = buf; ImageSize = sz; }
	std::wstring ProfileName() { return profilename; }
	std::wstring RealName() { return realname; }
	std::wstring Country() { return country; }
	std::wstring IsPrivate() { return isprivate; }
	std::wstring GetID64() { return id64;  }
	BOOL IsConnected() {
		DWORD state = 0;
		return InternetGetConnectedState(&state, 0);
	}
	BOOL LoadText(HWND);
	BOOL Refresh();
	VOID CloseFile() { file.CloseFile(); }
private:
	std::wstring id64;
	std::wstring imgurl;
	std::wstring profilename;
	std::wstring realname;
	std::wstring country;
	std::wstring isprivate;
	File file;
	int ImageSize;
	char* ImageBuffer = NULL;
};
#endif