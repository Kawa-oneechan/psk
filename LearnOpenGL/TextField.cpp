#include "TextField.h"

TextField::TextField()
{
	value = "test";
	caret = value.length();

	rect = glm::vec4(8, 32, 320, 80);
	color = glm::vec4(1, 1, 0, 1);
	font = 1;
	size = 100.0f;
}

void TextField::Draw(double dt)
{
	auto pos = glm::vec2(rect.x, rect.y);

	sprender->Flush();
	const auto h = (int)(rect.w - rect.y);
	glScissor((int)rect.x, (int)(height - rect.y) - h, (int)(rect.z - rect.x), h);
	glEnable(GL_SCISSOR_TEST);

	sprender->DrawText(font, value, pos, color, size);

	auto ms = sprender->MeasureText(font, value.substr(0, caret), size);
	sprender->DrawText(font, "_", pos + glm::vec2(ms.x, 0), glm::vec4(1, 1, 0, 1), size);

	sprender->Flush();
	glDisable(GL_SCISSOR_TEST);
}

bool TextField::Character(unsigned int codepoint)
{
	if (codepoint == '\b')
	{
		if (caret > 0)
		{
			int toDelete = 1;
			caret--;
			if ((value[caret] & 0x80) == 0)
			{
			}
			else
			{
				while (value[caret] & 0x80)
				{
					caret--;
					toDelete++;
					if ((value[caret + 1] & 0xC0) == 0xC0)
						break;
				}
				caret++;
				toDelete--;
			}
			value.erase(caret, toDelete);
		}
		return true;
	}
	else if (codepoint == 0xFFF0) //left
	{
		if (caret > 0)
		{
			caret--;
			if ((value[caret] & 0x80) == 0)
			{
			}
			else
			{
				while (value[caret] & 0x80)
				{
					caret--;
					if ((value[caret + 1] & 0xC0) == 0xC0)
						break;
				}
				caret++;
			}
		}
		return true;
	}
	else if (codepoint == 0xFFF1) //right
	{
		if (caret < value.length())
		{
			if ((value[caret] & 0xE0) == 0xE0)
				caret += 3;
			else if ((value[caret] & 0xE0) == 0xC0)
				caret += 2;
			else
				caret++;
		}
		return true;
	}

	std::string inserted;
	if (codepoint < 0x80)
		inserted += codepoint;
	else if (codepoint < 0x0800)
	{
		inserted += (char)(((codepoint >> 6) & 0x1F) | 0xC0);
		inserted += (char)(((codepoint >> 0) & 0x3F) | 0x80);
	}
	else if (codepoint < 0x10000)
	{
		inserted += (char)(((codepoint >> 12) & 0x0F) | 0xE0);
		inserted += (char)(((codepoint >> 6) & 0x3F) | 0x80);
		inserted += (char)(((codepoint >> 0) & 0x3F) | 0x80);
	}

	value.insert(caret, inserted);
	caret += inserted.length();
	return true;
}
