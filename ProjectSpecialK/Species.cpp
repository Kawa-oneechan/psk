#include "SpecialK.h"

Species::Species(JSONObject& value, const std::string& filename) : NameableThing(value, filename)
{
	auto filterNames = JSONObject();

	if (value["name"]->IsString())
	{
		auto& both = value["name"]->AsString();

		Text::Add(RefName + ":m", both);
		Text::Add(RefName + ":f", both);

		filterNames["USen"] = new JSONValue(value["name"]->AsString());
	}
	else if (value["name"]->IsArray())
	{
		auto narr = value["name"]->AsArray();
		filterNames = narr[0]->AsObject();
		if (narr.size() == 1)
		{
			Text::Add(RefName + ":m", *narr[0]);
			Text::Add(RefName + ":f", *narr[0]);
		}
		else
		{
			Text::Add(RefName + ":m", *narr[0]);
			Text::Add(RefName + ":f", *narr[1]);
		}
	}
	EnName[0] = StripMSBT(Text::Get(RefName + ":m", Language::EUen));
	EnName[1] = StripMSBT(Text::Get(RefName + ":f", Language::EUen));

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
		Text::Add(filter, filterNames);

		auto settings = UI::settings["contentFilters"]->AsObject();
		auto setting = true;
		if (settings.find(filter) != settings.end())
			setting = settings[filter]->AsBool();
		Database::Filters.insert_or_assign(filter, setting);
	}
	
	ModeledMuzzle = (value["hasMuzzle"] != nullptr) ? value["hasMuzzle"]->AsBool() : false;
}

std::string Species::Name()
{
	return Text::Get(RefName);
}


void Species::LoadModel()
{
	if (!_model)
		_model = std::make_shared<::Model>(fmt::format("{}/model.fbx", Path));
}

ModelP Species::Model()
{
	if (!_model)
		LoadModel();
	return _model;
}
