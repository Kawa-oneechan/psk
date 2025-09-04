#pragma once

#ifdef _WIN32
extern "C"
{
	//why bother including windows headers lol

	int __stdcall WinMain(struct HINSTANCE__*, struct HINSTANCE__*, char*, int);

	wchar_t* __stdcall CharNextW(const wchar_t* lpText);

	using GUID = struct _GUID
	{
		unsigned long  Data1;
		unsigned short Data2;
		unsigned short Data3;
		unsigned char  Data4[8];
	};
	const GUID FOLDERID_SavedGames = { 0x4C5C32FF, 0xBB9D, 0x43B0, 0xB5, 0xB4, 0x2D, 0x72, 0xE5, 0x4E, 0xAA, 0xA4 };
	const GUID FOLDERID_Documents = { 0xFDD39AD0, 0x238F, 0x46AF, 0xAD, 0xB4, 0x6C, 0x85, 0x48, 0x03, 0x69, 0xC7 };
	const GUID FOLDERID_Roaming = { 0x3EB685DB, 0x65F9, 0x4CF6, 0xA0, 0x3A, 0xE3, 0xEF, 0x65, 0x72, 0x9F, 0x3D };
	int __stdcall SHGetKnownFolderPath(const GUID* rfid, unsigned long dwFlags, void* hToken, wchar_t** ppszPath);
	int __stdcall WideCharToMultiByte(unsigned int CodePage, unsigned long dwFlags, const wchar_t* lpWideCharStr, int cchWideChar, char* lpMultiByteStr, int cbMultiByte, char* lpDefaultChar, bool* lpUsedDefaultChar);
	void _stdcall CoTaskMemFree(void* pv);

}
#endif

extern int CharUpper(int rune);
extern int CharLower(int rune);
extern void MessageBox(const std::string& message);
