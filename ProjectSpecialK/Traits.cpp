#include "SpecialK.h"

Personality::Personality(JSONObject& value, const std::string& filename)
{
	ID = value["id"]->AsString();
}

Hobby::Hobby(JSONObject& value, const std::string& filename)
{
	ID = value["id"]->AsString();
}
