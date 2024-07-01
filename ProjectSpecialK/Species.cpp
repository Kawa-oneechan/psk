#include "SpecialK.h"

Species::Species(JSONObject& value, const std::string& filename) : NameableThing(value, filename)
{
	if (value["name"]->IsString())
	{
		auto& both = value["name"]->AsString();
		TextAdd(RefName + ":m", both);
		TextAdd(RefName + ":f", both);
	}
	else if (value["name"]->IsArray())
	{
		auto narr = value["name"]->AsArray();
		if (narr.size() == 1)
		{
			TextAdd(RefName + ":m", *narr[0]);
			TextAdd(RefName + ":f", *narr[0]);
		}
		else
		{
			TextAdd(RefName + ":m", *narr[0]);
			TextAdd(RefName + ":f", *narr[1]);
		}
	}
	EnName[0] = StripMSBT(TextGet(RefName + ":m", Language::EUen));
	EnName[1] = StripMSBT(TextGet(RefName + ":f", Language::EUen));

	auto filter = fmt::format("filter:species:{}", ID);
	auto settings = UI::settings["contentFilters"]->AsObject();
	auto setting = true;
	if (settings.find(filter) != settings.end())
		setting = settings[filter]->AsBool();
	Database::Filters.insert_or_assign(filter, setting);
	
	ModeledMuzzle = (value["modeledMuzzle"] != nullptr) ? value["modeledMuzzle"]->AsBool() : false;
}

const std::string Species::Name()
{
	return TextGet(RefName);
}
