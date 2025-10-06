#pragma once
#include <vector>
#include <map>
#include <glm/glm.hpp>
#include <json5pp.hpp>

using jsonValue = json5pp::value;
using jsonObject = json5pp::value::object_type;
using jsonArray = json5pp::value::array_type;

//A single Unicode code point.
using rune = unsigned int;
//A CRC32 hash referring to a NameableThing.
using hash = unsigned int;

using polygon = std::vector<glm::vec2>;

//To make coordinate spaces more explicit, maybe?
#pragma warning(push)
#pragma warning(disable: 4455)
//Distance in pixels
inline constexpr int operator "" px(unsigned long long v) { return (int)v; }
//Distance in tiles
inline constexpr int operator "" t(unsigned long long v) { return (int)v; }
//Distance normalized to a 0.0-1.0 range
inline constexpr float operator "" pt(long double v) { return (float)v; }
#pragma warning(pop)

#define BJTSParams const std::vector<std::string>& tags, int start, int len
typedef void(*BJTSFunc)(std::string& data, BJTSParams);

struct ColorMap
{
	static constexpr int Rows = 32;
	static constexpr int Cols = 32;
	unsigned int values[Rows * Cols];
	int numRows, numCols;
};

struct SpriteAtlas
{
	std::vector<glm::vec4> frames;
	std::map<std::string, int> names;
	inline bool empty() const { return frames.empty(); }
	inline size_t size() const { return frames.size(); }
	inline void push_back(const glm::vec4& frame) { frames.push_back(frame); }
	inline glm::vec4 operator[](size_t i) const { return frames[i]; }
	inline glm::vec4 operator[](const std::string& s) const
	{
		auto it = names.find(s);
		if (it != names.cend())
			return frames[it->second];
		return frames[0];
	}
};

namespace UI
{
	extern std::map<std::string, glm::vec4> themeColors;
	extern std::vector<glm::vec4> textColors;

	extern jsonValue json;
	extern jsonValue settings;

	extern std::string initFile;
};
