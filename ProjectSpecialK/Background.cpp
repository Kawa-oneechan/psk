#include "Background.h"

void Background::Draw(float dt)
{
	time += dt * timeScale;

	scroller.Use();
	scroller.SetFloat("time", time);
	scroller.SetVec2("speed", Speed);
	sprender.DrawSprite(&scroller, wallpaper, glm::vec2(0), glm::vec2(width, height), glm::vec4(0, 0, width, height));
}
