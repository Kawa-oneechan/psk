#include <cstdio>
#include <ctime>
#include <cstdarg>
#include <algorithm>
#include <stack>
#include <functional>
#include "support/glad/glad.h"
#include <GLFW/glfw3.h>
#include "support/glm/glm.hpp"
#include "support/glm/gtc/matrix_transform.hpp"
#include "support/glm/gtc/type_ptr.hpp"
#include "support/stb_image.h"
#include "support/format.h"

#include "Shader.h"
#include "Texture.h"
#include "SpriteRenderer.h"
#include "Camera.h"

#include "VFS.h"

#define SCR_WIDTH 1920
#define SCR_HEIGHT 1080
#define WIN_WIDTH 1920
#define WIN_HEIGHT 1080

GLFWwindow* window;

Shader* spriteShader = nullptr;
Texture* whiteRect = nullptr;
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
SpriteRenderer* sprender = nullptr;

float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

bool wireframe = false;

float deltaTime = 0.0f;
float lastFrame = 0.0f;

float width = SCR_WIDTH, height = SCR_HEIGHT;
float scale = (float)WIN_WIDTH / (float)SCR_HEIGHT;

class InputsMap
{
public:
	bool Up, Down, Left, Right;
	bool Enter, Escape;

	void Process(int key, int action)
	{
		if (action == GLFW_PRESS)
		{
			switch (key)
			{
			case GLFW_KEY_UP: Up = true; break;
			case GLFW_KEY_DOWN: Down = true; break;
			case GLFW_KEY_LEFT: Left = true; break;
			case GLFW_KEY_RIGHT: Right = true; break;
			case GLFW_KEY_ENTER: Enter = true; break;
			case GLFW_KEY_ESCAPE: Escape = true; break;
			default: break;
			}
		}
	}

	void Clear()
	{
		Up = Down = Left = Right = Enter = Escape = false;
	}
};
static InputsMap& Inputs = InputsMap();

glm::vec2 GetJSONVec2(JSONValue* val)
{
	if (!val->IsArray())
		throw std::runtime_error("GetJSONVec2: given value is not an array.");
	auto arr = val->AsArray();
	if (arr.size() != 2)
		throw std::runtime_error(fmt::format("GetJSONVec2: given array has {} entries, not 2.", arr.size()));
	if (!arr[0]->IsNumber() || !arr[1]->IsNumber())
		throw std::runtime_error("GetJSONVec2: given array does not contain only numbers.");
	return glm::vec2(arr[0]->AsNumber(), arr[1]->AsNumber());
}

typedef std::vector<glm::vec4> TextureAtlas;

void GetAtlas(TextureAtlas &ret, const std::string& jsonFile)
{
	auto doc = ReadJSON(jsonFile)->AsObject();
	ret.clear();
	if (doc["type"]->AsString() == "simple")
	{
		auto size = GetJSONVec2(doc["size"]);
		auto dims = GetJSONVec2(doc["dims"]);
		for (int y = 0; y < (int)dims[0]; y++)
		{
			for (int x = 0; x < (int)dims[1]; x++)
			{
				ret.push_back(glm::vec4(x * size[0], y * size[1], size[0], size[1]));
			}
		}
		return;
	}
	throw std::runtime_error(fmt::format("GetAtlas: file {} has an unknown type \"{}\".", jsonFile, doc["type"]->AsString()));
}

template<typename T> static T clamp(T val, T minval, T maxval) { return std::max<T>(std::min<T>(val, maxval), minval); }

class Tickable
{
public:
	virtual void Tick(double dt) {};
	virtual void Draw(double dt) {};
};

class Cursor
{
private:
	Texture* hand;
	TextureAtlas atlas;
	glm::vec2 hotspot;
	glm::vec4 frame;
	glm::vec2 size;
	float lastX, lastY;

public:
	bool Left, Right, Middle;
	glm::vec2 Position;

	Cursor()
	{
		hand = new Texture("hand.png");
		GetAtlas(atlas, "hand.json");
		Select(0);
		size = glm::vec2(frame.w / 2);
		lastX = Position.x = width + 20;
		lastY = Position.y = height + 20;
		Left = Right = Middle = false;
	}

	void Select(int style)
	{
		frame = atlas[style];
	}

	void Move(float x, float y)
	{
		lastX = Position.x;
		lastY = Position.y;
		Position.x = x;
		Position.y = y;
	}

	bool Moved()
	{
		auto ret = (lastX != Position.x || lastY != Position.y);
		lastX = Position.x;
		lastY = Position.y;
		return ret;
	}

	void Draw()
	{
		sprender->DrawSprite(*hand, Position + hotspot, size, frame);
	}
};
Cursor* cursor = nullptr;

class UITest : public Tickable
{
	void Tick(double dt)
	{

	}

