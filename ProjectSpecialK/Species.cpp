#include "SpecialK.h"

Species::Species(JSONObject& value)
{
	ID = value["id"]->AsString();
	auto ref = fmt::format("species:{}", ID);
	StringToLower(ref);
	RefName = ref;

	if (value["name"]->IsString())
	{
		auto& both = value["name"]->AsString();
		TextAdd(ref + ":m", both);
		TextAdd(ref + ":f", both);
	}
	else if (value["name"]->IsArray())
	{
		auto narr = value["name"]->AsArray();
		TextAdd(ref + ":m", *narr[0]);
		TextAdd(ref + ":f", *narr[1]);
	}
	EnName[0] = StripMSBT(TextGet(ref + ":m", Language::EUen));
	EnName[1] = StripMSBT(TextGet(ref + ":f", Language::EUen));

	auto filter = fmt::format("filter:species:{}", ID);
	auto settings = UI::settings["contentFilters"]->AsObject();
	auto setting = true;
	if (settings.find(filter) != settings.end())
		setting = settings[filter]->AsBool();
	filters.insert_or_assign(filter, setting);
	
	ModeledMuzzle = (value["modeledMuzzle"] != nullptr) ? value["modeledMuzzle"]->AsBool() : false;
}

const std::string Species::Name()
{
	return TextGet(RefName);
}
