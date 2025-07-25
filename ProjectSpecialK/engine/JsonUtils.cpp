#include <algorithm>
#include <format.h>
#include "VFS.h"
#include "JsonUtils.h"
#include "TextUtils.h"

glm::vec2 GetJSONVec2(JSONValue* val)
{
	if (!val->IsArray())
		throw std::runtime_error(fmt::format("GetJSONVec2: given value {} is not an array.", val->Stringify()));
	auto arr = val->AsArray();
	if (arr.size() != 2)
		throw std::runtime_error(fmt::format("GetJSONVec2: given array {} has {} entries, not 2.", val->Stringify(), arr.size()));
	if (std::any_of(arr.cbegin(), arr.cend(), [](JSONValue* x) { return !x->IsNumber(); }))
		throw std::runtime_error(fmt::format("GetJSONVec2: given array {} does not contain only numbers.", val->Stringify()));
	return glm::vec2(arr[0]->AsNumber(), arr[1]->AsNumber());
}

glm::vec3 GetJSONVec3(JSONValue* val)
{
	if (!val->IsArray())
		throw std::runtime_error("GetJSONVec3: given value is not an array.");
	auto arr = val->AsArray();
	if (arr.size() != 3)
		throw std::runtime_error(fmt::format("GetJSONVec3: given array has {} entries, not 3.", arr.size()));
	if (std::any_of(arr.cbegin(), arr.cend(), [](JSONValue* x) { return !x->IsNumber(); }))
		throw std::runtime_error("GetJSONVec3: given array does not contain only numbers.");
	return glm::vec3(arr[0]->AsNumber(), arr[1]->AsNumber(), arr[2]->AsNumber());
}

glm::vec4 GetJSONVec4(JSONValue* val)
{
	if (!val->IsArray())
		throw std::runtime_error("GetJSONVec4: given value is not an array.");
	auto arr = val->AsArray();
	if (arr.size() != 4)
		throw std::runtime_error(fmt::format("GetJSONVec4: given array has {} entries, not 4.", arr.size()));
	if (std::any_of(arr.cbegin(), arr.cend(), [](JSONValue* x) { return !x->IsNumber(); }))
		throw std::runtime_error("GetJSONVec4: given array does not contain only numbers.");
	return glm::vec4(arr[0]->AsNumber(), arr[1]->AsNumber(), arr[2]->AsNumber(), arr[3]->AsNumber());
}

glm::vec4 GetJSONColor(JSONValue* val)
{
	if (val->IsString())
	{
		auto hex = val->AsString();
		int r = 0, g = 0, b = 0, a = 0;
		if (hex.empty() || hex[0] != '#')
			return glm::vec4(0, 0, 0, -1);
		//TODO: consider checking if the value is in UI::themeColors.
		//That way, colors in panel definitions can be hexcodes or float arrays too.
		if (hex.length() == 7)
		{
			a = 0xFF;
			r = std::stoi(hex.substr(1, 2), nullptr, 16);
			g = std::stoi(hex.substr(3, 2), nullptr, 16);
			b = std::stoi(hex.substr(5, 2), nullptr, 16);
		}
		else if (hex.length() == 9)
		{
			a = std::stoi(hex.substr(1, 2), nullptr, 16);
			r = std::stoi(hex.substr(3, 2), nullptr, 16);
			g = std::stoi(hex.substr(5, 2), nullptr, 16);
			b = std::stoi(hex.substr(7, 2), nullptr, 16);
		}
		else
			throw std::runtime_error(fmt::format("GetJSONColor: {} is not a valid color.", val->Stringify()));
		return glm::vec4(r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f);
	}
	if (val->IsArray())
	{
		auto arr = val->AsArray();
		for (auto x : arr)
			if (!x->IsNumber())
				throw std::runtime_error(fmt::format("GetJSONColor: {} is not a valid color.", val->Stringify()));
		float r, g, b, a;
		if (arr.size() == 3)
		{
			r = arr[0]->AsNumber();
			g = arr[1]->AsNumber();
			b = arr[2]->AsNumber();
			a = 1.0f;
		}
		else if (arr.size() == 4)
		{
			r = arr[0]->AsNumber();
			g = arr[1]->AsNumber();
			b = arr[2]->AsNumber();
			a = arr[3]->AsNumber();
		}
		else
			throw std::runtime_error(fmt::format("GetJSONColor: {} is not a valid color.", val->Stringify()));
		return glm::vec4(r, g, b, a);
	}
	throw std::runtime_error(fmt::format("GetJSONColor: {} is not a valid color.", val->Stringify()));
}

