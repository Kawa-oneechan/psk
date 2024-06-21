#pragma once

#include "SpecialK.h"

class Background : public Tickable
{
private:
	Texture wallpaper{ Texture("discobg2.jpg") };
	Shader scroller{ Shader("shaders/scroller.fs") };
	float time{ 0 };

public:
	void Draw(double dt);
};
