#pragma once

#include "SpecialK.h"

#define GLM_ENABLE_EXPERIMENTAL
#include "support/glm/gtx/easing.hpp"

typedef enum
{
	Linear,
	QuadIn,
	QuadOut,
	QuadInOut,
	CubeIn,
	CubeOut,
	CubeInOut,
	SineIn,
	SineOut,
	SineInOut,
	CircleIn,
	CircleOut,
	CircleInOut,
	ElastIn,
	ElastOut,
	ElastInOut,
	BounceIn,
	BounceOut,
	BounceInOut,
} Interpolation;

template<class T>
struct Tween
{
	T from, to;
	float speed;
	T* target;
	float progress;
	Interpolation type;

	Tween(T* tg, T f, T t, float s = 0.001f, Interpolation tp = Linear) : target(tg), from(f), to(t), speed(s), type(tp),  progress(0) {}

	T value()
	{
		progress = clamp(progress, 0.0f, 1.0f);
#define X(E, M) case E: return glm::mix(from, to, M(progress))
		switch (type)
		{
			default: [[__fallthrough]]
			X(Linear, glm::linearInterpolation);
			X(QuadIn, glm::quadraticEaseIn);
			X(QuadOut, glm::quadraticEaseOut);
			X(QuadInOut, glm::quadraticEaseInOut);
			X(CubeIn, glm::cubicEaseIn);
			X(CubeOut, glm::cubicEaseOut);
			X(CubeInOut, glm::cubicEaseInOut);
			X(SineIn, glm::sineEaseIn);
			X(SineOut, glm::sineEaseOut);
			X(SineInOut, glm::sineEaseInOut);
			X(CircleIn, glm::circularEaseIn);
			X(CircleOut, glm::circularEaseOut);
			X(CircleInOut, glm::circularEaseInOut);
			X(ElastIn, glm::elasticEaseIn);
			X(ElastOut, glm::elasticEaseOut);
			X(ElastInOut, glm::elasticEaseInOut);
			X(BounceIn, glm::bounceEaseIn);
			X(BounceOut, glm::bounceEaseOut);
			X(BounceInOut, glm::bounceEaseInOut);
		}
#undef X
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
	Tween<float>* Tween(float* target, float from, float to, float speed = 0.001f, Interpolation type = Linear);
};
