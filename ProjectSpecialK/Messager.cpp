#include "Messager.h"

Messager::Messager()
{
	cursor = 0;
}

Messager::Message* Messager::Add(const std::string& text, bool persist)
{
	auto message = &messages[cursor];
	message->Text = text;
	message->Lifetime = text.size() * 1.0f;
	message->Persistent = persist;
	conprint(8, text);
	cursor++;
	if (cursor == messages.size())
		cursor = 0;
	return message;
}

bool Messager::Tick(float dt)
{
	for (auto& message : messages)
	{
		if (message.Persistent || message.Lifetime <= 0.0f)
			continue;
		message.Lifetime -= dt * 5.0f;
		if (message.Lifetime < 0.0f)
		{
			message.Text.clear();
			message.Lifetime = 0.0f;
		}
	}
	return true;
}

void Messager::Draw(float dt)
{
	dt;
	auto pos = glm::vec2(8, 8);
	for (const auto& message : messages)
	{
		if (message.Text.empty())
			continue;
		auto color = glm::vec4(1);
		if (message.Lifetime < 1.0f)
			color.a = glm::mix(0.0f, 1.0f, message.Lifetime);
		Sprite::DrawText(1, message.Text, pos, color);
		pos.y += 32.0f;
	}
}
