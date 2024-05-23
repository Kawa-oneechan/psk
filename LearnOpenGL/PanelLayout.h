#pragma once

#include "SpecialK.h"

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

	PanelLayout(JSONValue* source);
	void Tick(double dt);
	void Tween(float* what, tweeny::tween<float> tween);
	void Draw(double dt);
};
