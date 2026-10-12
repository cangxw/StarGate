# StarGate

StarGate 是一个从零学习和实现的 C++ 游戏引擎项目，参考 [Piccolo](https://github.com/BoomingTech/Piccolo) 的设计逐步完善。当前使用 OpenGL 3.3，已经具备基础三维渲染、模型加载、场景编辑，以及场景保存与恢复功能。

当前编辑器包含顶部 File 菜单、Hierarchy 物体列表、Inspector 属性面板和独立 Scene 视口。可以选择物体、修改变换、保存场景，再通过菜单或重启程序恢复。

## 当前功能

- **构建与窗口：**使用 Visual Studio、CMake 和 C++20，当前面向 Windows；GLFW 创建窗口并在启动时最大化，通过 GLAD 加载 OpenGL 函数。
- **模型与材质：**通过 Assimp 导入静态 OBJ、FBX、glTF 模型，读取位置、UV、法线、三角形索引、子网格、材质颜色和外部纹理路径。默认场景使用一个含三种材质的测试立方体。
- **资源管理：**Shader、Mesh、Texture2D、Model、Framebuffer 管理对应资源的创建和释放。模型通过 unique_ptr 管理，成功加载新模型后再替换旧模型。
- **着色器：**顶点与片元着色器分别从 assets/shaders/lit.vert 和 lit.frag 读取，提供编译、链接错误输出和 uniform 设置接口。
- **渲染：**使用 VAO、VBO、EBO 和 glDrawElements 绘制索引网格，每个顶点包含位置、UV、法线，共 8 个浮点数。
- **纹理：**通过 stb_image 解码图片，上传为 RGBA8，生成 mipmap；图片读取失败时显示备用棋盘格。
- **光照：**实现方向光、常量环境光近似、Lambert 漫反射和 Blinn–Phong 高光；法线使用模型矩阵的逆转置变换到世界空间。
- **相机：**使用 GLM 和右手坐标系，支持 WASD 移动、鼠标右键转向；组合移动方向归一化，俯仰角限制为 ±89°。
- **Scene 视口：**场景先渲染到 Framebuffer 的颜色纹理，再通过 ImGui 显示；颜色纹理和深度/模板 Renderbuffer 随面板大小调整，投影宽高比使用场景附件尺寸。
- **编辑界面：**使用 Dear ImGui 的 docking 分支，支持面板停靠、拆分和标签页；字体与控件尺寸适配启动时的显示缩放比例。
- **场景编辑：**Hierarchy 选择物体，Inspector 编辑位置、旋转和缩放，并提供 Reset Transform；还可以调整光源颜色和环境光强度。
- **场景文件：**通过 json11 读写 JSON，保存共同的模型路径、各物体名称和变换；支持启动恢复及运行中重新加载。格式或变换检查失败时不覆盖已有场景数据。
- **帧率：**窗口标题每 0.5 秒更新平均 FPS 和帧耗时，Inspector 显示 ImGui 的帧率统计；禁用垂直同步，使用线程休眠手动限制到约 60 FPS。

## 构建与运行

需要 Windows、Visual Studio 的“使用 C++ 的桌面开发”工作负载及 CMake 工具，以及支持 OpenGL 3.3 Core 的显卡和驱动。

1. 在 Visual Studio 中通过“文件 → 打开 → 文件夹”打开项目根目录。
2. 等待 CMake 配置完成，选择 x64 Debug 配置和 StarGate 启动项。
3. 确认 assets 目录保留了着色器、默认模型及纹理。
4. 按 Ctrl + F5 运行。
5. 将 Hierarchy、Scene、Inspector 面板拖动停靠到需要的位置。

也可以在 Visual Studio 的 x64 开发者 PowerShell 中，从项目根目录构建：

```powershell
cmake --preset x64-debug
cmake --build out/build/x64-debug --target StarGate
.\out\build\x64-debug\StarGate.exe
```

现有预设使用 MSVC、Ninja 和 Visual Studio 的安装目录环境。资源路径由 CMake 的 STARGATE_ASSET_DIR 指向源码目录下的 assets，不依赖启动时的工作目录；移动项目后需要重新配置并构建。

## 编辑器操作

| 操作 | 效果 |
| --- | --- |
| Hierarchy 点击物体 | 选择要编辑的物体 |
| Inspector 的 Position / Rotation / Scale | 编辑选中物体的位置、旋转角度和缩放 |
| Reset Transform | 重置位置和旋转为 0、缩放为 1，保留名称 |
| Scene 画面内按住右键并移动鼠标 | 转动相机；松开后恢复光标 |
| W / S | 沿相机朝向前进 / 后退 |
| A / D | 向左 / 向右移动相机 |
| 左 / 右方向键 | 修改选中物体的 X 轴位置 |
| Q / E | 增加 / 减小选中物体的 Y 轴旋转角度 |
| Scene 画面内使用滚轮 | 等比例增减选中物体的缩放，最小为 0.1 |
| Inspector 的 Light Color / Ambient | 调整光源颜色和环境光强度 |
| 1 / 2 / 3 | 切换白光、暖色光、蓝色光 |
| File → Save Scene | 保存当前场景到 assets/scenes/default.json |
| File → Load Scene | 恢复保存的场景，下一帧开始时执行加载 |
| 拖动面板标题栏、标签页或分隔线 | 调整停靠布局及面板尺寸 |
| File → Exit、Esc 或关闭窗口 | 退出程序 |

ImGui 需要键盘输入时暂停上述键盘控制。相机移动跟随其当前朝向，包括俯仰后的竖直分量。相机初始位置为 (0, 0, 3)，朝世界 −Z 方向观察，世界上方向为 +Y；透视投影垂直视野角为 45°，近、远裁剪面为 0.1 和 100。

Inspector 缩放控件的范围为 0.1～10；滚轮缩放目前只限制下限。默认指向光源的方向为 (0, 0, 1)，环境光强度为 0.2，高光强度为 1，shininess 为 128。

## 场景保存与恢复

场景文件版本为 1，结构示例：

```json
{
  "version": 1,
  "modelPath": "models/material_test/cube.obj",
  "objects": [
    {
      "name": "Cube",
      "position": [0, 0, 0],
      "rotation": [20, 0, 0],
      "scale": [1, 1, 1]
    }
  ]
}
```

modelPath 相对于 assets，不能以斜杠或盘符开头；rotation 使用角度。json11 当前写出紧凑 JSON 文本，上面的缩进仅用于说明格式。

- 启动时先准备默认场景；文件存在时尝试读取，JSON 或数据校验失败时保留默认数据。
- 保存写入所有物体，不只是当前选中的物体；已有文件会被覆盖。
- 运行中加载先解析临时场景并创建新模型，全部成功后再替换当前状态；失败时保留原场景和模型。
- 加载完成后选中第一个物体，并重置帧时间基准，避免文件加载耗时造成下一次移动突然跳跃。
- 检查 JSON 版本、字段类型、三维向量长度、数值范围和正数缩放。当前要求场景至少有一个物体。

可以用“修改物体 → 保存 → 再次修改 → 加载恢复 → 重启恢复”验证完整流程。

## 模块结构

| 模块 | 职责 |
| --- | --- |
| StarGate.cpp | 应用初始化、主循环、输入、编辑器面板和场景切换 |
| Shader | 读取 GLSL 文件，编译、链接、设置 uniform 和释放程序 |
| Camera | 相机位置、朝向、移动与视图矩阵 |
| Mesh | VAO/VBO/EBO 管理和子网格索引范围绘制 |
| Texture2D | 图片解码、备用纹理、上传、采样参数与释放 |
| ModelLoader | 用 Assimp 将模型转换为 CPU 顶点、索引、子网格和材质数据 |
| Model / MaterialData | 模型网格与材质纹理管理，按子网格绘制 |
| Framebuffer | 离屏颜色纹理、深度/模板附件和尺寸调整 |
| SceneObject / SceneData | 物体名称与变换，以及场景模型路径和物体列表 |
| SceneSerializer | 场景 JSON 保存、读取和数据校验 |
| StbImage.cpp | stb_image 实现的编译入口 |

当前绘制流程：

```text
SceneData 中的物体变换 + Model 中的网格与材质
  → Shader 与 OpenGL 索引绘制
  → Framebuffer 的颜色纹理
  → ImGui Scene 图像
  → 窗口默认 Framebuffer
  → GLFW 交换缓冲区
```

默认测试资源位于 assets/models/material_test：cube.obj 通过 cube.mtl 指定红色、木箱纹理和蓝色三种材质。木箱图片位于 assets/textures/crate.png，是为学习演示生成的 PNG。

## 当前范围与后续方向

当前版本的核心流程已经接入：加载静态模型、选择物体、编辑变换、保存场景、重新加载和启动恢复。

目前所有物体共用一个模型资源，场景文件只有一个 modelPath；保存内容不包含相机、光照和编辑器布局。ImGui 的面板布局由它自己的配置机制保存，与场景 JSON 分开管理。

模型导入使用静态预变换流程，不保留可编辑节点层级或骨骼动画；只加载外部纹理，嵌入纹理尚未接入。当前光照属于基础 Blinn–Phong 演示，没有阴影、sRGB/Gamma 流程或完整 PBR。场景路径固定，尚无文件选择对话框、物体新增/删除、撤销/重做及完整资源浏览器。

手动限帧不保证严格达到 60 FPS，窗口标题中的平均帧耗时包含等待时间，不能当作 GPU 绘制耗时。

接下来完成运行验收、演示录制和项目整理；后续逐步加入基础性能面板、更完整的场景与资源管理、物体操作工具。完整复刻 Piccolo 是长期方向，Vulkan 后端、物理、AI 集成与 VR 研究仍属于后续工作。

## 第三方依赖

依赖以源码或头文件形式保存在 third_party，构建时无需额外下载，各依赖的许可说明保留在对应源码中。

- [GLFW](https://www.glfw.org/)：窗口、输入与 OpenGL 上下文。
- [GLAD](https://github.com/Dav1dde/glad)：OpenGL 函数加载代码，当前使用 OpenGL 3.3 Core。
- [GLM](https://github.com/g-truc/glm)：向量、矩阵和变换计算。
- [stb_image](https://github.com/nothings/stb)：图片解码。
- [Assimp](https://github.com/assimp/assimp)：静态模型导入，当前构建启用 OBJ、FBX、glTF 导入器。
- [Dear ImGui](https://github.com/ocornut/imgui/tree/docking)：编辑界面与停靠布局，使用 GLFW/OpenGL3 后端。
- json11：JSON 解析与序列化，来自参考工程中的第三方库。