	void Draw(double dt)
	{

	}
};

class Background : public Tickable
{
private:
	Texture* wallpaper;
	Shader* scroller;
	float time;
public:
	Background()
	{
		wallpaper = new Texture("90s-and-80s-style-seamless-pattern-vector-38114882.jpg");
		scroller = new Shader("sprite.vs", "scroller.fs");
		time = 0;
	}
	void Draw(double dt)
	{
		time += (float)dt * 0.005f;

		scroller->Use();
		scroller->SetFloat("time", time);
		sprender->DrawSprite(*scroller, *wallpaper, glm::vec2(0), glm::vec2(width, height), glm::vec4(0,0,width,height));
		sprender->DrawText(1, "Project Special K: UI test", glm::vec2(0), glm::vec4(1, 1, 1, 0.75), 50);
		//sprender->DrawText(1, fmt::format("Project Special K: UI test (SCALE = {})", scale), glm::vec2(0), glm::vec4(1, 1, 1, 0.75), 50);
		//sprender->DrawText(1, u8"日", glm::vec2(0, 20), glm::vec4(1, 1, 1, 0.75), 50);
		//sprender->DrawText(1, u8"ā", glm::vec2(0), glm::vec4(1, 1, 1, 0.75), 200);
	}
};

//TEMPORARY -- is in Text.cpp in real PSK
extern std::vector<std::string> Split(std::string& data, char delimiter);

class DialogueBox : public Tickable
{
private:
	Texture* bubble[5];
	Texture* gradient[2];
	Texture* nametag;
	Shader* wobble;
	std::string displayed;
	std::string toDisplay;
	size_t displayCursor;
	float time;
	glm::vec4 bubbleColor;
	glm::vec4 textColor;
	glm::vec4 nametagColor[2];
	std::string name;
	int font;
	int bubbleNum;

	float delay;

	void msbtPass(MSBTParams)
	{
		displayed += toDisplay.substr(start, len);
	}

	typedef void(DialogueBox::*MSBTFunc)(MSBTParams);
	const std::map<std::string, MSBTFunc> msbtPhase3
	{
		{ "color", &DialogueBox::msbtPass },
		{ "/color", &DialogueBox::msbtPass },
		{ "size", &DialogueBox::msbtPass },
		{ "/size", &DialogueBox::msbtPass },
		{ "font", &DialogueBox::msbtPass },
		{ "/font", &DialogueBox::msbtPass },
	};

public:

	DialogueBox()
	{
		bubble[0] = new Texture("dialogue.png");
		bubble[1] = new Texture("dialogue_exclamation.png");
		bubble[2] = new Texture("dialogue_dream.png");
		bubble[3] = new Texture("dialogue_system.png");
		//bubble[4] = new Texture("dialogue_wild.png", true, GL_REPEAT, GL_NEAREST);
		gradient[0] = new Texture("gradient_thin.png");
		gradient[1] = new Texture("gradient_wide.png");
		nametag = new Texture("dialogue_name.png");
		wobble = new Shader("wobble.fs");

		toDisplay.clear(); //u8"Truth is... <color:1>the game</color> was rigged\nfrom the start.";
		displayCursor = 0;
		time = 0;
		delay = 0;
		name = "Isabelle";
		nametagColor[0] = glm::vec4(1, 0.98f, 0.56f, 1);
		nametagColor[1] = glm::vec4(0.96f, 0.67f, 0.05f, 1);

		Style(0);
	}

	void Text(const std::string& text)
	{
		displayed.clear();
		toDisplay = text;
		displayCursor = 0;
		delay = 50;
	}

	void Style(int style)
	{
		style = clamp(style, 0, 3);

		if (style == 3) //system
		{
			bubbleColor = glm::vec4(0.15, 0.3, 0.43, 0.75);
			textColor = glm::vec4(0.84, 0.98, 0.99, 1);
			font = 1;
			bubbleNum = 3;
		}
		else
		{
			bubbleColor = glm::vec4(1, 0.98, 0.89, 1);
			textColor = glm::vec4(0.51, 0.45, 0.34, 1);
			font = 2;
			bubbleNum = style;
		}
	}

