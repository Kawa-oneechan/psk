#include <glad/glad.h>
#include "Framebuffer.h"
#include "SpriteRenderer.h"
#include "Shader.h"
#include "Texture.h"

extern int width, height;

void Framebuffer::setup()
{
	if (isSetup) return;

	unsigned int textureId;

	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	glGenTextures(1, &textureId);
	glBindTexture(GL_TEXTURE_2D, textureId);
	//glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_R11F_G11F_B10F, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
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
	glViewport(0, 0, ::width, ::height);
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
	glViewport(0, 0, ::width, ::height);
}

void Framebuffer::Draw(const glm::vec2& pos)
{
	if (lut)
		lut->Use(1);
	Sprite::DrawSprite(shader, *texture, pos, glm::vec2(texture->width, texture->height));
}

void Framebuffer::Draw(const glm::vec2& pos, const glm::vec2& size)
{
	if (lut)
		lut->Use(1);
	Sprite::DrawSprite(shader, *texture, pos, size);
}

void Framebuffer::ChangeShader(const std::string& newFragmentShader)
{
	if (shaderOwned)
		delete shader;
	shader = new Shader(newFragmentShader);
	shaderOwned = true;
}

void Framebuffer::ChangeShader(Shader* newShader, bool own)
{
	if (shaderOwned)
		delete shader;
	shader = newShader;
	shaderOwned = own;
}

void Framebuffer::ReloadShader()
{
	shader->Reload();
}

Texture& Framebuffer::GetTexture()
{
	return *texture;
}

void Framebuffer::SetLut(Texture* newLut)
{
	lut = newLut;
}

Texture* Framebuffer::GetLut()
{
	return lut;
}
