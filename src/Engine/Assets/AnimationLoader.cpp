#include "AnimationLoader.h"
#include "SkinnedModelData.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/anim.h>
#include <assimp/postprocess.h>
#include <iostream>
#include <string>
#include <utility>
#include <assimp/material.h>
#include <filesystem>

namespace
{
	void PrintNodeHierarchy(const aiNode* node, int depth)
	{
		if (node == nullptr) return;

		// 每深入一层，多缩进两个空格
		std::cout << std::string(depth * 2, ' ') << node->mName.C_Str() << '\n';

		// 依次打印当前节点的所有子节点
		for (unsigned int i = 0; i < node->mNumChildren; ++i)
			PrintNodeHierarchy(node->mChildren[i], depth + 1);
	}

	// GLM 的构造参数按列排列
	//这与 OpenGL 上传矩阵时常用的排列方式一致，方便直接传递数据。 这是库的设计约定
	glm::mat4 ToGlmMatrix(const aiMatrix4x4& matrix)
	{
		return glm::mat4(
			matrix.a1, matrix.b1, matrix.c1, matrix.d1,
			matrix.a2, matrix.b2, matrix.c2, matrix.d2,
			matrix.a3, matrix.b3, matrix.c3, matrix.d3,
			matrix.a4, matrix.b4, matrix.c4, matrix.d4
		);
	}

	AnimationNode CopyNodeHierarchy(const aiNode* source)
	{
		AnimationNode result;
		result.name = source->mName.C_Str();
		result.localTransform = ToGlmMatrix(source->mTransformation);

		aiVector3D position;
		aiQuaternion rotation;
		aiVector3D scale;

		// Assimp ：缩放，旋转，位置
		source->mTransformation.Decompose(scale, rotation, position);
		result.defaultPosition = glm::vec3(position.x, position.y, position.z);
		result.defaultRotation = glm::quat(rotation.w, rotation.x, rotation.y, rotation.z);
		result.defaultScale = glm::vec3(scale.x, scale.y, scale.z);

		// 已知孩子数量，提前准备存储空间
		result.children.reserve(source->mNumChildren);
		for (unsigned int i = 0; i < source->mNumChildren; ++i)
		{
			result.children.push_back(CopyNodeHierarchy(source->mChildren[i]));
		}

		return result;
	}

	AnimationChannel CopyAnimationChannel(const aiNodeAnim* source)
	{
		AnimationChannel result;

		result.nodeName = source->mNodeName.C_Str();
		result.positions.reserve(source->mNumPositionKeys);
		result.rotations.reserve(source->mNumRotationKeys);
		result.scales.reserve(source->mNumScalingKeys);

		// 位置关键帧
		for (unsigned int i = 0; i < source->mNumPositionKeys; ++i)
		{
			const aiVectorKey& key = source->mPositionKeys[i];

			result.positions.push_back({
				key.mTime,
				glm::vec3(key.mValue.x, key.mValue.y, key.mValue.z)
				});
		}

		// 旋转关键帧
		for (unsigned int i = 0; i < source->mNumRotationKeys; ++i)
		{
			const aiQuatKey& key = source->mRotationKeys[i];

			result.rotations.push_back({
				key.mTime,
				glm::quat(
					key.mValue.w,
					key.mValue.x,
					key.mValue.y,
					key.mValue.z
				)
				});
		}

		// 缩放关键帧
		for (unsigned int i = 0; i < source->mNumScalingKeys; ++i)
		{
			const aiVectorKey& key = source->mScalingKeys[i];

			result.scales.push_back({
				key.mTime,
				glm::vec3(key.mValue.x, key.mValue.y, key.mValue.z)
				});
		}

		return result;
	}

	AnimationClip CopyAnimationClip(const aiAnimation* source)
	{
		AnimationClip result;

		result.name = source->mName.C_Str();
		result.durationTicks = source->mDuration;
		result.ticksPerSecond = source->mTicksPerSecond;

		result.channels.reserve(source->mNumChannels);

		for (unsigned int i = 0; i < source->mNumChannels; ++i)
		{
			result.channels.push_back(
				CopyAnimationChannel(source->mChannels[i])
			);
		}

		return result;
	}

	void AddBoneInfluence(SkinnedVertex& vertex, int boneIndex, float weight)
	{
		if (weight <= 0.0f)
			return;

		// 找出四个位置中权重最小的一个
		int smallestSlot = 0;
		for (int i = 1; i < 4; ++i)
		{
			if (vertex.boneWeights[i] < vertex.boneWeights[smallestSlot])
				smallestSlot = i;
		}

		// 保留影响最大的四根骨骼
		if (weight > vertex.boneWeights[smallestSlot])
		{
			vertex.boneIndices[smallestSlot] = boneIndex;
			vertex.boneWeights[smallestSlot] = weight;
		}
	}

