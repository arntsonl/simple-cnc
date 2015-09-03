#define WIN32_LEAN_AND_MEAN

//#ifdef _MSC_VER
//#define _CRT_SECURE_NO_WARNINGS
//#endif

#include <windows.h>
#include <stdlib.h>     /* srand, rand */
#include <time.h>       /* time */
#include "messages.h"
#include "registry.h"

#ifdef _DEBUG
int main(int argc, char * argv[])
#else
int CALLBACK WinMain(
	_In_ HINSTANCE hInstance,
	_In_ HINSTANCE hPrevInstance,
	_In_ LPSTR     lpCmdLine,
	_In_ int       nCmdShow)
#endif
{
	srand(time(NULL));
	initRegistry();
	initMessages();
	int quit = 0;
	while (!quit) {
		Sleep(updateDecay); // wait 1 minute
		quit = runMessages(); // run our message interpreter
	}
	freeMessages();
}