	void Draw(double dt)
	{
		time += (float)dt * 0.005f;

		auto dlgScale = scale;
		auto dlgWidth = bubble[0]->width * dlgScale;
		auto dlgHeight = bubble[0]->height * dlgScale;
		auto dlgLeft = (int)(width / 2) - dlgWidth;
		auto dlgTop = (int)height - dlgHeight - 10;

		wobble->Use();
		gradient[0]->Use(1);
		gradient[1]->Use(2);
		wobble->SetInt("gradient1", 1);
		wobble->SetInt("gradient2", 2);
		wobble->SetFloat("time", time);

		//if (bubbleNum == 4)
		//	sprender->DrawSprite(*bubble[bubbleNum], glm::vec2(dlgLeft, dlgTop), glm::vec2(dlgWidth * 2, dlgHeight), glm::vec4(0));
		//else
		{
			sprender->DrawSprite(*wobble, *bubble[bubbleNum], glm::vec2(dlgLeft, dlgTop), glm::vec2(dlgWidth, dlgHeight), glm::vec4(0), 0, bubbleColor, 0);
			sprender->DrawSprite(*wobble, *bubble[bubbleNum], glm::vec2(dlgLeft + dlgWidth, dlgTop), glm::vec2(dlgWidth, dlgHeight), glm::vec4(0, 0, bubble[0]->width, bubble[0]->height), 0, bubbleColor, 1);
		}

		sprender->DrawText(font, displayed, glm::vec2(dlgLeft + (200 * scale), dlgTop + (100 * scale)), textColor, 150 * scale);

		if (!name.empty())
		{
			auto tagPos = glm::vec2(dlgLeft + (150 * scale), dlgTop + (sinf(time * 2) * 10) * scale);
			auto tagSize = glm::vec2(nametag->width, nametag->height) * scale;
			auto tagMidWidth = 128;
			auto tagSrc = glm::vec4(0.45, 0.25, nametag->width - 0.25, nametag->height - 0.25);

			//TODO: draw this at any angle
			sprender->DrawSprite(*nametag, tagPos, tagSize, tagSrc, -1.0f, nametagColor[0]);
			sprender->DrawSprite(*whiteRect, tagPos + glm::vec2(nametag->width - 0.75, 11 - 1.5) * scale, glm::vec2(tagMidWidth, 72) * scale, glm::vec4(0), -1.0f, nametagColor[0]);
			sprender->DrawSprite(*nametag, tagPos + glm::vec2(nametag->width + tagMidWidth - 1.75, -3) * scale, tagSize, tagSrc, -1.0f, nametagColor[0], 1);
			sprender->DrawText(1, name, tagPos + (glm::vec2(48, 24) * scale), nametagColor[1], 120 * scale, -1.0f);
		}

		//sprender->DrawText(1, fmt::format("DialogueBox: {}", time), glm::vec2(0, 16), glm::vec4(0, 0, 0, 0.25), 50);
	}

	void Tick(double dt)
	{
		delay -= (float)dt;
		if (delay > 0)
			return;

		if (displayCursor >= toDisplay.length())
			return;

		//We use UTF-8 to store strings but display in UTF-16.
		unsigned int ch = toDisplay[displayCursor++] & 0xFF;
		if ((ch & 0xE0) == 0xC0)
		{
			ch = (ch & 0x1F) << 6;
			ch |= (toDisplay[displayCursor++] & 0x3F);
		}
		else if ((ch & 0xF0) == 0xE0)
		{
			ch = (ch & 0x1F) << 12;
			ch |= (toDisplay[displayCursor++] & 0x3F) << 6;
			ch |= (toDisplay[displayCursor++] & 0x3F);
		}

		if (ch == '<')
		{
			auto msbtEnd = toDisplay.find_first_of('>', displayCursor);
			auto msbtStart = displayCursor;
			displayCursor = msbtEnd + 1;

			auto msbtWhole = toDisplay.substr(msbtStart, msbtEnd - msbtStart);
			auto msbt = Split(msbtWhole, ':');
			auto func = msbtPhase3.find(msbt[0]);
			if (func != msbtPhase3.end())
			{
				std::invoke(func->second, this, msbt, (int)msbtStart - 1, (int)(msbtEnd - msbtStart) + 2);
				//func->second(msbt, (int)msbtStart - 1, (int)(msbtEnd - msbtStart) + 2);
			}
		}
		else
		{
			//Gotta re-encode the UTF-16 to UTF-8 here.
			if (ch < 0x80)
				displayed += ch;
			else if (ch < 0x0800)
			{
				displayed += (char)(((ch >> 6) & 0x1F) | 0xC0);
				displayed += (char)(((ch >> 0) & 0x3F) | 0x80);
			}
			else if (ch < 0x10000)
			{
				displayed += (char)(((ch >> 12) & 0x0F) | 0xE0);
				displayed += (char)(((ch >> 6) & 0x3F) | 0x80);
				displayed += (char)(((ch >> 0) & 0x3F) | 0x80);
			}
		}
		delay = 50;
	}
};
DialogueBox* dlgBox = nullptr;

typedef enum
{
	Invalid, Text, Options, Slider, Checkbox, Page, Custom, Back
} DoomMenuTypes;

class DoomMenuItem
{
public:
	std::string caption = "???";
	std::vector<std::string> options;
	int selection = 0;
	DoomMenuTypes type = DoomMenuTypes::Invalid;
	int minVal = 0;
	int maxVal = 100;
	int step = 1;
	std::function<std::string(DoomMenuItem*)> format;
	std::function<void(DoomMenuItem*)> change;
	std::vector<DoomMenuItem*>* page;

