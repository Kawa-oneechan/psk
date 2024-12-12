#include "Iris.h"

Iris::Iris()
{
	shader = new Shader("shaders/iris.fs");
	time = 0.0;
}

Iris::~Iris()
{
	delete shader;
}

void Iris::Tick(float dt)
{
	if (state == State::Idle)
		return;
	if (state == State::Out)
	{
		time += dt;
		if (time >= 1.0)
		{
			time = 1.0;
			state = State::Idle;
		}
	}
	else
	{
		time -= dt;
		if (time <= 0.0)
		{
			time = 0.0;
			state = State::Idle;
		}
	}
}

void Iris::Draw(float dt)
{
	dt;
	shader->Use();
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

extern Iris* iris;
