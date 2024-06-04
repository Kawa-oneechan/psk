#pragma once

#include "SpecialK.h"

class Background : public Tickable
{
private:
	Texture* wallpaper;
	Shader* scroller;
	float time;

public:
	Background();
	void Draw(double dt);
};
