#include "Steam.h"
#include "Resource.h"
#include <gdiplus.h>
#include <Windows.h>
#include <Shlwapi.h>
#include <CommCtrl.h>

#pragma comment(lib, "gdiplus.lib")
#pragma comment(lib, "shlwapi.lib")
#pragma comment(lib, "wininet.lib")
#pragma comment(lib, "Version.lib")
#pragma comment(lib, "Advapi32.lib")
#pragma comment(lib, "Comctl32.lib")

#define CLASS_NAME L"MLSA"

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

ULONG_PTR m_gdiplusToken;
HFONT font;
HFONT profFont;
HANDLE hEvent;
HWND Main;
HWND Edit;
HWND Loading;
HWND Save, Cancel, Trash, Refresh;
HWND Web, Steam, TabControl, Current, StatusBar;


ATOM InitializeClass(HINSTANCE hInstance) {
	WNDCLASSEXW wnd = { 0 };
	wnd.cbClsExtra = 0;
	wnd.cbSize = sizeof(wnd);
	wnd.cbWndExtra = 0;
	wnd.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
	wnd.hCursor = LoadCursorW(NULL, IDC_ARROW);
	wnd.hIcon = LoadIconW(hInstance, MAKEINTRESOURCEW(IDI_ICON));
	wnd.hIconSm = LoadIconW(hInstance, MAKEINTRESOURCEW(IDI_ICON));
	wnd.hInstance = hInstance;
	wnd.lpfnWndProc = WndProc;
	wnd.lpszClassName = CLASS_NAME;
	wnd.lpszMenuName = MAKEINTRESOURCEW(IDR_MENU);

	return RegisterClassExW(&wnd);
}

BOOL LoadingLabel(HINSTANCE hInstance, int nCmdShow) {
	RECT rc;
	GetClientRect(Main, &rc);
	Loading = CreateWindowW(L"STATIC", L"Loading...",
		WS_CHILD | WS_VISIBLE, (rc.right / 2) - 50, (rc.bottom / 2) - 60, 100, 100,
		Main, nullptr, hInstance, nullptr);
	if (!Loading)
		return FALSE;

	return TRUE;
}

BOOL MainWindow(HINSTANCE hInstance, int nCmdShow) {
	int x = (GetSystemMetrics(SM_CXFULLSCREEN) / 2) - 250;
	int y = (GetSystemMetrics(SM_CYFULLSCREEN) / 2) - 250;

	Main = CreateWindowW(CLASS_NAME, L"Make Lobbies Sexy Again",
		((WS_OVERLAPPEDWINDOW | WS_EX_CLIENTEDGE) & ~ (WS_MAXIMIZEBOX | WS_THICKFRAME)), x, y, 500, 540,
		nullptr, nullptr, hInstance, nullptr);

	if (Main == NULL)
		return FALSE;

	LoadingLabel(hInstance, nCmdShow);
	ShowWindow(Main, nCmdShow);
	UpdateWindow(Main);

	return TRUE;
}

BOOL EditBox(HINSTANCE hInstance, int nCmdShow) {
	RECT rc;
	GetClientRect(Main, &rc);
	Edit = CreateWindowW(L"EDIT", NULL,
		WS_CHILD | WS_VISIBLE | WS_BORDER | ES_MULTILINE, rc.left + 10, 150, rc.right - 30, (rc.bottom / 2) + 20,
		Main, (HMENU)ID_EDIT, hInstance, nullptr);
	if (!Edit)
		return FALSE;

	SendMessage(Edit, EM_SETLIMITTEXT, 2000, 0);

	return TRUE;
}