static glm::vec2 checkDate(glm::vec2 date)
{
	date[0] = (float)glm::clamp((int)date[0], 1, 31);
	date[1] = (float)glm::clamp((int)date[0], 1, 12);
	return date;
}
glm::vec2 GetJSONDate(JSONValue* val)
{
	if (val->IsArray())
		return checkDate(GetJSONVec2(val));
	if (val->IsString())
	{
		auto str = val->AsString();
		auto split = str.find_last_of(' ');
		if (split == str.npos)
			split = str.find_last_of('/');
		if (split == str.npos)
			throw std::runtime_error(fmt::format("GetJSONDate: value {} can't split on space or slash.", val->Stringify()));
		auto day = std::stoi(str.substr(split + 1));
		auto mon = str.substr(0, 3);
		StringToLower(mon);
		static const std::string names[] = { "jan", "feb", "mar", "apr", "may", "jun", "jul", "aug", "sep", "oct", "nov", "dec" };
		for (int i = 0; i < 12; i++)
		{
			if (names[i] == mon)
				return checkDate(glm::vec2(day, i + 1));
		}
	}
	throw std::runtime_error(fmt::format("GetJSONDate: value {} is not a month/day pair.", val->Stringify()));
}

JSONValue* GetJSONVec(const glm::vec2& vec, bool asInt)
{
	JSONArray ret;
	ret.reserve(2);
	if (asInt)
	{
		ret.push_back(new JSONValue((int)vec.x));
		ret.push_back(new JSONValue((int)vec.y));
	}
	else
	{
		ret.push_back(new JSONValue(vec.x));
		ret.push_back(new JSONValue(vec.y));
	}
	return new JSONValue(ret);
}

JSONValue* GetJSONVec(const glm::vec3& vec, bool asInt)
{
	JSONArray ret;
	ret.reserve(3);
	if (asInt)
	{
		for (int i = 0; i < 3; i++)
			ret.push_back(new JSONValue((int)vec[i]));
	}
	else
	{
		for (int i = 0; i < 3; i++)
			ret.push_back(new JSONValue(vec[i]));
	}
	return new JSONValue(ret);
}

JSONValue* GetJSONVec(const glm::vec4& vec, bool asInt)
{
	JSONArray ret;
	ret.reserve(4);
	if (asInt)
	{
		for (int i = 0; i < 4; i++)
			ret.push_back(new JSONValue((int)vec[i]));
	}
	else
	{
		for (int i = 0; i < 4; i++)
			ret.push_back(new JSONValue(vec[i]));
	}
	return new JSONValue(ret);
}

void GetAtlas(std::vector<glm::vec4> &ret, const std::string& jsonFile)
{
	auto rjs = VFS::ReadJSON(jsonFile);
	if (!rjs)
		return;
	auto doc = rjs->AsObject();
	ret.clear();
	if (doc["type"] == nullptr)
	{
		delete rjs;
		return;
	}

	if (doc["type"]->AsString() == "simple")
	{
		auto size = GetJSONVec2(doc["size"]);
		auto dims = GetJSONVec2(doc["dims"]);
		for (int y = 0; y < (int)dims[1]; y++)
		{
			for (int x = 0; x < (int)dims[0]; x++)
			{
				ret.push_back(glm::vec4(x * size[0], y * size[1], size[0], size[1]));
			}
		}
	}
	else if (doc["type"]->AsString() == "atlas")
	{
		auto rects = doc["rects"]->AsArray();
		for (const auto& rect : rects)
		{
			ret.push_back(GetJSONVec4(rect));
		}
	}
	else
		throw std::runtime_error(fmt::format("GetAtlas: file {} has an unknown type \"{}\".", jsonFile, doc["type"]->AsString()));
	delete rjs;
}
