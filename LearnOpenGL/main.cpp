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
#include "support/tweeny-3.2.0.h"

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

glm::vec4 GetJSONVec4(JSONValue* val)
{
	if (!val->IsArray())
		throw std::runtime_error("GetJSONVec4: given value is not an array.");
	auto arr = val->AsArray();
	if (arr.size() != 4)
		throw std::runtime_error(fmt::format("GetJSONVec4: given array has {} entries, not 4.", arr.size()));
	if (!arr[0]->IsNumber() || !arr[1]->IsNumber() || !arr[2]->IsNumber() || !arr[3]->IsNumber())
		throw std::runtime_error("GetJSONVec4: given array does not contain only numbers.");
	return glm::vec4(arr[0]->AsNumber(), arr[1]->AsNumber(), arr[2]->AsNumber(), arr[3]->AsNumber());
}

namespace UI
{
	glm::vec4 primaryColor;
	glm::vec4 secondaryColor;
	std::vector<glm::vec4> textColors;

	JSONObject& json = JSONObject();

	static void Load(const JSONValue* source)
	{
		json = source->AsObject();
		auto colors = json["colors"]->AsObject();
		primaryColor = GetJSONVec4(colors["primary"]);
		secondaryColor = GetJSONVec4(colors["secondary"]);
		for (auto& ink : colors["text"]->AsArray())
		{
			textColors.push_back(GetJSONVec4(ink));
		}
	}
};

class InputsMap
{
private:
	glm::vec2 lastMousePos;

public:
	bool Up, Down, Left, Right;
	bool Enter, Escape;

	bool MouseLeft, MouseRight, MouseMiddle;
	bool MouseHoldLeft;
	glm::vec2 MousePosition;

	InputsMap()
	{
		Up = Down = Left = Right = false;
		Enter = Escape = false;

		lastMousePos = MousePosition = glm::vec2(width, height) + 20.0f;
		MouseLeft = MouseRight = MouseMiddle = false;
		MouseHoldLeft = false;
	}

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
	
	void MouseMove(float x, float y)
	{
		lastMousePos = MousePosition;
		MousePosition.x = x;
		MousePosition.y = y;
	}

	bool MouseMoved()
	{
		auto ret = (lastMousePos != MousePosition);
		lastMousePos = MousePosition;
		return ret;
	}

	void Clear()
	{
		Up = Down = Left = Right = Enter = Escape = false;
		MouseLeft = MouseRight = MouseMiddle = false;
	}
};
static InputsMap& Inputs = InputsMap();

typedef std::vector<glm::vec4> TextureAtlas;

