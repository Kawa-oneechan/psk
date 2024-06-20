#include "SpecialK.h"


glm::vec2 GetJSONVec2(JSONValue* val)
{
	if (!val->IsArray())
		throw std::runtime_error(fmt::format("GetJSONVec2: given value {} is not an array.", val->Stringify()));
	auto arr = val->AsArray();
	if (arr.size() != 2)
		throw std::runtime_error(fmt::format("GetJSONVec2: given array {} has {} entries, not 2.", val->Stringify(), arr.size()));
	//if (!arr[0]->IsNumber() || !arr[1]->IsNumber())
	for (auto x : arr)
		if (!x->IsNumber())
			throw std::runtime_error(fmt::format("GetJSONVec2: given array {} does not contain only numbers.", val->Stringify()));
	return glm::vec2(arr[0]->AsNumber(), arr[1]->AsNumber());
}

glm::vec4 GetJSONVec4(JSONValue* val)
{
	if (!val->IsArray())
		throw std::runtime_error("GetJSONVec4: given value is not an array.");
	auto arr = val->AsArray();
	if (arr.size() != 4)
		throw std::runtime_error(fmt::format("GetJSONVec4: given array has {} entries, not 4.", arr.size()));
	//if (!arr[0]->IsNumber() || !arr[1]->IsNumber() || !arr[2]->IsNumber() || !arr[3]->IsNumber())
	for (auto x : arr)
		if (!x->IsNumber())
			throw std::runtime_error("GetJSONVec4: given array does not contain only numbers.");
	return glm::vec4(arr[0]->AsNumber(), arr[1]->AsNumber(), arr[2]->AsNumber(), arr[3]->AsNumber());
}

//Converts [R,G,B], [R,G,B,A], "#RRGGBB" or "#AARRGGBB" to glm::vec4.
//Returns an alpha of -1, which is impossible, on error.
glm::vec4 GetJSONColor(JSONValue* val)
{
	if (val->IsString())
	{
		auto hex = val->AsString();
		int r = 0, g = 0, b = 0, a = 0;
		if (hex.empty() || hex[0] != '#')
			return glm::vec4(0, 0, 0, -1);
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
			r = (float)arr[0]->AsNumber();
			g = (float)arr[1]->AsNumber();
			b = (float)arr[2]->AsNumber();
			a = 1.0f;
		}
		else if (arr.size() == 4)
		{
			r = (float)arr[0]->AsNumber();
			g = (float)arr[1]->AsNumber();
			b = (float)arr[2]->AsNumber();
			a = (float)arr[3]->AsNumber();
		}
		else
			throw std::runtime_error(fmt::format("GetJSONColor: {} is not a valid color.", val->Stringify()));
		return glm::vec4(arr[0]->AsNumber(), arr[1]->AsNumber(), arr[2]->AsNumber(), arr[3]->AsNumber());
	}
	throw std::runtime_error(fmt::format("GetJSONColor: {} is not a valid color.", val->Stringify()));
}

void GetAtlas(TextureAtlas &ret, const std::string& jsonFile)
{
	auto rjs = ReadJSON(jsonFile);
	if (rjs == nullptr)
		return;
	auto doc = rjs->AsObject();
	ret.clear();
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
		return;
	}
	else if (doc["type"]->AsString() == "atlas")
	{
		auto rects = doc["rects"]->AsArray();
		for (const auto& rect : rects)
		{
			ret.push_back(GetJSONVec4(rect));
		}
		return;
	}

	throw std::runtime_error(fmt::format("GetAtlas: file {} has an unknown type \"{}\".", jsonFile, doc["type"]->AsString()));
}

bool PointInPoly(const glm::vec2 point, const std::vector<glm::vec2>& polygon)
{
	int crossings = 0;
	const auto numPts = polygon.size() - 1;

	for (auto i = 0; i < numPts; i++)
	{
		if (((polygon[i].y <= point.y) && (polygon[i + 1].y > point.y))
			|| ((polygon[i].y > point.y) && (polygon[i + 1].y <= point.y)))
		{
			auto vt = (point.y - polygon[i].y) / (polygon[i + 1].y - polygon[i].y);
			if (point.x < polygon[i].x + vt * (polygon[i + 1].x - polygon[i].x))
			{
				++crossings;
			}
		}
	}
	return (crossings & 1) == 1;
}

bool PointInRect(const glm::vec2 point, const glm::vec4 rect)
{
	return
		(point.x >= rect.x) &&
		(point.x < rect.x + rect.z) &&
		(point.y >= rect.y) &&
		(point.y < rect.y + rect.w);
}

void Table(std::vector<std::string> data, size_t stride)
{
	size_t width[64] = { 0 };
	auto rows = data.size() / stride;
	for (auto col = 0; col < stride; col++)
	{
		for (auto row = 0; row < rows; row++)
		{
			const auto& cel = data[row * stride + col];
			if (cel.length() > width[col])
				width[col] = cel.length();
		}
	}

	std::string top;
	std::string middle;
	std::string bottom;
	for (auto col = 0; col < stride; col++)
	{
		for (auto i = 0; i < width[col] + 2; i++)
		{
			top += u8"─";
			middle += u8"─";
			bottom += u8"─";
		}
		if (col < stride - 1)
		{
			top += u8"┬";
			middle += u8"┼";
			bottom += u8"┴";
		}
	}

	conprint(7, u8"┌{}┐", top);

	for (auto row = 0; row < rows; row++)
	{
		std::string line;
		for (auto col = 0; col < stride; col++)
		{
			const auto& cel = data[row * stride + col];
			line += fmt::format(fmt::format(u8"│ {{:{}}} ", width[col]), cel);
		}
		line += u8"│";
		conprint(7, line);
		if (row == 0)
			conprint(7, fmt::format(u8"├{}┤", middle));
	}

	conprint(7, u8"└{}┘", bottom);
}
