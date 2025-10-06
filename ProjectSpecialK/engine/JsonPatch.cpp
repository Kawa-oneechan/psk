#include "JsonUtils.h"
#ifdef DEEPERDOWN
#include "Console.h"
#endif

namespace JSONPatch
{

	//RFC 7396
	/*
	define MergePatch(Target, Patch):
		if Patch is an Object:
			if Target is not an Object:
				Target = {} # Ignore the contents and set it to an empty Object
			for each Name/Value pair in Patch:
				if Value is null:
					if Name exists in Target:
						remove the Name/Value pair from Target
					else:
						Target[Name] = MergePatch(Target[Name], Value)
			return Target
		else:
			return Patch
	*/

	static void mergeWorker(jsonValue& target, jsonValue& patch)
	{
		if (patch.is_object())
		{
			if (!target.is_object())
			{
				target = json5pp::object({});
			}
			else
			{
				//auto merged = target.as_object();
				for (const auto& p : patch.as_object())
				{
					if (p.second.is_null())
					{
						//merged.erase(p.first);
						target.as_object().erase(p.first);
					}
					else
					{
						//auto res = merged.insert(p);
						auto res = target.as_object().insert(p);
						if (!res.second)
							mergeWorker(res.first->second, static_cast<jsonValue&>(std::decay_t<jsonValue>(p.second)));
					}
				}
			}
		}
	}

	jsonValue& ApplyPatch(jsonValue& source, jsonValue& patch)
	{
#ifdef DEEPERDOWN
		conprint(0, "Original: {}", source.stringify5(json5pp::rule::tab_indent<>()));
		conprint(0, "Patch:    {}", patch.stringify5(json5pp::rule::tab_indent<>()));
		mergeWorker(source, patch);
		conprint(0, "New:      {}", source.stringify5(json5pp::rule::tab_indent<>()));
		console->Flush();
		return source;
#else
		mergeWorker(source, patch);
		return source;
#endif
	}
}
