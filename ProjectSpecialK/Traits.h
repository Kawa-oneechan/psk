#pragma once
#include <memory>
#include "NameableThing.h"

/*
Thoughts
--------
1. Should these be NameableThings?

2. What extra properties would a Personality hold?
According to the JSON so far, the answer is this:
  * ID
  * Name (gives you something to use in the content filter)
  * Voice bank name (just the one in ACNH, gendered pair for PSK)
  * Base (fallback voice bank in case it's missing)
So maybe at least *that* could do with being a NameableThing...

3. What does a Hobby need to define?
There is no answer in the JSON so far...
*/

class Personality: public NameableThing
{
public:
	std::string Voices[2];

	Personality(jsonObject& value, const std::string& filename = "");
	Personality() = default;
};

using PersonalityP = std::shared_ptr<Personality>;

//PLACEHOLDER
class Hobby
{
public:
	std::string ID;
	Hobby(jsonObject& value, const std::string& filename = "");
	Hobby() = default;
};

using HobbyP = std::shared_ptr<Hobby>;
