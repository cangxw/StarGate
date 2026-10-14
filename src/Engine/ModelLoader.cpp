#include "ModelLoader.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/material.h>

#include <iostream>
#include <limits>
#include <utility>
#include <filesystem>


bool LoadModel(const char* path, ModelData& data)
{
	Assimp::Importer importer;

	const aiScene* scene = importer.ReadFile(
		path,
		aiProcess_Triangulate |		//将多边形拆成三角形
		aiProcess_GenNormals |	//模型缺少法线时生成面法线
		aiProcess_PreTransformVertices |	//把模型节点的变换合入顶点，适合当前静态模型加载
		aiProcess_SortByPType		//将三角形、线、点分到不同网格，方便筛选
	);

	if (scene == nullptr || scene->mRootNode == nullptr ||
		(scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) != 0)
	{
		std::cerr << "Failed to load model: " << path
			<< "\nReason: " << importer.GetErrorString()
			<< std::endl;

		return false;
	}

	ModelData result;

	//模型所在目录，用于解析贴图的相对路径
	const std::filesystem::path modelDirectory = std::filesystem::path(path).parent_path();

	for (unsigned int i = 0; i < scene->mNumMaterials; ++i)
	{
		const aiMaterial* material = scene->mMaterials[i];

		MaterialData materialData;

		aiColor3D color(1.0f, 1.0f, 1.0f);

		// AI_MATKEY_BASE_COLOR 是 PBR的基础颜色
		// 暂时优先使用basecolor,后面增加对PBR的支持
		if (material->Get(AI_MATKEY_BASE_COLOR, color) != AI_SUCCESS)
		{
			// 漫反射是blinn-phong模型使用的
			material->Get(AI_MATKEY_COLOR_DIFFUSE, color);
		}

		materialData.diffuseColor = glm::vec3(color.r, color.g, color.b);

		aiString texturePath;

		aiReturn textureResult = material->GetTexture(aiTextureType_BASE_COLOR, 0, &texturePath);
		if (textureResult != AI_SUCCESS)
		{
			textureResult = material->GetTexture(aiTextureType_DIFFUSE, 0, &texturePath);
		}

		if (textureResult == AI_SUCCESS && texturePath.length > 0)
		{
			// "*0" 等名称表示内嵌纹理，这一版先处理外部图片
			if (texturePath.C_Str()[0] != '*')
			{
				const std::filesystem::path fullPath =modelDirectory / std::filesystem::path(texturePath.C_Str());

				materialData.diffuseTexturePath = fullPath.lexically_normal().string();
			}
		}

		result.materials.push_back(std::move(materialData));
	}

	for (unsigned int meshIndex = 0; meshIndex < scene->mNumMeshes; ++meshIndex)
	{
		//一个模型文件是一个aiScene
		// 一个aiScene可以有多个网格组成，车身，车轮等
		// 每个aiMesh保存自己的位置，法线，uv，索引，材质索引等
		// 同一个物体使用不同材质的部分，导入时可能会拆成多个网格。
		const aiMesh* mesh = scene->mMeshes[meshIndex];

		//当前只会渲染三角形，跳过点和线
		if ((mesh->mPrimitiveTypes & aiPrimitiveType_TRIANGLE) == 0)
			continue;

		if (!mesh->HasPositions() ||
			!mesh->HasNormals() ||
			!mesh->HasFaces())
		{
			std::cerr << "Model mesh is missing required data."
				<< std::endl;

			return false;
		}

		// 计算一共就几个顶点，每个顶点包含8个float
		const std::size_t vertexCount = result.vertices.size() / 8;
		const std::size_t maxVertexCount = std::numeric_limits<unsigned int>::max();

		//确保不到顶点索引值上限
		//顶点索引值是unsigned int
		if (vertexCount > maxVertexCount || 
			maxVertexCount - vertexCount < mesh->mNumVertices)
		{
			std::cerr << "Model has too many vertices." << std::endl;
			return false;
		}

		const unsigned int baseVertex = static_cast<unsigned int>(vertexCount);

		if (mesh->mMaterialIndex >= result.materials.size())
		{
			std::cerr << "Model contains an invalid material index."
				<< std::endl;

			return false;
		}

		SubMeshData subMesh;
		subMesh.firstIndex = result.indices.size();
		subMesh.materialIndex = mesh->mMaterialIndex;

		// 读取每个顶点，整理成现有Mesh使用的格式
		for (unsigned int i = 0; i < mesh->mNumVertices; ++i)
		{
			const aiVector3D& position = mesh->mVertices[i];
			const aiVector3D& normal = mesh->mNormals[i];

			aiVector3D uv(0.0f, 0.0f, 0.0f);
			if (mesh->HasTextureCoords(0))
				uv = mesh->mTextureCoords[0][i];

			result.vertices.insert(
				result.vertices.end(),
				{
					position.x, position.y, position.z,
					uv.x, uv.y,
					normal.x, normal.y, normal.z
				}
			);
		}

		for (unsigned int i = 0; i < mesh->mNumFaces; ++i)
		{
			const aiFace& face = mesh->mFaces[i];

			if (face.mNumIndices != 3)
			{
				std::cerr << "Model contains a non-triangle face."
					<< std::endl;

				return false;
			}

			for (unsigned int j = 0; j < 3; ++j)
			{
				const unsigned int localIndex = face.mIndices[j];

				if (localIndex >= mesh->mNumVertices)
				{
					std::cerr << "Model contains an invalid index."
						<< std::endl;

					return false;
				}

				//baseVertex 合并后的数组从哪个编号开始
				result.indices.push_back(baseVertex + localIndex);
			}
		}

		// 加入后的数量减去加入前的数量，就是这个网格的索引数量
		subMesh.indexCount = result.indices.size() - subMesh.firstIndex;

		result.subMeshes.push_back(subMesh);
	}

	if (result.vertices.empty() || result.indices.empty())
	{
		std::cerr << "Model contains no triangle geometry: "
			<< path << std::endl;

		return false;
	}

	//将临时结果的存储转交给 data，避免再复制一遍两个大数组。
	data = std::move(result);
	return true;
}