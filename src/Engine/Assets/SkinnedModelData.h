#pragma once

#include <string>
#include <vector>
#include <glm/glm.hpp>

#include "Engine/Animation/AnimationData.h"
#include "MaterialData.h"

// 一个带骨骼影响信息的顶点
struct SkinnedVertex
{
	glm::vec3 position{ 0.0f };
	glm::vec2 texCoord{ 0.0f };
	glm::vec3 normal{ 0.0f, 1.0f, 0.0f };

	// -1表示没有骨骼
	// 这个版本每个顶点最多受到四根骨骼的影响
	// ivec4表示四个整数向量
	glm::ivec4 boneIndices{ -1 };
	glm::vec4 boneWeights{ 0.0f };
};

// 当前网格使用的一根骨骼
struct BoneInfo
{
	// 对应节点树中的节点名字
	std::string nodeName;

	// 将网格局部坐标转换到骨骼的绑定空间
	glm::mat4 offsetMatrix{ 1.0f };
};

struct SkinnedMeshData
{
	// 网格挂在哪个节点上
	std::string nodeName;

	std::vector<SkinnedVertex> vertices;
	std::vector<unsigned int> indices;

	// 顶点中的boneIndices，指向这个数组
	std::vector<BoneInfo> bones;

	unsigned int materialIndex = 0;
};

struct SkinnedModelData
{
	std::vector<SkinnedMeshData> meshes;
	std::vector<MaterialData> materials;

	// 复用我们已经完成的节点数和动画数据
	AnimationData animation;
};