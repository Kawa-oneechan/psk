#include "DateTimePanel.h"
#include "Town.h"

extern Audio* bgm;

DateTimePanel::DateTimePanel()
{
	auto now = time(nullptr);
	localtime_s(&gm, &now);

	//layout = new PanelLayout(UI::json["datetime"]);

	Update();
}

void DateTimePanel::Update()
{
	if (UI::settings["24hour"]->AsBool())
	{
		//24 hours, easy
		layout.GetPanel("time")->Text = fmt::format("{:2}:{:02}", gm.tm_hour, gm.tm_min);
		layout.GetPanel("ampm")->Text.clear();
	}
	else
	{
		//12 hours?
		auto h = gm.tm_hour;
		auto pm = h >= 12;
		if (h == 0) h += 12;
		else if (h > 12) h -= 12;

		layout.GetPanel("time")->Text = fmt::format("{:2}:{:02}", h, gm.tm_min);
		layout.GetPanel("ampm")->Text = pm ? "PM" : "AM";
	}

	auto wd = gm.tm_wday;
	if (wd == 0) wd = 7; //gm.tm_wday is 0-Sun to 6-Sat. We want 1-Mon to 7-Sun.

	layout.GetPanel("date")->Text = TextDateMD(gm.tm_mon + 1, gm.tm_mday);

	if (lastHour == 4 && gm.tm_hour == 5)
	{
		//trigger reset
	}
	if (lastHour == -1 || (gm.tm_hour != lastHour && gm.tm_min == 0))
	{
		lastHour = gm.tm_hour;
		//TODO: leave actually changing the BGM to MusicManager.
		//MusicManager.Play(fmt::format("music/bgm/clock/{:02}sunny.ogg", lastHour), MusicType::HourlyBGM);
		//That can then handle the chimes and fade out sequence.
		delete bgm;
		auto weather = "sunny";
		{
			town.UpdateWeather();
			if (town.Clouds >= Town::Weather::RainClouds)
				weather = "rainy";
		}
		bgm = new Audio(fmt::format("music/bgm/clock/{:02}{}.ogg", lastHour, weather));
		bgm->Play();
	}
}

void DateTimePanel::Tick(float dt)
{
	layout.Tick(dt);

	auto now = time(nullptr);
	localtime_s(&gm, &now);
	if (lastMinute != gm.tm_min)
	{
		Update();
		lastMinute = gm.tm_min;
	}
}

void DateTimePanel::Draw(float dt)
{
	layout.Draw(dt);
}
