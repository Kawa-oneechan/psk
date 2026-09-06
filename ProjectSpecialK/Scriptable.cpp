#include <sol.hpp>
#include "engine/Types.h"
#include "engine/Console.h"
#include "engine/Scripting.h"
#include "engine/Random.h"
#include "engine/VFS.h"
#include "Scriptable.h"

namespace Scripting
{
	extern sol::state* Sol;
}

Scriptable::~Scriptable()
{
	//delete currentCoro;
	currentCoro.reset();
	if (!ScriptID.empty())
		(*Scripting::Sol)[ScriptID] = nullptr;
}

void Scriptable::Bind(const std::string& file)
{
	if (ScriptID.empty())
		ScriptID = fmt::format("SCR_{:x}", Random::GetInt(0x10000, 0x20000));
	(*Scripting::Sol)[ScriptID] = (*Scripting::Sol).do_string(VFS::ReadString(file));
}

void Scriptable::Execute(const std::string& entryPoint, bool* mutex)
{
	if (ScriptID.empty())
	{
		conprint(4, "Can't run method {} on object without a script ID.", entryPoint);
		return;
	}

	Mutex = mutex;
	currentCoro = std::make_shared<sol::coroutine>((*Scripting::Sol)[ScriptID][entryPoint]);
}

bool Scriptable::CanRun() const
{
	return currentCoro && currentCoro->runnable();
}

void Scriptable::Call()
{
	//auto dlgBox = root.GetChild<DialogueBox>();
	//dlgBox->Mutex = Mutex;
	(*Scripting::Sol)["this"] = (*Scripting::Sol)[ScriptID];
	currentCoro->call();
}

sol::call_status Scriptable::Status()
{
	return currentCoro->status();
}
