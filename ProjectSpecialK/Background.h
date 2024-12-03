#pragma once

#include "SpecialK.h"

class Background : public Tickable
{
private:
	std::unique_ptr<Texture> wallpaper;
	Shader scroller{ Shader("shaders/scroller.fs") };
	float time{ 0 };
	const float timeScale{ 0.005f };

public:
	Background(const std::string& file, glm::vec2 speed = { 0.1f, -0.1f });

	void Draw(float dt);

	glm::vec2 Speed{ 0.1f, -0.1f };
	glm::vec4 RecolorBlack{ 0.0f };
	glm::vec4 RecolorWhite{ 0.0f };
};

