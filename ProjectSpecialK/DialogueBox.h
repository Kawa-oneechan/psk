#pragma once

#include "engine/Tickable.h"
#include "engine/Texture.h"
#include "Villager.h"

class DialogueBox : public Tickable
{
private:
	Texture bubble[5] = {
		Texture("ui/dialogue/dialogue.png", GL_MIRRORED_REPEAT), //regular dialogue
		Texture("ui/dialogue/exclamation.png", GL_MIRRORED_REPEAT), //WOAG
		Texture("ui/dialogue/dream.png", GL_MIRRORED_REPEAT), //thoughts and such
		Texture("ui/dialogue/system.png", GL_MIRRORED_REPEAT), //single squircle, dark
	};
	Texture gradient[2] = {
		Texture("gradient_thin.png"),
		Texture("gradient_wide.png")
	};
	Texture nametag{ Texture("ui/dialogue/nametag.png") };
	std::string displayed;
	std::string toDisplay;
	size_t displayCursor{ 0 };
	float time{ 0 };
	glm::vec4 bubbleColor;
	glm::vec4 textColor;
	glm::vec4 nametagColor[2];
	float nametagWidth{ 0 };
	std::string name;
	VillagerP speaker{ nullptr };
	int font{ 0 };
	int bubbleNum{ 0 };
	float tween{ 0 };

	float delay{ 0 };

	enum class State
	{
		Opening, Writing, WaitingForKey, Closing, Done
	} state{ State::Done };

	typedef void(DialogueBox::*BJTSFunc)(BJTSParams);

	void bjtsDelay(BJTSParams);
	void bjtsEmote(BJTSParams);
	void bjtsBreak(BJTSParams);
	void bjtsClear(BJTSParams);
	void bjtsEnd(BJTSParams);
	void bjtsPass(BJTSParams);

	//BJTS functions that affect timing and such
	const std::map<std::string, BJTSFunc> bjtsPhase2 = {
		{ "delay", &DialogueBox::bjtsDelay },
		{ "emote", &DialogueBox::bjtsEmote },
		{ "break", &DialogueBox::bjtsBreak },
		{ "clr", &DialogueBox::bjtsClear },
		{ "end", &DialogueBox::bjtsEnd },

		//Passed over so DrawString can worry about it
		{ "color", &DialogueBox::bjtsPass },
		{ "/color", &DialogueBox::bjtsPass },
		{ "size", &DialogueBox::bjtsPass },
		{ "/size", &DialogueBox::bjtsPass },
		{ "font", &DialogueBox::bjtsPass },
		{ "/font", &DialogueBox::bjtsPass },
	};

	void Wrap();

public:
	enum class Sound
	{
		Silent, Animalese, Bebebese
	} Sound{ Sound::Bebebese };

	DialogueBox();
	void Text(const std::string& text);
	void Text(const std::string& text, int style, const std::string& speaker, const glm::vec4& tagBack, const glm::vec4& tagInk);
	void Text(const std::string& text, int style);
	void Text(const std::string& text, VillagerP speaker);
	void Style(int style);
	void Draw(float dt) override;
	bool Tick(float dt) override;

	VillagerP Speaker() { return speaker; }
};

extern std::shared_ptr<DialogueBox> dlgBox;
