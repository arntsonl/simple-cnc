#include "helper.h"

#include <VersionHelpers.h>

// RTL_OSVERSIONINFOEXW is defined in winnt.h
BOOL GetOsVersion(RTL_OSVERSIONINFOEXW* pk_OsVer)
{
	typedef LONG(WINAPI* tRtlGetVersion)(RTL_OSVERSIONINFOEXW*);

	memset(pk_OsVer, 0, sizeof(RTL_OSVERSIONINFOEXW));
	pk_OsVer->dwOSVersionInfoSize = sizeof(RTL_OSVERSIONINFOEXW);

	HMODULE h_NtDll = GetModuleHandleW(L"ntdll.dll");
	tRtlGetVersion f_RtlGetVersion = (tRtlGetVersion)GetProcAddress(h_NtDll, "RtlGetVersion");

	if (!f_RtlGetVersion)
		return FALSE; // This will never happen (all processes load ntdll.dll)

	LONG Status = f_RtlGetVersion(pk_OsVer);
	return Status == 0; // STATUS_SUCCESS;
}
