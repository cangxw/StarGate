# CMakeList.txt: StarGate 的 CMake 项目，在此处包括源代码并定义
# 项目特定的逻辑。
#
cmake_minimum_required (VERSION 3.10)

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

# 将 GLAD 编译成静态库
add_library(glad STATIC third_party/glad/src/gl.c)

# 提供头文件搜索目录
target_include_directories(glad PUBLIC third_party/glad/include)

# 将源代码添加到此项目的可执行文件。
add_executable (StarGate 
	"src/StarGate.cpp"
 	"src/Shader.h"
	"src/Shader.cpp"
	"src/Camera.h"
	"src/Camera.cpp"
	"src/StbImage.cpp"
	"src/Mesh.h"
	"src/Mesh.cpp"
)

# 资源文件目录，这样读取图片不依赖vs的目录
target_compile_definitions(StarGate PRIVATE
	STARGATE_ASSET_DIR="${CMAKE_CURRENT_SOURCE_DIR}/assets"
)

# 再给这个目标设置头文件搜索目录
target_include_directories(StarGate PRIVATE 
	third_party/glm
	third_party/stb
)

# 让 StarGate 使用 GLFW。
target_link_libraries(StarGate PRIVATE glfw glad winmm opengl32)

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET StarGate PROPERTY CXX_STANDARD 20)
endif()

# TODO: 如有需要，请添加测试并安装目标。
