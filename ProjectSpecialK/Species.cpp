#include <cctype>
#include "engine/Text.h"
#include "engine/TextUtils.h"
#include "Species.h"
#include "Types.h"
#include "Database.h"

Species::Species(jsonObject& value, const std::string& filename) : NameableThing(value, filename)
{
	auto filterNamesObj = json5pp::object({});
	auto filterNames = filterNamesObj.as_object();

	if (value["name"].is_string())
	{
		auto& both = value["name"].as_string();

		Text::Add(RefName + ":m", both);
		Text::Add(RefName + ":f", both);

		filterNames["USen"] = jsonValue(value["name"].as_string());
	}
	else if (value["name"].is_array())
	{
		auto narr = value["name"].as_array();
		filterNames = narr[0].as_object();
		if (narr.size() == 1)
		{
			Text::Add(RefName + ":m", narr[0]);
			Text::Add(RefName + ":f", narr[0]);
		}
		else
		{
			Text::Add(RefName + ":m", narr[0]);
			Text::Add(RefName + ":f", narr[1]);
		}
	}
	EnName[0] = StripBJTS(Text::Get(RefName + ":m"));
	EnName[1] = StripBJTS(Text::Get(RefName + ":f"));

	if (value["filterAs"])
	{
		FilterAs = value["filterAs"].as_string();
	}
	else
	{
		auto filter = fmt::format("filter:species:{}", ID);
		if (value["filter"])
			filterNames = value["filter"].as_object();
		else
		{
			for (auto sn : filterNames)
			{
				auto it = sn.second.as_string();
				it = StripBJTS(it);
				if (it[0] > 32 && it[0] < 127 && std::islower(it[0]))
					it[0] = (char)std::toupper(it[0]);
				filterNames[sn.first] = jsonValue(it);
			}
		}
		Text::Add(filter, filterNamesObj);

		auto settings = UI::settings["contentFilters"].as_object();
		auto setting = true;
		if (settings.find(filter) != settings.end())
			setting = settings[filter].as_boolean();
		Database::Filters.insert_or_assign(filter, setting);
	}
	
	ModeledMuzzle = value["hasMuzzle"].is_boolean() ? value["hasMuzzle"].as_boolean() : false;
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
