#include "stdafx.h"
#include <Windows.h>
#include "BasicInjector.h"

int main()
{
	// Reserve MAX_PATH space for the full path of hook.dll
	wchar_t path[MAX_PATH];

	// Create an instance of the BasicInjector
	BasicInjector bi = BasicInjector();

	// Obtain the full path from the relative path to hook.dll
	if (GetFullPathName(L"hook.dll", MAX_PATH, path, NULL)) {

		// Find the first PID for notepad.exe
		DWORD pid = bi.GetPIDFromName(L"notepad.exe");
		printf("notepad.exe pid: %d\r\n", pid);
		if (pid) {
			// Inject our DLL into that PID
			HMODULE library = bi.Inject(pid, path);
			printf("library handle: %d\r\n", library);
			if (!library) { // if library != 0 then our code is now being run in notepad.exe's process!
				printf("injection failed\r\n");
			}
		}
	}

    return 0;
}

