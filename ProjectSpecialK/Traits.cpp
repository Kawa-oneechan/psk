#include "SpecialK.h"

Personality::Personality(JSONObject& value, const std::string& filename) : NameableThing(value, filename)
{
	std::string voices[2];
	{
		auto v = value["voice"]->AsArray();
		voices[0] = v[0]->AsString();
		voices[1] = v[1]->AsString();
	}
	auto& base = value["base"]->AsString();

	/*
	TODO: check if "sound/animalese/{}/a.wav" or whatever exists for each of the two voice banks.
	If not, set that voice to the base and try again.
	If it still doesn't exist, clear that one out so DialogueBox can tell.
	Either way, base doesn't need to be exposed outside of here.

	Note that base may also work as a fallback in dialogue, so maybe it should be a field.
	*/
}

Hobby::Hobby(JSONObject& value, const std::string& filename)
{
	ID = value["id"]->AsString();
}
