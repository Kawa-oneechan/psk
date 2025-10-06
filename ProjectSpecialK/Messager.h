#pragma once

#include <array>
#include "engine/Tickable.h"

class Messager : public Tickable
{
public:
	struct Message
	{
		std::string Text;
		float Lifetime;
		bool Persistent;
	};

	Messager();
	Message* Add(const std::string& text, bool persist = false);
	bool Tick(float dt) override;
	void Draw(float dt) override;

private:
	std::array<Message, 8> messages {};
	int cursor;
};

extern std::shared_ptr<Messager> messager;
