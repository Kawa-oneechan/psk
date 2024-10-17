#include "DateTimePanel.h"
#include "Town.h"
#include "MusicManager.h"

extern void PlayMusic(const std::string& id);

DateTimePanel::DateTimePanel()
{
	auto now = time(nullptr);
	localtime_s(&gm, &now);

	Update();
}

void DateTimePanel::Update()
{
	if (UI::settings["24hour"]->AsBool())
	{
		layout.GetPanel("time")->Text = fmt::format("{:2}:{:02}", gm.tm_hour, gm.tm_min);
		layout.GetPanel("ampm")->Text.clear();
	}
	else
	{
		auto h = gm.tm_hour;
		auto pm = h >= 12;
		if (h == 0) h += 12;
		else if (h > 12) h -= 12;

		layout.GetPanel("time")->Text = fmt::format("{:2}:{:02}", h, gm.tm_min);
		layout.GetPanel("ampm")->Text = pm ? "PM" : "AM";
	}

	auto wd = gm.tm_wday;
	if (wd == 0) wd = 7; //gm.tm_wday is 0-Sun to 6-Sat. We want 1-Mon to 7-Sun.

	layout.GetPanel("date")->Text = Text::DateMD(gm.tm_mon + 1, gm.tm_mday);

	if (lastHour == 4 && gm.tm_hour == 5)
	{
		//trigger morning reset
	}
	if (lastHour == -1 || (gm.tm_hour != lastHour && gm.tm_min == 0))
	{
		lastHour = gm.tm_hour;
		//TODO: check for specific maps and events
		auto& thisMap = town;
		if (!thisMap.CanOverrideMusic || musicManager.Override.empty())
			musicManager.Play(thisMap.Music);
		else
			musicManager.Play(musicManager.Override);
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
