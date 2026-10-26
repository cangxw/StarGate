#pragma once

#include <string>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <unordered_map>

// 模型中的一个节点，可能是骨骼，也可能是普通节点
struct AnimationNode
{
	std::string name;

	// 相对于父节点的局部变换
	glm::mat4 localTransform{ 1.0f };

	// 当前节点的所有孩子
	std::vector<AnimationNode> children;

	// 从原始局部矩阵中分解出的默认值
	glm::vec3 defaultPosition{ 0.0f };
	glm::quat defaultRotation{ 1.0f, 0.0f, 0.0f, 0.0f };
	glm::vec3 defaultScale{ 1.0f };
};

// 一个位置关键帧
struct PositionKey
{
	double time = 0.0;	// tick
	glm::vec3 value{ 0.0 };
};

// 一个旋转关键帧
struct RotationKey
{
	double time = 0.0;

	// 构造参数顺序,  w, x, y, z
	// 这里表示没有旋转
	glm::quat value{ 1.0f, 0.0f, 0.0f, 0.0f };
};

//一个缩放关键帧
struct ScaleKey
{
	double time = 0.0;

	glm::vec3 value{ 1.0f };
};

// 一个节点的动画通道
struct AnimationChannel
{
	// 对应AnimationNode的名字
	std::string nodeName;

	std::vector<PositionKey> positions;
	std::vector<RotationKey> rotations;
	std::vector<ScaleKey> scales;
};

// 一段完整动画，例如 Idle、Walk
struct AnimationClip
{
	std::string name;
	double durationTicks = 0.0;
	double ticksPerSecond = 0.0;
	std::vector<AnimationChannel> channels;
};

//AnimationData
//├── rootNode：节点树，记录父子关系和原始局部变换
//└── clips：动画列表
//			└── 某段动画，例如挥手
//					└── channels：各个节点的运动记录
//					└── 某个节点，例如前臂
//							├── positions：位置关键帧
//							├── rotations：旋转关键帧
//							└── scales：缩放关键帧

struct AnimationData
{
	AnimationNode rootNode;

	// 模型包含的所有动画
	std::vector<AnimationClip> clips;
};

// 某个时刻，整棵节点树的姿势
struct AnimationPose
{
	// 节点名字，从根节点累计下来的变换矩阵
	std::unordered_map<std::string, glm::mat4> globalTransforms;
};