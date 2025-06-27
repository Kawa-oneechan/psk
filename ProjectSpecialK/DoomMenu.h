#pragma once

#include "InputsMap.h"
#include "ButtonGuide.h"

class DoomMenuPage;

class DoomMenuItem
{
private:
	std::string key;
	std::vector<std::string> optionKeys;
public:
	std::string caption;
	std::string description;
	std::string formatText;
	std::vector<std::string> options;
	int selection{ 0 };
	enum class Type
	{
		Text, Options, Slider, Checkbox, Page, KeyBind, Action, Custom
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

	DoomMenuItem(const std::string& cap, Binds bind) : key(cap), type(Type::KeyBind), selection((int)bind) {}

	DoomMenuItem(const std::string& cap, std::function<void(DoomMenuItem*)> chg) : key(cap), type(Type::Action), change(chg) {}

	void Translate();

	void Beep();
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

	DoomMenuPage(const std::string& hed, const std::string& sub) : headerKey(hed), subKey(sub), DrawSpecial(nullptr) {}

	void Translate();
	std::function<void(DoomMenuPage*, DoomMenuItem*)> DrawSpecial;
};

class DoomMenu : public Tickable
{
protected:
	Texture panels{ Texture("ui/panels.png") };
	int highlight{ 0 };
	int mouseHighlight{ 0 };
	int scroll{ 0 };
	int visible{ 12 }; //TODO: determine this dynamically
	int remapping{ -1 };
	bool remapBounce{ false };

	DoomMenuPage* items{ nullptr };

	std::stack<DoomMenuPage*> stack;

	std::vector<float> itemY;
	float itemX{ 0 };
	float sliderStart{ 0 };
	float sliderEnd{ 0 };
	int sliderHolding{ -1 };

	ButtonGuide buttonGuide;

	void UpdateButtonGuide();

public:
	DoomMenu();
	void Translate();
	bool Tick(float dt);
	void Draw(float dt);
};