void GetAtlas(TextureAtlas &ret, const std::string& jsonFile)
{
	auto rjs = ReadJSON(jsonFile);
	if (rjs == nullptr)
		return;
	auto doc = rjs->AsObject();
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
	else if (doc["type"]->AsString() == "atlas")
	{
		auto rects = doc["rects"]->AsArray();
		for (const auto& rect : rects)
		{
			ret.push_back(GetJSONVec4(rect));
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
	virtual bool Character(unsigned int codepoint) { return false; }
};

class Cursor
{
private:
	Texture* hand;
	TextureAtlas atlas;
	std::vector<glm::vec2> hotspots;
	glm::vec2 hotspot;
	glm::vec4 frame;
	glm::vec2 size;
	float scale;

public:
	Cursor()
	{
		hand = new Texture("ui/cursors.png");
		GetAtlas(atlas, "ui/cursors.json");
		//atlas = new _TextureAtlas("ui/cursors.json");

		auto hsj = ReadJSON("ui/cursors.json")->AsObject();
		for (auto& hs : hsj["hotspots"]->AsArray())
			hotspots.push_back(GetJSONVec2(hs));

		SetScale(100);
		Select(0);
		size = glm::vec2(frame.w);
	}

	void Select(int style)
	{
		frame = atlas[style];
		hotspot = hotspots[style];
	}

	void SetScale(int newScale)
	{
		scale = newScale / 100.0f;
		size = glm::vec2(frame.w * scale);
	}

	void Draw()
	{
		sprender->DrawSprite(hand, Inputs.MousePosition - (hotspot * scale), size, frame);
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
		wallpaper = new Texture("discobg2.jpg");
		scroller = new Shader("shaders/scroller.fs");
		time = 0;
	}

	void Draw(double dt)
	{
		time += (float)dt * 0.005f;

		scroller->Use();
		scroller->SetFloat("time", time);
		sprender->DrawSprite(scroller, wallpaper, glm::vec2(0), glm::vec2(width, height), glm::vec4(0, 0, width, height));

		//auto lol = sprender->MeasureText(1, "Project Special K: UI test", 50);
		//sprender->DrawSprite(*whiteRect, glm::vec2(0), lol, glm::vec4(0), 0, glm::vec4(1, 1, 1, 0.5f));
		sprender->DrawText(1, "Project Special K: UI test", glm::vec2(0), glm::vec4(1, 1, 1, 0.75), 50);
		//sprender->DrawText(1, fmt::format("Project Special K: UI test (SCALE = {})", scale), glm::vec2(0), glm::vec4(1, 1, 1, 0.75), 50);
		//sprender->DrawText(1, u8"日", glm::vec2(0, 20), glm::vec4(1, 1, 1, 0.75), 50);
		//sprender->DrawText(1, u8"ā", glm::vec2(0), glm::vec4(1, 1, 1, 0.75), 200);
	}
};

class Panel
{
public:
	glm::vec2 Position;
	int Type;
	float Alpha;
	int Texture, Shader;
	int Frame;
	int Font;
	float Size;
	std::string Text;
	glm::vec4 Color;
};

class PanelLayout : public Tickable
{
private:
	std::vector<Panel*> panels;
	std::vector<Texture*> textures;
	std::vector<TextureAtlas> atlases;
	std::vector<Shader*> shaders;

	std::vector<tweeny::tween<float>> tweens;

public:
	glm::vec2 Position;
	float Alpha;

	PanelLayout(JSONValue* source)
	{
		auto src = source->AsObject();
		auto texturesSet = src["textures"]->AsArray();
		auto& panelsSet = src["panels"]->AsArray();

		Position = src["position"] != nullptr ? GetJSONVec2(src["position"]) : glm::vec2(0);
		Alpha = src["alpha"] != nullptr ? (float)src["alpha"]->AsNumber() : 1.0f;

		for (const auto& t : texturesSet)
		{
			auto tfn = t->AsString();
			auto tex = new Texture(tfn.c_str());
			textures.push_back(tex);

			tfn = tfn.replace(tfn.length() - 4, 4, ".json");
			TextureAtlas atl;
			atl.push_back(glm::vec4(0, 0, tex->width, tex->height));
			GetAtlas(atl, tfn);
			atlases.push_back(atl);
		}

		for (const auto& p : panelsSet)
		{
			auto pnl = p->AsObject();
			auto panel = new Panel();

			panel->Position = GetJSONVec2(pnl["position"]);
			auto& type = pnl["type"]->AsString();
			if (type == "image") panel->Type = 0;
			else if (type == "text") panel->Type = 1;
			if (panel->Type == 0)
			{
				panel->Texture = pnl["texture"] != nullptr ? (int)pnl["texture"]->AsNumber() : 0;
				panel->Frame = pnl["frame"] != nullptr ? (int)pnl["frame"]->AsNumber() : 0;
			}
			else if (panel->Type == 1)
			{
				panel->Text = pnl["text"] != nullptr ? pnl["text"]->AsString() : "???";
				panel->Font = pnl["font"] != nullptr ? (int)pnl["font"]->AsNumber() : 1;
				panel->Size = pnl["size"] != nullptr ? (float)pnl["size"]->AsNumber() : 100.0f;
			}
			panel->Color = glm::vec4(1, 1, 1, 1);
			if (pnl["color"] != nullptr)
			{
				if (pnl["color"]->IsString())
				{
					auto& clr = pnl["color"]->AsString();
					if (clr == "primary")
						panel->Color = UI::primaryColor;
					else if (clr == "secondary")
						panel->Color = UI::secondaryColor;
				}
				else if (pnl["color"]->IsArray())
					panel->Color = GetJSONVec4(pnl["color"]);
			}
			panel->Alpha = pnl["alpha"] != nullptr ? (float)pnl["alpha"]->AsNumber() : 1.0f;
			panels.push_back(panel);
		}
	}

	void Tick(double dt)
	{
		if (tweens.size() > 0)
		{
			for (auto i = 0; i < tweens.size(); i++)
			{
				auto& tween = tweens[i];
				if (tween.progress() < 1.0f)
					tween.step(1); //(int)(dt * 1) + 1);
				else
					tweens.erase(tweens.begin() + i);
			}
		}
	}

	void Tween(float* what, tweeny::tween<float> tween)
	{
		tween.onStep([what](float v) { *what = v; return false; });
		tweens.push_back(tween);
	}

	void Draw(double dt)
	{
		for (const auto& panel : panels)
		{
			auto color = panel->Color;
			color.a = clamp(Alpha * panel->Alpha, 0.0f, 1.0f);
			if (color.a == 0)
				continue;

			if (panel->Type == 0) //image
			{
				auto texture = textures[panel->Texture];
				auto frame = atlases[panel->Texture][panel->Frame];
				auto shader = spriteShader; //shaders[panel->Shader];

				sprender->DrawSprite(
					shader, texture,
					Position + panel->Position,
					glm::vec2(frame.z, frame.w),
					frame,
					0.0f,
					color,
					0
				);
			}
			else if (panel->Type == 1) //text
			{
				sprender->DrawText(
					panel->Font,
					panel->Text,
					Position + panel->Position,
					color,
					100.0f
				);
			}
		}
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
	TextureAtlas nametagAtlas;
	Shader* wobble;
	std::string displayed;
	std::string toDisplay;
	size_t displayCursor;
	float time;
	glm::vec4 bubbleColor;
	glm::vec4 textColor;
	glm::vec4 nametagColor[2];
	float nametagWidth;
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
		bubble[0] = new Texture("ui/dialogue/dialogue.png");
		bubble[1] = new Texture("ui/dialogue/exclamation.png");
		bubble[2] = new Texture("ui/dialogue/dream.png");
		bubble[3] = new Texture("ui/dialogue/system.png");
		//bubble[4] = new Texture("ui/dialogue/wildworld.png", true, GL_REPEAT, GL_NEAREST);
		gradient[0] = new Texture("gradient_thin.png");
		gradient[1] = new Texture("gradient_wide.png");
		nametag = new Texture("ui/dialogue/nametag.png");
		GetAtlas(nametagAtlas, "ui/dialogue/nametag.json");
		nametagWidth = 0;
		wobble = new Shader("shaders/wobble.fs");

		displayCursor = 0;
		time = 0;
		delay = 0;

		Text(u8"Truth is... <color:1>the game</color> was rigged\nfrom the start.", 0, "Isabelle", glm::vec4(1, 0.98f, 0.56f, 1), glm::vec4(0.96f, 0.67f, 0.05f, 1));
	}

	void Text(const std::string& text)
	{
		displayed.clear();
		toDisplay = text;
		displayCursor = 0;
		delay = 50;
	}

	void Text(const std::string& text, int style, const std::string& speaker, const glm::vec4& tagBack, const glm::vec4& tagInk)
	{
		Style(style);
		if (style != 3)
		{
			name = speaker;
			nametagWidth = sprender->MeasureText(1, name, 120).x - 32;
			nametagColor[0] = tagBack;
			nametagColor[1] = tagInk;
		}
		else
		{
			name.clear();
		}
		Text(text);
	}

	void Text(const std::string& text, int style)
	{
		Style(style);
		Text(text);
	}

	//void Text(const std::string& text, Villager* speaker)

	void Style(int style)
	{
		style = clamp(style, 0, 3);

		if (style == 3) //system
		{
			bubbleColor = UI::primaryColor;
			textColor = UI::textColors[8];
			font = 1;
			bubbleNum = 3;
		}
		else
		{
			bubbleColor = UI::secondaryColor;
			textColor = UI::textColors[0];
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
			sprender->DrawSprite(wobble, bubble[bubbleNum], glm::vec2(dlgLeft, dlgTop), glm::vec2(dlgWidth, dlgHeight), glm::vec4(0), 0, bubbleColor, 0);
			sprender->DrawSprite(wobble, bubble[bubbleNum], glm::vec2(dlgLeft + dlgWidth, dlgTop), glm::vec2(dlgWidth, dlgHeight), glm::vec4(0, 0, bubble[0]->width, bubble[0]->height), 0, bubbleColor, 1);
		}

		sprender->DrawText(font, displayed, glm::vec2(dlgLeft + (200 * scale), dlgTop + (100 * scale)), textColor, 150 * scale);

		if (!name.empty())
		{
			const auto tagAngle = -2.0f;
			const auto tagPos = glm::vec2(dlgLeft + (150 * scale), dlgTop + (sinf(time * 2) * 10) * scale);
			const auto tagSize = glm::vec2(nametagAtlas[0].z, nametagAtlas[0].w) * scale;
			//const auto tagMidWidth = 128; //pre-measure this to fit in the Text() calls.

			const auto tagPosL = tagPos;
			const auto tagPosM = tagPosL + glm::vec2(cosf(glm::radians(tagAngle)) * tagSize.x, sinf(glm::radians(tagAngle)) * tagSize.x);
			const auto tagPosR = tagPosM + glm::vec2(cosf(glm::radians(tagAngle)) * nametagWidth, sinf(glm::radians(tagAngle)) * nametagWidth);
			//TODO: figure this one out properly
			const auto tagPosT = tagPosL + glm::vec2(cosf(glm::radians(tagAngle)) * (tagSize.x - 16), sinf(glm::radians(tagAngle)) * (tagSize.x - 512));
			sprender->DrawSprite(nametag, tagPosL, tagSize, nametagAtlas[0], tagAngle, nametagColor[0], SPR_TOPLEFT);
			sprender->DrawSprite(nametag, tagPosM, glm::vec2(nametagWidth, tagSize.y), nametagAtlas[2], tagAngle, nametagColor[0], SPR_TOPLEFT);
			sprender->DrawSprite(nametag, tagPosR, tagSize, nametagAtlas[1], tagAngle, nametagColor[0], SPR_TOPLEFT);
			sprender->DrawText(1, name, tagPosT, nametagColor[1], 120 * scale, tagAngle);
		}

		//maybe afterwards port this to the UI Panel system?

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
		options.push_back(new DoomMenuItem("Cursor scale", 50, 150, 100, 10, percent, [&](DoomMenuItem*i) { cursor->SetScale(i->selection); }));
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
		controls = new Texture("ui/controls.png");
		GetAtlas(controlsAtlas, "ui/controls.json");

		rebuild();

		highlight = 0;
		mouseHighlight = 0;
		scroll = 0;
		
		//to be filled in at first draw
		sliderStart = 0;
		sliderEnd = 1;
		sliderHolding = -1;
		itemX = 0;

		stack.push(options);
		items = &stack.top();
	}

	void Tick(double dt)
	{
		//visible = (int)(12.0f * scale);

		if (Inputs.MouseMoved())
		{
			const int col = (int)(400 * scale);
			mouseHighlight = -1;
			if (Inputs.MousePosition.x >= itemX && Inputs.MousePosition.x <= itemX + (col * 2.5f))
			{
				for (int i = 0; i < itemY.size() - 1; i++)
				{
					if (Inputs.MousePosition.y >= itemY[i] && Inputs.MousePosition.y < itemY[i + 1])
					{
						mouseHighlight = highlight = i;
						break;
					}
				}
			}
		}
		cursor->Select(0);
		if (mouseHighlight != -1 && items->at(highlight)->type == DoomMenuTypes::Slider)
		{
			if (Inputs.MousePosition.x >= sliderStart && Inputs.MousePosition.x <= sliderEnd)
			{
				if (sliderHolding == -1)
					sliderHolding = highlight;
				if (highlight == sliderHolding)
				{
					cursor->Select(5);
					if (Inputs.MouseHoldLeft)
					{
						cursor->Select(6);
						auto item = items->at(highlight);

						//thanks GZDoom
						auto x = clamp(Inputs.MousePosition.x, sliderStart, sliderEnd);
						auto  v = item->minVal + ((x - sliderStart) * (item->maxVal - item->minVal)) / (sliderEnd - sliderStart);
						item->selection = (int)(round(v / item->step) * item->step);
						if (item->change != nullptr)
							item->change(item);
						/*
						auto ccur = clamp(item->selection, item->minVal, item->maxVal) - item->minVal;
						auto thumbPos = partSize + ((ccur * (barLength - (partSize * 2))) / range);
						*/
						//return;
					}
					else
						sliderHolding = -1;
				}
			}
		}
		if (mouseHighlight != highlight && Inputs.MouseLeft)
			Inputs.MouseLeft = false;

		while (items->at(highlight)->type == DoomMenuTypes::Text)
			highlight++;

		if (Inputs.Escape)
		{
			Inputs.Escape = false;
			if (stack.size() > 1)
			{
				highlight = 0;
				mouseHighlight = -1;
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
			if (Inputs.Enter || Inputs.MouseLeft)
			{
				stack.push(*item->page);
				items = item->page;
				highlight = 0;
				mouseHighlight = -1;
			}
		}
		else if (item->type == DoomMenuTypes::Back)
		{
			if (Inputs.Enter || Inputs.MouseLeft)
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
			if (Inputs.Enter || Inputs.MouseLeft)
			{
				item->selection ^= 1;
				if (item->change != nullptr)
					item->change(item);
			}
		}
		else if (item->type == DoomMenuTypes::Options)
		{
			if (Inputs.Enter || Inputs.MouseLeft)
			{
				Inputs.Enter = false;
				Inputs.Right = true;
				Inputs.MouseLeft = false;
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
		Inputs.MouseLeft = false;
	}

	void Draw(double dt)
	{
		const int col = (int)(400 * scale);
		auto pos = glm::vec2((width / 2) - ((col * 3) / 2), 80);

		itemX = pos.x;
		
		const auto black = glm::vec4(0, 0, 0, 0.5);

		const auto start = items->at(0)->type == DoomMenuTypes::Text ? 1 : 0;
		const auto shown = std::min(visible, (int)items->size() - scroll);

		const auto partSize = controlsAtlas[4].w * 0.75f *  scale;
		const auto thumbSize = glm::vec2(controlsAtlas[3].z, controlsAtlas[3].w) * 0.50f * scale;

		sprender->DrawText(0, fmt::format("DoomMenu: {}/{} {} {},{} - {},{}", highlight, mouseHighlight, Inputs.MouseHoldLeft, Inputs.MousePosition.x, Inputs.MousePosition.y, sliderStart, sliderEnd), glm::vec2(0, 16));

		itemY.clear();

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
			else if (item->type == DoomMenuTypes::Slider)
			{
				if (item->format != nullptr)
				{
					auto fmt = item->format(item);
					sprender->DrawText(1, fmt, pos + glm::vec2(col + col+ 56, 12), black, size * 0.75f);
					sprender->DrawText(1, fmt, pos + glm::vec2(col + col + 54, 10), color, size * 0.75f);
				}
			}

			itemY.push_back(pos.y);
			pos.y += (40 * scale) + size - (100 * scale);
		}

		//terminator
		itemY.push_back(pos.y);

		for (int i = 0; i < shown; i++)
		{
			auto item = i == 0 ? items->at(0) : items->at(i + scroll);
			auto color = glm::vec4(1, 1, i + scroll == highlight ? 0.25 : 1, 1);
			auto offset = glm::vec2(item->type == DoomMenuTypes::Checkbox ? (40 * scale) : 0, 0);
			auto font = 1;
			auto size = 100 * scale;

			pos.y = itemY[i];

			if (item->type == DoomMenuTypes::Checkbox)
			{
				auto checkColor = color * glm::vec4(1, 1, 1, 0.5);
				sprender->DrawSprite(controls, pos + glm::vec2(0, 4 * scale), glm::vec2(partSize), controlsAtlas[4], 0, checkColor);
				if (item->selection)
					sprender->DrawSprite(controls, pos + glm::vec2(0, 4 * scale), glm::vec2(partSize), controlsAtlas[5], 0, color);
			}
			else if (item->type == DoomMenuTypes::Slider)
			{
				auto trackColor = color * glm::vec4(1, 1, 1, 0.5);
				auto barLength = col;
				auto partSize = controlsAtlas[0].w * 0.5f;
				sprender->DrawSprite(controls, pos + glm::vec2(col, 10), glm::vec2(partSize), controlsAtlas[0], 0, trackColor);
				sprender->DrawSprite(controls, pos + glm::vec2(col + barLength + (partSize * 1), 10), glm::vec2(partSize), controlsAtlas[1], 0, trackColor);
				sprender->DrawSprite(controls, pos + glm::vec2(col + partSize, 10), glm::vec2(barLength, partSize), controlsAtlas[2], 0, trackColor);

				sliderStart = pos.x + col + partSize;
				sliderEnd = sliderStart + barLength;

				//thanks GZDoom
				auto range = item->maxVal - item->minVal;
				auto ccur = clamp(item->selection, item->minVal, item->maxVal) - item->minVal;
				auto thumbPos = partSize + ((ccur * (barLength - (partSize * 2))) / range);

				auto thumb = glm::vec2(col + (int)thumbPos, 10);
				sprender->DrawSprite(controls, pos + thumb, thumbSize, controlsAtlas[3], 0, color);
			}
		}
	}
};

class TextField : public Tickable
{
	//TODO: since we have a scissor test, why not have scrolling?
	//As in, [lo, world!_ ] with the "hel" off-screen, via an offset?

public:
	glm::vec4 rect;
	glm::vec4 color;
	int font;
	float size;
	std::string value;
	size_t caret;

	TextField()
	{
		value = "test";
		caret = value.length();

		rect = glm::vec4(8, 32, 320, 80);
		color = glm::vec4(1, 1, 0, 1);
		font = 1;
		size = 100.0f;
	}

	void Draw(double dt)
	{
		auto pos = glm::vec2(rect.x, rect.y);

		sprender->Flush();
		const auto h = (int)(rect.w - rect.y);
		glScissor((int)rect.x, (int)(height - rect.y) - h, (int)(rect.z - rect.x), h);
		glEnable(GL_SCISSOR_TEST);

		sprender->DrawText(font, value, pos, color, size);

		auto ms = sprender->MeasureText(font, value.substr(0, caret), size);
		sprender->DrawText(font, "_", pos + glm::vec2(ms.x, 0), glm::vec4(1, 1, 0, 1), size);

		sprender->Flush();
		glDisable(GL_SCISSOR_TEST);
	}

	bool Character(unsigned int codepoint)
	{
		if (codepoint == '\b')
		{
			if (caret > 0)
			{
				int toDelete = 1;
				caret--;
				if ((value[caret] & 0x80) == 0)
				{
				}
				else
				{
					while (value[caret] & 0x80)
					{
						caret--;
						toDelete++;
						if ((value[caret + 1] & 0xC0) == 0xC0)
							break;
					}
					caret++;
					toDelete--;
				}
				value.erase(caret, toDelete);
			}
			return true;
		}
		else if (codepoint == 0xFFF0) //left
		{
			if (caret > 0)
			{
				caret--;
				if ((value[caret] & 0x80) == 0)
				{
				}
				else
				{
					while (value[caret] & 0x80)
					{
						caret--;
						if ((value[caret + 1] & 0xC0) == 0xC0)
							break;
					}
					caret++;
				}
			}
			return true;
		}
		else if (codepoint == 0xFFF1) //right
		{
			if (caret < value.length())
			{
				if ((value[caret] & 0xE0) == 0xE0)
					caret += 3;
				else if ((value[caret] & 0xE0) == 0xC0)
					caret += 2;
				else
					caret++;
			}
			return true;
		}

		std::string inserted;
		if (codepoint < 0x80)
			inserted += codepoint;
		else if (codepoint < 0x0800)
		{
			inserted += (char)(((codepoint >> 6) & 0x1F) | 0xC0);
			inserted += (char)(((codepoint >> 0) & 0x3F) | 0x80);
		}
		else if (codepoint < 0x10000)
		{
			inserted += (char)(((codepoint >> 12) & 0x0F) | 0xE0);
			inserted += (char)(((codepoint >> 6) & 0x3F) | 0x80);
			inserted += (char)(((codepoint >> 0) & 0x3F) | 0x80);
		}
		
		value.insert(caret, inserted);
		caret += inserted.length();
		return true;
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

void char_callback(GLFWwindow* window, unsigned int codepoint)
{
	for (unsigned int i = (unsigned int)tickables.size(); i-- > 0; )
	{
		auto t = tickables[i];
		if (t->Character(codepoint))
			break;
	}
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	Inputs.Process(key, action);

	//Passthroughs
	if (key == GLFW_KEY_BACKSPACE && action == GLFW_PRESS)
		char_callback(window, '\b');
	else if (key == GLFW_KEY_LEFT && action == GLFW_PRESS)
		char_callback(window, 0xFFF0);
	else if (key == GLFW_KEY_RIGHT && action == GLFW_PRESS)
		char_callback(window, 0xFFF1);

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

	Inputs.MouseMove(xpos, ypos);

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

void mousebutton_callback(GLFWwindow* window, int button, int action, int mods)
{
	if (button == GLFW_MOUSE_BUTTON_LEFT)
	{
		Inputs.MouseHoldLeft = action == GLFW_PRESS;
		if (!Inputs.MouseLeft && action == GLFW_RELEASE) //-V1051 no I'm pretty sure I meant this
			Inputs.MouseLeft = true;
	}
	else if (button == GLFW_MOUSE_BUTTON_MIDDLE)
	{
		if (!Inputs.MouseMiddle && action == GLFW_RELEASE)
			Inputs.MouseMiddle = true;
	}
	else if (button == GLFW_MOUSE_BUTTON_RIGHT)
	{
		if (!Inputs.MouseRight && action == GLFW_RELEASE)
			Inputs.MouseRight = true;
	}
}

int main()
{
	setlocale(LC_ALL, ".UTF8");

	InitVFS();
	UI::Load(ReadJSON("ui/ui.json"));

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
	glfwSetCharCallback(window, char_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetMouseButtonCallback(window, mousebutton_callback);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
	
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		printf("Failed to initialize GLAD\n");
		return -1;
	}

	framebuffer_size_callback(window, WIN_WIDTH, WIN_HEIGHT);

	Shader ourShader("shaders/model.vs", "shaders/model.fs");
	spriteShader = new Shader("shaders/sprite.fs");
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

	//Texture texture("apple.png");
	//Texture sprite("itemicons.png");

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
	auto hotbar = new PanelLayout(UI::json["hotbar"]);
	tickables.push_back(hotbar);
	hotbar->Tween(&hotbar->Position.y, tweeny::from(-100.0f).to(0).during(100));
	hotbar->Tween(&hotbar->Alpha, tweeny::from(0.0f).to(0.75f).during(200));
	//tickables.push_back(new TextField());

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

		//texture.Use();
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

		//sprender->DrawSprite(sprite, glm::vec2(100, 100), glm::vec2(128), glm::vec4(256, 256, 128, 128), 0, glm::vec4(1), 0);
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
