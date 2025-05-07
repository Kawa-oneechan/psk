#include "Iris.h"

Iris::Iris()
{
	time = 1.0f; //assume we're all black, irising IN
}

bool Iris::Tick(float dt)
{
	if (state == State::Idle)
		return true;
	if (state == State::Out)
	{
		time += dt;
		if (time >= 1.0f)
		{
			time = 1.0f;
			state = State::Idle;
		}
	}
	else
	{
		time -= dt;
		if (time <= 0.0f)
		{
			time = 0.0f;
			state = State::Idle;
		}
	}
	return false;
}

void Iris::Draw(float dt)
{
	dt;
	auto shader = Shaders["iris"];
	shader->Set("smoothness", 0.02f);
	shader->Set("progress", 1.0f - time);
	Sprite::DrawSprite(shader, *whiteRect, glm::vec2(0), glm::vec2(width, height));
}

void Iris::In()
{
	state = State::In;
	time = 1.0f;
}

void Iris::Out()
{
	state = State::Out;
	time = 0.0f;
}

bool Iris::Done()
{
	return state == State::Idle;
}
