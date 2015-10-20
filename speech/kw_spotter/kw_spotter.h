typedef int (*CallbackType)(char* utter_hyp);
static CallbackType listenCallback = 0;

extern "C"  __declspec(dllexport) int run(CallbackType callback, char* path);
extern "C"  __declspec(dllexport) void stop();