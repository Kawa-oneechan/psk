#pragma once

#include <cstdio>
#include <ctime>
#include <cstdarg>
#include <vector>
#include <string>
#include <functional>
#include <stack>
#include <algorithm>

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

extern float width, height;
extern float scale;

template<typename T> static T clamp(T val, T minval, T maxval) { return std::max<T>(std::min<T>(val, maxval), minval); }

typedef std::vector<glm::vec4> TextureAtlas;

extern void GetAtlas(TextureAtlas &ret, const std::string& jsonFile);

extern SpriteRenderer* sprender;
extern Shader* spriteShader;
extern Texture* whiteRect;

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
