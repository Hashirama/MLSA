#include "Str.h"


std::string to_utf8(const std::wstring& s) {

	std::string utf8;
	int len = WideCharToMultiByte(CP_UTF8, 0, s.c_str(), s.length(), NULL, 0, NULL, NULL);
	if (len > 0)
	{
		utf8.resize(len);
		WideCharToMultiByte(CP_UTF8, 0, s.c_str(), s.length(), &utf8[0], len, NULL, NULL);
	}
	return utf8;
}

std::wstring to_utf16(const std::string& s) {
	std::wstring utf16;
	int len = MultiByteToWideChar(CP_UTF8, 0, s.c_str(), s.length(), NULL, 0);
	if (len > 0)
	{
		utf16.resize(len);
		MultiByteToWideChar(CP_UTF8, 0, s.c_str(), s.length(), &utf16[0], len);
	}
	return utf16;
}