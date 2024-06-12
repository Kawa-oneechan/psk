#include "SpecialK.h"


glm::vec2 GetJSONVec2(JSONValue* val)
{
	if (!val->IsArray())
		throw std::runtime_error("GetJSONVec2: given value is not an array.");
	auto arr = val->AsArray();
	if (arr.size() != 2)
		throw std::runtime_error(fmt::format("GetJSONVec2: given array has {} entries, not 2.", arr.size()));
	if (!arr[0]->IsNumber() || !arr[1]->IsNumber())
		throw std::runtime_error("GetJSONVec2: given array does not contain only numbers.");
	return glm::vec2(arr[0]->AsNumber(), arr[1]->AsNumber());
}

glm::vec4 GetJSONVec4(JSONValue* val)
{
	if (!val->IsArray())
		throw std::runtime_error("GetJSONVec4: given value is not an array.");
	auto arr = val->AsArray();
	if (arr.size() != 4)
		throw std::runtime_error(fmt::format("GetJSONVec4: given array has {} entries, not 4.", arr.size()));
	if (!arr[0]->IsNumber() || !arr[1]->IsNumber() || !arr[2]->IsNumber() || !arr[3]->IsNumber())
		throw std::runtime_error("GetJSONVec4: given array does not contain only numbers.");
	return glm::vec4(arr[0]->AsNumber(), arr[1]->AsNumber(), arr[2]->AsNumber(), arr[3]->AsNumber());
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
