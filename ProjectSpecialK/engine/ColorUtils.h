#pragma once
#include <vector>
#include <glm/glm.hpp>
#include <json5pp.hpp>
#include "Tickable.h"
#include "Types.h"

//Converts a linear sRGB color value to OkLab color space.
extern glm::vec3 LinearSRGBtoOkLab(const glm::vec3& c);
inline glm::vec3 LinearSRGBtoOkLab(float r, float g, float b) { return LinearSRGBtoOkLab(glm::vec3(r, g, b)); }
//Converts an OkLab color to linear sRGB.
extern glm::vec3 OkLabToLinearSRGB(const glm::vec3& c);
inline glm::vec3 OkLabToLinearSRGB(float l, float a, float b) { return OkLabToLinearSRGB(glm::vec3(l, a, b)); }

//Converts an RGB color value to HSV color space.
extern glm::vec3 RGBtoHSV(const glm::vec3& c);
inline glm::vec3 RGBtoHSV(float r, float g, float b) { return RGBtoHSV(glm::vec3(r, g, b)); }
//Converts an HSV color to RGB.
extern glm::vec3 HSVtoRGB(const glm::vec3& hsv);
inline glm::vec3 HSVtoRGB(float h, float s, float v) { return HSVtoRGB(glm::vec3(h, s, v)); }
//Converts an RGB color value to HSL color space.
extern glm::vec3 RGBtoHSL(const glm::vec3& c);
inline glm::vec3 RGBtoHSL(float r, float g, float b) { return RGBtoHSV(glm::vec3(r, g, b)); }
//Converts an HSL color to RGB.
extern glm::vec3 HSLtoRGB(const glm::vec3& hsv);
inline glm::vec3 HSLtoRGB(float h, float s, float l) { return HSLtoRGB(glm::vec3(h, s, l)); }
