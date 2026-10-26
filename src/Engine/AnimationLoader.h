#pragma once

#include "AnimationData.h"
#include "SkinnedModelData.h"

bool LoadAnimationData(const char* path, AnimationData& data);
bool LoadSkinnedModelData(const char* path, SkinnedModelData& data);