#pragma once
#include <vector>
#include <memory>
#include <algorithm>
#include "InputsMap.h"

class Tickable;

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
	virtual bool Tick(float dt)
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
		if (iterating)
			addQueue.push_back(std::shared_ptr<Tickable>(newChild));
		else
			ChildTickables.push_back(std::shared_ptr<Tickable>(newChild));
	}
	void AddChild(std::shared_ptr<Tickable> newChild)
	{
		if (iterating)
			addQueue.push_back(newChild);
		else
			ChildTickables.push_back(newChild);
	}
	void RemoveChild(size_t i)
	{
		if (i >= ChildTickables.size())
			return;
		if (iterating)
			ChildTickables[i]->Dead = true;
		else
			ChildTickables.erase(ChildTickables.begin() + i);
	}
	void RemoveChild(const std::string& n)
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
	void RemoveChild(std::shared_ptr<Tickable> c)
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

	void RemoveAll()
	{
		if (iterating)
		{
			for (auto& e : ChildTickables)
				e->Dead = true;
		}
		else
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

	Tickable* operator[](size_t i) const
	{
		return GetChild<Tickable>(i);
	}
	Tickable* operator[](const std::string& n) const
	{
		auto it = std::find_if(ChildTickables.begin(), ChildTickables.end(), [n](auto e)
		{
			return e->ID == n;
		});
		if (it != ChildTickables.end())
			return it->get();
	}

	size_t size() const
	{
		return ChildTickables.size();
	}
};

using TickableP = std::shared_ptr<Tickable>;
