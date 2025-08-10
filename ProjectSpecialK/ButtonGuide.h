#pragma once

#include <string>

class ButtonGuide
{
public:
	//Replaces the buttons for this Guide entirely. Precede an entry with '!' to highlight it.
	void SetButtons(std::initializer_list<std::string>);
	void Draw();
};

//Not a Tickable because the ButtonGuide does not animate or anything. Let other things call Draw on it.