	DoomMenuItem(const std::string& cap, int min, int max, int val, int stp, std::function<std::string(DoomMenuItem*)> fmt, std::function<void(DoomMenuItem*)> chg = nullptr) : caption(cap), type(DoomMenuTypes::Slider), minVal(min), maxVal(max), selection(val), step(stp), format(fmt), change(chg), page(nullptr)
	{
	}

	DoomMenuItem(const std::string& cap, std::vector<DoomMenuItem*>* tgt) : caption(cap), type(DoomMenuTypes::Page), page(tgt)
	{
	}

	DoomMenuItem(const std::string& cap, bool val, std::function<void(DoomMenuItem*)> chg = nullptr) : caption(cap), type(DoomMenuTypes::Checkbox), selection(val ? 1 : 0), change(chg), page(nullptr)
	{
	}

	DoomMenuItem(const std::string& cap, int val, std::initializer_list<std::string> opts, std::function<void(DoomMenuItem*)> chg = nullptr) : caption(cap), type(DoomMenuTypes::Options), selection(val), change(chg), page(nullptr)
	{
		for (auto i : opts)
			options.emplace_back(i);
	}

	DoomMenuItem(const std::string& cap, int fnt = 0, int siz = 100) : caption(cap), type(DoomMenuTypes::Text), selection(fnt), maxVal(siz), page(nullptr)
	{
	}
};

class DoomMenu : public Tickable
{
private:
	Texture* scrollbar;
	TextureAtlas scrollbarAtlas;
	Texture* checkbox;
	TextureAtlas checkboxAtlas;
	std::vector<DoomMenuItem*>* items;
	int highlight, mouseHighlight;
	int scroll, visible = 12;

	std::vector<DoomMenuItem*> options;
	std::vector<DoomMenuItem*> content;
	std::vector<DoomMenuItem*> volume;

	std::stack<std::vector<DoomMenuItem*>> stack;

	void rebuild()
	{
		auto back = new DoomMenuItem("Back", nullptr);
		back->type = DoomMenuTypes::Back;

		auto minutes = [&](DoomMenuItem* i)
		{
			return fmt::format("{} minutes", i->selection);
		};
		auto percent = [&](DoomMenuItem* i)
		{
			return fmt::format("{}%", i->selection);
		};

		options.clear();
		content.clear();
		volume.clear();

		options.push_back(new DoomMenuItem("Options", 2, 120));

		options.push_back(new DoomMenuItem("Content Manager...", &content));
		options.push_back(new DoomMenuItem("Language", 0, { "US English", u8"Japanese / 日本語", "German / Deutsch", "Spanish / español", u8"French / français", "Italian / italiano", "Hungarian / magyar", "Dutch / Nederlands" }, [&](DoomMenuItem*i) { dlgBox->Text(fmt::format("You chose <color:1>{}</color>.", i->options[i->selection])); }));
		options.push_back(new DoomMenuItem("Continue from", 0, { "Front door", "Main room", "Last used bed", "Last location" }));
		options.push_back(new DoomMenuItem("Speech", 1, { "Silence", "Bebebese", "Animalese" }));
		options.push_back(new DoomMenuItem("Ping rate", 2, 60, 3, 1, minutes));
		options.push_back(new DoomMenuItem("Balloon chance", 10, 60, 15, 5, percent));
		options.push_back(new DoomMenuItem("Volume...", &volume));

		content.push_back(new DoomMenuItem("Content Manager", 2, 120));
		content.push_back(new DoomMenuItem("Venomous bugs <size:50>(tarantulas, scorpions et al)", true));
		content.push_back(new DoomMenuItem("Sea bass", true, [&](DoomMenuItem*i)
		{
			dlgBox->Text(i->selection ? "Whatever you say..." : "Aye aye, Miss Mayor! We'll start\npouring anti-freeze in their\nspawning grounds right away!");
		}));
		content.push_back(new DoomMenuItem("Cranky villagers", true));
		content.push_back(new DoomMenuItem("Horse villagers", true));
		content.push_back(new DoomMenuItem("Easter", true));
		content.push_back(back);

		volume.push_back(new DoomMenuItem("Volume", 2, 120));
		volume.push_back(new DoomMenuItem("Music", 0, 100, 70, 10, percent));
		volume.push_back(new DoomMenuItem("Ambience", 0, 100, 70, 10, percent));
		volume.push_back(new DoomMenuItem("Sound effects", 0, 100, 70, 10, percent));
		volume.push_back(new DoomMenuItem("Speech", 0, 100, 70, 10, percent));
		volume.push_back(back);
	}

public:
	DoomMenu()
	{
		scrollbar = new Texture("scrollbar.png");
		GetAtlas(scrollbarAtlas, "scrollbar.json");
		checkbox = new Texture("checkbox.png");
		GetAtlas(checkboxAtlas, "checkbox.json");

		scrollbarAtlas[1].z /= 2;
		//Quick hack to use less of the scrollbar track's area
		//and prevent blending. Alternatively, you'd either have both end parts in the
		//sheet instead of flipping, or use manual coordinates.

		rebuild();

		highlight = 0;
		mouseHighlight = 0;
		scroll = 0;

		stack.push(options);
		items = &stack.top();
	}

