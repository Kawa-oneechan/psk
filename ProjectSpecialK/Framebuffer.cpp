#include "SpecialK.h"
#include "Framebuffer.h"

void Framebuffer::setup()
{
	if (isSetup) return;

	unsigned int textureId;

	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	glGenTextures(1, &textureId);
	glBindTexture(GL_TEXTURE_2D, textureId);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureId, 0);

	glGenRenderbuffers(1, &rbo);
	glBindRenderbuffer(GL_RENDERBUFFER, rbo);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

	texture = new Texture(textureId, width, height, 4);
	texture->SetFilter(GL_NEAREST);
	isSetup = true;
}


Framebuffer::Framebuffer(const std::string& fragmentShader, int width, int height) : width(width), height(height)
{
	glGenFramebuffers(1, &fbo);
	shader = new Shader(fragmentShader);
	shaderOwned = true;
}

Framebuffer::Framebuffer(Shader* fragmentShader, int width, int height) : width(width), height(height)
{
	glGenFramebuffers(1, &fbo);
	shader = fragmentShader;
	shaderOwned = false;
}

Framebuffer::~Framebuffer()
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	delete texture;
	if (shaderOwned)
		delete shader;
}

void Framebuffer::Use()
{
	if (!isSetup) setup();
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	glViewport(0, 0, width, height);
}

void Framebuffer::Drop()
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Framebuffer::Draw(glm::vec2 pos)
{
	Sprite::DrawSprite(shader, *texture, pos, glm::vec2(texture->width, texture->height));
}

void Framebuffer::Draw(glm::vec2 pos, glm::vec2 size)
{
	Sprite::DrawSprite(shader, *texture, pos, size);
}

void Framebuffer::ChangeShader(const std::string& newFragmentShader)
{
	if (shaderOwned)
		delete shader;
	shader = new Shader(newFragmentShader);
	shaderOwned = true;
}

void Framebuffer::ChangeShader(Shader* newShader)
{
	if (shaderOwned)
		delete shader;
	shader = newShader;
	shaderOwned = false;
}

Texture& Framebuffer::GetTexture()
{
	return *texture;
}
