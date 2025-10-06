#include "engine/SpriteRenderer.h"
#include "engine/Utilities.h"
#include "engine/Shader.h"
#include "Background.h"
#include "Game.h"

Background::Background(const std::string& file, glm::vec2 speed) : wallpaper(std::make_unique<Texture>(file)), Speed(speed)
{
}

void Background::Draw(float dt)
{
	time += dt;
	auto scroller = Shaders["scroller"];
	scroller->Set("time", time);
	scroller->Set("speed", Speed);
	scroller->Set("recolorB", RecolorBlack);
	scroller->Set("recolorW", RecolorWhite);
	Sprite::DrawSprite(scroller, *wallpaper, glm::vec2(0), glm::vec2(width, height), glm::vec4(0, 0, width, height));
}
