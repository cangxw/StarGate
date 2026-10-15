# CMakeList.txt: StarGate 的 CMake 项目，在此处包括源代码并定义
# 项目特定的逻辑。
#
cmake_minimum_required (VERSION 3.22)

# 如果支持，请为 MSVC 编译器启用热重载。
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project ("StarGate")

# 只构建GLFW库，关闭它附带的示例，测试和文档
set(GLFW_BUILD_EXAMPLES OFF)
set(GLFW_BUILD_TESTS OFF)
set(GLFW_BUILD_DOCS OFF)
set(GLFW_INSTALL OFF)

# 加入 GLFW 的构建配置。
add_subdirectory(third_party/glfw)

# 使用静态库
set(BUILD_SHARED_LIBS OFF)

# 关闭当前不需要的工具，示例，测试和导出功能
set(ASSIMP_BUILD_ASSIMP_TOOLS OFF)
set(ASSIMP_BUILD_ASSIMP_VIEW OFF)
set(ASSIMP_BUILD_SAMPLES OFF)
set(ASSIMP_BUILD_TESTS OFF)
set(ASSIMP_INSTALL OFF)
set(ASSIMP_NO_EXPORT ON)
set(ASSIMP_WARNINGS_AS_ERRORS OFF)

# 使用 Assimp 自带的 zlib
set(ASSIMP_BUILD_ZLIB ON)

# 先只启用 OBJ、FBX、glTF 模型导入
set(ASSIMP_BUILD_ALL_IMPORTERS_BY_DEFAULT OFF)
set(ASSIMP_BUILD_OBJ_IMPORTER ON)
set(ASSIMP_BUILD_FBX_IMPORTER ON)
set(ASSIMP_BUILD_GLTF_IMPORTER ON)

add_subdirectory(third_party/assimp)

# 将 GLAD 编译成静态库
add_library(glad STATIC third_party/glad/src/gl.c)

# 提供头文件搜索目录
target_include_directories(glad PUBLIC third_party/glad/include)

# Dear ImGui 核心与 GLFW/OpenGL 后端
add_library(imgui STATIC
    third_party/imgui/imgui.cpp
    third_party/imgui/imgui_draw.cpp
    third_party/imgui/imgui_tables.cpp
    third_party/imgui/imgui_widgets.cpp
    third_party/imgui/imgui_demo.cpp
    third_party/imgui/backends/imgui_impl_glfw.cpp
    third_party/imgui/backends/imgui_impl_opengl3.cpp
)

target_include_directories(imgui PUBLIC
    third_party/imgui
    third_party/imgui/backends
)

target_link_libraries(imgui PUBLIC glfw)

# JSON 解析与序列化
add_library(json11 STATIC
    third_party/json11/json11.cpp
)

target_include_directories(json11 PUBLIC
    third_party/json11
)

# 将源代码添加到此项目的可执行文件。
add_executable (StarGate 
	"src/StarGate.cpp"
 	"src/Engine/Shader.h"
	"src/Engine/Shader.cpp"
	"src/Engine/Camera.h"
	"src/Engine/Camera.cpp"
	"src/Engine/StbImage.cpp"
	"src/Engine/Mesh.h"
	"src/Engine/Mesh.cpp"
	"src/Engine/Texture2D.h"
	"src/Engine/Texture2D.cpp"
	"src/Engine/SceneObject.h"
	"src/Engine/ModelLoader.h"
	"src/Engine/ModelLoader.cpp"
	"src/Engine/MaterialData.h" 
	"src/Engine/Model.h" 
	"src/Engine/Model.cpp" 
	"src/Engine/Framebuffer.h"
	"src/Engine/Framebuffer.cpp"
	"src/Engine/SceneSerializer.h"
	"src/Engine/SceneSerializer.cpp"
	"src/Engine/SceneData.h"
	"src/Engine/PathUtils.h"
	"src/Engine/AnimationLoader.h"
	"src/Engine/AnimationLoader.cpp"
	"src/Engine/AnimationData.h"
	"src/Editor/EditorUI.h"
	"src/Editor/EditorUI.cpp"
	"src/Editor/EditorInput.h"
	"src/Editor/EditorInput.cpp"
)

# 资源文件目录，这样读取图片不依赖vs的目录
target_compile_definitions(StarGate PRIVATE
	STARGATE_ASSET_DIR="${CMAKE_CURRENT_SOURCE_DIR}/assets"
)

# 再给这个目标设置头文件搜索目录
target_include_directories(StarGate PRIVATE 
	"${CMAKE_CURRENT_SOURCE_DIR}/src"
	third_party/glm
	third_party/stb
)

# 让 StarGate 使用 GLFW。
target_link_libraries(StarGate PRIVATE 
	glfw 
	glad 
	imgui
	winmm 
	opengl32
	assimp::assimp
	json11)

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET StarGate PROPERTY CXX_STANDARD 20)
endif()

# TODO: 如有需要，请添加测试并安装目标。
