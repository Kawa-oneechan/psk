#include "engine/Text.h"
#include "engine/TextUtils.h"
#include "engine/JSONUtils.h"
#include "engine/Random.h"
#include "engine/Console.h"
#include "Person.h"
#include "Town.h"
#include "Player.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/rotate_vector.hpp>

bool botherColliding = true;

void Person::Turn(float facing, float dt)
{
	auto m = Facing;
	if (m < 0) m += 360.0f;

	auto cw = facing - m;
	if (cw < 0.0) cw += 360.0f;
	auto ccw = m - facing;
	if (ccw < 0.0) ccw += 360.0f;

	constexpr auto radius = 45.0f;
	constexpr auto timeScale = 20.0f;

	auto t = (ccw < cw) ? -glm::min(radius, ccw) : glm::min(radius, cw);

	auto f = m + (t * (dt * timeScale));
	if (f < 0) f += 360.0f;

	Facing = glm::mod(f, 360.0f);
}

bool Person::Move(float facing, float dt)
{
	Turn(facing, dt);

	const auto movement = glm::rotate(glm::vec2(0, 0.25f), glm::radians(Facing)) * dt;

	constexpr auto speed = 120.0f;

	auto newPos = Position;
	newPos.x -= movement.x * speed;
	newPos.z += movement.y * speed;

	//TODO: This is kinda fucked up, not gonna lie. Gonna need a much better way to do this.
	//But it's SOMETHING I guess?
	auto aheadPos = Position;
	aheadPos.x -= movement.x * (speed * 15);
	aheadPos.z += movement.y * (speed * 15);

	//Shit cliff collision detection. Do not use. Replace it later.
	/*
	auto myHeight = Position.y;
	//TODO: use current map instead of just the town in due time
	auto newHeight = town->GetHeight(aheadPos + glm::vec3(0, 10, 0));
	auto heightDiff = glm::abs(newHeight - myHeight);
	if (heightDiff > 5.0f)
		return false;
	*/

	if (botherColliding)
	{
		float c2c = 0.0f;
		//float c2c = FindVillagerCollision(newPos);
		//TODO: use current map, not town.
		for (auto p : town->People)
		{
			if (p == this)
				continue;
			auto dist = glm::distance(p->Position, newPos);

			const auto r = 2.0f;
			if (dist <= r + r)
			{
				c2c = r + r - dist;
				if (c2c > 0.0f)
				{
					//TODO: PUSH.
					return true;
				}
			}
		}
	}

	Position = newPos;
	return true;
}

void Person::SetFace(int index)
{
	face = glm::clamp(index, 0, 15);
}

void Person::SetMouth(int index)
{
	mouth = glm::clamp(index, 0, 8);
}

void Person::Draw(float)
{
	//Remember: call this from Villager::Draw or Player::Draw.
	//This ONLY handles outfits and held items.

	for (int i = 0; i < NumClothes; i++)
	{
		if (!_clothesModels[i])
			continue;
		auto& c = _clothesModels[i];
		_model->CopyBoneTransforms(c);
		//May need to special-case the glasses and mask, possibly the hat.

		//c->Draw(Position, Facing);
		c->Draw();
	}
}
