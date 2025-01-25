#include "SpecialK.h"

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

std::string Text::Entry::get(Language lang)
{
	if (condition.size())
	{
		bool result = Sol.script("return (" + condition + ")");
		return Get(result ? ifTrue : ifElse);
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

std::string Text::Entry::get()
{
	return get(gameLang);
}

Text::Entry& Text::Add(const std::string& key, JSONObject& map)
{
	auto entry = new Entry();

	for (auto& langs : map)
	{
		if (langs.first == "condition")
		{
			entry->condition = langs.second->AsString();
			entry->ifTrue = map["true"]->AsString();
			entry->ifElse = map["false"]->AsString();
			break;
		}

		auto langEnum = GetLangCode(langs.first);
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
		entry->rep = entry->rep.substr(0, 16) + "...";
	entry->rep.shrink_to_fit();

	textEntries[key] = *entry;
	return *entry;
}

Text::Entry& Text::Add(const std::string& key, const std::string& english)
{
	auto map = JSONObject();
	map["USen"] = new JSONValue(english);
	return Add(key, map);
}

Text::Entry& Text::Add(const std::string& key, JSONValue& value)
{
	if (value.IsObject())
	{
		auto obj = value.AsObject();
		return Add(key, obj);
	}
	else if (value.IsString())
	{
		auto str = value.AsString();
		return Add(key, str);
	}
	throw "TextAdd<Value>: JSONValue is not an Object or String.";
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

void Text::Add(JSONValue& doc)
{
	for (auto& entry : doc.AsObject())
	{
		std::string& key = (std::string&)entry.first;
		auto map = entry.second->AsObject();
		Add(key, map);
	}
	delete &doc;
}

std::string Text::Get(std::string key, Language lang)
{
	auto oldLang = gameLang;
	if (lang != Language::Default)
		gameLang = lang;
	auto ret = std::string("???" + key + "???");
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

std::string Text::DateMD(int month, int day)
{
	auto format = Get("month:format"s);
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
