#include <algorithm>
#include <format.h>
#include "VFS.h"
#include "JsonUtils.h"
#include "TextUtils.h"

glm::vec2 GetJSONVec2(const jsonValue& val)
{
	if (!val.is_array())
		throw std::runtime_error(fmt::format("GetJSONVec2: given value {} is not an array.", val.stringify5()));
	auto arr = val.as_array();
	if (arr.size() != 2)
		throw std::runtime_error(fmt::format("GetJSONVec2: given array {} has {} entries, not 2.", val.stringify5(), arr.size()));
	if (std::any_of(arr.cbegin(), arr.cend(), [](jsonValue x) { return !x.is_number(); }))
		throw std::runtime_error(fmt::format("GetJSONVec2: given array {} does not contain only numbers.", val.stringify5()));
	return glm::vec2(arr[0].as_number(), arr[1].as_number());
}

glm::vec3 GetJSONVec3(const jsonValue& val)
{
	if (!val.is_array())
		throw std::runtime_error("GetJSONVec3: given value is not an array.");
	auto arr = val.as_array();
	if (arr.size() != 3)
		throw std::runtime_error(fmt::format("GetJSONVec3: given array has {} entries, not 3.", arr.size()));
	if (std::any_of(arr.cbegin(), arr.cend(), [](jsonValue x) { return !x.is_number(); }))
		throw std::runtime_error("GetJSONVec3: given array does not contain only numbers.");
	return glm::vec3(arr[0].as_number(), arr[1].as_number(), arr[2].as_number());
}

glm::vec4 GetJSONVec4(const jsonValue& val)
{
	if (!val.is_array())
		throw std::runtime_error("GetJSONVec4: given value is not an array.");
	auto arr = val.as_array();
	if (arr.size() != 4)
		throw std::runtime_error(fmt::format("GetJSONVec4: given array has {} entries, not 4.", arr.size()));
	if (std::any_of(arr.cbegin(), arr.cend(), [](jsonValue x) { return !x.is_number(); }))
		throw std::runtime_error("GetJSONVec4: given array does not contain only numbers.");
	return glm::vec4(arr[0].as_number(), arr[1].as_number(), arr[2].as_number(), arr[3].as_number());
}

glm::vec4 GetJSONColor(const jsonValue& val)
{
	if (val.is_string())
	{
		auto hex = val.as_string();
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
			throw std::runtime_error(fmt::format("GetJSONColor: {} is not a valid color.", val.stringify5()));
		return glm::vec4(r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f);
	}
	if (val.is_array())
	{
		auto arr = val.as_array();
		if (std::any_of(arr.cbegin(), arr.cend(), [](jsonValue x) { return !x.is_number(); }))
			throw std::runtime_error(fmt::format("GetJSONColor: {} is not a valid color.", val.stringify5()));
		float r, g, b, a;
		if (arr.size() == 3)
		{
			r = (float)arr[0].as_number();
			g = (float)arr[1].as_number();
			b = (float)arr[2].as_number();
			a = 1.0f;
		}
		else if (arr.size() == 4)
		{
			r = (float)arr[0].as_number();
			g = (float)arr[1].as_number();
			b = (float)arr[2].as_number();
			a = (float)arr[3].as_number();
		}
		else
			throw std::runtime_error(fmt::format("GetJSONColor: {} is not a valid color.", val.stringify5()));
		return glm::vec4(r, g, b, a);
	}
	throw std::runtime_error(fmt::format("GetJSONColor: {} is not a valid color.", val.stringify5()));
}

static glm::vec2 checkDate(glm::vec2 date)
{
	date[0] = (float)glm::clamp((int)date[0], 1, 31);
	date[1] = (float)glm::clamp((int)date[1], 1, 12);
	return date;
}
glm::vec2 GetJSONDate(const jsonValue& val)
{
	if (val.is_array())
		return checkDate(GetJSONVec2(val));
	if (val.is_string())
	{
		auto str = val.as_string();
		auto split = str.find_last_of(' ');
		if (split == str.npos)
			split = str.find_last_of('/');
		if (split == str.npos)
			throw std::runtime_error(fmt::format("GetJSONDate: value {} can't split on space or slash.", val.stringify5()));
		auto day = std::stoi(str.substr(split + 1));
		auto mon = str.substr(0, 3);
		StringToLower(mon);
		static const std::string names[] = { "jan", "feb", "mar", "apr", "may", "jun", "jul", "aug", "sep", "oct", "nov", "dec" };
		const auto& it = std::find_if(std::begin(names), std::end(names), [mon](const auto& e)
		{
			return e == mon;
		});
		if (it != std::end(names))
		{
			return checkDate(glm::vec2(day, (int)(it - std::begin(names)) + 1));
		}
	}
	throw std::runtime_error(fmt::format("GetJSONDate: value {} is not a month/day pair.", val.stringify5()));
}