	bool CopySkinnedMesh(const aiMesh* source, const std::string& nodeName, SkinnedMeshData& data)
	{
		if (!source->HasPositions() || !source->HasNormals() ||
			!source->HasFaces())
		{
			std::cerr << "Skinned mesh is missing required data.\n";
			return false;
		}


		SkinnedMeshData result;
		result.nodeName = nodeName;
		result.materialIndex = source->mMaterialIndex;
		result.vertices.resize(source->mNumVertices);

		// 复制顶点信息
		for (unsigned int i = 0; i < source->mNumVertices; ++i)
		{
			SkinnedVertex& vertex = result.vertices[i];

			const aiVector3D& position = source->mVertices[i];
			const aiVector3D& normal = source->mNormals[i];

			vertex.position = glm::vec3(position.x, position.y, position.z);
			vertex.normal = glm::vec3(normal.x, normal.y, normal.z);

			if (source->HasTextureCoords(0))
			{
				const aiVector3D& uv = source->mTextureCoords[0][i];
				vertex.texCoord = glm::vec2(uv.x, uv.y);
			}
		}

		// 复制三角形索引
		for (unsigned int i = 0; i < source->mNumFaces; ++i)
		{
			const aiFace& face = source->mFaces[i];
			if (face.mNumIndices != 3)
			{
				std::cerr << "Skinned mesh contains a non-triangle face.\n";
				return false;
			}

			for (unsigned int j = 0; j < 3; ++j)
			{
				const unsigned int index = face.mIndices[j];

				if (index >= source->mNumVertices)
				{
					std::cerr << "Invalid skinned mesh index.\n";
					return false;
				}

				result.indices.push_back(index);
			}
		}

		// 复制骨骼，并把影响写入对应的顶点
		result.bones.reserve(source->mNumBones);

		for (unsigned int i = 0; i < source->mNumBones; ++i)
		{
			const aiBone* bone = source->mBones[i];

			BoneInfo info;
			info.nodeName = bone->mName.C_Str();
			info.offsetMatrix = ToGlmMatrix(bone->mOffsetMatrix);

			result.bones.push_back(std::move(info));

			for (unsigned int j = 0; j < bone->mNumWeights; ++j)
			{
				const aiVertexWeight& influence = bone->mWeights[j];

				if (influence.mVertexId >= result.vertices.size())
				{
					std::cerr << "Invalid bone vertex index.\n";
					return false;
				}

				AddBoneInfluence(result.vertices[influence.mVertexId], static_cast<int>(i), influence.mWeight);
			}
		}

		// 保留四个影响后，重新让权重之和等于 1
		for (SkinnedVertex& vertex : result.vertices)
		{
			const float total = vertex.boneWeights.x + vertex.boneWeights.y + vertex.boneWeights.z + vertex.boneWeights.w;

			if (total > 0.0f)
			{
				vertex.boneWeights /= total;
			}
		}

		data = std::move(result);
		return true;
	}

	bool CopyNodeMeshes(const aiScene* scene, const aiNode* node, SkinnedModelData& result)
	{
		// 当前节点应用的所有网格
		for (unsigned int i = 0; i < node->mNumMeshes; ++i)
		{
			const unsigned int meshIndex = node->mMeshes[i];

			if (meshIndex >= scene->mNumMeshes)
			{
				std::cerr << "Invalid node mesh index.\n";
				return false;
			}

			const aiMesh* sourceMesh = scene->mMeshes[meshIndex];

			// 当前只处理三角形网格
			if ((sourceMesh->mPrimitiveTypes & aiPrimitiveType_TRIANGLE) == 0)
				continue;

			SkinnedMeshData meshData;

			if(!CopySkinnedMesh(sourceMesh, node->mName.C_Str(), meshData))
				return false;

			result.meshes.push_back(std::move(meshData));
		}

		// 继续处理所有子节点
		for (unsigned int i = 0; i < node->mNumChildren; ++i)
		{
			if (!CopyNodeMeshes(scene, node->mChildren[i], result))
			{
				return false;
			}
		}
		
		return true;
	}

	bool CopySkinnedMaterial(const aiScene* scene, const char* modelPath, std::vector<MaterialData>& materials)
	{
		const std::filesystem::path modelDirectory = std::filesystem::path(modelPath).parent_path();

		materials.clear();
		materials.reserve(scene->mNumMaterials);

		for (unsigned int i = 0; i < scene->mNumMaterials; ++i)
		{
			const aiMaterial* source = scene->mMaterials[i];
			MaterialData material;

			aiColor3D color(1.0f, 1.0f, 1.0f);

			if (source->Get(AI_MATKEY_BASE_COLOR, color) != AI_SUCCESS)
			{
				source->Get(AI_MATKEY_COLOR_DIFFUSE, color);
			}

			material.diffuseColor = glm::vec3(color.r, color.g, color.b);

			aiString texturePath;
			aiReturn textureResult = source->GetTexture(aiTextureType_BASE_COLOR,0, &texturePath);

			if (textureResult != AI_SUCCESS)
			{
				textureResult = source->GetTexture(aiTextureType_DIFFUSE, 0, &texturePath);
			}

			if (textureResult == AI_SUCCESS && texturePath.length > 0)
			{
				if (texturePath.C_Str()[0] == '*')
				{
					std::cerr << "Embedded textures are not supported yet.\n";
					return false;
				}

				const std::filesystem::path relativePath(texturePath.C_Str());

				// 先按模型的路径查找
				std::filesystem::path fullPath = modelDirectory / relativePath;
				if (!std::filesystem::is_regular_file(fullPath))
				{
					// 再到source旁边的textures目录查找
					fullPath = modelDirectory.parent_path()
						/ "textures"
						/ relativePath.filename();
				}

				if (!std::filesystem::is_regular_file(fullPath))
				{
					std::cerr << "TexturePath not found" << texturePath.C_Str() << '\n';
					return false;
				}

				material.diffuseTexturePath = fullPath.lexically_normal().string();
			}

			materials.push_back(std::move(material));
		}

		return true;
	}
}

