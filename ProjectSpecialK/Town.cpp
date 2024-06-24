#include "Town.h"

Town::Town()
{
	weatherSeed = std::rand();
}

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

		std::srand(weatherSeed + (month << 8) + (day << 16));

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

void Town::UpdateWeather()
{
	tm gm;
	auto now = time(nullptr);
	localtime_s(&gm, &now);
	auto hour = gm.tm_hour;

	Weather = (::Weather)weatherRain[hour];
	
	if (weatherWind[hour] == 0)
		Wind = 0;
	else
	{
		auto baseWind = std::abs(weatherWind[hour]) - 1;
		auto landIsEast = ((weatherSeed >> 16) % 2) == 1;
		auto windIsLand = weatherWind[hour] < 0;
		auto windStrength = ((1 << baseWind) - 1) + (std::rand() % 3);
		if (windIsLand && landIsEast)
			windStrength = -windStrength;
		Wind = windStrength;
	}
}

Town town;
