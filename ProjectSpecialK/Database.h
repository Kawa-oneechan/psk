#pragma once

#include <vector>

//#define AMENABLE_TO_ERRORS

extern std::vector<Item> items;
extern std::vector<Species> species;
extern std::vector<Personality> personalities;
extern std::vector<Hobby> hobbies;
extern std::vector<Villager> villagers;

namespace Database
{
	typedef enum
	{
		None,
		Reference,
		Enumeration,
	} RefType;

	extern void LoadGlobalStuff();

	//Find a database entry by reference-ID. The type parameter, if not null, will be set to the kind of reference-ID passed.
	template<typename T>
	const T* Find(const std::string& target, std::vector<T>* source, RefType *type)
	{
		char* t = (char*)target.c_str();

#ifdef AMENABLE_TO_ERRORS
		if (type != nullptr)
		{
			if (*type == RefType::Reference && target[0] != '#')
				printf("WARNING: \"%s\" is not a #reference, should probably be \"#%s\"... but I'll let it slide.\n", target, target);
			else if (*type == RefType::Enumeration && target[0] != '$')
				printf("WARNING: \"%s\" is not an $enumeration, should probably be \"$%s\"... but I'll let it slide.\n", target, target);
			else if (*type == RefType::None && (target[0] != '#' || target[0] != '$'))
				printf("WARNING: \"%s\" is not supposed to be a #reference or #enumeration... but I'll let it slide.\n", target);
		}
#endif

		if (target[0] == '#')
		{
			if (type != nullptr)
				*type = RefType::Reference;
			t++;
		}
		else if (target[0] == '$')
		{
			if (type != nullptr)
				*type = RefType::Enumeration;
			t++;
		}
		else if (type != nullptr)
			*type = RefType::None;

		//for (const auto& v : source)
		for (int i = 0; i < source->size(); i++)
		{
			T* v = &source->at(i);
			if (v->ID == t)
			{
				return v;
			}
		}
		return nullptr;
	}

	//Find a database entry by reference-ID.
	template<typename T>
	const T* Find(const std::string& target, std::vector<T>* source, RefType type)
	{
		RefType type2;
		type2 = type;
		return Find<T>(target, source, type2);
	}

	//Find a database entry by reference-ID.
	template<typename T>
	const T* Find(const std::string& target, std::vector<T>* source)
	{
		return Find<T>(target, source, nullptr);
	}
}
