#include "SceneSerializer.h"

#include <json11.hpp>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <system_error>

#include <sstream>
#include <cmath>
#include <limits>
#include <utility>

//使用匿名命名空间，是为了让两个辅助函数只供当前 SceneSerializer.cpp 使用
namespace
{
    // 将一个三维向量转换为 JSON 数组
    json11::Json Vec3ToJson(const glm::vec3& value)
    {
        return json11::Json::array{
            value.x,
            value.y,
            value.z
        };
    }

    bool JsonToVec3(const json11::Json& json, glm::vec3& result)
    {
        if (!json.is_array() || json.array_items().size() != 3)
        {
            return false;
        }

        const auto& values = json.array_items();
        const double maxFloat = std::numeric_limits<float>::max();

        for (int i = 0; i < 3; ++i)
        {
            if (!values[i].is_number())
            {
                return false;
            }

            const double value = values[i].number_value();

            // 确认能够转换为有效的 float
            if (!std::isfinite(value) ||
                value < -maxFloat ||
                value > maxFloat)
            {
                return false;
            }

            result[i] = static_cast<float>(value);
        }

        return true;
    }
}


bool SaveScene(const char* filePath,const char* modelPath,const SceneObject* objects,std::size_t objectCount)
{
    if (!filePath || !modelPath || !objects || objectCount == 0)
    {
        std::cerr << "Invalid scene data\n";
        return false;
    }

    //收集所有物体的数据
    json11::Json::array objectArray;

    for (std::size_t i = 0; i < objectCount; ++i)
    {
        const SceneObject& object = objects[i];

        objectArray.push_back(
            json11::Json::object
            {
                {"name", object.name},
                { "position", Vec3ToJson(object.position) },
                { "rotation", Vec3ToJson(object.rotation) },
                { "scale", Vec3ToJson(object.scale) },
            }
        );
    }

    // 构建整个场景的数据
    const json11::Json sceneJson = json11::Json::object{
        { "version", 1 },
        { "modelPath", modelPath },
        { "objects", objectArray }
    };

    const std::filesystem::path path(filePath);
    // 如果目标文件夹不存在，创建它
    if (!path.parent_path().empty())
    {
        std::error_code error;

        std::filesystem::create_directories(path.parent_path(), error);

        if (error)
        {
            std::cerr << "Failed to create scene directory: "<< error.message() << '\n';
            return false;
        }
    }

    // 打开文件；已有文件会被覆盖
    std::ofstream file(path, std::ios::binary | std::ios::trunc);
    if (!file)
    {
        std::cerr << "Failed to open scene file: "<< filePath << '\n';
        return false;
    }

    //JSON写入文件
    file << sceneJson.dump() << '\n';
    file.close();

    if (file.fail())
    {
        std::cerr << "Failed to write scene file: "<< filePath << '\n';
        return false;
    }

    return true;
}

bool LoadScene(const char* filePath, SceneData& scene)
{
    if (!filePath)
    {
        std::cerr << "Invalid scene file path\n";
        return false;
    }

    std::ifstream file(
        std::filesystem::path(filePath),
        std::ios::binary
    );

    if (!file)
    {
        std::cerr << "Failed to open scene file: "
            << filePath << '\n';
        return false;
    }

    // 将文件内容读取到字符串流中
    std::ostringstream buffer;
    buffer << file.rdbuf();

    if (file.bad() || buffer.bad())
    {
        std::cerr << "Failed to read scene file\n";
        return false;
    }

    // 将文本解析为 JSON
    std::string error;
    const json11::Json root = json11::Json::parse(buffer.str(), error);

    if (!error.empty())
    {
        std::cerr << "Failed to parse scene: "
            << error << '\n';
        return false;
    }

    // 检查场景文件的基本结构
    if (!root.is_object() ||
        !root["version"].is_number() ||
        root["version"].number_value() != 1.0 ||
        !root["modelPath"].is_string() ||
        !root["objects"].is_array() ||
        root["objects"].array_items().empty())
    {
        std::cerr << "Invalid scene format\n";
        return false;
    }

    // 先读取到临时数据，全部成功后再修改输出
    SceneData result;
    result.modelPath = root["modelPath"].string_value();

    if (result.modelPath.empty() || std::filesystem::path(result.modelPath).has_root_path())
    {
        std::cerr << "Model path must be relative to assets\n";
        return false;
    }

    for (const auto& objectJson : root["objects"].array_items())
    {
        if (!objectJson.is_object() || !objectJson["name"].is_string())
        {
            std::cerr << "Invalid scene object\n";
            return false;
        }

        SceneObject object;
        object.name = objectJson["name"].string_value();

        if (!JsonToVec3(objectJson["position"], object.position) ||
            !JsonToVec3(objectJson["rotation"], object.rotation) ||
            !JsonToVec3(objectJson["scale"], object.scale))
        {
            std::cerr << "Invalid object transform: "
                << object.name << '\n';
            return false;
        }

        // 当前编辑器使用正数缩放
        if (object.scale.x <= 0.0f ||
            object.scale.y <= 0.0f ||
            object.scale.z <= 0.0f)
        {
            std::cerr << "Invalid object scale: "
                << object.name << '\n';
            return false;
        }

        //std::move减少不必要的复制操作，直接新对象接管所有指针
        result.objects.push_back(std::move(object));
    }

    scene = std::move(result);
    return true;
}