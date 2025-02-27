
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include "tlhelp32.h"

DWORD processId;
char *account;
char *password;
int delay;

void AL_Key(HANDLE hWnd, WORD vk, BOOL press)
{
	INPUT ip;
	ZeroMemory(&ip, sizeof(INPUT));

	ip.type = INPUT_KEYBOARD;
	ip.ki.wVk = vk;
	ip.ki.dwFlags = (press ? 0 : KEYEVENTF_KEYUP);
	SendInput(1, &ip, sizeof(INPUT));
}

void AL_SendKey(HANDLE hWnd, char key)
{
	WORD vk = VkKeyScan(key);
	BOOL isUpper = IsCharUpper(key);

	if (isUpper) {
		AL_Key(hWnd, VK_LSHIFT, TRUE);
	}

	AL_Key(hWnd, vk, TRUE);
	AL_Key(hWnd, vk, FALSE);

	if (isUpper) {
		AL_Key(hWnd, VK_LSHIFT, FALSE);
	}
}

void AL_SendInfo(HANDLE hWnd)
{
	char *pKey = account;
	while (*pKey != '\0') {
		AL_SendKey(hWnd, *pKey);
		pKey++;
	}

	AL_SendKey(hWnd, '\t');

	pKey = password;
	while (*pKey != '\0') {
		AL_SendKey(hWnd, *pKey);
		pKey++;
	}

	AL_SendKey(hWnd, '\n');
}

BOOL AL_FindChild(PDWORD childId) 
{
	HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

	PROCESSENTRY32 pe32;
	ZeroMemory(&pe32, sizeof(pe32));
	pe32.dwSize = sizeof(PROCESSENTRY32);

	if(!Process32First(snapshot, &pe32)) {
		CloseHandle(snapshot);
		return FALSE;
	}

	do {
		if(pe32.th32ParentProcessID == processId) {
			CloseHandle(snapshot);
			*childId = pe32.th32ProcessID;
			return TRUE;
		}
	}
	while(Process32Next(snapshot, &pe32));

	CloseHandle(snapshot);
	return FALSE;
}

BOOL CALLBACK AL_Window_Callback(HWND hWnd, LONG lParam) {
    if (IsWindowVisible(hWnd)) {
    	DWORD testId, childId;

		GetWindowThreadProcessId(hWnd, &testId);
		BOOL foundChild = AL_FindChild(&childId);
		
		if (testId == processId || (foundChild && testId == childId)) {
			Sleep(delay);
			AL_SendInfo(hWnd);
			exit(0);
		}
	}
    return TRUE;
}

int main(int argc, char *argv[])
{
	if (argc != 5) {
		printf("Usage: %s GAMEPATH ACCOUNT PASSWORD DELAY_MS\n", argv[0]);
		return 1;
	}

	STARTUPINFO si;
	ZeroMemory(&si, sizeof(STARTUPINFO));
	si.cb = sizeof(STARTUPINFO);

	PROCESS_INFORMATION pi;
	ZeroMemory(&pi, sizeof(PROCESS_INFORMATION));

	BOOL bResult = CreateProcess(argv[1],
					argv[1],
					NULL,
					NULL,
					FALSE,
					0,
					NULL,
					NULL,
					&si,
					&pi);

	if (!bResult) {
		printf("Failed to start WoW.\n");
		return 1;
	}

	processId = pi.dwProcessId;
	account = argv[2];
	password = argv[3];
	delay = atoi(argv[4]);

	for (;;) {
		Sleep(200);
		EnumWindows(AL_Window_Callback, 0);
	}

	return 0;
}
