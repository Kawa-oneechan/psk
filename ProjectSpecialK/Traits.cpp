#include "SpecialK.h"

Personality::Personality(JSONObject& value)
{
	ID = value["id"]->AsString();
}

Hobby::Hobby(JSONObject& value)
{
	ID = value["id"]->AsString();
}
