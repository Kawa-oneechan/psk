#pragma once

#include "SpecialK.h"

#define GLM_ENABLE_EXPERIMENTAL
#include "support/glm/gtx/easing.hpp"

template<class T>
struct Tween
{
	T from, to;
	float speed;
	T* target;
	float progress;
	int type;

	Tween(T* tg, T f, T t, float s = 0.001f, int tp = 0) : target(tg), from(f), to(t), speed(s), type(tp),  progress(0) {}

	T value()
	{
		auto l = 0.0f;
		progress = clamp(progress, 0.0f, 1.0f);
		switch (type)
		{
		case 0: l = glm::linearInterpolation(progress); break;
		case 1: l = glm::quadraticEaseOut(progress); break;
		case 2: l = glm::elasticEaseOut(progress); break;
		}
		return glm::mix(from, to, l);
	}

	bool step()
	{
		if (progress < 1.0)
		{
			progress += speed;
			*target = value();
		}
		return (progress >= 1.0);
	}
};

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
		int Parent;
	};

private:
	std::vector<Panel*> panels;
	std::vector<Texture*> textures;
	std::vector<Shader*> shaders;
	std::vector < std::vector<glm::vec2>> polygons;

	std::vector<Tween<float>> tweens;
	Panel* highlighted = nullptr;

public:
	glm::vec2 Position;
	float Alpha;

	PanelLayout(JSONValue* source);
	void Tick(double dt);
	void Draw(double dt);
	Panel* GetPanel(const std::string& id);
	Tween<float>* Tween(float* target, float from, float to, float speed = 0.001f, int type = 0);
};
