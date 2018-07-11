#include "crashdump.h"
#include "globals.h"
#include <dbghelp.h>
#include <shlobj.h>
#include <tchar.h>
#include <time.h>
#include <TlHelp32.h>
#include <memory>

LONG WINAPI UnhandledExFilter(PEXCEPTION_POINTERS ExPtr)
{
	BOOL(WINAPI* pMiniDumpWriteDump)(IN HANDLE hProcess, IN DWORD ProcessId, IN HANDLE hFile, IN MINIDUMP_TYPE DumpType, IN CONST PMINIDUMP_EXCEPTION_INFORMATION ExceptionParam, OPTIONAL IN CONST PMINIDUMP_USER_STREAM_INFORMATION UserStreamParam, OPTIONAL IN CONST PMINIDUMP_CALLBACK_INFORMATION CallbackParam OPTIONAL) = NULL;

	HMODULE hLib = LoadLibrary(_T("dbghelp"));
	if (hLib)
		*(void**)&pMiniDumpWriteDump = (void*)GetProcAddress(hLib, "MiniDumpWriteDump");

	TCHAR buf[MAX_PATH], buf2[MAX_PATH];

	if (pMiniDumpWriteDump) {
		//SHGetFolderPath(NULL, CSIDL_DESKTOPDIRECTORY, NULL, SHGFP_TYPE_CURRENT, buf);
		//int rnd;
		//__asm push edx
		//__asm rdtsc
		//__asm pop edx
		//__asm mov rnd, eax
		//rnd &= 0xFFFF;
		//wsprintf(buf2, _T("%s\\Crash_%x.dmp"), buf, rnd);

		time_t timer;
		char timebuffer[26];
		struct tm* tm_info;
		time(&timer);
		tm_info = localtime(&timer);
		strftime(timebuffer, 26, "%Y%m%d%H%M%S", tm_info);
		//convert from char* to wchar_t*
		size_t newsize = strlen(timebuffer) + 1;
		wchar_t * wtimestring = new wchar_t[newsize]; 
		size_t convertedChars = 0;
		mbstowcs_s(&convertedChars, wtimestring, newsize, timebuffer, _TRUNCATE);

		wsprintf(buf2, _T("BBTAG_IM\\Crash_%s.dmp"), wtimestring);
		delete[] wtimestring;

		HANDLE hFile = CreateFile(buf2, GENERIC_READ | GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

		if (hFile != INVALID_HANDLE_VALUE) 
		{
			MINIDUMP_EXCEPTION_INFORMATION md;
			md.ThreadId = GetCurrentThreadId();
			md.ExceptionPointers = ExPtr;
			md.ClientPointers = FALSE;
			BOOL win = pMiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), hFile, MiniDumpNormal, &md, 0, 0);

			if (!win)
				wsprintf(buf, _T("MiniDumpWriteDump failed. Error: %u \n(%s)"), GetLastError(), buf2);
			else
				wsprintf(buf, _T("Minidump created:\n%s"), buf2);
			CloseHandle(hFile);

		}
		else 
		{
			wsprintf(buf, _T("Could not create minidump:\n%s"), buf2);
		}
	}
	else 
	{
		wsprintf(buf, _T("Could not load dbghelp"));
	}
	MessageBox(NULL, buf, _T("Unhandled exception"), MB_OK | MB_ICONERROR);
	ExitProcess(0);    //do whatever u want here
}