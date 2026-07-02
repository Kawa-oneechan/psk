#include <array>
#include "Particles.h"
#include "Random.h"
#include "SpriteRenderer.h"

extern Texture* whiteRect;

Particle& ParticleEmitter::findParticle()
{
	if (pool[cursor].lifetime <= 0.0f)
		return pool[cursor];
	int loops = 1;
	while (pool[cursor].lifetime > 0.0f && loops > 0)
	{
		cursor++;
		if (cursor == 256)
		{
			cursor = 0;
			loops--;
		}
	}
	return pool[cursor];
}

ParticleEmitter::ParticleEmitter()
{
	prototype.color = glm::vec4(1); //glm::vec4(0.5, 0, 0, 1);
	prototype.angle = 0.0f;
	prototype.rotation = 0.0f;
	prototype.rotationSpeed = 0.5f;
	prototype.speed = 1.0f;
	prototype.speedMultiplier = 1.0f;
	prototype.lifetime = 2.0f;

	prototype.draw = [&](const Particle& p)
	{
		constexpr auto size = 8.0f;
		auto a = p.lifetime < 1.0f ? p.lifetime : 1.0f;
		Sprite::DrawSprite(*whiteRect, glm::vec2(p.position.x, p.position.z) - (size * 0.5f), glm::vec2(size), glm::vec4(0), p.rotation, p.color * glm::vec4(1, 1, 1, a));
	};

	colorRandom = glm::vec4(0); //glm::vec4(0.5, 0, 0, 0);
	angleRandom = 360.0f;
	rotationRandom = 90.0f;
	rotationSpeedRandom = 0.0f;
	speedRandom = 0.5f;
	speedMultiplierRandom = 0.0f;
	lifetimeRandom = 0.5f;
	spawnSpeed = 0.01f;

	spawnTimer = spawnSpeed;
}

bool ParticleEmitter::Tick(float dt)
{
	spawnTimer -= dt;
	if (spawnTimer <= 0.0f)
	{
		spawnTimer = spawnSpeed;

		auto& p = findParticle();
		p.position = prototype.position;
		p.color = prototype.color - (colorRandom * 0.5f) + glm::vec4(Random::GetFloat(colorRandom.r), Random::GetFloat(colorRandom.g), Random::GetFloat(colorRandom.b), Random::GetFloat(colorRandom.a));
		p.angle = prototype.angle - (angleRandom * 0.5f) + Random::GetFloat(angleRandom);
		p.rotation = prototype.rotation - (rotationRandom * 0.5f) + Random::GetFloat(rotationRandom);
		p.rotationSpeed = prototype.rotationSpeed - (rotationSpeedRandom * 0.5f) + Random::GetFloat(rotationSpeedRandom);
		p.speed = prototype.speed - (speedRandom * 0.5f) + Random::GetFloat(speedRandom);
		p.speedMultiplier = prototype.speedMultiplier - (speedMultiplierRandom * 0.5f) + Random::GetFloat(speedMultiplierRandom);
		p.lifetime = prototype.lifetime - (lifetimeRandom * 0.5f) + Random::GetFloat(lifetimeRandom);
		p.draw = prototype.draw;
	}

	for (auto& p : pool)
	{
		p.lifetime -= dt;
		if (p.lifetime <= 0.0f)
			continue;

		p.position.x += glm::sin(glm::radians(p.angle)) * p.speed;
		//Y?
		p.position.z += glm::cos(glm::radians(p.angle)) * p.speed;
		p.speed *= p.speedMultiplier;

		p.rotation += p.rotationSpeed;
	}
	
	return true;
}

void ParticleEmitter::Draw(float dt)
{
	for (auto& p : pool)
	{
		if (p.lifetime <= 0.0f)
			continue;
		p.draw(p);
	}
}
