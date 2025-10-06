#pragma once

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
	bool Tick(float dt) override;
	void Draw(float dt) override;
	void Show();
	void Hide();
};

extern std::shared_ptr<DateTimePanel> dateTimePanel;