BOOL SaveCancel(HINSTANCE hInstance, int nCmdShow) {
	InitCommonControls();

	RECT rc;
	GetClientRect(Main, &rc);
	Save = CreateWindowW(L"BUTTON", L"   Save",
		WS_CHILD | WS_VISIBLE, rc.left + 10, rc.bottom - 60, 80, 25,
		Main, (HMENU)ID_SAVE, hInstance, nullptr);
	Cancel = CreateWindowW(L"BUTTON", L"   Cancel",
		WS_CHILD | WS_VISIBLE, rc.left + 110, rc.bottom - 60, 80, 25,
		Main, (HMENU)ID_CANCEL, hInstance, nullptr);
	Trash = CreateWindowW(L"BUTTON", L"   Delete",
		WS_CHILD | WS_VISIBLE, rc.left + 210, rc.bottom - 60, 80, 25,
		Main, (HMENU)ID_TRASH, hInstance, nullptr);
	Web = CreateWindowW(L"BUTTON", L"",
		WS_CHILD | WS_VISIBLE | BS_ICON, rc.left + 350, rc.bottom - 60, 35, 25,
		Main, (HMENU)ID_WEB, hInstance, nullptr);
	Steam = CreateWindowW(L"BUTTON", L"",
		WS_CHILD | WS_VISIBLE | BS_ICON, rc.left + 410, rc.bottom - 60, 35, 25,
		Main, (HMENU)ID_STEAM, hInstance, nullptr);
	Refresh = CreateWindowW(L"BUTTON", L"",
		WS_CHILD | WS_VISIBLE | BS_ICON, rc.left + 430, rc.bottom - 360, 35, 25,
		Main, (HMENU)ID_REFRESH, hInstance, nullptr);


	StatusBar = CreateWindowExW(
		0,                     
		STATUSCLASSNAMEW,       
		(PCWSTR)NULL,           
		/*SBARS_SIZEGRIP |*/       
		WS_CHILD | WS_VISIBLE,   
		0, 0, 0, 0,              
		Main,           
		(HMENU)ID_STATUS,       
		hInstance,                
		NULL);

	int statwidths[] = { 100, -1};

	SendMessageA(StatusBar, SB_SETPARTS, 1, (LPARAM)statwidths);

	if (!Save || !Cancel || !Web || !Steam || !Trash || !Refresh || !StatusBar)
		return FALSE;

	EnableWindow(Save, FALSE);
	EnableWindow(Cancel, FALSE);
	EnableWindow(Trash, FALSE);

	HICON hIconSave = (HICON)LoadImageW(GetModuleHandle(NULL), MAKEINTRESOURCEW(IDI_SAVE),
		IMAGE_ICON, 16, 16, FALSE);
	SendMessageW(Save, BM_SETIMAGE, (WPARAM)IMAGE_ICON, (LPARAM)hIconSave);

	HICON hIconCancel = (HICON)LoadImageW(GetModuleHandle(NULL), MAKEINTRESOURCEW(IDI_CANCEL),
		IMAGE_ICON, 16, 16, FALSE);
	SendMessageW(Cancel, BM_SETIMAGE, (WPARAM)IMAGE_ICON, (LPARAM)hIconCancel);

	HICON hIconTrash = (HICON)LoadImageW(GetModuleHandle(NULL), MAKEINTRESOURCEW(IDI_TRASH),
		IMAGE_ICON, 16, 16, FALSE);
	SendMessageW(Trash, BM_SETIMAGE, (WPARAM)IMAGE_ICON, (LPARAM)hIconTrash);

	HICON hIconGlobe = (HICON)LoadImageW(GetModuleHandle(NULL), MAKEINTRESOURCEW(IDI_WEB),
		IMAGE_ICON, 16, 16, FALSE);
	SendMessageW(Web, BM_SETIMAGE, (WPARAM)IMAGE_ICON, (LPARAM)hIconGlobe);

	HICON hIconSteam = (HICON)LoadImageW(GetModuleHandle(NULL), MAKEINTRESOURCEW(IDI_STEAM),
		IMAGE_ICON, 16, 16, FALSE);
	SendMessageW(Steam, BM_SETIMAGE, (WPARAM)IMAGE_ICON, (LPARAM)hIconSteam);

	HICON hIconRefresh = (HICON)LoadImageW(GetModuleHandle(NULL), MAKEINTRESOURCEW(IDI_REFRESH),
		IMAGE_ICON, 16, 16, FALSE);
	SendMessageW(Refresh, BM_SETIMAGE, (WPARAM)IMAGE_ICON, (LPARAM)hIconRefresh);

	return TRUE;
}


BOOL CALLBACK EnumChildProc(HWND hWnd, LPARAM lParam) {
	SendMessageW(hWnd, WM_SETFONT, reinterpret_cast<WPARAM>(font), lParam);
	return TRUE;
}

VOID Invalidate() {
	RECT rc;
	GetClientRect(Main, &rc);
	rc.left = 10;
	rc.bottom = 150;
	InvalidateRect(Main, &rc, TRUE);
}


DWORD WINAPI ThreadFunc(VOID* ptr) {
	SteamUser* p = (SteamUser*)(ptr);
	if (p->FetchUser() && p->FetchImage()) {
		return TRUE;
	}
	return FALSE;
}

