#include "SpecialK.h"

Language gameLang = Language::USen;

static std::map<std::string, TextEntry> textEntries;

static Language LangStrToEnum(const std::string &lang)
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
		//Shorter (matches fallbacks in TextEntry::get)
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

std::string TextEntry::get(Language lang)
{
	if (condition.size())
	{
		bool result = Sol.script(fmt::format("return ({})", condition));
		return TextGet(result ? ifTrue : ifElse);
	}

	auto t = text.find(lang);
	if (t != text.end())
		return t->second;

	switch (lang)
	{
	case USen: return "<404>";
	case EUes: return get(USes);
	case EUfr: return get(USfr);
	case TWzh: return get(CNzh);
	default: return get(USen);
	}
}

std::string TextEntry::get()
{
	return get(gameLang);
}

TextEntry& TextAdd(std::string& key, JSONObject& map) //-V813
{
	auto entry = new TextEntry();

	for (auto& langs : map)
	{
		if (langs.first == "condition")
		{
			entry->condition = langs.second->AsString();
			entry->ifTrue = map["true"]->AsString();
			entry->ifElse = map["false"]->AsString();
			break;
		}

		auto langEnum = LangStrToEnum(langs.first);
		if (langEnum == Unknown)
			continue;
		entry->text[langEnum] = langs.second->AsString();
	}

	if (entry->condition.empty())
		entry->rep = entry->get(Language::USen);
	else
		entry->rep = entry->condition;
	if (entry->rep.length() > 16)
		entry->rep = StripMSBT(entry->rep);
	if (entry->rep.length() > 16)
		entry->rep = entry->rep.substr(0, 16) + "..."s;
	entry->rep.shrink_to_fit();

	textEntries[key] = *entry;
	return *entry;
}

TextEntry& TextAdd(const std::string& key, const std::string& english) //-V813
{
	auto map = new JSONObject();
	auto p = map->insert(map->begin(), std::pair<std::string, JSONValue*>("USen", new JSONValue(english)));
	return TextAdd(key, *map);
}

TextEntry& TextAdd(std::string& key, JSONValue& value) //-V813
{
	if (value.IsObject())
	{
		auto obj = value.AsObject();
		return TextAdd(key, obj);
	}
	else if (value.IsString())
	{
		auto str = value.AsString();
		return TextAdd(key, str);
	}
	throw "TextAdd<Value>: JSONValue is not an Object or String.";
}

/*
TextEntry& TextAdd(const char* key, const char* english)
{
	return TextAdd(std::string(key), std::string(english));
}
*/

void TextAdd(JSONValue& doc)
{
	for (auto& entry : doc.AsObject())
	{
		std::string& key = (std::string&)entry.first;
		auto map = entry.second->AsObject();
		TextAdd(key, map);
	}
}

std::string TextGet(const std::string key, Language lang)
{
	auto oldLang = gameLang;
	if (lang != Language::Default)
		gameLang = lang;
	auto ret = fmt::format("?{}?", key);
	for (const auto& entry : textEntries)
	{
		if (entry.first == key)
		{
			auto e = entry.second;
			ret = e.get();
			break;
		}
	}
	gameLang = oldLang;
	return ret;
}

/*
std::string TextGet(const char* key)
{
	return TextGet(std::string(key));
}
*/

std::string TextDateMD(int month, int day)
{
	auto format = TextGet("month:format"s);
	std::string ret;
	for (int i = 0; i < format.length(); i++)
	{
		if (format[i] == '%')
		{
			i++;
			if (format[i] == 'm')
			{
				ret += TextGet(fmt::format("month:{}", std::to_string(month)));
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

void StringToLower(std::string& data)
{
	std::transform(data.begin(), data.end(), data.begin(), [](unsigned char c) { return tolower(c); });
}

void StripSpaces(std::string& data)
{
	while (data.find(' ') != -1)
		data.erase(std::find(data.begin(), data.end(), ' '));
}

std::string StripMSBT(const std::string& data)
{
	std::string ret = data;
	size_t msbtStart;
	while ((msbtStart = ret.find_first_of('<', 0)) != std::string::npos)
	{
		auto msbtEnd = ret.find_first_of('>', msbtStart);
		ret.replace(msbtStart, msbtEnd - msbtStart + 1, "");
	}
	return ret;
}

std::vector<std::string> Split(const std::string& data, char delimiter)
{
	std::vector<std::string> ret;
	std::string part;
	std::istringstream stream(data);
	while (std::getline(stream, part, delimiter))
		ret.push_back(part);
	return ret;
}
