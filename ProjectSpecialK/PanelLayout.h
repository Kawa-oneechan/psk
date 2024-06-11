#pragma once

#include "SpecialK.h"

class PanelLayout : public Tickable
{
	enum PanelType
	{
		Image, Text, ItemIcon,
	};

	class Panel
	{
	public:
		std::string ID;
		glm::vec2 Position;
		PanelType Type;
		float Alpha;
		int Texture, Shader;
		int Frame;
		int Font;
		float Size;
		int Alignment;
		std::string Text;
		glm::vec4 Color;
		int Polygon;
	};

private:
	std::vector<Panel*> panels;
	std::vector<Texture*> textures;
	std::vector<TextureAtlas> atlases;
	std::vector<Shader*> shaders;
	std::vector < std::vector<glm::vec2>> polygons;

	std::vector<tweeny::tween<float>> tweens;
	Panel* highlighted = nullptr;

public:
	glm::vec2 Position;
	float Alpha;

	PanelLayout(JSONValue* source);
	void Tick(double dt);
	void Tween(float* what, tweeny::tween<float> tween);
	void Draw(double dt);
	Panel* GetPanel(const std::string& id);
};
