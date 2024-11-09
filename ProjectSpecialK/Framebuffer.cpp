#include "SpecialK.h"
#include "Framebuffer.h"

void Framebuffer::setup()
{
	if (isSetup) return;

	unsigned int textureId;

	glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	glGenTextures(1, &textureId);
	glBindTexture(GL_TEXTURE_2D, textureId);
	glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, GL_RGBA,
		(format == GL_RGB16F || format == GL_RGBA16F) ? GL_FLOAT : GL_UNSIGNED_BYTE
		, NULL);
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


Framebuffer::Framebuffer(const std::string& fragmentShader, int width, int height, int format) : width(width), height(height), format(format)
{
	shader = new Shader(fragmentShader);
	shaderOwned = true;
}

Framebuffer::Framebuffer(Shader* fragmentShader, int width, int height) : width(width), height(height)
{
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
	glViewport(0, 0, ::width, ::height);
}

void Framebuffer::Draw(const glm::vec2& pos)
{
	Sprite::DrawSprite(shader, *texture, pos, glm::vec2(texture->width, texture->height));
}

void Framebuffer::Draw(const glm::vec2& pos, const glm::vec2& size)
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

Shader& Framebuffer::GetShader()
{
	return *shader;
}