	void Tick(double dt)
	{
		//visible = (int)(12.0f * scale);

		/*
		if (cursor->Moved())
		{
			//TODO: memorize heights of items
			mouseHighlight = -1;
			if (cursor->Position.x >= 80 && cursor->Position.x <= 80 + 600)
			{
				if (cursor->Position.y >= 80 && cursor->Position.y < 80 + (items->size() * 30))
				{
					mouseHighlight = highlight = (int)((cursor->Position.y - 80) / 30);
				}
			}
		}
		*/

		while (items->at(highlight)->type == DoomMenuTypes::Text)
			highlight++;

		if (Inputs.Escape)
		{
			Inputs.Escape = false;
			if (stack.size() > 1)
			{
				highlight = 0;
				stack.pop();
				items = &stack.top();
				return;
			}
		}

		if (Inputs.Up)
		{
			Inputs.Clear();
			int top = items->at(0)->type == DoomMenuTypes::Text ? 1 : 0;
			if (highlight == top)
			{
				highlight = (int)items->size();
				scroll = (int)items->size() - visible;
				if (scroll < 0) scroll = 0;
			}
			highlight--;
			if (highlight <= scroll)
				scroll--;
			if (scroll == -1)
			{
				scroll = 0;
				highlight = top;
			}
			while (items->at(highlight - scroll)->type == DoomMenuTypes::Text)
			{
				if (highlight - scroll == top)
				{
					highlight = (int)items->size();
					scroll = (int)items->size() - visible;
				}
				highlight--;
			}
		}
		else if (Inputs.Down)
		{
			Inputs.Clear();
			highlight++;
			if (highlight - scroll >= visible)
				scroll++;
			if (highlight == items->size())
			{
				highlight = 0;
				scroll = 0;
			}
		}

		if (highlight == -1)
			return;

		auto item = items->at(highlight);

		if (item->type == DoomMenuTypes::Page)
		{
			if (Inputs.Enter)
			{
				stack.push(*item->page);
				items = item->page;
				highlight = 0;
			}
		}
		else if (item->type == DoomMenuTypes::Back)
		{
			if (Inputs.Enter)
			{
				if (stack.size() > 1)
				{
					highlight = 0;
					stack.pop();
					items = &stack.top();
					Inputs.Clear();
					return;
				}
			}
		}
		else if (item->type == DoomMenuTypes::Checkbox)
		{
			if (Inputs.Enter)
			{
				item->selection ^= 1;
				if (item->change != nullptr)
					item->change(item);
			}
		}
		else if (item->type == DoomMenuTypes::Options)
		{
			if (Inputs.Enter)
			{
				Inputs.Enter = false;
				Inputs.Right = true;
			}
			if (Inputs.Left)
			{
				Inputs.Clear();
				if (item->selection == 0) item->selection = (int)item->options.size();
				item->selection--;
				if (item->change != nullptr)
					item->change(item);
			}
			else if (Inputs.Right)
			{
				Inputs.Clear();
				item->selection++;
				if (item->selection == item->options.size()) item->selection = 0;
				if (item->change != nullptr)
					item->change(item);
			}
		}
		else if (item->type == DoomMenuTypes::Slider)
		{
			if (Inputs.Left)
			{
				Inputs.Clear();
				if (item->selection > item->minVal)
				{
					item->selection -= item->step;
					if (item->change != nullptr)
						item->change(item);
				}
			}
			else if (Inputs.Right)
			{
				Inputs.Clear();
				if (item->selection < item->maxVal)
				{
					item->selection += item->step;
					if (item->change != nullptr)
						item->change(item);
				}
			}
		}

		Inputs.Enter = false;
	}

