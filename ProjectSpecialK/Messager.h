#pragma once

#include "SpecialK.h"
#include "PanelLayout.h"

class Messager : public Tickable
{
	struct Message
	{
		std::string Text;
		float Lifetime;
		bool Persistent;
	};

private:
	std::array<Message, 8> messages;
	int cursor;

public:
	Messager();
	Message Add(const std::string& text, bool persist = false);
	void Tick(float dt);
	void Draw(float dt);
};
