#include "Background.h"

Background::Background(const std::string& file, glm::vec2 speed)
{
	wallpaper = new Texture(file);
	Speed = speed;
}

Background::~Background()
{
	delete wallpaper;
}

void Background::Draw(float dt)
{
	time += dt * timeScale;

	scroller.Use();
	scroller.SetFloat("time", time);
	scroller.SetVec2("speed", Speed);
	scroller.SetVec4("recolorB", RecolorBlack);
	scroller.SetVec4("recolorW", RecolorWhite);
	sprender.DrawSprite(&scroller, *wallpaper, glm::vec2(0), glm::vec2(width, height), glm::vec4(0, 0, width, height));
}
