#pragma once

class Tickable
{
public:
	bool* mutex{ nullptr };
	bool dead{ false };

	virtual ~Tickable() {}
	virtual void Tick(float) {};
	virtual void Draw(float) {};
	virtual bool Character(unsigned int) { return false; }
};

using TickableP = std::shared_ptr<Tickable>;

