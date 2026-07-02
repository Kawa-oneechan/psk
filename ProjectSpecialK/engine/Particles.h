#pragma once
#include <functional>
#include "Tickable.h"
#include "Types.h"

struct Particle
{
public:
	glm::vec3 position;
	glm::vec4 color;
	float angle;
	float rotation;
	float rotationSpeed;
	float speed;
	float speedMultiplier;
	float lifetime;
	std::function<void(const Particle&)> draw;
};

class ParticleEmitter : public Tickable
{
private:
	float spawnTimer;
	std::array<Particle, 256> pool{};
	int cursor = 0;
	Particle& findParticle();

public:
	Particle prototype;
	glm::vec4 colorRandom;
	float angleRandom;
	float rotationRandom;
	float rotationSpeedRandom;
	float speedRandom;
	float speedMultiplierRandom;
	float lifetimeRandom;
	float spawnSpeed;
	explicit ParticleEmitter();
	//ParticleEmitter(Particle proto);
	bool Tick(float) override;
	void Draw(float) override;
};
