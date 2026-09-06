#pragma once
#include "engine/Types.h"
#include "engine/Scripting.h"
#include <sol.hpp>

class Scriptable
{
private:
	std::shared_ptr<sol::coroutine> currentCoro{ nullptr };

public:
	~Scriptable();

	void Bind(const std::string& file);
	void Execute(const std::string& entryPoint, bool* mutex);

	bool CanRun() const;
	void Call();
	sol::call_status Status();

	std::string ScriptID{ "" };
	bool* Mutex{ nullptr };
};
