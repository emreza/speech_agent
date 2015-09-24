
#include <sapi.h>;
#include <strsafe.h>;

typedef int (*CallbackType)(char * text, int option);


static CallbackType listenCallback = 0;

extern "C" {
	__declspec(dllexport) int _cdecl init(const char * options, CallbackType lCallback);
	__declspec(dllexport) void _cdecl stop();
}