# StarGate

StarGate 是一个从零学习和实现的 C++ 游戏引擎项目，参考 [Piccolo](https://github.com/BoomingTech/Piccolo) 的设计逐步完善。

## 当前进度

- 使用 Visual Studio、CMake 和 C++20 构建。
- 已接入 GLFW，创建 1280 × 720 的窗口。
- 支持窗口事件处理、Esc 退出和基础错误输出。
- 当前只创建窗口，尚未实现图形绘制。

## 构建与运行

1. 在 Visual Studio Installer 中安装“使用 C++ 的桌面开发”，并确认包含“用于 Windows 的 C++ CMake 工具”。
2. 克隆仓库后，在 Visual Studio 中通过“文件 → 打开 → 文件夹”打开包含 `CMakeLists.txt` 的根目录。
3. 等待 CMake 配置完成，选择 `x64 Debug` 配置和 `StarGate` 启动项。
4. 按 `Ctrl + F5` 运行，按 `Esc` 或点击窗口关闭按钮退出。

GLFW 源码已包含在 `third_party/glfw` 中，无需额外下载。

## 目录结构

```text
StarGate/
├── CMakeLists.txt       # 构建目标和依赖
├── CMakePresets.json    # Visual Studio 构建配置
├── StarGate.cpp         # 程序入口和窗口循环
├── StarGate.h           # 项目头文件
└── third_party/glfw/    # GLFW 源码
```

## 后续计划

逐步实现帧时间与输入管理、基础渲染、摄像机、场景和资源管理。计划支持 OpenGL 与 Vulkan 两套渲染后端，并在启动时选择；目前尚未实现。

## 第三方依赖

[GLFW](https://www.glfw.org/) 用于窗口与输入处理，其许可说明保留在 [third_party/glfw/LICENSE.md](third_party/glfw/LICENSE.md) 中。
