#pragma once

//PLACEHOLDER
class Personality
{
public:
	std::string ID;
	Personality(JSONObject& value, const std::string& filename = "");
	Personality() = default;
};
typedef std::shared_ptr<Personality> PersonalityP;

//PLACEHOLDER
class Hobby
{
public:
	std::string ID;
	Hobby(JSONObject& value, const std::string& filename = "");
	Hobby() = default;
};
typedef std::shared_ptr<Hobby> HobbyP;
