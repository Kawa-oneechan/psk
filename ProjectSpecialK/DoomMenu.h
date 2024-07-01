#pragma once

#include "SpecialK.h"

class DoomMenuPage;

class DoomMenuItem
{
private:
	std::string key;
	std::vector<std::string> optionKeys;
public:
	std::string caption;
	std::string description;
	std::vector<std::string> options;
	int selection{ 0 };
	enum class Type
	{
		Text, Options, Slider, Checkbox, Page, Custom, Back
	} type { Type::Text };
	int minVal{ 0 };
	int maxVal{ 100 };
	int step{ 1 };
	std::function<std::string(DoomMenuItem*)> format;
	std::function<void(DoomMenuItem*)> change;
	DoomMenuPage* page{ nullptr };

	DoomMenuItem(const std::string& cap, int min, int max, int val, int stp, std::function<std::string(DoomMenuItem*)> fmt, std::function<void(DoomMenuItem*)> chg = nullptr) : key(cap), type(Type::Slider), minVal(min), maxVal(max), selection(val), step(stp), format(fmt), change(chg), page(nullptr) {}

	DoomMenuItem(const std::string& cap, DoomMenuPage* tgt) : key(cap), type(Type::Page), page(tgt) {}

	DoomMenuItem(const std::string& cap, bool val, std::function<void(DoomMenuItem*)> chg = nullptr) : key(cap), type(Type::Checkbox), selection(val ? 1 : 0), change(chg), page(nullptr) {}

	DoomMenuItem(const std::string& cap, int val, std::initializer_list<std::string> opts, std::function<void(DoomMenuItem*)> chg);

	DoomMenuItem(const std::string& cap, int fnt = 0, int siz = 100) : key(cap), type(Type::Text), selection(fnt), maxVal(siz), page(nullptr) {}

	void Translate();
};

class DoomMenuPage
{
public:
	std::string headerKey;
	std::string subKey;

	std::string header;
	std::string subheader;
	std::vector<DoomMenuItem*> items;
	
	DoomMenuPage() = default;

	DoomMenuPage(const std::string& hed, const std::string& sub) : headerKey(hed), subKey(sub) {}

	void Translate();
};

class DoomMenu : public Tickable
{
private:
	Texture panels{ Texture("ui/panels.png") };
	int highlight{ 0 };
	int mouseHighlight{ 0 };
	int scroll{ 0 };
	int visible{ 12 };

	DoomMenuPage options, content, volume, species;
	DoomMenuPage* items{ nullptr };

	std::stack<DoomMenuPage*> stack;

	std::vector<float> itemY;
	float itemX{ 0 };
	float sliderStart{ 0 };
	float sliderEnd{ 0 };
	int sliderHolding{ -1 };

	std::vector<Texture*> speciesPreviews;
	std::string speciesText;

	void Build();

public:
	DoomMenu();
	void Translate();
	void Tick(float dt);
	void Draw(float dt);
};
