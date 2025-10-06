#include <glad/glad.h>
#include "Texture.h"
#include "DropLabel.h"
#include "SpriteRenderer.h"

extern int width, height;

static void createAndBindBuffer(unsigned int* tid, unsigned int* fbo, int width, int height)
{
	glGenTextures(1, tid);

	glGenFramebuffers(1, fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, *fbo);

	glBindTexture(GL_TEXTURE_2D, *tid);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, *tid, 0);

	glViewport(0, 0, width, height);

	glColorMask(true, true, true, true);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT);
}

void DropLabel::update()
{
	if (canvas.ID != 0)
		glDeleteTextures(1, &canvas.ID);

	Sprite::FlushBatch();

	size = Sprite::MeasureText(font, text, textSize);
	size += glm::vec2(16);

	int sizeX = (int)size.x, sizeY = (int)size.y;
	unsigned int finalTID;

	unsigned int tempFBO, tempTID, blurFBO, blurTID, finalFBO;
	blurTID = (unsigned int)-1;
	blurFBO = (unsigned int)-1;

	//Step 1 - Draw text to TEMP
	{
		createAndBindBuffer(&tempTID, &tempFBO, sizeX, sizeY);
		Sprite::DrawText(font, text, glm::vec2(8), glm::vec4(1), textSize);
		Sprite::FlushBatch();
	}

	::Texture tempTexture(tempTID, sizeX, sizeY, 4);

	//Step 2 - Draw TEMP to BLUR
	if (style == Style::Blur)
	{
		const int div = 2;
		createAndBindBuffer(&blurTID, &blurFBO, sizeX / div, sizeY / div);
		Sprite::DrawSprite(tempTexture,
			glm::vec2(0),
			glm::vec2(sizeX / div, sizeY / div),
			glm::vec4(0, 0, sizeX, sizeY),
			0.0f, glm::vec4(0, 0, 0, 1));
		Sprite::FlushBatch();
	}

	::Texture blurTexture(blurTID, sizeX / 2, sizeY / 2, 4);

	//Step 3 - Draw BLUR and TEMP to FINAL
	{
		createAndBindBuffer(&finalTID, &finalFBO, sizeX, sizeY);
		const int disp = 2;
		if (style == Style::Blur)
		{
			for (int i = -disp; i <= disp; i++)
				for (int j = -disp; j <= disp; j++)
					Sprite::DrawSprite(blurTexture, glm::vec2(i, j), glm::vec2(sizeX, sizeY));
			//Sprite::DrawSprite(blurTexture, glm::vec2(0), glm::vec2(width, height));
		}
		else if (style == Style::Drop)
		{
			Sprite::DrawSprite(tempTexture, glm::vec2((float)disp), glm::vec2(sizeX, sizeY), glm::vec4(0), 0.0, glm::vec4(0, 0, 0, 1));
		}
		Sprite::DrawSprite(tempTexture, glm::vec2(0), glm::vec2(sizeX, sizeY));
		Sprite::FlushBatch();

		canvas.ID = finalTID;
		canvas.width = sizeX;
		canvas.height = sizeY;
		canvas.channels = 4;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, ::width, ::height);

	glDeleteTextures(1, &tempTID);
	if (blurTID != 1) glDeleteTextures(1, &blurTID);
	glDeleteFramebuffers(1, &tempFBO);
	if (blurFBO != 1) glDeleteFramebuffers(1, &blurFBO);
	glDeleteFramebuffers(1, &finalFBO);
}

DropLabel::DropLabel(const std::string& text, int font, float size, glm::vec4 color, Style style) : text(text), textSize(size), font(font), style(style)
{
	update();
}

void DropLabel::SetText(const std::string& newText)
{
	if (this->text != newText)
	{
		this->text = newText;
		update();
	}
}
