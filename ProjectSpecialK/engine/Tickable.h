#pragma once
#include <vector>
#include <memory>
#include <algorithm>
#include "InputsMap.h"

class Tickable;

class Tickable
{
protected:
	std::vector<std::shared_ptr<Tickable>> ChildTickables;
public:
	bool* Mutex{ nullptr };
	bool Dead{ false };
	bool Visible{ true };
	bool Enabled{ true };
	std::string ID;

	virtual ~Tickable() {}
	virtual bool Tick(float dt)
	{
		for (unsigned int i = (unsigned int)ChildTickables.size(); i-- > 0; )
		{
			auto t = ChildTickables[i];
			if (!t->Enabled)
				continue;
			if (!t->Tick(dt))
				Inputs.Clear();
		}
		return true;
	}

	virtual void Draw(float dt)
	{
		for (const auto& t : ChildTickables)
		{
			if (!t->Visible)
				continue;
			t->Draw(dt);
		}
	}
	virtual bool Character(unsigned int ch)
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
	virtual bool Scancode(unsigned int sc)
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

	void AddChild(Tickable* newChild)
	{
		ChildTickables.push_back(std::shared_ptr<Tickable>(newChild));
	}
	void AddChild(std::shared_ptr<Tickable> newChild)
	{
		ChildTickables.push_back(newChild);
	}
	void RemoveChild(size_t i)
	{
		if (i >= ChildTickables.size())
			return;
		ChildTickables.erase(ChildTickables.begin() + i);
	}
	void RemoveChild(const std::string& n)
	{
		auto it = std::find_if(ChildTickables.begin(), ChildTickables.end(), [n](auto e)
		{
			return e->ID == n;
		});
		if (it != ChildTickables.end())
			ChildTickables.erase(it);
	}
	void RemoveChild(std::shared_ptr<Tickable> c)
	{
		auto it = std::find_if(ChildTickables.begin(), ChildTickables.end(), [c](auto e)
		{
			return e == c;
		});
		if (it != ChildTickables.end())
			ChildTickables.erase(it);
	}
	void RemoveAll()
	{
		ChildTickables.clear();
	}
	
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
		for (auto i : ChildTickables)
		{
			if (i->ID == n)
				return (T*)(i.get());
		}
		return nullptr;
	}
	
	Tickable* operator[](size_t i) const
	{
		return GetChild<Tickable>(i);
	}
	Tickable* operator[](const std::string& n) const
	{
		return GetChild<Tickable>(n);
	}

};

using TickableP = std::shared_ptr<Tickable>;
