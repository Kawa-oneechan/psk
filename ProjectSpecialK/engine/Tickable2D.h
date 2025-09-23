#pragma once
#include <glm/glm.hpp>
#include "Tickable.h"
#include "SpriteRenderer.h"
#include "InputsMap.h"

class Tickable2D;

class Tickable2D : public Tickable
{
protected:
	std::shared_ptr<Tickable2D> parent;
public:
	glm::vec2 Position;
	glm::vec2 AbsolutePosition;

	virtual bool Tick(float dt)
	{
		AbsolutePosition = (parent ? parent->AbsolutePosition + Position : Position);
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

	virtual void Draw(float dt)
	{
		for (const auto& t : ChildTickables)
		{
			if (!t->Visible)
				continue;
			t->Draw(dt);
		}
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

	TextLabel(Tickable2DP parent, const std::string& text, glm::vec2 position) : Text(text)
	{
		this->parent = std::move(parent);
		Position = position;
	}

	void Draw(float)
	{
		Sprite::DrawText(Font, Text, AbsolutePosition, Color, Size, Angle, Raw);
	}
};

using TextLabelP = std::shared_ptr<TextLabel>;