bool LoadAnimationData(const char* path, AnimationData& data)
{
	Assimp::Importer importer;

	const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate);
	if (scene == nullptr || scene->mRootNode == nullptr ||
		(scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) != 0)
	{
		std::cerr << "Failed to inspect model: "
			<< importer.GetErrorString() << '\n';
		return false;
	}

	std::cout << "\n--- Animation Info ---\n";
	std::cout << "Meshes: " << scene->mNumMeshes << '\n';
	std::cout << "Animations: " << scene->mNumAnimations << '\n';

	//查看每个网格引用了多少根骨骼
	for (unsigned int i = 0; i < scene->mNumMeshes; ++i)
	{
		const aiMesh* mesh = scene->mMeshes[i];
		
		std::cout << "Mesh " << i << ": " << mesh->mName.C_Str()
			<< ", Bones: " << mesh->mNumBones << '\n';
	}

	for (unsigned int i = 0; i < scene->mNumAnimations; ++i)
	{
		const aiAnimation* animation = scene->mAnimations[i];

		std::cout << "\nAnimation " << i << ": " << animation->mName.C_Str() << '\n';

		//有多少个ticks
		std::cout << "Duration (ticks): "<< animation->mDuration << '\n';
		//每秒多少ticks
		std::cout << "Ticks per second: "<< animation->mTicksPerSecond << '\n';
		// 动画为每根需要控制的骨骼保存一条通道。 每条通道里包含这根骨骼的位置、旋转、缩放关键帧。
		//	准确一点说，通道对应的是节点，所以它也可能控制角色的根节点或其他非骨骼节点。因此，mNumChannels 不一定等于骨骼总数。
		//	另外，父骨骼运动会带着子骨骼一起运动，子骨骼即使没有自己的动画通道，也可能跟着移动。
		std::cout << "Node channels: " << animation->mNumChannels << '\n';

		if (animation->mTicksPerSecond > 0.0)
		{
			const double seconds = animation->mDuration / animation->mTicksPerSecond;

			std::cout << "Duration (seconds): " << seconds << '\n';
		}
	}

	std::cout << "\n--- Node Hierarchy ---\n";
	PrintNodeHierarchy(scene->mRootNode, 0);

	AnimationData result;
	result.rootNode = CopyNodeHierarchy(scene->mRootNode);

	result.clips.reserve(scene->mNumAnimations);
	for (unsigned int i = 0; i < scene->mNumAnimations; ++i)
	{
		result.clips.push_back(CopyAnimationClip(scene->mAnimations[i]));
	}

	//局部变量 importer 析构后，它拥有的 aiScene、aiNode 等数据就会被释放。 
	// 我们保存的是自己的字符串、矩阵和子节点，不再依赖那些指针。
	data = std::move(result);

	return true;
}

bool LoadSkinnedModelData(const char* path, SkinnedModelData& data)
{
	Assimp::Importer importer;

	const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_GenNormals | aiProcess_SortByPType);

	if (scene == nullptr || scene->mRootNode == nullptr ||
		(scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) != 0)
	{
		std::cerr << "Failed to load skinned model: " << importer.GetErrorString() << '\n';
		return false;
	}

	SkinnedModelData result;
	if (!CopySkinnedMaterial(scene, path, result.materials))
		return false;

	if (!CopyNodeMeshes(scene, scene->mRootNode, result))
		return false;

	if (result.meshes.empty())
	{
		std::cerr << "Model contains no triangle meshes.\n";
		return false;
	}

	// 检查materialIndex是否在合理的范围内
	for (const SkinnedMeshData& mesh : result.meshes)
	{
		if (mesh.materialIndex >= result.materials.size())
		{
			std::cerr << "Invalid skinned mesh material index.\n";
			return false;
		}
	}

	// 从同一份场景复制节点树和动画
	result.animation.rootNode = CopyNodeHierarchy(scene->mRootNode);

	result.animation.clips.reserve(scene->mNumAnimations);

	for (unsigned int i = 0; i < scene->mNumAnimations; ++i)
	{
		result.animation.clips.push_back(CopyAnimationClip(scene->mAnimations[i]));
	}

	data = std::move(result);
	return true;
}