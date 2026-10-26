#pragma once

#include "AnimationData.h"

glm::vec3 SamplePosition(
	const std::vector<PositionKey>& key,
	double timeTicks,
	const glm::vec3& fallback);

glm::quat SampleRotation(
	const std::vector<RotationKey>& keys,
	double timeTicks,
	const glm::quat& fallback
);

glm::vec3 SampleScale(
	const std::vector<ScaleKey>& keys,
	double timeTicks,
	const glm::vec3& fallback
);

glm::mat4 SampleNodeTransform(
	const AnimationNode& node,
	const AnimationChannel* channel,
	double timeTicks
);

void SampleAnimationPose(
	const AnimationNode& rootNode, 
	const AnimationClip* clip,
	double timeTicks, 
	AnimationPose& pose
);
