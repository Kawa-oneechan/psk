#pragma once

#include <vector>
#include <string>
#include <functional>
#include <stack>
#include <algorithm>

#include "Shader.h"
#include "Texture.h"
#include "SpriteRenderer.h"
#include "Camera.h"
#include "VFS.h"

extern float width, height;
extern float scale;

template<typename T> static T clamp(T val, T minval, T maxval) { return std::max<T>(std::min<T>(val, maxval), minval); }

typedef std::vector<glm::vec4> TextureAtlas;

extern void GetAtlas(TextureAtlas &ret, const std::string& jsonFile);

extern SpriteRenderer* sprender;

extern glm::vec2 GetJSONVec2(JSONValue* val);
extern glm::vec4 GetJSONVec4(JSONValue* val);

//TEMPORARY -- is in Text.cpp in real PSK
extern std::vector<std::string> Split(std::string& data, char delimiter);

namespace UI
{
	extern glm::vec4 primaryColor;
	extern glm::vec4 secondaryColor;
	extern std::vector<glm::vec4> textColors;

	extern JSONObject& json;
};

class Tickable
{
public:
	virtual void Tick(double dt) {};
	virtual void Draw(double dt) {};
	virtual bool Character(unsigned int codepoint) { return false; }
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

	InputsMap();
	void Process(int key, int action);
	void MouseMove(float x, float y);
	bool MouseMoved();
	void Clear();
};
extern InputsMap& Inputs;

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
	Cursor();
	void Select(int style);
	void SetScale(int newScale);
	void Draw();
};
extern Cursor* cursor;
