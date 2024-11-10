#version 330 core

in vec2 TexCoords;
flat in int index;

out vec4 color;

#include "common.txt"

uniform sampler2D image;

vec3 uncharted2Tonemap(vec3 x)
{
	float A = 0.15;
	float B = 0.50;
	float C = 0.10;
	float D = 0.20;
	float E = 0.02;
	float F = 0.30;
	float W = 11.2;
	return ((x * (A * x + C * B) + D * E) / (x * (A * x + B) + D * F)) - E / F;
}

vec3 uncharted2(vec3 color)
{
	const float W = 2.2;
	float exposureBias = 2.0;
	vec3 curr = uncharted2Tonemap(exposureBias * color);
	vec3 whiteScale = 1.0 / uncharted2Tonemap(vec3(W));
	return curr * whiteScale;
}

vec3 neutral(vec3 color)
{
	const float startCompression = 0.8 - 0.04;
	const float desaturation = 0.0;

	float x = min(color.r, min(color.g, color.b));
	float offset = x < 0.08 ? x - 6.25 * x * x : 0.04;
	color -= offset;

	float peak = max(color.r, max(color.g, color.b));
	if (peak < startCompression) return color;

	const float d = 1.0 - startCompression;
	float newPeak = 1.0 - d * d / (peak + d - startCompression);
	color *= newPeak / peak;

	float g = 1.0 - 1.0 / (desaturation * (peak - newPeak) + 1.0);
	return mix(color, vec3(newPeak), g);
}

vec3 uchimura(vec3 x, float P, float a, float m, float l, float c, float b)
{
	float l0 = ((P - m) * l) / a;
	float L0 = m - m / a;
	float L1 = m + (1.0 - m) / a;
	float S0 = m + l0;
	float S1 = m + a * l0;
	float C2 = (a * P) / (P - S1);
	float CP = -C2 / P;

	vec3 w0 = vec3(1.0 - smoothstep(0.0, m, x));
	vec3 w2 = vec3(step(m + l0, x));
	vec3 w1 = vec3(1.0 - w0 - w2);

	vec3 T = vec3(m * pow(x / m, vec3(c)) + b);
	vec3 S = vec3(P - (P - S1) * exp(CP * (x - S0)));
	vec3 L = vec3(m + a * (x - m));

	return T * w0 + L * w1 + S * w2;
}

vec3 uchimura(vec3 x)
{
	const float P = 1.0;	// max display brightness
	const float a = 1.0;	// contrast
	const float m = 0.22;	// linear section start
	const float l = 0.4;	// linear section length
	const float c = 1.33;	// black
	const float b = 0.0;	// pedestal

	return uchimura(x, P, a, m, l, c, b);
}

void main()
{
	vec3 hdrColor = texture(image, TexCoords).rgb;
	
	color = vec4(uchimura(hdrColor), 1.0);
}
