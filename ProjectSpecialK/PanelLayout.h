#pragma once

#include <functional>
#include <vector>
#include <glm/glm.hpp>
#include "engine/Tickable.h"
#include "engine/Shader.h"
#include "engine/Texture.h"
#include "engine/JsonUtils.h"
#include "engine/Types.h"
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/easing.hpp>

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
	std::vector<polygon> polygons;

	bool hasAnimations{ false };
	float animationTime{ 0.0f };
	std::string currentAnimation;
	std::map<std::string, Animation> animations;

	Panel* highlighted = nullptr;

public:
	glm::vec2 Position;
	enum class CornerOrigin
	{
		TopLeft, TopRight, BottomLeft, BottomRight
	} Origin = CornerOrigin::TopLeft;
	float Alpha{ 1.0f };

	std::function<void(const std::string&)> onClick{ nullptr };

	PanelLayout() = default;
	explicit PanelLayout(jsonValue& source);
	bool Tick(float dt) override;
	void Draw(float dt) override;
	Panel* GetPanel(const std::string& id);
	void Play(const std::string& anim);
	const bool Playing() { return !currentAnimation.empty(); };
};
