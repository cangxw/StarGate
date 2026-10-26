#include "AnimationSampler.h"
#include <cstddef>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

namespace
{
    // 按节点名字寻找动画通道
    const AnimationChannel* FindAnimationChannel(const AnimationClip* clip, const std::string& nodeName)
    {
        if (clip == nullptr)
            return nullptr;

        for (const AnimationChannel& channel : clip->channels)
        {
            if (channel.nodeName == nodeName)
                return &channel;
        }

        return nullptr;
    }

    void SampleNodeHierarchy(const AnimationNode& node, const AnimationClip* clip, 
        double timeTicks, const glm::mat4& parentTransform, AnimationPose& pose)
    {
        const AnimationChannel* channel = FindAnimationChannel(clip, node.name);
        const glm::mat4 localTransform = SampleNodeTransform(node, channel, timeTicks);

        // 子节点继承父节点的变换
        const glm::mat4 globalTransform = parentTransform * localTransform;

        pose.globalTransforms[node.name] = globalTransform;

        for (const AnimationNode& child : node.children)
        {
            SampleNodeHierarchy(child, clip, timeTicks, globalTransform, pose);
        }
    }
}

glm::vec3 SamplePosition(const std::vector<PositionKey>& keys,
    double timeTicks, const glm::vec3& fallback)
{
    // 如果没有关键帧，保留原始位置
    if (keys.empty())
        return fallback;

    // 如果时间再第一帧之前，或者只有一个关键帧
    if (keys.size() == 1 || timeTicks <= keys.front().time)
    {
        //返回第一个关键帧
        return keys.front().value;
    }

    // 如果时间在最后一帧之后
    if (timeTicks >= keys.back().time)
    {
        return keys.back().value;
    }

    // 寻找当前时间所在的两个关键帧
    for (std::size_t i = 0; i + 1 < keys.size(); ++i)
    {
        const PositionKey& left = keys[i];
        const PositionKey& right = keys[i + 1];

        if (timeTicks < right.time)
        {
            const double interval = right.time - left.time;

            // 避免时间重复时除以0
            if (interval <= 0.0)
                return right.value;

            const float factor = static_cast<float>((timeTicks - left.time) / interval);

            return glm::mix(left.value, right.value, factor);
        }
    }

    return keys.back().value;
}

glm::quat SampleRotation(const std::vector<RotationKey>& keys,
    double timeTicks, const glm::quat& fallback)
{
    // 如果没有关键帧，保留原始位置
    if (keys.empty())
        return fallback;

    // 如果时间再第一帧之前，或者只有一个关键帧
    if (keys.size() == 1 || timeTicks <= keys.front().time)
    {
        //返回第一个关键帧
        return glm::normalize(keys.front().value);
    }

    // 如果时间在最后一帧之后
    if (timeTicks >= keys.back().time)
    {
        return glm::normalize(keys.back().value);
    }

    // 寻找当前时间所在的两个关键帧
    for (std::size_t i = 0; i + 1 < keys.size(); ++i)
    {
        const RotationKey& left = keys[i];
        const RotationKey& right = keys[i + 1];

        if (timeTicks < right.time)
        {
            const double interval = right.time - left.time;

            // 避免时间重复时除以0
            if (interval <= 0.0)
                return glm::normalize(right.value);

            const float factor = static_cast<float>((timeTicks - left.time) / interval);

            //两个旋转之间做球面线性插值
            return glm::normalize(glm::slerp(left.value, right.value, factor));
        }
    }

    return glm::normalize(keys.back().value);
}

glm::vec3 SampleScale(const std::vector<ScaleKey>& keys,
    double timeTicks, const glm::vec3& fallback)
{
    // 如果没有关键帧，保留原始位置
    if (keys.empty())
        return fallback;

    // 如果时间再第一帧之前，或者只有一个关键帧
    if (keys.size() == 1 || timeTicks <= keys.front().time)
    {
        //返回第一个关键帧
        return keys.front().value;
    }

    // 如果时间在最后一帧之后
    if (timeTicks >= keys.back().time)
    {
        return keys.back().value;
    }

    // 寻找当前时间所在的两个关键帧
    for (std::size_t i = 0; i + 1 < keys.size(); ++i)
    {
        const ScaleKey& left = keys[i];
        const ScaleKey& right = keys[i + 1];

        if (timeTicks < right.time)
        {
            const double interval = right.time - left.time;

            // 避免时间重复时除以0
            if (interval <= 0.0)
                return right.value;

            const float factor = static_cast<float>((timeTicks - left.time) / interval);

            return glm::mix(left.value, right.value, factor);
        }
    }

    return keys.back().value;
}

glm::mat4 SampleNodeTransform(const AnimationNode& node,
    const AnimationChannel* channel, double timeTicks)
{
    // 没有动画通道，保留原始矩阵
    if (channel == nullptr)
        return node.localTransform;

    if (channel->positions.empty() && channel->rotations.empty()
        && channel->scales.empty())
        return node.localTransform;

    const glm::vec3 position = SamplePosition(channel->positions, timeTicks, node.defaultPosition);
    const glm::quat rotation = SampleRotation(channel->rotations, timeTicks, node.defaultRotation);
    const glm::vec3 scale = SampleScale(channel->scales, timeTicks, node.defaultScale);

    const glm::mat4 translationMatrix = glm::translate(glm::mat4(1.0f), position);
    const glm::mat4 rotationMatrix = glm::mat4_cast(rotation); //将四元数转换为旋转矩阵
    const glm::mat4 scaleMatrix = glm::scale(glm::mat4(1.0f), scale);

    return translationMatrix * rotationMatrix * scaleMatrix;
}


void SampleAnimationPose(const AnimationNode& rootNode, const AnimationClip* clip,
    double timeTicks, AnimationPose& pose)
{
    pose.globalTransforms.clear();

    SampleNodeHierarchy(
        rootNode,
        clip,
        timeTicks,
        glm::mat4(1.0f),
        pose
    );
}