	void Draw(double dt)
	{
		const int col = (int)(400 * scale);
		auto pos = glm::vec2((width / 2) - ((col * 3) / 2), 80);
		
		const auto black = glm::vec4(0, 0, 0, 0.5);

		const auto start = items->at(0)->type == DoomMenuTypes::Text ? 1 : 0;
		const auto shown = std::min(visible, (int)items->size() - scroll);

		for (int i = 0; i < shown; i++)
		{
			auto item = i == 0 ? items->at(0) : items->at(i + scroll);
			auto color = glm::vec4(1, 1, i + scroll == highlight ? 0.25 : 1, 1);
			auto offset = glm::vec2(item->type == DoomMenuTypes::Checkbox ? (40 * scale) : 0, 0);
			auto font = 1;
			auto size = 100 * scale;
			
			if (item->type == DoomMenuTypes::Text)
			{
				font = item->selection;
				size = item->maxVal * scale;
			}

			sprender->DrawText(font, item->caption, pos + offset + glm::vec2(2), black, size);
			sprender->DrawText(font, item->caption, pos + offset, color, size);

			if (item->type == DoomMenuTypes::Options)
			{
				sprender->DrawText(1, item->options[item->selection], pos + glm::vec2(col + 2, 2), black, size);
				sprender->DrawText(1, item->options[item->selection], pos + glm::vec2(col, 0), color, size);
			}
			else if (item->type == DoomMenuTypes::Checkbox)
			{
				auto checkColor = color * glm::vec4(1, 1, 1, 0.5);
				auto partSize = checkboxAtlas[0].w * 0.75f *  scale;
				sprender->DrawSprite(*checkbox, pos + glm::vec2(0, 4 * scale), glm::vec2(partSize), checkboxAtlas[0], 0, checkColor);
				if (item->selection)
					sprender->DrawSprite(*checkbox, pos + glm::vec2(0, 4 * scale), glm::vec2(partSize), checkboxAtlas[1], 0, color);
			}
			else if (item->type == DoomMenuTypes::Slider)
			{
				auto trackColor = color * glm::vec4(1, 1, 1, 0.5);
				auto barLength = col;
				auto partSize = scrollbarAtlas[0].w * 0.5f;
				sprender->DrawSprite(*scrollbar, pos + glm::vec2(col, 10), glm::vec2(partSize), scrollbarAtlas[0], 0, trackColor);
				sprender->DrawSprite(*scrollbar, pos + glm::vec2(col + barLength + (partSize * 1), 10), glm::vec2(partSize), scrollbarAtlas[0], 180, trackColor);
				sprender->DrawSprite(*scrollbar, pos + glm::vec2(col + partSize, 10), glm::vec2(barLength, partSize), scrollbarAtlas[1], 0, trackColor);

				//thanks GZDoom
				auto range = item->maxVal - item->minVal;
				auto ccur = clamp(item->selection, item->minVal, item->maxVal) - item->minVal;
				auto thumbPos = partSize + ((ccur * (barLength - (partSize * 2))) / range);

				auto thumb = glm::vec2(col + (int)thumbPos, 10);
				sprender->DrawSprite(*scrollbar, pos + thumb, glm::vec2(partSize), scrollbarAtlas[2], 0, color);
				sprender->DrawSprite(*scrollbar, pos + thumb + glm::vec2(partSize, 0), glm::vec2(partSize), scrollbarAtlas[2], 180, color);

				if (item->format != nullptr)
				{
					auto fmt = item->format(item);
					sprender->DrawText(1, fmt, pos + glm::vec2(col + barLength + 56, 12), black, size * 0.75f);
					sprender->DrawText(1, fmt, pos + glm::vec2(col + barLength + 54, 10), color, size * 0.75f);
				}
			}
			
			pos.y += (40*scale) + size - (100 * scale);
		}
	}
};

std::vector<Tickable*> tickables;

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	::width = (float)width;
	::height = (float)height;
	scale = ::height / SCR_HEIGHT;
	glViewport(0, 0, width, height);
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	Inputs.Process(key, action);

	//if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
	//	glfwSetWindowShouldClose(window, 1);
	if (key == GLFW_KEY_F1 && action == GLFW_PRESS)
	{
		wireframe = !wireframe;
		glPolygonMode(GL_FRONT_AND_BACK, wireframe ? GL_LINE : GL_FILL);
	}

	if (key == GLFW_KEY_W)
		camera.ProcessKeyboard(CameraMovement::Forward, 0.025f);
	else if (key == GLFW_KEY_A)
		camera.ProcessKeyboard(CameraMovement::Left, 0.025f);
	else if (key == GLFW_KEY_S)
		camera.ProcessKeyboard(CameraMovement::Backward, 0.025f);
	else if (key == GLFW_KEY_D)
		camera.ProcessKeyboard(CameraMovement::Right, 0.025f);
}

void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
	float xpos = static_cast<float>(xposIn);
	float ypos = static_cast<float>(yposIn);

	cursor->Move(xpos, ypos);

	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

	lastX = xpos;
	lastY = ypos;

	camera.ProcessMouseMovement(xoffset, yoffset);
}

