# MVSCamera

基于海康 MVS SDK 的 C++ 相机封装与实用工具集合，提供设备枚举、实时预览、抓图、标定、去畸变与录制等能力。

很早的雷达云台是依赖这个库的，checkout时务必注意

## 目录结构

- `include/`：对外头文件（`MVSCamera.h`）。
- `src/`：核心库实现。
- `utils/`：可执行工具示例。
- `install/`：`cmake --install` 输出与 `setup_env.sh`。

## 依赖

- OpenCV（CMake 会自动查找）。
- 海康 MVS SDK（默认使用 `/opt/MVS`，包含 `MvCameraControl` 头文件与库）。
- 已连接的工业相机设备。

如 SDK 安装路径不同，请修改 `CMakeLists.txt` 中的 `/opt/MVS/include` 与 `/opt/MVS/lib/64`。

## 构建

```bash
cmake -S . -B build
cmake --build build -j
```

## 安装（可选）

```bash
cmake --install build
source install/setup_env.sh
```

## 工具列表

- `utils/readme/README-ListDevice.md`：列出当前设备。
- `utils/readme/README-TestCamera.md`：打开相机预览并显示 FPS。
- `utils/readme/README-Capture.md`：预览并手动抓图保存。
- `utils/readme/README-Calibration.md`：棋盘格标定与参数保存。
- `utils/readme/README-Undistort.md`：加载标定结果进行去畸变显示。
- `utils/readme/README-Record.md`：录制视频文件。
- `utils/readme/README-RosPublisher.md`：发布相机图像到 ROS 话题（mono8），供下游标定/预览工具订阅。
- `preview_window.py`：订阅话题实时预览；
- `capture_charuco_ros.py`：订阅话题采集标定照片；
- `calibrate_charuco.py`：离线求解相机内参。

## 运行说明（通用）

- 可执行文件默认位于 `build/` 目录（如 `./build/list_devices`）。
- 交互退出通常使用 `q` 或 `ESC`。
- 结果输出目录可能包含：`CaptureResult/`、`CalibrationResult/`、`RecordResult/`。
