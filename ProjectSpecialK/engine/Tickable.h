#pragma once
#include <vector>
#include <memory>

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
	virtual bool Character(unsigned int ch)
	{
		for (unsigned int i = (unsigned int)tickables.size(); i-- > 0; )
		{
			auto t = tickables[i];
			if (!t->Enabled)
				continue;
			if (t->Character(ch))
				return true;
		}
		return false;
	}
	virtual bool Scancode(unsigned int sc)
	{
		for (unsigned int i = (unsigned int)tickables.size(); i-- > 0; )
		{
			auto t = tickables[i];
			if (!t->Enabled)
				continue;
			if (t->Scancode(sc))
				return true;
		}
		return false;
	}
};

using TickableP = std::shared_ptr<Tickable>;
