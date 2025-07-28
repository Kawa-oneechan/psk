#pragma once
#include <glm/glm.hpp>
#include <json5pp.hpp>

using jsonValue = json5pp::value;
using jsonObject = json5pp::value::object_type;

//Given a JSON array with two numbers in it, returns a vec2 with those numbers.
extern glm::vec2 GetJSONVec2(const jsonValue& val);
//Given a JSON array with three numbers in it, returns a vec3 with those numbers.
extern glm::vec3 GetJSONVec3(const jsonValue& val);
//Given a JSON array with four numbers in it, returns a vec4 with those numbers.
extern glm::vec4 GetJSONVec4(const jsonValue& val);
//Converts [R,G,B], [R,G,B,A], "#RRGGBB" or "#AARRGGBB" to glm::vec4.
//Returns an alpha of -1, which is impossible, on error.
extern glm::vec4 GetJSONColor(const jsonValue& val);
//Given a JSON array with two numbers in it, returns a vec2 with those numbers.
//But given a string that parses as "mmm dd" or "mmm/dd", parses that as a day and returns those numbers.
//Either way, the first value is a day from 1-31 and the second a month from 1-12.
extern glm::vec2 GetJSONDate(const jsonValue& val);
//Returns a JSON array initialized from a vec2, optionally casting to int.
extern jsonValue GetJSONVec(const glm::vec2& vec, bool asInt = false);
//Returns a JSON array initialized from a vec3, optionally casting to int.
extern jsonValue GetJSONVec(const glm::vec3& vec, bool asInt = false);
//Returns a JSON array initialized from a vec4, optionally casting to int.
extern jsonValue GetJSONVec(const glm::vec4& vec, bool asInt = false);

extern void GetAtlas(std::vector<glm::vec4> &ret, const std::string& jsonFile);
