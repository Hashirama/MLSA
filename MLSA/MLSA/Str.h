#include <string>
#include <Windows.h>

std::string to_utf8(const std::wstring& s);
std::wstring to_utf16(const std::string& s);