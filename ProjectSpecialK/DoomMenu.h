#pragma once

#include "SpecialK.h"

typedef enum
{
	Text, Options, Slider, Checkbox, Page, Custom, Back
} DoomMenuTypes;

class DoomMenuItem
{
public:
	std::string caption = "???";
	std::vector<std::string> options;
	int selection = 0;
	DoomMenuTypes type = DoomMenuTypes::Text;
	int minVal = 0;
	int maxVal = 100;
	int step = 1;
	std::function<std::string(DoomMenuItem*)> format;
	std::function<void(DoomMenuItem*)> change;
	std::vector<DoomMenuItem*>* page;

	DoomMenuItem(const std::string& cap, int min, int max, int val, int stp, std::function<std::string(DoomMenuItem*)> fmt, std::function<void(DoomMenuItem*)> chg = nullptr) : caption(cap), type(DoomMenuTypes::Slider), minVal(min), maxVal(max), selection(val), step(stp), format(fmt), change(chg), page(nullptr) {}

	DoomMenuItem(const std::string& cap, std::vector<DoomMenuItem*>* tgt) : caption(cap), type(DoomMenuTypes::Page), page(tgt) {}

	DoomMenuItem(const std::string& cap, bool val, std::function<void(DoomMenuItem*)> chg = nullptr) : caption(cap), type(DoomMenuTypes::Checkbox), selection(val ? 1 : 0), change(chg), page(nullptr) {}

	DoomMenuItem(const std::string& cap, int val, std::initializer_list<std::string> opts, std::function<void(DoomMenuItem*)> chg);

	DoomMenuItem(const std::string& cap, int fnt = 0, int siz = 100) : caption(cap), type(DoomMenuTypes::Text), selection(fnt), maxVal(siz), page(nullptr) {}
};

class DoomMenu : public Tickable
{
private:
	Texture* controls;
	TextureAtlas controlsAtlas;
	std::vector<DoomMenuItem*>* items;
	int highlight, mouseHighlight;
	int scroll, visible = 12;

	std::vector<DoomMenuItem*> options;
	std::vector<DoomMenuItem*> content;
	std::vector<DoomMenuItem*> volume;

	std::stack<std::vector<DoomMenuItem*>> stack;

	std::vector<float> itemY;
	float itemX;
	float sliderStart, sliderEnd;
	int sliderHolding;

	void rebuild();

public:
	DoomMenu();
	void Tick(double dt);
	void Draw(double dt);
};
