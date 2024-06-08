#include "DateTimePanel.h"

DateTimePanel::DateTimePanel()
{
	auto now = time(nullptr);
	localtime_s(&gm, &now);

	layout = new PanelLayout(UI::json["datetime"]);

	Update();
}

void DateTimePanel::Update()
{
	//24 hours, easy
	//shownTime = fmt::format("{:2}:{:02}", gm.tm_hour, gm.tm_min);

	//12 hours?
	auto h = gm.tm_hour;
	auto pm = h >= 12;
	if (h == 0) h += 12;
	else if (h > 12) h -= 12;

	layout->GetPanel("time")->Text = fmt::format("{:2}:{:02} {}", h, gm.tm_min, pm ? "PM" : "AM");

	auto wd = gm.tm_wday;
	if (wd == 0) wd = 7; //gm.tm_wday is 0-Sun to 6-Sat. We want 1-Mon to 7-Sun.

	//TODO : use "month:format".
	layout->GetPanel("date")->Text = fmt::format("{} {}, {}", TextGet(fmt::format("month:{}", gm.tm_mon + 1)), gm.tm_mday, TextGet(fmt::format("day:short:{}", wd)));

	if (lastHour == 4 && gm.tm_hour == 5)
	{
		//trigger reset
	}
	if (lastHour == -1 || (gm.tm_hour != lastHour && gm.tm_min == 0))
	{
		lastHour = gm.tm_hour;
		//fmt::print("\x1B[12;40H Ding dong~! {} now", lastHour);
		//trigger music

		//TODO: fade out first, probably have something else handle that.
		//delete bgm;
		//bgm = new Audio(fmt::format("music/bgm/clock/{:02}sunny.ogg", lastHour));
		//bgm->Play();
	}
}

void DateTimePanel::Tick(double dt)
{
	layout->Tick(dt);

	auto now = time(nullptr);
	localtime_s(&gm, &now);
	if (lastMinute != gm.tm_min)
	{
		Update();
		lastMinute = gm.tm_min;
	}
}

void DateTimePanel::Draw(double dt)
{
	layout->Draw(dt);
}
