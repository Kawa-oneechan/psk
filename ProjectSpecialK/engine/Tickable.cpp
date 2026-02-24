#include "Tickable.h"
#include "InputsMap.h"

class Tickable;
class Tickable2D;

bool Tickable::Tick(float dt)
{
	iterating = true;
	for (unsigned int i = (unsigned int)ChildTickables.size(); i-- > 0; )
	{
		auto t = ChildTickables[i];
		if (!t->Enabled)
			continue;
		if (!t->Tick(dt))
			Inputs.Clear();
	}
	iterating = false;

	ChildTickables.erase(std::remove_if(ChildTickables.begin(), ChildTickables.end(), [](std::shared_ptr<Tickable> i)
	{
		return i->Dead;
	}), ChildTickables.end());

	if (addQueue.size() > 0)
	{
		for (const auto& t : addQueue)
			AddChild(t);
		addQueue.clear();
	}

	return true;
}

void Tickable::Draw(float dt)
{
	for (const auto& t : ChildTickables)
	{
		if (!t->Visible)
			continue;
		t->Draw(dt);
	}
}

bool Tickable::Character(unsigned int ch)
{
	for (unsigned int i = (unsigned int)ChildTickables.size(); i-- > 0; )
	{
		auto t = ChildTickables[i];
		if (!t->Enabled)
			continue;
		if (t->Character(ch))
			return true;
	}
	return false;
}

bool Tickable::Scancode(unsigned int sc)
{
	for (unsigned int i = (unsigned int)ChildTickables.size(); i-- > 0; )
	{
		auto t = ChildTickables[i];
		if (!t->Enabled)
			continue;
		if (t->Scancode(sc))
			return true;
	}
	return false;
}

void Tickable::AddChild(Tickable* newChild)
{
	if (iterating)
		addQueue.push_back(std::shared_ptr<Tickable>(newChild));
	else
		ChildTickables.push_back(std::shared_ptr<Tickable>(newChild));
}

void Tickable::AddChild(std::shared_ptr<Tickable> newChild)
{
	if (iterating)
		addQueue.push_back(newChild);
	else
		ChildTickables.push_back(newChild);
}

void Tickable::RemoveChild(size_t i)
{
	if (i >= ChildTickables.size())
		return;
	if (iterating)
		ChildTickables[i]->Dead = true;
	else
		ChildTickables.erase(ChildTickables.begin() + i);
}

void Tickable::RemoveChild(const std::string& n)
{
	auto it = std::find_if(ChildTickables.begin(), ChildTickables.end(), [n](auto e)
	{
		return e->ID == n;
	});
	if (it != ChildTickables.end())
	{
		if (iterating)
			it->get()->Dead = true;
		else
			ChildTickables.erase(it);
	}
}

void Tickable::RemoveChild(std::shared_ptr<Tickable> c)
{
	auto it = std::find_if(ChildTickables.begin(), ChildTickables.end(), [c](auto e)
	{
		return e == c;
	});
	if (it != ChildTickables.end())
	{
		if (iterating)
			it->get()->Dead = true;
		else
			ChildTickables.erase(it);
	}
}

void Tickable::RemoveAll()
{
	if (iterating)
	{
		for (const auto& e : ChildTickables)
			e->Dead = true;
	}
	else
		ChildTickables.clear();
}

Tickable* Tickable::operator[](size_t i) const
{
	return GetChild<Tickable>(i);
}

Tickable* Tickable::operator[](const std::string& n) const
{
	auto it = std::find_if(ChildTickables.begin(), ChildTickables.end(), [n](auto e)
	{
		return e->ID == n;
	});
	if (it != ChildTickables.end())
		return it->get();
	return nullptr;
}

size_t Tickable::size() const
{
	return ChildTickables.size();
}

using TickableP = std::shared_ptr<Tickable>;



bool Tickable2D::Tick(float dt)
{
	AbsolutePosition = (parent ? (parent->AbsolutePosition + Position) : Position);

	for (int i = 0; i < ChildTickables.size(); i++)
	{
		if (auto t2D = std::dynamic_pointer_cast<Tickable2D>(ChildTickables[i]))
			t2D->parent = this;
	}

	return Tickable::Tick(dt);
}

glm::vec2 Tickable2D::GetMinimalSize()
{
	auto ret = glm::vec2(0);
	for (int i = 0; i < ChildTickables.size(); i++)
	{
		if (auto t2D = std::dynamic_pointer_cast<Tickable2D>(ChildTickables[i]))
		{
			auto tl = t2D->Position;
			auto br = tl + t2D->GetSize();
			if (br.x > ret.x)
				ret.x = br.x;
			if (br.y > ret.y)
				ret.y = br.y;
		}
	}
	return ret;
}

glm::vec2 Tickable2D::GetSize()
{
	return GetMinimalSize();
}

void Tickable2D::UpdatePosition()
{
	if (parent)
	{
		AbsolutePosition = parent->AbsolutePosition + Position;
	}
	else
	{
		AbsolutePosition = Position;
	}
	for (int i = 0; i < ChildTickables.size(); i++)
	{
		if (auto t2D = std::dynamic_pointer_cast<Tickable2D>(ChildTickables[i]))
			t2D->UpdatePosition();
	}
}

using Tickable2DP = std::shared_ptr<Tickable2D>;



TextLabel::TextLabel(const std::string& text, glm::vec2 position) : Text(text)
{
	parent = nullptr;
	Position = position;
}

void TextLabel::Draw(float)
{
	Sprite::DrawText(Font, Text, AbsolutePosition, Color, Size, Angle, Raw);
}

SimpleSprite::SimpleSprite(const std::string& texture, int frame, glm::vec2 position)
{
	parent = nullptr;
	this->texture = std::make_shared<Texture>(texture);
	Position = position;
	Frame = frame;
}

SimpleSprite::SimpleSprite(Texture* texture, int frame, glm::vec2 position)
{
	parent = nullptr;
	this->texture = std::shared_ptr<Texture>(texture);
	Position = position;
	Frame = frame;
}

SimpleSprite::~SimpleSprite()
{
	//delete this->texture;
}

void SimpleSprite::Draw(float)
{
	auto frame = texture->operator[](Frame);
	auto scaledSize = glm::vec2(frame.z, frame.w) * ImgScale;
	Sprite::DrawSprite(*texture, AbsolutePosition, scaledSize, frame, 0.0f, Color, Flags);
}
