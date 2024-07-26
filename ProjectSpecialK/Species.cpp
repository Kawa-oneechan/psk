#include "SpecialK.h"

Species::Species(JSONObject& value, const std::string& filename) : NameableThing(value, filename)
{
	auto filterNames = JSONObject();

	if (value["name"]->IsString())
	{
		auto& both = value["name"]->AsString();

		TextAdd(RefName + ":m", both);
		TextAdd(RefName + ":f", both);

		filterNames["USen"] = new JSONValue(value["name"]->AsString());
	}
	else if (value["name"]->IsArray())
	{
		auto narr = value["name"]->AsArray();
		filterNames = narr[0]->AsObject();
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

	if (value["filterAs"])
	{
		FilterAs = value["filterAs"]->AsString();
	}
	else
	{
		auto filter = fmt::format("filter:species:{}", ID);
		if (value["filter"])
			filterNames = value["filter"]->AsObject();
		else
		{
			for (auto sn : filterNames)
			{
				auto it = sn.second->AsString();
				it = StripMSBT(it);
				if (it[0] > 32 && it[0] < 127 && std::islower(it[0]))
					it[0] = (char)std::toupper(it[0]);
				filterNames[sn.first] = new JSONValue(it);
			}
		}
		TextAdd(filter, filterNames);

		auto settings = UI::settings["contentFilters"]->AsObject();
		auto setting = true;
		if (settings.find(filter) != settings.end())
			setting = settings[filter]->AsBool();
		Database::Filters.insert_or_assign(filter, setting);
	}
	
	ModeledMuzzle = (value["modeledMuzzle"] != nullptr) ? value["modeledMuzzle"]->AsBool() : false;
}

std::string Species::Name()
{
	return TextGet(RefName);
}
