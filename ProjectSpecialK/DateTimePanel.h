#pragma once

#include "SpecialK.h"
#include "PanelLayout.h"

class DateTimePanel : public Tickable
{
private:
	int lastHour{ -1 };
	int lastMinute{ -1 };
	tm gm;
	PanelLayout layout;

public:
	DateTimePanel();
	void Update();
	bool Tick(float dt);
	void Draw(float dt);
	void Show();
	void Hide();
};

extern DateTimePanel* dateTimePanel;
