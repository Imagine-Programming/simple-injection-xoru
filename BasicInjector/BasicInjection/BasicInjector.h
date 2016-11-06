#pragma once
#include <Windows.h>

#ifdef UNICODE
#define LOADLIBRARY "LoadLibraryW"
#define strcomparei lstrcmpi
#define strlenx lstrlen
#define soc 2
#else 
#define LOADLIBRARY "LoadLibraryA"
#define strcomparei strcmpi
#define strlenx strlen
#define soc 1
#endif 

class BasicInjector {
	public:
		DWORD GetPIDFromName(const LPCTSTR name);
		DWORD CountPIDFromName(const LPCTSTR name);
		DWORD GetPIDListFromName(const LPCTSTR name, DWORD list[]);
		void SetDebugPrivileges(BOOL state);

		HMODULE Inject(DWORD processId, const LPCTSTR szDllPath);
		void Eject(DWORD processId, HMODULE module);
};