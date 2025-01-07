#pragma once

class Tickable
{
public:
	bool* mutex{ nullptr };
	bool dead{ false };
	bool Visible{ true };
	bool Enabled{ true };
	std::vector<std::shared_ptr<Tickable>> tickables;

	virtual ~Tickable() {}
	virtual bool Tick(float) { return true; };
	virtual void Draw(float) {};
	virtual bool Character(unsigned int) { return false; }
	virtual bool Scancode(unsigned int) { return false; }
};

using TickableP = std::shared_ptr<Tickable>;
