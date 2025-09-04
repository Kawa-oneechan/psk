#include <stdlib.h>

#ifdef _WIN32
extern int main(int argc, char** argv);
extern "C" int __stdcall WinMain(struct HINSTANCE__*, struct HINSTANCE__*, char*, int) { return main(__argc, __argv); };
#else
#pragma error("Why are you compiling PlatformWin32 on whatever this is?");
#endif