jsonValue GetJSONVec(const glm::vec2& vec)
{
	return json5pp::array({
		vec.x, vec.y
	});
}

jsonValue GetJSONVecI(const glm::vec2& vec)
{
	return json5pp::array({
		(int)vec.x, (int)vec.y
	});
}

jsonValue GetJSONVec(const glm::vec3& vec)
{
	return json5pp::array({
		vec.x, vec.y, vec.z
	});
}

jsonValue GetJSONVecI(const glm::vec3& vec)
{
	return json5pp::array({
		(int)vec.x, (int)vec.y, (int)vec.z
	});
}

jsonValue GetJSONVec(const glm::vec4& vec)
{
	return json5pp::array({
		vec.x, vec.y, vec.z, vec.w
	});
}

jsonValue GetJSONVecI(const glm::vec4& vec)
{
	return json5pp::array({
		(int)vec.x, (int)vec.y, (int)vec.z, (int)vec.w
	});
}

void GetAtlas(SpriteAtlas &ret, const std::string& jsonFile)
{
	auto rjs = VFS::ReadJSON(jsonFile);
	if (!rjs)
		return;
	auto doc = rjs.as_object();
	ret.frames.clear();
	ret.names.clear();
	if (!doc["type"].is_string())
		return;

	if (doc["type"].as_string() == "simple")
	{
		auto size = GetJSONVec2(doc["size"]);
		auto dims = GetJSONVec2(doc["dims"]);
		auto padd = doc["padding"].is_integer() ? doc["padding"].as_integer() : 0;
		//TODO: also check size and dims
		if (padd < 0)
			throw std::runtime_error(fmt::format("GetAtlas: file {} has negative padding???", jsonFile));

		for (int y = 0; y < (int)dims[1]; y++)
		{
			for (int x = 0; x < (int)dims[0]; x++)
			{
				ret.frames.push_back(glm::vec4(
					padd + (x * (size[0] + padd)),
					padd + (y * (size[1] + padd)),
					size[0],
					size[1]
				));
			}
		}
	}
	else if (doc["type"].as_string() == "atlas")
	{
		auto rects = doc["rects"].as_array();
		for (const auto& rect : rects)
		{
			ret.frames.push_back(GetJSONVec4(rect));
		}
	}
	else
		throw std::runtime_error(fmt::format("GetAtlas: file {} has an unknown type \"{}\".", jsonFile, doc["type"].as_string()));

	if (doc["names"].is_array())
	{
		auto names = doc["names"].as_array();
		if (names.size() != ret.frames.size())
			throw std::runtime_error(fmt::format("GetAtlas: file {} has a mismatched number of names ({}) to frames ({}).", jsonFile, names.size(), ret.frames.size()));
		int i = 0;
		for (auto& n : names)
		{
			if (n.is_string())
				ret.names[n.as_string()] = i;
			i++;
		}
	}
	if (doc["aliases"].is_object())
	{
		if (doc["names"].is_null())
			throw std::runtime_error(fmt::format("GetAtlas: file {} has aliases but no names.", jsonFile));
		for (auto& a : doc["aliases"].as_object())
		{
			if (!a.second.is_string())
				throw std::runtime_error(fmt::format("GetAtlas: file {} has a non-string alias ({}).", jsonFile, a.first));
			auto from = a.first;
			auto to = a.second.as_string();
			if (ret.names.find(a.second.as_string()) == ret.names.cend())
				continue; //throw error
			ret.names[from] = ret.names[to];
		}
	}
}

void GetColorMap(ColorMap& ret, const std::string& jsonFile)
{
	auto colorData = VFS::ReadJSON(jsonFile);
	auto alts = colorData.as_array();
	ret.numRows = (int)alts.size();
	ret.numCols = (int)alts[0].as_array().size();
	if (ret.numCols >= ColorMap::Cols || ret.numRows >= ColorMap::Rows)
		throw std::runtime_error("Too many entries in colormap.");
	for (int i = 0; i < ret.numRows; i++)
	{
		auto vals = alts[i].as_array();
		for (int j = 0; j < ret.numCols; j++)
		{
			if (!vals[j].is_string())
				throw std::runtime_error("Colormap entries should be \"#RRGGBB\" hexcodes.");
			auto hex = vals[j].as_string();
			if (hex.empty() || hex[0] != '#')
				throw std::runtime_error("Colormap entries should be \"#RRGGBB\" hexcodes.");
			auto r = std::stoi(hex.substr(1, 2), nullptr, 16);
			auto g = std::stoi(hex.substr(3, 2), nullptr, 16);
			auto b = std::stoi(hex.substr(5, 2), nullptr, 16);
			auto col = (r << 0) | (g << 8) | (b << 16) | (255 << 24);
			ret.values[(i * ColorMap::Cols) + j] = col;
		}
	}
}
