#include "stdafx.h";
#include <sapi.h>;
#include <strsafe.h>;

typedef int (*CallbackType)(short* data, int dataSize, int option);


static CallbackType listenCallback = 0;

extern "C" {
	__declspec(dllexport) int _cdecl run(const char * options, bool kws, int words, int sntc, double waitTime, double vad, CallbackType lCallback);
	__declspec(dllexport) void _cdecl stop();
}