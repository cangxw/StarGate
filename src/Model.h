#pragma once

#include <memory>
#include <vector>

#include "ModelLoader.h"
#include "Mesh.h"
#include "Texture2D.h"
#include "Shader.h"

class Model
{
public:
	explicit Model(const char* path);

	~Model() = default;

	bool IsValid() const;
	void Draw(const Shader& shader) const;

	Model(const Model&) = delete;
	Model& operator=(const Model&) = delete;

private:
	ModelData m_Data;

	//GPU 网格资源
	// 智能指针，一个拥有者，负责自动销毁对象（离开作用域）
	std::unique_ptr<Mesh> m_Mesh;

	//贴图资源
	std::vector<std::unique_ptr<Texture2D>> m_Textures;
};