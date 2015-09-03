#include "registry.h"

// Registry stuff
#include <Windows.h>
#include "install.h"

void initRegistry()
{
	// read our settings file from the registry
	HKEY hKey;
	LONG lRes = RegOpenKeyExA(HKEY_CURRENT_USER, "SOFTWARE\\SimpleCnC", 0, KEY_READ, &hKey);
	if (lRes != ERROR_SUCCESS)
	{
		DWORD dwDisposition;
		RegCreateKeyExA(HKEY_CURRENT_USER, "SOFTWARE\\SimpleCnC", 0, NULL, 0, KEY_WRITE, NULL, &hKey, &dwDisposition);
		RegCloseKey(hKey);
	}

	// Also generate a new install?
	generateInstallInfo();
}

char * readRegStr(char * key, char * def)
{
	// read our settings file from the registry
	HKEY hKey;
	LONG lRes = RegOpenKeyExA(HKEY_CURRENT_USER, "SOFTWARE\\SimpleCnC", 0, KEY_READ, &hKey);
	char * value = (char*)malloc(sizeof(char)*2048);
	DWORD valLen = sizeof(char)*2048;
	DWORD type;
	if (RegQueryValueExA(hKey, key, 0, &type, (LPBYTE)value, &valLen) == ERROR_SUCCESS)
	{
		RegCloseKey(hKey);
		return value;
	}
	return def;
}

void writeRegInt(char* key, int value)
{
	// read our settings file from the registry
	HKEY hKey;
	LONG lRes = RegOpenKeyExA(HKEY_CURRENT_USER, "SOFTWARE\\SimpleCnC", 0, KEY_READ, &hKey);
	RegSetValueExA(hKey, (LPCSTR)key, 0, REG_DWORD, (BYTE*)value, sizeof(DWORD));
	RegCloseKey(hKey);
}

void writeRegStr(char* key, char* value)
{
	HKEY hKey;
	LONG lRes = RegOpenKeyExA(HKEY_CURRENT_USER, "SOFTWARE\\SimpleCnC", 0, KEY_WRITE, &hKey);
	DWORD ret = RegSetValueExA(hKey, (LPCSTR)key, 0, REG_SZ, (BYTE*)value, strlen(value));
	RegCloseKey(hKey);
}
