#include "Town.h"

void Town::StartNewDay()
{
	//Select weather
	{
		tm gm;
		auto now = time(nullptr);
		localtime_s(&gm, &now);
		const auto month = gm.tm_mon + 1;
		const auto day = gm.tm_mday;
		conprint(0, "Today is {} {}. Let's see.", day, month);
		auto json = ReadJSON("weather.json")->AsObject();
		auto calendar = json[Hemisphere == Hemisphere::North ? "north" : "south"]->AsArray();
		auto monthlyRangeS = -1;
		auto monthlyRangeE = -1;
		//NOTE: We assume all entries are sorted by date.
		//TODO: there's probably a better method here.
		for (int i = 1; 0 < calendar.size(); i++)
		{
			auto c = calendar[i]->AsObject();
			const auto until = GetJSONVec2(c["until"]);
			if ((int)until[1] >= month)
			{
				if (monthlyRangeS == -1)
					monthlyRangeS = i;
				else
				{
					monthlyRangeE = i;
					if ((int)until[1] > month)
						break;
				}
			}
		}
		auto here = calendar[0]->AsObject();
		for (int i = monthlyRangeS; i < monthlyRangeE; i++)
		{
			auto c = calendar[i]->AsObject();
			const auto until = GetJSONVec2(c["until"]);
			if ((int)until[0] >= day)
				here = calendar[i]->AsObject();
		}
		//Now to pick a weather pattern for this day...
		//TODO: seed the randomizer with weatherSeed and a hash of today.
		auto rates = here["rates"]->AsArray();
		auto pick = 0;
		auto attempts = 1000;
		while (--attempts)
		{
			pick = std::rand() % rates.size();
			int roll = std::rand() % 100;
			if ((int)rates[pick]->AsNumber() > roll)
				break;
		}
		if (attempts == 0)
			conprint(0, "Gave up.");
		//take this pic
		auto patterns = json["patterns"]->AsArray();
		auto pattern = patterns[pick]->AsObject();
		conprint(0, "Weather: picked {}, \"{}\".", pick, pattern["id"]->AsString());
		auto rain = pattern["rain"]->AsArray();
		auto wind = pattern["wind"]->AsArray();
		for (int i = 0; i < 24; i++)
		{
			weatherRain[i] = (int)rain[i]->AsNumber();
			weatherWind[i] = (int)wind[i]->AsNumber();
		}
	}
}

std::tuple<int, int> Town::GetWeather()
{
	tm gm;
	auto now = time(nullptr);
	localtime_s(&gm, &now);
	auto hour = gm.tm_hour;
	//TODO: don't just return the wind value as-is, but calculate a random speed from it.
	return{ weatherRain[hour], weatherWind[hour] };
}

Town town;
