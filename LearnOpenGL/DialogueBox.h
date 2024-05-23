#pragma once

#include "SpecialK.h"

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

	void msbtPass(MSBTParams);
	typedef void(DialogueBox::*MSBTFunc)(MSBTParams);
	const std::map<std::string, MSBTFunc> msbtPhase3 = {
		{ "color", &DialogueBox::msbtPass },
		{ "/color", &DialogueBox::msbtPass },
		{ "size", &DialogueBox::msbtPass },
		{ "/size", &DialogueBox::msbtPass },
		{ "font", &DialogueBox::msbtPass },
		{ "/font", &DialogueBox::msbtPass },
	};

public:

	DialogueBox();
	void Text(const std::string& text);
	void Text(const std::string& text, int style, const std::string& speaker, const glm::vec4& tagBack, const glm::vec4& tagInk);
	void Text(const std::string& text, int style);
	void Style(int style);
	void Draw(double dt);
	void Tick(double dt);
};

extern DialogueBox* dlgBox;
