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
	std::function<T(float)> interpolator;

	Tween(T* tg, T f, T t, float s = 0.001f, std::function<T(float)> i = glm::linearInterpolation) : target(tg), from(f), to(t), speed(s), interpolator(i),  progress(0) {}

	T value()
	{
		progress = clamp(progress, 0.0f, 1.0f);
		return glm::mix(from, to, interpolator(progress));
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

	struct Panel
	{
		std::string ID;
		glm::vec2 Position;
		enum class Type
		{
			Image, Text, ItemIcon,
		} Type;
		float Alpha;
		int Texture, Shader;
		int Frame;
		int Font;
		float Size;
		int Alignment;
		float Angle;
		std::string Text;
		glm::vec4 Color;
		int Polygon;
		int Parent;
		bool Enabled;
	};

	struct AnimationBit
	{
		std::string ID;
		std::string Property;
		float FromTime, ToTime;
		float FromVal, ToVal;
		std::function<float(float)> Function;
	};

	struct Animation
	{
		std::vector<AnimationBit> Bits;
		std::string Next;
	};

private:
	std::vector<Panel*> panels;
	std::vector<Texture*> textures;
	std::vector<Shader*> shaders;
	std::vector<std::vector<glm::vec2>> polygons;

	bool hasAnimations;
	float animationTime;
	std::string currentAnimation;
	std::map<std::string, Animation> animations;

	std::vector<Tween<float>> tweens;
	Panel* highlighted = nullptr;

public:
	glm::vec2 Position;
	enum class CornerOrigin
	{
		TopLeft, TopRight, BottomLeft, BottomRight
	} Origin;
	float Alpha;

	PanelLayout() = default;
	PanelLayout(JSONValue* source);
	void Tick(float dt);
	void Draw(float dt);
	Panel* GetPanel(const std::string& id);
	Tween<float>* Tween(float* target, float from, float to, float speed = 0.001f, std::function<float(float)> interpolator = glm::linearInterpolation<float>);
	void Play(const std::string& anim);
	const bool Playing() { return !currentAnimation.empty(); };
};
