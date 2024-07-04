#pragma once

class Tickable
{
public:
	bool* mutex{ nullptr };
	bool dead{ false };

	virtual ~Tickable() {}
	virtual void Tick(float dt) {};
	virtual void Draw(float dt) {};
	virtual bool Character(unsigned int codepoint) { return false; }
};

using TickableP = std::shared_ptr<Tickable>;

