#pragma once

#include "SpecialK.h"

class Background : public Tickable
{
private:
	Texture wallpaper{ Texture("discobg2.jpg") };
	Shader scroller{ Shader("shaders/scroller.fs") };
	float time{ 0 };
	const float timeScale{ 0.005f };

public:
	glm::vec2 Speed{ 0.1f, -0.1f };
	void Draw(float dt);
};
