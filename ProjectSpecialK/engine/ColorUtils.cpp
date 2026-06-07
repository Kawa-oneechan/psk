#include <ctime>
#include <numeric>
#include <glad/glad.h>
#include <stb_image_write.h>
#include "Utilities.h"
#include "Console.h"
#include "InputsMap.h"
#include "Tickable.h"

//From https://bottosson.github.io/posts/oklab/
glm::vec3 LinearSRGBtoOkLab(const glm::vec3& c)
{
	float l = 0.4122214708f * c.r + 0.5363325363f * c.g + 0.0514459929f * c.b;
	float m = 0.2119034982f * c.r + 0.6806995451f * c.g + 0.1073969566f * c.b;
	float s = 0.0883024619f * c.r + 0.2817188376f * c.g + 0.6299787005f * c.b;

	float l_ = cbrtf(l);
	float m_ = cbrtf(m);
	float s_ = cbrtf(s);

	return glm::vec3(
		0.2104542553f * l_ + 0.7936177850f * m_ - 0.0040720468f * s_,
		1.9779984951f * l_ - 2.4285922050f * m_ + 0.4505937099f * s_,
		0.0259040371f * l_ + 0.7827717662f * m_ - 0.8086757660f * s_
		);
}

glm::vec3 OkLabToLinearSRGB(const glm::vec3& c)
{
	float l_ = c.r + 0.3963377774f * c.g + 0.2158037573f * c.b;
	float m_ = c.r - 0.1055613458f * c.g - 0.0638541728f * c.b;
	float s_ = c.r - 0.0894841775f * c.g - 1.2914855480f * c.b;

	float l = l_ * l_ * l_;
	float m = m_ * m_ * m_;
	float s = s_ * s_ * s_;

	return glm::vec3(
		+4.0767416621f * l - 3.3077115913f * m + 0.2309699292f * s,
		-1.2684380046f * l + 2.6097574011f * m - 0.3413193965f * s,
		-0.0041960863f * l - 0.7034186147f * m + 1.7076147010f * s
		);
}

//From https://gist.github.com/flowb/6325155c7cb5f8e07cc9837e845fe780
glm::vec3 RGBtoHSV(const glm::vec3& c)
{
	auto max = glm::max(c.r, glm::max(c.g, c.b)); //Value
	auto min = glm::min(c.r, glm::min(c.g, c.b));

	auto delta = max - min;

	auto sat = delta / (max + glm::step(max, 0.0f)); //Saturation

	/*
	This is a vec3 wrapped around 3 boolean evaluations
	cast to floats. it's used to figure out which third
	of the hue cycle we are in. Only one of the three
	components should be true and be cast to 1.0.
	*/
	auto isMax = glm::vec3(
		float(c.r >= c.g && c.r >= c.b), //Yellow to magenta
		float(c.g > c.r && c.g >= c.b),  //Yellow to cyan
		float(c.b > c.r && c.b > c.g)	 //Magenta to cyan
	);

	/*
	For each channel: get the difference between the other
	two channels. then divide all by the overall delta.
	*/
	auto secondary = glm::vec3(
		(c.g - c.b),
		(c.b - c.r),
		(c.r - c.g))
		/ (delta + glm::step(delta, 0.0f));

	/*
	Pad the G and B channels to make a 1/3 phase offset in
	a [0-5] domain.
	*/
	secondary += glm::vec3(0.0f, 2.0f, 4.0f);

	/*
	The ternary catches achromatic input colors and
	returns hue = 0.0 instead. this could be probably be
	done with funky step/mix voodoo instead but IMHO the
	branch here is a small price to pay for readibility.
	the dot product performs componentwise multiplication
	and sums all components. this lets us take all three
	secondary deltas, scales the ones we don't need to
	zero and retain the result we actually need.
	*/
	auto hue = sat > 0.0f ?
		glm::fract(dot(isMax, secondary) / 6.0f) :
		0.0f;

	return glm::vec3(hue, sat, max);
}

glm::vec3 HSVtoRGB(const glm::vec3& hsv)
{
	/*E
	valuates three phase-shifted triangle wave functions
	for the input of hue to get fully saturated R, G and B.
	*/
	auto rgb = glm::clamp(
		glm::abs(
			glm::mod(
				hsv.x * 6.0f + glm::vec3(0.0f, 4.0f, 2.0f),
				6.0f) -
			3.0f) -
		1.0f, 0.0f, 1.0f);

	/*
	Mixes between white and fully saturated RGB
	then scales by value
	*/
	return glm::mix(glm::vec3(1.0f), rgb, hsv.y) * hsv.z;
}

//From various places
glm::vec3 RGBtoHSL(const glm::vec3& c)
{
	auto hsv = RGBtoHSV(c);
	auto l = hsv.z * (1.0f - (hsv.y * 0.5f));
	return glm::vec3(hsv.x, (l == 0.0f || l == 1.0f) ? 0 : (hsv.z - l) / glm::min(l, 1.0f - l), l);
}

glm::vec3 HSLtoRGB(const glm::vec3& hsl)
{
	auto rgb = glm::clamp(
		glm::abs(
			glm::mod(
				hsl.x * 6.0f + glm::vec3(0.0f, 4.0f, 2.0f),
				6.0f) -
			3.0f) -
		1.0f, 0.0f, 1.0f);
	auto c = (1.0f - glm::abs(2.0f * hsl.z - 1.0f)) * hsl.y;
	return (rgb - 0.5f) * c + hsl.z;
}
