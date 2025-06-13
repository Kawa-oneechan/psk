#pragma once

#include "SpecialK.h"
#include "PanelLayout.h"

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
	bool Tick(float dt);
	void Draw(float dt);

private:
	std::array<Message, 8> messages;
	int cursor;
};

extern std::shared_ptr<Messager> messager;
