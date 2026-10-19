#include "AnimationLoader.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/anim.h>
#include <assimp/postprocess.h>
#include <iostream>
#include <string>
#include <utility>

namespace
{
	void PrintNodeHierarchy(const aiNode* node, int depth)
	{
		if (node == nullptr) return;

		// 每深入一层，多缩进两个空格
		std::cout << std::string(depth * 2, ' ')<< node->mName.C_Str()<< '\n';

		// 依次打印当前节点的所有子节点
		for (unsigned int i = 0; i < node->mNumChildren; ++i)
			PrintNodeHierarchy(node->mChildren[i],depth + 1);
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