DWORD __stdcall WaitForSingleObj(VOID* ptr) {
	SteamUser* pt = (SteamUser*)ptr;
	HANDLE hThread = CreateThread(NULL, 0, ThreadFunc, pt, 0, NULL);
	if (hThread) {
		WaitForSingleObject(hThread, INFINITE);
		CloseHandle(hThread);
	}

	if (!pt->LoadText(Edit)) {
		EnableWindow(Trash, FALSE);
	}
	else {
		MessageBeep(MB_ICONINFORMATION);
		EnableWindow(Trash, TRUE);
	}

	Invalidate();
	UpdateWindow(Main);

	return 0;
}

DWORD __stdcall UpdateOnTimer(VOID* p) {
	wchar_t FilePath[MAX_PATH];
	ExpandEnvironmentStringsW(L"%localappdata%", FilePath, MAX_PATH);
	wcscat_s(FilePath, MAX_PATH, L"\\DeadByDaylight\\Saved\\Logs\\");

	HANDLE FindFirst = FindFirstChangeNotificationW(FilePath, FALSE, FILE_NOTIFY_CHANGE_LAST_WRITE);
	HANDLE rename = FindFirstChangeNotificationW(FilePath, FALSE, FILE_NOTIFY_CHANGE_FILE_NAME);
	DWORD r = 0;
	
	if (FindFirst == INVALID_HANDLE_VALUE) {
		//MessageBoxW(NULL, L"Invalid handle", L"Error", MB_OK);
		return 0;
	}
	while (WaitForSingleObject(hEvent, 1000) != WAIT_OBJECT_0) {
		r = WaitForSingleObject(FindFirst, 2000);
		if (r == WAIT_OBJECT_0) {
			SendMessage(Main, WM_COMMAND, ID_REFRESH, 0);
			FindNextChangeNotification(FindFirst);
			FindNextChangeNotification(FindFirst);
			Sleep(1000);
		}
		r = WaitForSingleObject(rename, 2000);
		if (r == WAIT_OBJECT_0) {
			SendMessage(Main, WM_COMMAND, ID_CLOSE, 0);
			FindNextChangeNotification(rename);
			Sleep(1000);
			SendMessage(Main, WM_COMMAND, ID_REFRESH, 0);
		}
	}
	if(FindFirst)
		FindCloseChangeNotification(FindFirst);
	if (rename)
		FindCloseChangeNotification(rename);

	return 0;
}

VOID UserFolder() {
	DWORD dwAttrib = GetFileAttributesW(L"Users");
	if (dwAttrib != INVALID_FILE_ATTRIBUTES &&
		(dwAttrib & FILE_ATTRIBUTE_DIRECTORY)) {
		return;
	}
	CreateDirectoryW(L"Users", NULL);
}

SteamUser* user;

INT WINAPI WinMain(
	_In_ HINSTANCE hInstance, 
    _In_opt_ HINSTANCE hPrev,
	_In_ LPSTR cmdLine, 
	_In_ int nCmdShow) 
{

	MSG msg;

	InitializeClass(hInstance);
	MainWindow(hInstance, nCmdShow);

	UserFolder();

	user = new SteamUser();

	if (user->GetID64() != L"") {
		HANDLE ThreadHandle = CreateThread(NULL, 0, WaitForSingleObj, user, 0, NULL);
		if (!ThreadHandle) {
			//MessageBoxW(NULL, L"Failed to create thread", L"Error", MB_OK);
		}
		if (ThreadHandle) {
			CloseHandle(ThreadHandle);
		}
	}

	ShowWindow(Loading, SW_HIDE);
	EditBox(hInstance, nCmdShow);
	SaveCancel(hInstance, nCmdShow);
	EnumChildWindows(Main, EnumChildProc,
		0);
	UpdateWindow(Main);

	hEvent = CreateEventW(NULL, TRUE, FALSE, L"UpdateTimer");
	if (!hEvent) {
		MessageBoxW(NULL, L"Failed to create event", L"Error", MB_ICONERROR);
		return 0;
	}

	HANDLE hThread = CreateThread(NULL, 0, UpdateOnTimer, user, 0, 0);

	BOOL ret;
	while ((ret = GetMessageW(&msg, nullptr, 0, 0)) > 0) {
			TranslateMessage(&msg);
			DispatchMessageW(&msg);
	}

	UnregisterClassW(CLASS_NAME, hInstance);

	SetEvent(hEvent);

	if (hEvent && hThread) {
		WaitForSingleObject(hThread, INFINITE);
		CloseHandle(hEvent);
		CloseHandle(hThread);
	}

	return 0;
}

