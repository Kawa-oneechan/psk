#pragma once
#include <vector>
#include <functional>
#include <glm/glm.hpp>
#include <json5pp.hpp>
#include "Tickable.h"
#include "Types.h"

//Takes a screenshot, saving as a timestamped PNG.
extern void Screenshot();

//Displays a multi-threaded loading screen with a progress bar, optionally over a cinematic file.
//If no cinematic file is specified, a bouncing loading.png is used.
extern void ThreadedLoader(std::function<void(float*)> loader, const std::string& cinematicFile = "");

#define arraysize(A) (sizeof(A) / sizeof((A)[0]))
#define sizeof_member(T, M) sizeof(((T *)0)->M)
