#include <stdlib.h>
#include "../Game.h"

#ifdef _WIN32
extern int main(int argc, char** argv);

namespace Platform
{
	extern "C" {

		//Normally, CharUpper/CharLower take either strings *or* characters, by checking the high bytes or sumth.
		//It's a hack, but we don't *want* to use it on entire strings anyway cos we're UTF8.
		int __stdcall CharUpperW(int lpsz);
		int __stdcall CharLowerW(int lpsz);

		int __stdcall MessageBoxW(void* hWnd, const wchar_t* lpText, const wchar_t* lpCaption, unsigned int uType);
		int __stdcall MultiByteToWideChar(unsigned int CodePage, unsigned long dwFlags, const char* lpMultiByteStr, int cbMultiByte, wchar_t* lpWideCharStr, int cchWideChar);
	}

	int CharUpper(int rune)
	{
		return CharUpperW(rune);
	}

	int CharLower(int rune)
	{
		return CharLowerW(rune);
	}

	void MessageBox(const std::string& message)
	{
		wchar_t w[1024] = { 0 };
		MultiByteToWideChar(65001, 0, message.c_str(), -1, w, 1024);
		MessageBoxW(nullptr, w, L"" BECKETT_GAMENAME, 0x30);
	}
}

int __stdcall WinMain(struct HINSTANCE__*, struct HINSTANCE__*, char*, int)
{
	return main(__argc, __argv);
};

#else
#pragma error("Why are you compiling PlatformWin32 on whatever this is?");
//Note that PlatformWhatever.cpp should include CharUpper/Lower.
#endif