std::wstring ConcatVersion(DWORD a, DWORD b, DWORD c, DWORD d) {
	std::wstring tmp;
	tmp += std::to_wstring(a); tmp += L".";
	tmp += std::to_wstring(b); tmp += L".";
	tmp += std::to_wstring(c); tmp += L".";
	tmp += std::to_wstring(d);

	return tmp;
}

INT_PTR DlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	switch (uMsg) {
	case WM_INITDIALOG: {
		HRSRC hResInfo;
		DWORD dwSize;
		HGLOBAL hResData;
		LPVOID pRes, pResCopy;
		UINT uLen;
		VS_FIXEDFILEINFO* lpFfi = NULL;
		HINSTANCE hInst = GetModuleHandle(NULL);

		hResInfo = FindResourceW(hInst, MAKEINTRESOURCEW(VS_VERSION_INFO), RT_VERSION);
		if (!hResInfo)
			return FALSE;
		dwSize = SizeofResource(hInst, hResInfo);
		hResData = LoadResource(hInst, hResInfo);
		if (!hResData)
			return FALSE;
		pRes = LockResource(hResData);
		pResCopy = LocalAlloc(LMEM_FIXED, dwSize);
		if (!pResCopy)
			return FALSE;
		CopyMemory(pResCopy, pRes, dwSize);
		FreeResource(hResData);

		VerQueryValueW(pResCopy, L"\\", (LPVOID*)&lpFfi, &uLen);

		DWORD dwFileVersionMS = lpFfi->dwFileVersionMS;
		DWORD dwFileVersionLS = lpFfi->dwFileVersionLS;

		DWORD dwLeftMost = (dwFileVersionMS >> 16) & 0xfff;
		DWORD dwSecondLeft = (dwFileVersionMS >> 0) & 0xfff;
		DWORD dwSecondRight = (dwFileVersionLS >> 16) & 0xfff;
		DWORD dwRightMost = (dwFileVersionLS >> 0) & 0xfff;


		LPVOID pvProductName = NULL;
		unsigned int iProductNameLen = 0;

		LPVOID pvLegal = NULL;
		unsigned int iLegal = 0;

		LPVOID pvCompany = NULL;
		unsigned int iCompany = 0;

		VerQueryValueW(pResCopy, L"\\StringFileInfo\\080904b0\\ProductName", &pvProductName, &iProductNameLen);
		VerQueryValueW(pResCopy, L"\\StringFileInfo\\080904b0\\LegalCopyright", &pvLegal, &iLegal);
		VerQueryValueW(pResCopy, L"\\StringFileInfo\\080904b0\\CompanyName", &pvCompany, &iCompany);


		std::wstring temp = (wchar_t*)pvProductName;
		temp += L", "; temp += ConcatVersion(dwLeftMost, dwSecondLeft, dwSecondRight, dwRightMost);
		SetDlgItemTextW(hWnd, IDC_PROD_VER, temp.c_str());

		temp = (wchar_t*)pvCompany;
		SetDlgItemTextW(hWnd, IDC_NAME, temp.c_str());

		temp = (wchar_t*)pvLegal;
		SetDlgItemTextW(hWnd, IDC_COPY, temp.c_str());
		LocalFree(pResCopy);


		int x, y;
		x = GetSystemMetrics(SM_CXSMICON) * 2;
		y = GetSystemMetrics(SM_CYSMICON) * 2;
		HICON hIcon = (HICON)LoadImageW(GetModuleHandle(NULL),
			MAKEINTRESOURCEW(IDI_ICON), IMAGE_ICON, x, y, FALSE);

		SendMessageW(hWnd, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);
		SendMessageW(hWnd, WM_SETICON, ICON_BIG, (LPARAM)hIcon);

		SendMessageW(GetDlgItem(hWnd, ID_PICTURE), STM_SETIMAGE, (WPARAM)IMAGE_ICON, (LPARAM)hIcon);

		return TRUE;
	}
	case WM_COMMAND: {
		switch (LOWORD(wParam)) {
		case IDOK:
		case IDCANCEL:
			EndDialog(hWnd, wParam);
			return TRUE;
		}
	}
	break;
  }
	return FALSE;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg,
	WPARAM wParam, LPARAM lParam) {
	switch (uMsg) {
	case WM_CREATE: {
		font = CreateFontW(15, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS,
			CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_ROMAN,
			L"Tahoma");
		profFont = CreateFontW(20, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS,
			CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_ROMAN,
			L"Tahoma");
		Gdiplus::GdiplusStartupInput gdiplusStartupInput;
		Gdiplus::GdiplusStartup(&m_gdiplusToken, &gdiplusStartupInput, NULL);
		return 0;
	}
	case WM_PAINT: {
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(Main, &ps);
		if (user) {
			if (user->ReturnBuffer()) {
				int x = 10, y = 10;
				Gdiplus::Graphics graphics(hdc);
				IStream* pStream = SHCreateMemStream((BYTE*)user->ReturnBuffer(), user->ReturnBufferSize());
				Gdiplus::Image* image = Gdiplus::Image::FromStream(pStream);
				graphics.DrawImage(image, x, y, 128, 128);
				x += 130;

				HFONT hOld = (HFONT)SelectObject(hdc, profFont);
				if (user->ProfileName().size() > 0) {
					TextOutW(hdc, x, y, user->ProfileName().c_str(), wcslen(user->ProfileName().c_str()));
					y += 40;
				}
				SelectObject(hdc, font);
				if (user->RealName().size() > 0) {
					TextOutW(hdc, x, y, user->RealName().c_str(), wcslen(user->RealName().c_str()));
					y += 40;
				}
				if (user->Country().size() > 0) {
					TextOutW(hdc, x, y, user->Country().c_str(), wcslen(user->Country().c_str()));
					y += 40;
				}
				if (user->IsPrivate().size() > 0) {
					SetTextColor(hdc, RGB(255, 0, 0));
					TextOutW(hdc, x, y, user->IsPrivate().c_str(), wcslen(user->IsPrivate().c_str()));
					y += 40;
				}
				SelectObject(hdc, hOld);
			}
		}
		EndPaint(hWnd, &ps);
	}
	break;
	case WM_CTLCOLORSTATIC:
		return reinterpret_cast<LRESULT>(CreateSolidBrush(RGB(0xff, 0xff, 0xff)));
	case WM_COMMAND: {
		if ((HWND)lParam == Edit) {
			if (HIWORD(wParam) != EN_SETFOCUS && HIWORD(wParam) != EN_KILLFOCUS) {
				EnableWindow(Save, TRUE);
				EnableWindow(Cancel, TRUE);
				int ret = GetWindowTextLength(Edit);
				if (ret == 0) {
					EnableWindow(Save, FALSE);
					EnableWindow(Cancel, FALSE);
					SendMessage(StatusBar, SB_SETTEXT, 0, (LPARAM)L"");
				}
				else {
					SendMessage(StatusBar, SB_SETTEXT, 0, (LPARAM)L"NOT SAVED*");
				}
			}
		}
		switch (LOWORD(wParam)) {
		case ID_HELP_ABOUT:
			DialogBoxW(NULL, MAKEINTRESOURCEW(IDD_ABOUT_DLG), hWnd,
				reinterpret_cast<DLGPROC>(DlgProc));
			break;
		case ID_FILE_EXIT:
			PostMessageW(hWnd, WM_CLOSE, 0, 0);
			break;
		case ID_SAVE: {
			wchar_t buffer[2048] = {0};
			int ret = GetWindowTextW(Edit, buffer, 2047);
			std::wstring utf16(buffer);
			std::string utf8 =  to_utf8(utf16);
			if (!ret)
				break;

			unsigned char smarker[3];
			smarker[0] = 0xEF;
			smarker[1] = 0xBB;
			smarker[2] = 0xBF;

			std::wstring p = L"Users\\" + user->GetID64();
			HANDLE FileID = CreateFileW(p.c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS,
				FILE_ATTRIBUTE_NORMAL, NULL);
			DWORD written = 0;
			if (FileID == INVALID_HANDLE_VALUE) {
				MessageBoxW(NULL, L"Failed to create file", L"Error", MB_ICONERROR);
				break;
			}
			WriteFile(FileID, smarker, 3, &written, NULL);
			WriteFile(FileID, utf8.c_str(), utf8.size(), &written, NULL);
			if (utf8.size() != written) {
				MessageBoxW(NULL, L"Failed to write all data to file", L"Error", MB_ICONERROR);
				CloseHandle(FileID);
				break;
			}
			CloseHandle(FileID);
			EnableWindow(Save, FALSE);
			EnableWindow(Cancel, FALSE);
			EnableWindow(Trash, TRUE);
			SendMessage(StatusBar, SB_SETTEXT, 0, (LPARAM)L"Saved");
		}
		break;
		case ID_CANCEL: {
			if (GetWindowTextLengthW(Edit) > 0) {
				SetWindowTextW(Edit, L"");
				EnableWindow(Save, FALSE);
				EnableWindow(Cancel, FALSE);
			}
		}
		break;
		case ID_TRASH: {
			int ret = MessageBoxW(NULL, L"Are you sure you want to remove this file?", L"Delete file", MB_OKCANCEL);
			if (ret == IDOK) {
				std::wstring p = L"Users\\" + user->GetID64();
				DeleteFileW(p.c_str());
				EnableWindow(Trash, FALSE);
				SetWindowTextW(Edit, L"");
				break;
			}
		}
		break;
		case ID_REFRESH: {
			int r = user->Refresh();
			if (r == 0 || r == -1) {
				break;
			}
			if (r == -2) {
				return r;
			}
			if (IsWindowEnabled(Save)) {
				int ret = MessageBoxW(NULL, L"Are you sure you want refresh without saving", L"Notice", MB_OKCANCEL);
				if (ret != IDOK) {
					break;
				}
			}
			if (user->ReturnBuffer()) {
				VirtualFree(user->ReturnBuffer(), 0, MEM_RELEASE);
				user->SetBufSz(NULL, 0);
			}
			HANDLE hThread = CreateThread(NULL, 0, WaitForSingleObj, user, 0, 0);
			if (hThread)
				CloseHandle(hThread);
			EnableWindow(Save, FALSE);
			EnableWindow(Cancel, FALSE);
			SendMessage(StatusBar, SB_SETTEXT, 0, (LPARAM)L"");
		}
		break;
		case ID_WEB: {
			if (user->GetID64().size() == 0) {
				break;
			}
			std::wstring url = L"https://www.steamcommunity.com/profiles/" + user->GetID64();
			ShellExecuteW(NULL, L"open", url.c_str(), NULL, NULL, SW_SHOWNORMAL);
		}
		break;
		case ID_STEAM: {
			if (user->GetID64().size() == 0) {
				break;
			}
			HKEY hKey;
			LONG ret = RegOpenKeyExW(HKEY_CURRENT_USER, L"Software\\Valve\\Steam", 0, KEY_READ, &hKey);
			if (ret != ERROR_SUCCESS) {
				MessageBoxW(NULL, L"Is Steam installed?", L"Error", MB_ICONERROR);
				break;
			}
			wchar_t buffer[512];
			DWORD bufsz = 512;

			ret = RegQueryValueExW(hKey, L"SteamExe", 0, NULL, (LPBYTE)buffer, &bufsz);
			if (ret != ERROR_SUCCESS) {
				MessageBoxW(NULL, L"Couldn't find steam installation location", L"Error", MB_ICONERROR);
				break;
			}
			wchar_t cmd[512];
			wcscpy_s(cmd, 512, L"steam://openurl/https://steamcommunity.com/profiles/");
			wcscat_s(cmd, 512, user->GetID64().c_str());
			STARTUPINFOW start;
			ZeroMemory(&start, sizeof(STARTUPINFOW));
			start.cb = sizeof(STARTUPINFOW);
			start.wShowWindow = SW_SHOW;
			PROCESS_INFORMATION proc;
			ZeroMemory(&proc, sizeof(PROCESS_INFORMATION));

			BOOL hProcess = CreateProcessW(buffer, cmd, NULL, NULL, 0, 0, NULL, NULL, &start, &proc);
			if (!hProcess) {
				MessageBoxW(NULL, L"Couldn't start Steam", L"Error", MB_ICONERROR);
			}
			CloseHandle(proc.hThread);
			CloseHandle(proc.hProcess);

		}
		break;
		case ID_CLOSE: {
			if (user) {
				user->CloseFile();
			}
		}
		break;
		default:
			return DefWindowProcW(hWnd, uMsg, wParam, lParam);
		}
	}
	break;
	case WM_CLOSE: {
		if (IsWindowEnabled(Save)) {
			int r = MessageBoxW(NULL, L"Are you sure you want to quit without saving?", L"Quit", MB_OKCANCEL);
			if (r == IDOK) {
				DestroyWindow(hWnd);
			}
		}
		else {
			DestroyWindow(hWnd);
		}
	}
	break;
	case WM_DESTROY:
		Gdiplus::GdiplusShutdown(m_gdiplusToken);
		DeleteObject(profFont);
		DeleteObject(font);
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProcW(hWnd, uMsg, wParam, lParam);
	}
	return 0;
}