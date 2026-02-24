#pragma once
#include <vector>
#include <memory>
#include <algorithm>
#include "Texture.h"
#include "SpriteRenderer.h"

class Tickable;
class Tickable2D;

class Tickable
{
private:
	std::vector<std::shared_ptr<Tickable>> addQueue;
	bool iterating = false;

protected:
	std::vector<std::shared_ptr<Tickable>> ChildTickables;

public:
	bool* Mutex{ nullptr };
	bool Dead{ false };
	bool Visible{ true };
	bool Enabled{ true };
	std::string ID;

	virtual ~Tickable() {}
	virtual bool Tick(float dt);
	virtual void Draw(float dt);
	virtual bool Character(unsigned int ch);
	virtual bool Scancode(unsigned int sc);

	void AddChild(Tickable* newChild);
	void AddChild(std::shared_ptr<Tickable> newChild);
	void RemoveChild(size_t i);
	void RemoveChild(const std::string& n);
	void RemoveChild(std::shared_ptr<Tickable> c);

	template<typename T>
	void RemoveChild()
	{
		for (int i = 0; i < ChildTickables.size(); i++)
		{
			auto e = std::dynamic_pointer_cast<T>(ChildTickables[i]);
			if (e)
			{
				RemoveChild(i);
				return;
			}
		}
	}

	void RemoveAll();

	template<typename T>
	T* GetChild(size_t i) const
	{
		if (i >= ChildTickables.size())
			return nullptr;
		return (T*)ChildTickables[i].get();
	}

	template<typename T>
	T* GetChild(const std::string& n) const
	{
		for (auto& i : ChildTickables)
		{
			auto e = std::dynamic_pointer_cast<T>(i);
			if (e)
			{
				if (n.empty())
					return e.get();
				else if (i->ID == n)
					return e.get();
			}
		}

		return nullptr;
	}

	template<typename T>
	T* GetChild() const
	{
		for (auto& i : ChildTickables)
		{
			auto e = std::dynamic_pointer_cast<T>(i);
			if (e)
				return e.get();
		}

		return nullptr;
	}

	Tickable* operator[](size_t i) const;
	Tickable* operator[](const std::string& n) const;

	size_t size() const;
};

using TickableP = std::shared_ptr<Tickable>;

class Tickable2D : public Tickable
{
protected:
	Tickable2D* parent{ nullptr };
public:
	glm::vec2 Position;
	glm::vec2 AbsolutePosition;

	virtual bool Tick(float dt) override;

	virtual glm::vec2 GetMinimalSize();

	virtual glm::vec2 GetSize();

	void UpdatePosition();
};

using Tickable2DP = std::shared_ptr<Tickable2D>;

class TextLabel : public Tickable2D
{
public:
	std::string Text;
	glm::vec4 Color{ 1, 1, 1, 1 };
	float Size{ 100.0f };
	float Angle{ 0.0f };
	int Font{ 1 };
	bool Raw{ false };

	TextLabel(const std::string& text, glm::vec2 position);

	void Draw(float) override;
};

using TextLabelP = std::shared_ptr<TextLabel>;

class SimpleSprite : public Tickable2D
{
private:
	std::shared_ptr<Texture> texture;
public:
	Sprite::SpriteFlags Flags{ Sprite::SpriteFlags::NoFlags };
	int Frame;
	float ImgScale{ 1.0f };
	glm::vec4 Color{ 1.0f };

	SimpleSprite(const std::string& texture, int frame, glm::vec2 position);

	SimpleSprite(Texture* texture, int frame, glm::vec2 position);

	~SimpleSprite() override;

	void Draw(float) override;
};
