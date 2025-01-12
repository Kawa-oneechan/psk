#include <JSON/JSON.h>

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

	static JSONValue* mergeWorker(JSONValue* target, JSONValue* patch)
	{
		if (patch->IsObject())
		{
			if (!target->IsObject())
			{
				target = new JSONValue(*(new JSONObject()));
				return target;
			}
			else
			{
				auto merged = target->AsObject();
				for (const auto& p : patch->AsObject())
				{
					if (p.second->IsNull())
					{
						merged.erase(p.first);
					}
					else
					{
						auto res = merged.insert(p);
						if (!res.second)
							res.first->second = mergeWorker(res.first->second, p.second);
					}
				}
				return new JSONValue(merged);
			}
		}
		return patch;
	}

	JSONValue* ApplyPatch(JSONValue& source, JSONValue& patch)
	{
#if 0
		conprint(0, "Original: %s", source.Stringify());
		conprint(0, "Patch:    %s", patch.Stringify());
		auto ret = mergeWorker(&source, &patch);
		conprint(0, "New:      %s", ret->Stringify());
		return ret;
#else
		return mergeWorker(&source, &patch);
#endif
	}
}