#include "SpecialK.h"

//#define JUSTTHEONELANGUAGE Language::USen

Language gameLang = Language::EUhu;

static std::map<std::string, TextEntry> textEntries;

std::map<std::string, textCondVar> theVars = std::map<std::string, textCondVar>();

static Language LangStrToEnum(const std::string& lang)
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
		{ "ja", JPja },{ "ko", KRko },{ "zh", TWzh },
		{ "de", EUde },{ "it", EUit },{ "nl", EUnl },{ "ru", EUru },{ "hu", EUhu }
	});
	auto match = map.find(lang);
	if (match != map.end())
		return match->second;
	return USen;
}

std::string TextCondition(std::string condition, std::string ifTrue, std::string ifElse, Language lang);

std::string TextEntry::get(Language lang)
{
	if (condition.size())
		return TextCondition(condition, ifTrue, ifElse, lang);

	auto t = text.find(lang);
	if (t != text.end())
		return t->second;
	//Couldn't find it? Fall back.
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

TextEntry& TextAdd(std::string& key, JSONObject& map)
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
#ifdef JUSTTHEONELANGUAGE
		if (langEnum != JUSTTHEONELANGUAGE)
			continue;
#endif
		entry->text[langEnum] = langs.second->AsString();
#ifdef JUSTTHEONELANGUAGE
		if (langEnum == JUSTTHEONELANGUAGE)
			break;
#endif
	}

	textEntries[key] = *entry;
	return *entry;
}

TextEntry& TextAdd(std::string& key, const std::string& english)
{
	auto map = new JSONObject();
	auto p = map->insert(map->begin(), std::pair<std::string, JSONValue*>("USen", new JSONValue(english)));
	return TextAdd(key, *map);
}

TextEntry& TextAdd(std::string& key, JSONValue& value)
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

TextEntry& TextAdd(const char* key, const char* english)
{
	return TextAdd(std::string(key), std::string(english));
}

void TextAdd(JSONValue& doc)
{
	for (auto& entry : doc.AsObject())
	{
		std::string& key = (std::string&)entry.first;
		auto map = entry.second->AsObject();
		TextAdd(key, map);
	}
}

std::string TextGet(std::string& key, Language lang)
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

std::string TextGet(const char* key)
{
	return TextGet(std::string(key));
}

std::string TextDateMD(int month, int day)
{
	auto format = TextGet("month:format");
	std::string ret;
	for (int i = 0; i < format.length(); i++)
	{
		if (format[i] == '%')
		{
			i++;
			if (format[i] == 'm')
			{
				ret += TextGet("month:" + std::to_string(month));
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

static std::tuple<int, std::string, int> parseValue(const std::string& token)
{
	auto type = TextCondVarType::Integer;
	int val = 0;
	std::string str;
	if (isdigit(token[0]))
	{
		type = TextCondVarType::Integer;
		val = std::stoi(token);
	}
	else if (token[0] == '"')
	{
		type = TextCondVarType::String;
		str = token.substr(1, token.length() - 2);
	}
	else if (theVars.find(token) != theVars.end())
	{
		auto second = theVars.find(token)->second;
		type = second.type;
		if (type == TextCondVarType::Integer)
			val = *(int*)second.variable;
		else if (type == TextCondVarType::String)
			str = *(std::string*)second.variable;
		else if (type == TextCondVarType::ConstInt)
		{
			val = second.constant;
			type = TextCondVarType::Integer;
		}
	}

	return{ val, str, type };
	//return std::make_tuple(val, str, type);
}

static std::string TextCondition(std::string condition, std::string ifTrue, std::string ifElse, Language lang)
{
	auto tokens = Split(condition, ' ');
	auto verdict = false;
	auto logicalAnd = false;

	for (int i = 0; i < tokens.size(); i++)
	{
		auto& theVar = tokens[i++];
		auto& theTest = tokens[i++];
		auto& theVal = tokens[i];

		auto lValue = parseValue(theVar);
		auto rValue = parseValue(theVal);

		auto verdictSoFar = false;
		if (std::get<2>(lValue) != std::get<2>(rValue))
		{
			fmt::print("TextCond: vals aren't same type ({} vs {})\n", std::get<2>(lValue), std::get<2>(rValue));
			break;
		}
		if (std::get<2>(lValue) == TextCondVarType::Integer)
		{
			auto l = std::get<0>(lValue);
			auto r = std::get<0>(rValue);
			if (theTest == "=") verdictSoFar = l == r;
			else if (theTest == "!=") verdictSoFar = l != r;
			else if (theTest == "<") verdictSoFar = l < r;
			else if (theTest == ">") verdictSoFar = l > r;
			else if (theTest == "<=") verdictSoFar = l <= r;
			else if (theTest == "=>") verdictSoFar = l >= r;
		}
		else if (std::get<2>(lValue) == TextCondVarType::String)
		{
			auto l = std::get<1>(lValue);
			auto r = std::get<1>(rValue);
			if (theTest == "=") verdictSoFar = l == r;
			else if (theTest == "!=") verdictSoFar = l != r;
			else if (theTest == "startsWith") verdictSoFar = (l.length() >= r.length() && l.substr(0, r.length()) == r);
		}

		if (i + 1 < tokens.size())
		{
			if (tokens[i + 1] == "|")
			{
				if (verdictSoFar)
				{
					verdict = true;
					break;
				}
				else
				{
					i++;
					continue;
				}
			}

			if (tokens[i + 1] == "&")
			{
				if (!verdictSoFar)
					return TextGet(ifElse, lang);
				i++;
				verdict = verdictSoFar; //-V547
				logicalAnd = true;
				continue;
			}
		}
		else
		{
			if (logicalAnd && (!verdict))
				verdictSoFar = false;
			verdict = verdictSoFar;
		}
	}
	return TextGet(verdict ? ifTrue : ifElse, lang);
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

std::vector<std::string> Split(std::string& data, char delimiter)
{
	std::vector<std::string> ret;
	std::string part;
	std::istringstream stream(data);
	while (std::getline(stream, part, delimiter))
		ret.push_back(part);
	return ret;
}
