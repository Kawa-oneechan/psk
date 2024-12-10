#pragma once

#include "SpecialK.h"
#include "PanelLayout.h"

class DateTimePanel : public Tickable
{
private:
	int lastHour{ -1 };
	int lastMinute{ -1 };
	tm gm;
	PanelLayout layout{ PanelLayout(UI::json["datetime"]) };

public:
	DateTimePanel();
	void Update();
	void Tick(float dt);
	void Draw(float dt);
};

extern DateTimePanel* dateTimePanel;
