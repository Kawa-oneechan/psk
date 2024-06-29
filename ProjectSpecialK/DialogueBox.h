#pragma once

#include "SpecialK.h"

enum DialogueBoxState
{
	Opening, Writing, WaitingForKey, Closing, Done
};
enum DialogueBoxSound
{
	Silent, Animalese, Bebebese
};

class DialogueBox : public Tickable
{
private:
	Texture bubble[4] = {
		Texture("ui/dialogue/dialogue.png", GL_MIRRORED_REPEAT),
		Texture("ui/dialogue/exclamation.png", GL_MIRRORED_REPEAT),
		Texture("ui/dialogue/dream.png", GL_MIRRORED_REPEAT),
		Texture("ui/dialogue/system.png", GL_MIRRORED_REPEAT),
	};
	Texture gradient[2] = {
		Texture("gradient_thin.png"),
		Texture("gradient_wide.png")
	};
	Texture nametag{ Texture("ui/dialogue/nametag.png") };
	Shader wobble{ Shader("shaders/wobble.fs") };
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

	Audio* bebebese;
	DialogueBoxState state;

	typedef void(DialogueBox::*MSBTFunc)(MSBTParams);

	void msbtStr(MSBTParams);
	void msbtEllipses(MSBTParams);

	//MSBT functions that actually change the string content.
	const std::map<std::string, MSBTFunc> msbtPhase1 = {
		{ "str", &DialogueBox::msbtStr },
		{ "...", &DialogueBox::msbtEllipses },
	};

	void msbtDelay(MSBTParams);
	void msbtEmote(MSBTParams);
	void msbtBreak(MSBTParams);
	void msbtClear(MSBTParams);
	void msbtEnd(MSBTParams);
	void msbtPass(MSBTParams);

	//MSBT functions that affect timing and such
	const std::map<std::string, MSBTFunc> msbtPhase2 = {
		{ "delay", &DialogueBox::msbtDelay },
		{ "emote", &DialogueBox::msbtEmote },
		{ "break", &DialogueBox::msbtBreak },
		{ "clr", &DialogueBox::msbtClear },
		{ "end", &DialogueBox::msbtEnd },

		//Passed over so DrawString can worry about it
		{ "color", &DialogueBox::msbtPass },
		{ "/color", &DialogueBox::msbtPass },
		{ "size", &DialogueBox::msbtPass },
		{ "/size", &DialogueBox::msbtPass },
		{ "font", &DialogueBox::msbtPass },
		{ "/font", &DialogueBox::msbtPass },
	};
	//TODO: look into using JSON and/or Lua to extend this.

	void Preprocess();
	void Wrap();

public:
	DialogueBoxSound Sound;

	DialogueBox();
	void Text(const std::string& text);
	void Text(const std::string& text, int style, const std::string& speaker, const glm::vec4& tagBack, const glm::vec4& tagInk);
	void Text(const std::string& text, int style);
	void Text(const std::string& text, std::shared_ptr<Villager> speaker);
	void Style(int style);
	void Draw(double dt);
	void Tick(double dt);
};

extern DialogueBox* dlgBox;
