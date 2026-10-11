#include "Model.h"
#include <iostream>
#include <limits>

Model::Model(const char* path)
{
	// 1. 读取模型的数据
	if (!LoadModel(path, m_Data))
	{
		return;
	}

	// 2. 检查索引数量是否可以转换成GLsizei
	if (m_Data.indices.size() > static_cast<std::size_t>(std::numeric_limits<GLsizei>::max()))
	{
		std::cerr << "Model has too many indices to draw."<< std::endl;
		return;
	}

	// 检查顶点数据的字节数是否超出 GLsizeiptr 的范围
	const std::size_t maxBufferSize =static_cast<std::size_t>(std::numeric_limits<GLsizeiptr>::max());
	if (m_Data.vertices.size() > maxBufferSize / sizeof(float))
	{
		std::cerr << "Model vertex buffer is too large."<< std::endl;
		return;
	}

	// 3. 为每个材质加载贴图
	m_Textures.reserve(m_Data.materials.size());

	for (const MaterialData& material : m_Data.materials)
	{
		if (material.diffuseTexturePath.empty())
		{
			m_Textures.push_back(nullptr);
		}
		else
		{
			m_Textures.push_back(std::make_unique<Texture2D>(material.diffuseTexturePath.c_str()));
		}
	}
	// 4. 创建GPU网格
	const GLsizei indexCount = static_cast<GLsizei>(m_Data.indices.size());

	const GLsizeiptr vertexDataSize = static_cast<GLsizeiptr>(m_Data.vertices.size() * sizeof(float));

	m_Mesh = std::make_unique<Mesh>(
		m_Data.vertices.data(),
		vertexDataSize,
		m_Data.indices.data(),
		indexCount
	);
}

bool Model::IsValid() const
{
	return m_Mesh != nullptr;
}

void Model::Draw(const Shader& shader) const
{
	if (!IsValid())
		return;

	shader.Bind();
	shader.SetInt("uTexture", 0);

	for (const SubMeshData& subMesh : m_Data.subMeshes)
	{
		const MaterialData& material = m_Data.materials[subMesh.materialIndex];

		const auto& texture = m_Textures[subMesh.materialIndex];

		shader.SetInt("uHasTexture", texture != nullptr ? 1 : 0);

		if (texture != nullptr)
			texture->Bind(0);

		shader.SetVec3("uMaterialColor", material.diffuseColor);

		//绘制当前子网格
		m_Mesh->Draw(subMesh.firstIndex, static_cast<GLsizei>(subMesh.indexCount));
	}
}