int main()
{
	setlocale(LC_ALL, ".UTF8");

	InitVFS();

	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_SAMPLES, 2);

	if (WIN_WIDTH == SCR_WIDTH)
	{
		const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
		if (mode->width == SCR_WIDTH && mode->height == SCR_HEIGHT)
			glfwWindowHint(GLFW_DECORATED, 0);
		glfwWindowHint(GLFW_RESIZABLE, 0);
	}

	window = glfwCreateWindow(WIN_WIDTH, WIN_HEIGHT, "Project Special K maybe", NULL, NULL);
	if (window == NULL)
	{
		printf("Failed to create GLFW window\n");
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetKeyCallback(window, key_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
	
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		printf("Failed to initialize GLAD\n");
		return -1;
	}

	framebuffer_size_callback(window, WIN_WIDTH, WIN_HEIGHT);

	Shader ourShader("3.3.shader.vs", "3.3.shader.fs");
	spriteShader = new Shader("sprite.fs");
	whiteRect = new Texture("white.png", true, GL_CLAMP_TO_EDGE);

	float vertices[] = {
		// Back face
		-0.5f, -0.5f, -0.5f,  0.0f, 0.0f, // Bottom-left
		0.5f,  0.5f, -0.5f,  1.0f, 1.0f, // top-right
		0.5f, -0.5f, -0.5f,  1.0f, 0.0f, // bottom-right
		0.5f,  0.5f, -0.5f,  1.0f, 1.0f, // top-right
		-0.5f, -0.5f, -0.5f,  0.0f, 0.0f, // bottom-left
		-0.5f,  0.5f, -0.5f,  0.0f, 1.0f, // top-left
		// Front face
		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f, // bottom-left
		0.5f, -0.5f,  0.5f,  1.0f, 0.0f, // bottom-right
		0.5f,  0.5f,  0.5f,  1.0f, 1.0f, // top-right
		0.5f,  0.5f,  0.5f,  1.0f, 1.0f, // top-right
		-0.5f,  0.5f,  0.5f,  0.0f, 1.0f, // top-left
		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f, // bottom-left
		// Left face
		-0.5f,  0.5f,  0.5f,  1.0f, 0.0f, // top-right
		-0.5f,  0.5f, -0.5f,  1.0f, 1.0f, // top-left
		-0.5f, -0.5f, -0.5f,  0.0f, 1.0f, // bottom-left
		-0.5f, -0.5f, -0.5f,  0.0f, 1.0f, // bottom-left
		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f, // bottom-right
		-0.5f,  0.5f,  0.5f,  1.0f, 0.0f, // top-right
		// Right face
		0.5f,  0.5f,  0.5f,  1.0f, 0.0f, // top-left
		0.5f, -0.5f, -0.5f,  0.0f, 1.0f, // bottom-right
		0.5f,  0.5f, -0.5f,  1.0f, 1.0f, // top-right
		0.5f, -0.5f, -0.5f,  0.0f, 1.0f, // bottom-right
		0.5f,  0.5f,  0.5f,  1.0f, 0.0f, // top-left
		0.5f, -0.5f,  0.5f,  0.0f, 0.0f, // bottom-left
		// Bottom face
		-0.5f, -0.5f, -0.5f,  0.0f, 1.0f, // top-right
		0.5f, -0.5f, -0.5f,  1.0f, 1.0f, // top-left
		0.5f, -0.5f,  0.5f,  1.0f, 0.0f, // bottom-left
		0.5f, -0.5f,  0.5f,  1.0f, 0.0f, // bottom-left
		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f, // bottom-right
		-0.5f, -0.5f, -0.5f,  0.0f, 1.0f, // top-right
		// Top face
		-0.5f,  0.5f, -0.5f,  0.0f, 1.0f, // top-left
		0.5f,  0.5f,  0.5f,  1.0f, 0.0f, // bottom-right
		0.5f,  0.5f, -0.5f,  1.0f, 1.0f, // top-right
		0.5f,  0.5f,  0.5f,  1.0f, 0.0f, // bottom-right
		-0.5f,  0.5f, -0.5f,  0.0f, 1.0f, // top-left
		-0.5f,  0.5f,  0.5f,  0.0f, 0.0f  // bottom-left
	};

	unsigned int VBO, VAO, EBO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	//glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	//positions
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), 0);
	glEnableVertexAttribArray(0);
	//texture coords
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), 3 * sizeof(float));
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	//glFrontFace(GL_CW);

	//glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
	
	//Required for sprites
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	sprender = new SpriteRenderer();
	cursor = new Cursor();

	Texture texture("apple.png");
	Texture sprite("itemicons.png");

	camera.Position.z = 4;

	glm::vec3 cubePositions[] = {
		glm::vec3(0.0f,  0.0f,  0.0f),
		glm::vec3(2.0f,  5.0f, -15.0f),
		glm::vec3(-1.5f, -2.2f, -2.5f),
		glm::vec3(-3.8f, -2.0f, -12.3f),
		glm::vec3(2.4f, -0.4f, -3.5f),
		glm::vec3(-1.7f,  3.0f, -7.5f),
		glm::vec3(1.3f, -2.0f, -2.5f),
		glm::vec3(1.5f,  2.0f, -2.5f),
		glm::vec3(1.5f,  0.2f, -1.5f),
		glm::vec3(-1.3f,  1.0f, -1.5f)
	};

	tickables.push_back(new Background());
	dlgBox = new DialogueBox();
	tickables.push_back(dlgBox);
	tickables.push_back(new DoomMenu());

	int oldTime = 0;

	while (!glfwWindowShouldClose(window))
	{
		int newTime = std::clock();
		int deltaTime = newTime - oldTime;
		oldTime = newTime;
		double dt = deltaTime;
		
		auto time = (float)glfwGetTime();

		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//important: disable depth testing to allow multiple sprites to overlap.
		glDisable(GL_DEPTH_TEST);

		for (unsigned int i = (unsigned int)tickables.size(); i-- > 0; )
		{
			auto t = tickables[i];
			t->Tick(dt);
		}

		for (unsigned int i = 0; i < (unsigned int)tickables.size(); i++)
		{
			auto t = tickables[i];
			t->Draw(0.25);
		}
		cursor->Draw();
		sprender->Flush();

		//turn depth testing back on for 3D shit
		glEnable(GL_DEPTH_TEST);
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	while (!glfwWindowShouldClose(window))
	{
		auto time = (float)glfwGetTime();

		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
		tickables[0]->Draw(0.25);
		glClear(GL_DEPTH_BUFFER_BIT); //hack to quickly reset for 3D-on-2D

		ourShader.Use();

		glm::mat4 projection = glm::perspective(glm::radians(45.0f), width / height, 0.1f, 100.0f);

		//glm::mat4 view = glm::mat4(1.0f);
		// note that we're translating the scene in the reverse direction of where we want to move
		//view = glm::translate(view, glm::vec3(0.0f, 0.0f, -3.0f));
		auto view = camera.GetViewMatrix();

		//ourShader.setMat4("model", model);
		ourShader.SetMat4("view", view);
		ourShader.SetMat4("projection", projection);

		texture.Use();
		//glBindVertexArray(VAO);
		//glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		//glDrawArrays(GL_TRIANGLES, 0, 36);

		glBindVertexArray(VAO);
		for (unsigned int i = 0; i < 10; i++)
		{
			glm::mat4 model = glm::mat4(1.0f);
			model = glm::translate(model, cubePositions[i]);
			float angle = 20.0f * i;
			if (i % 3 == 0)
				angle = time * 25.0f;
			model = glm::rotate(model, glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));
			ourShader.SetMat4("model", model);

			glDrawArrays(GL_TRIANGLES, 0, 36);
		}
		glBindVertexArray(0);


		//SPRITES!

		//important: disable depth testing to allow multiple sprites to overlap.
		glDisable(GL_DEPTH_TEST);

		sprender->DrawSprite(sprite, glm::vec2(100, 100), glm::vec2(128), glm::vec4(256, 256, 128, 128), 0, glm::vec4(1), 0);
		//sprender.DrawSprite(sprite, glm::vec2(138, 10), glm::vec2(128), glm::vec4(256, 256, 128, 128), 0, glm::vec4(1), 1);
		//sprender.DrawSprite(sprite, glm::vec2(10, 138), glm::vec2(128), glm::vec4(256, 256, 128, 128), 0, glm::vec4(1), 2);
		//sprender.DrawSprite(sprite, glm::vec2(138,138), glm::vec2(128), glm::vec4(256, 256, 128, 128), 0, glm::vec4(1), 3);

		//sprender.DrawSprite(sprite, glm::vec2(320, 10), glm::vec2(256), glm::vec4(128, 256, 128, 128), sinf(time) * glm::radians(1000.0f), glm::vec4(abs(sinf(time)), abs(cosf(time)), abs(sinf(time + 0.5f)), 1));
		//sprender.DrawSprite(sprite, glm::vec2(10, 10), glm::vec2(256), glm::vec4(128, 0, 128, 128), 0, glm::vec4(1, 1, 1, abs(sinf(time))));

		//sprender.DrawSprite(sprite, glm::vec2(200), glm::vec2(300, 400), glm::vec4(0), 45, glm::vec4(0, 1, 0, 1));
		//sprender.DrawSprite(sprite, glm::vec2(1));

	
		sprender->DrawText(0, fmt::format("time: {}", time), glm::vec2(0));

		//turn depth testing back on for 3D shit
		glEnable(GL_DEPTH_TEST);
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	glDeleteBuffers(1, &EBO);

	glfwTerminate();
	return 0;
}
