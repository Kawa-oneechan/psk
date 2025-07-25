#pragma once
#include <vector>
#include <glm/glm.hpp>

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

