#include "BasicInjector.h"
#include <Windows.h>
#include <TlHelp32.h>
#include <Processthreadsapi.h>
#include <stdio.h>

DWORD BasicInjector::GetPIDFromName(const LPCTSTR name) {
	DWORD pid = 0;
	HANDLE hSnapshot;
	PROCESSENTRY32 pi;
	BOOL p32res;

	hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hSnapshot != INVALID_HANDLE_VALUE) {
		pi.dwSize = sizeof(PROCESSENTRY32);
		p32res = Process32First(hSnapshot, &pi);

		while (p32res) {
			if (strcomparei(pi.szExeFile, name) == 0) {
				pid = pi.th32ProcessID;
				break;
			}

			p32res = Process32Next(hSnapshot, &pi);
		}
	}

	return pid;
}

DWORD BasicInjector::CountPIDFromName(const LPCTSTR name) {
	DWORD count = 0;
	HANDLE hSnapshot;
	PROCESSENTRY32 pi;
	BOOL p32res;

	hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hSnapshot != INVALID_HANDLE_VALUE) {
		pi.dwSize = sizeof(PROCESSENTRY32);
		p32res = Process32First(hSnapshot, &pi);

		while (p32res) {
			if (strcomparei(pi.szExeFile, name) == 0) {
				count++;
			}

			p32res = Process32Next(hSnapshot, &pi);
		}
	}

	return count;
}

DWORD BasicInjector::GetPIDListFromName(const LPCTSTR name, DWORD list[]) {
	DWORD count = 0;
	HANDLE hSnapshot;
	PROCESSENTRY32 pi;
	BOOL p32res;

	hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hSnapshot != INVALID_HANDLE_VALUE) {
		pi.dwSize = sizeof(PROCESSENTRY32);
		p32res = Process32First(hSnapshot, &pi);

		while (p32res) {
			if (strcomparei(pi.szExeFile, name) == 0) {
				list[count] = pi.th32ProcessID;
				count++;
			}

			p32res = Process32Next(hSnapshot, &pi);
		}
	}

	return count;
}

void BasicInjector::SetDebugPrivileges(BOOL state) {
	TOKEN_PRIVILEGES tk;
	HANDLE hToken;

	// Get current process token
	if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY | TOKEN_READ, &hToken))
		return;

	if (!LookupPrivilegeValue(NULL, L"SeDebugPrivilege", &tk.Privileges[0].Luid)) {
		CloseHandle(hToken);
		return;
	}
		
	// Modify token
	tk.PrivilegeCount = 1;
	tk.Privileges[0].Attributes = (state ? SE_PRIVILEGE_ENABLED : NULL);
		
	// Adjust token privileges
	AdjustTokenPrivileges(hToken, NULL, &tk, NULL, NULL, NULL);

	CloseHandle(hToken);
}

HMODULE BasicInjector::Inject(DWORD processId, const LPCTSTR szDllPath) {
	HMODULE hLibModule = NULL;
	HANDLE hThread, hProcess;
	LPVOID pLibraryRemote;
	
	SetDebugPrivileges(true);

	hProcess = OpenProcess(PROCESS_ALL_ACCESS, false, processId);
	if (hProcess != NULL) {
		pLibraryRemote = VirtualAllocEx(hProcess, NULL, MAX_PATH, MEM_COMMIT, PAGE_READWRITE);
		if (pLibraryRemote) {
			if (WriteProcessMemory(hProcess, pLibraryRemote, szDllPath, (strlenx(szDllPath) + 1) * soc, NULL)) {
				hThread = CreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)GetProcAddress(GetModuleHandle(L"kernel32"), LOADLIBRARY), pLibraryRemote, 0, NULL);
				if (hThread != NULL) {
					WaitForSingleObject(hThread, INFINITE);
					GetExitCodeThread(hThread, (LPDWORD)&hLibModule);
					CloseHandle(hThread);
				}
			}


			VirtualFreeEx(hProcess, pLibraryRemote, MAX_PATH, MEM_RELEASE);
		}

		CloseHandle(hProcess);
	}

	SetDebugPrivileges(false);

	return hLibModule;
}

void BasicInjector::Eject(DWORD processId, HMODULE module) {
	HANDLE hThread, hProcess;
	
	SetDebugPrivileges(true);

	hProcess = OpenProcess(PROCESS_ALL_ACCESS, false, processId);
	if (hProcess) {
		hThread = CreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)GetProcAddress(GetModuleHandle(L"kernel32"), "FreeLibrary"), module, 0, NULL);
		if (hThread) {
			WaitForSingleObject(hThread, INFINITE);
			CloseHandle(hThread);
		}

		CloseHandle(hProcess);
	}

	SetDebugPrivileges(false);
}