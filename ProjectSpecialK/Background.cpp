#include "Background.h"

void Background::Draw(float dt)
{
	time += dt * timeScale;

	scroller.Use();
	scroller.SetFloat("time", time);
	sprender->DrawSprite(&scroller, wallpaper, glm::vec2(0), glm::vec2(width, height), glm::vec4(0, 0, width, height));

	//auto lol = sprender->MeasureText(1, "Project Special K: UI test", 50);
	//sprender->DrawSprite(*whiteRect, glm::vec2(0), lol, glm::vec4(0), 0, glm::vec4(1, 1, 1, 0.5f));
	sprender->DrawText(1, "Project Special K: UI test", glm::vec2(0), glm::vec4(1, 1, 1, 0.75), 50);
	//sprender->DrawText(1, fmt::format("Project Special K: UI test (SCALE = {})", scale), glm::vec2(0), glm::vec4(1, 1, 1, 0.75), 50);
	//sprender->DrawText(1, u8"日", glm::vec2(0, 20), glm::vec4(1, 1, 1, 0.75), 50);
	//sprender->DrawText(1, u8"ā", glm::vec2(0), glm::vec4(1, 1, 1, 0.75), 200);
}
