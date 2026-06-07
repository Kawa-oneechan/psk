#pragma once
#include <vector>
#include <glm/glm.hpp>
#include <json5pp.hpp>
#include "Tickable.h"
#include "Types.h"

//Takes a screenshot, saving as a timestamped PNG.
extern void Screenshot();

#define arraysize(A) (sizeof(A) / sizeof((A)[0]))
#define sizeof_member(T, M) sizeof(((T *)0)->M)
