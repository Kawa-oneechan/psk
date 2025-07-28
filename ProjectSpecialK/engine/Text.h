#pragma once

#include <string>
#include <sstream>
#include <map>
#include <algorithm>
#include "JsonUtils.h"

//Supported game languages.
enum Language
{
	USen, USes, USfr,
	JPja, KRko, CNzh, TWzh,
	EUde, EUen, EUes, EUfr, EUit, EUnl, EUru, EUhu,
	DontCare, Default, Unknown
};

//Language used by Get.
extern Language gameLang;

namespace Text
{
	struct Entry
	{
		std::string rep;
		std::string text;
		std::string condition;
		std::string ifTrue, ifElse;

		std::string get();
	};

	Language GetLangCode(const std::string& lang);
	std::string GetLangCode(Language lang = Language::Default);

	//Adds a presumably English string to the string database.
	extern Entry& Add(const std::string& key, const std::string& english);
	//Adds a JSONValue that can be an object *or* a string to the string database.
	//See Add(std::string& key, JSONObject& map) for details.
	extern Entry& Add(const std::string& key, jsonValue& value);
	//Removes every key starting with the given prefix.
	extern void Forget(const std::string& ns);
	//Adds maps or strings from a JSONValue to the string database.
	//This deletes the original JSONValue afterwards - do NOT delete it yourself.
	extern void Add(jsonValue& doc);
	//Returns a localized string.
	extern std::string Get(std::string key);
	//Returns a localized date.
	extern std::string DateMD(int month, int day);
	//Returns the number of entries starting with the given key.
	extern size_t Count(const std::string& key);
}
