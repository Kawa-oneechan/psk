#include "Tickable.h"
#include "Texture.h"

class NineSlicer : public Tickable
{
private:
	std::shared_ptr<Texture> texture;

public:
	glm::vec2 Position, Size;
	glm::vec4 Color{ 1, 1, 1, 1 };
	float Scale{ 1.0f };

	NineSlicer(const std::string& texture, int left, int top, int width, int height);
	void Draw(float dt);
};
