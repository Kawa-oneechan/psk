#pragma once

#include <string>
#include <sstream>
#include <map>
#include <algorithm>

//Supported game languages.
typedef enum
{
	USen, USes, USfr,
	JPja, KRko, CNzh, TWzh,
	EUde, EUen, EUes, EUfr, EUit, EUnl, EUru, EUhu,
	DontCare, Default, Unknown
} Language;

//Language used by TextGet.
extern Language gameLang;

extern int playerGender; //for testing only

typedef enum
{
	Integer, String, ConstInt
} TextCondVarType;

typedef struct
{
	TextCondVarType type;
	union
	{
		void* variable;
		int constant;
	};
} textCondVar;

extern std::map<std::string, textCondVar> theVars;

typedef struct
{
	std::string rep;
	std::map<Language, std::string> text;
	std::string condition;
	std::string ifTrue, ifElse;

	std::string get(Language lang);
	std::string get();
} TextEntry;

//Adds a JSONObject full of localized strings to the string database.
//The JSONObject must map strings and *only* strings to language IDs.
extern TextEntry& TextAdd(std::string& key, JSONObject& map);
//Adds a presumably English string to the string database.
extern TextEntry& TextAdd(std::string& key, const std::string& english);
//Adds a JSONValue that can be an object *or* a string to the string database.
//See TextAdd(std::string& key, JSONObject& map) for details.
extern TextEntry& TextAdd(std::string& key, JSONValue& value);
//Adds a presumably English string to the string database.
extern TextEntry& TextAdd(const char* key, const char* english);
//Adds maps or strings from a JSONObject to the string database.
extern void TextAdd(JSONValue& doc);
//Returns a localized string.
extern std::string TextGet(std::string& key, Language lang = Language::Default);
//Returns a localized string.
extern std::string TextGet(const char* key);
//Returns a localized date.
extern std::string TextDateMD(int month, int day);
//Splits a string into a vector of strings by the specified delimiter.
std::vector<std::string> Split(std::string& data, char delimiter);
//Changes a string's characters to lowercase, in place.
extern void StringToLower(std::string& data);
//Removes spaces from a string, in place.
extern void StripSpaces(std::string& data);
//Removes MSBT tags from a string.
extern std::string StripMSBT(const std::string& data);
