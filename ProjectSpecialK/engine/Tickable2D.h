#pragma once
#include <glm/glm.hpp>
#include "Tickable.h"
#include "SpriteRenderer.h"
#include "InputsMap.h"
#include "Texture.h"

extern float scale;

class Tickable2D;

class Tickable2D : public Tickable
{
protected:
	Tickable2D* parent{ nullptr };
public:
	glm::vec2 Position;
	glm::vec2 AbsolutePosition;
	float Scale{ -1 };

	virtual bool Tick(float dt) override
	{
		AbsolutePosition = (parent ? parent->AbsolutePosition + Position : (Position * (Scale > 0 ? Scale : ::scale)));
		for (unsigned int i = (unsigned int)ChildTickables.size(); i-- > 0; )
		{
			auto t = ChildTickables[i];
			if (!t->Enabled)
				continue;
			if (!t->Tick(dt))
				Inputs.Clear();
		}
		return true;
	}

	void AddChild(Tickable2D* newChild)
	{
		newChild->parent = this;
		ChildTickables.push_back(std::shared_ptr<Tickable2D>(newChild));
	}
	void AddChild(std::shared_ptr<Tickable2D> newChild)
	{
		newChild->parent = this;
		ChildTickables.push_back(newChild);
	}
};

using Tickable2DP = std::shared_ptr<Tickable2D>;

class TextLabel : public Tickable2D
{
public:
	std::string Text;
	glm::vec4 Color{ 1, 1, 1, 1 };
	float Size{ 100.0f };
	float Angle{ 0.0f };
	int Font{ 1 };
	bool Raw{ false };

	TextLabel(const std::string& text, glm::vec2 position) : Text(text)
	{
		parent = nullptr;
		Position = position;
	}

	void Draw(float) override
	{
		float s = Scale > 0 ? Scale : scale;
		Sprite::DrawText(Font, Text, AbsolutePosition, Color, Size * s, Angle, Raw);
	}
};

using TextLabelP = std::shared_ptr<TextLabel>;

class SimpleSprite : public Tickable2D
{
private:
	Texture* texture;
public:
	Sprite::SpriteFlags Flags{ Sprite::SpriteFlags::NoFlags };
	int Frame;

	SimpleSprite(const std::string& texture, int frame, glm::vec2 position)
	{
		parent = nullptr;
		this->texture = new Texture(texture);
		Position = position;
		Frame = frame;
	}

	~SimpleSprite() override
	{
		delete this->texture;
	}

	void Draw(float) override
	{
		float s = Scale > 0 ? Scale : scale;
		auto frame = texture->operator[](Frame);
		auto scaledSize = glm::vec2(frame.z, frame.w) * s;
		Sprite::DrawSprite(*texture, AbsolutePosition, scaledSize, frame, 0.0f, glm::vec4(1), Flags);
	}
};
