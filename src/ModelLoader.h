#pragma once

#include <vector>
#include <cstddef>

#include "MaterialData.h"

// 子网格的绘制范围和材质
struct  SubMeshData
{
	// 从索引数组的第几个元素开始
	std::size_t firstIndex = 0;

	// 一共有多少个索引
	std::size_t indexCount = 0;

	//对应 materials 数组中的哪一个材质
	unsigned int materialIndex = 0;
};

struct ModelData
{
	//每个顶点8个float:
	//xyz, uv, 法线
	std::vector<float> vertices;

	std::vector<unsigned int> indices;

	std::vector<SubMeshData> subMeshes;
	std::vector<MaterialData> materials;
};

bool LoadModel(const char* path, ModelData& data);