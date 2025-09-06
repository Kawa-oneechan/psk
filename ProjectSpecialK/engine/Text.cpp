#include <string>
#include <format.h>
#include <sol.hpp>
#include "Text.h"
#include "TextUtils.h"

using namespace std::literals;

extern sol::state Sol;

Language gameLang = Language::USen;

static std::map<std::string, Text::Entry> textEntries;

Language Text::GetLangCode(const std::string &lang)
{
	const auto map = std::map<std::string, Language>(
	{
		//Proper
		{ "USen", USen },{ "USes", USes },{ "USfr", USfr },
		{ "JPja", JPja },{ "KRko", KRko },{ "CNzh", CNzh },{ "TWzh", TWzh },
		{ "EUde", EUde },{ "EUen", EUen },{ "EUes", EUes },{ "EUfr", EUfr },
		{ "EUit", EUit },{ "EUnl", EUnl },{ "EUru", EUru },{ "EUhu", EUhu },
		//Lower
		{ "usen", USen },{ "uses", USes },{ "usfr", USfr },
		{ "jpja", JPja },{ "krko", KRko },{ "cnzh", CNzh },{ "twzh", TWzh },
		{ "eude", EUde },{ "euen", EUen },{ "eues", EUes },{ "eufr", EUfr },
		{ "euit", EUit },{ "eunl", EUnl },{ "euru", EUru },{ "euhu", EUhu },
		//Shorter (matches fallbacks in Entry::get)
		{ "en", USen },{ "es", USes },{ "fr", USfr },
		{ "ja", JPja },{ "jp", JPja },{ "ko", KRko },{ "zh", TWzh },
		{ "de", EUde },{ "it", EUit },{ "nl", EUnl },{ "ru", EUru },{ "hu", EUhu },
		{ "uk", EUen }
	});
	auto match = map.find(lang);
	if (match != map.end())
		return match->second;
	return Unknown;
}

std::string Text::GetLangCode(Language lang)
{
	const auto map = std::map<Language, std::string>(
	{
		{ USen, "USen" },
		{ USes, "USes" },
		{ USfr, "USfr" },
		{ JPja, "JPja" },
		{ KRko, "KRko" },
		{ CNzh, "CNzh" },
		{ TWzh, "TWzh" },
		{ EUde, "EUde" },
		{ EUen, "EUen" },
		{ EUes, "EUes" },
		{ EUfr, "EUfr" },
		{ EUit, "EUit" },
		{ EUnl, "EUnl" },
		{ EUru, "EUru" },
		{ EUhu, "EUhu" },
	});
	if (lang == Default)
		lang = gameLang;
	return map.at(lang);
}

std::string Text::Entry::get()
{
	if (condition.size())
	{
		bool result = Sol.script("return (" + condition + ")");
		return Get(result ? ifTrue : ifElse);
	}

	return text;
}

Text::Entry& Text::Add(const std::string& key, jsonValue& value)
{
	if (value.is_string())
	{
		auto map = json5pp::object({ { GetLangCode(gameLang), value.as_string() } });
		return Add(key, map);
	}

	if (!value.is_object())
	{
		throw "TextAdd<Value>: JSONValue is not an Object or String.";
	}

	auto entry = new Entry();
	auto map = value.as_object();

	for (auto& langs : map)
	{
		if (langs.first == "condition")
		{
			entry->condition = langs.second.as_string();
			entry->ifTrue = map["true"].as_string();
			entry->ifElse = map["false"].as_string();
			break;
		}

		auto langEnum = GetLangCode(langs.first);
		if (langEnum == gameLang || langEnum == Language::USen)
			entry->text = langs.second.as_string();
	}

	if (entry->condition.empty())
		entry->rep = entry->get();
	else
		entry->rep = entry->condition;
	if (entry->rep.length() > 16)
		entry->rep = StripBJTS(entry->rep);
	if (entry->rep.length() > 16)
		entry->rep = entry->rep.substr(0, 16) + "...";
	entry->rep.shrink_to_fit();

	textEntries[key] = *entry;
	return *entry;
}

Text::Entry& Text::Add(const std::string& key, const std::string& english)
{
	auto map = json5pp::object({ { GetLangCode(gameLang), english } });
	return Add(key, map);
}

void Text::Forget(const std::string& ns)
{
	auto nsl = ns.length();
	while (true)
	{
		auto e = textEntries.begin();
		auto any = false;
		while (e != textEntries.end())
		{
			auto key = e->first;
			if (key.length() > nsl && key.substr(0, nsl) == ns)
			{
				textEntries.erase(e);
				any = true;
				break;
			}
			++e;
		}
		if (!any)
			break;
	}
}

void Text::Add(jsonValue& doc)
{
	for (auto& entry : doc.as_object())
	{
		std::string& key = (std::string&)entry.first;
		auto& map = entry.second;
		Add(key, map);
	}
}

std::string Text::Get(std::string key)
{
	for (const auto& entry : textEntries)
	{
		if (entry.first == key)
		{
			auto e = entry.second;
			return e.get();
		}
	}
	return std::string("???" + key + "???");
}

std::string Text::DateMD(int month, int day)
{
	auto format = Get("month:format");
	std::string ret;
	for (int i = 0; i < format.length(); i++)
	{
		if (format[i] == '%')
		{
			i++;
			if (format[i] == 'm')
			{
				ret += Get(fmt::format("month:{}", std::to_string(month)));
			}
			else if (format[i] == 'd')
			{
				ret += std::to_string(day);
			}
			else if (format[i] == '%')
			{
				ret += '%';
			}
		}
		else
			ret += format[i];
	}
	return ret;
}

size_t Text::Count(const std::string& key)
{
	const auto kl = key.length();

	size_t ret = 0;

	for (const auto& entry : textEntries)
	{
		if (entry.first[0] != key[0])
		{
			if (ret != 0)
				return ret;
			continue;
		}

		const auto e = entry.first;
		const auto el = e.length();
		if (el < kl || e.substr(0, kl) != key)
		{
			if (ret != 0)
				return ret;
			continue;
		}
		ret++;
	}
	return ret;
}
