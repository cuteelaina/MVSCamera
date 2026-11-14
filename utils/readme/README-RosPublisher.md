# ros_publisher

发布相机图像到 ROS 话题（mono8），为下游工具提供图像数据源。

## 设计目的

- 主要是**与其他工具衔接**：把海康 GigE 相机图像发布成 ROS `sensor_msgs/Image`（`mono8`）到 `/camera/image_raw` 话题。
- 初始设计目标是为来自 27-DRONE 的移植版 ChArUco 标定程序服务，即：
  - `preview_window.py`：订阅话题实时预览；
  - `capture_charuco_ros.py`：订阅话题采集标定照片；
  - `calibrate_charuco.py`：离线求解相机内参。

## 构建

```bash
cmake --build build --target ros_publisher
```

## 运行

```bash
source /opt/ros/noetic/setup.bash
./build/ros_publisher
```

## 交互

- 启动时列出所有相机（索引 + 序列号 + 型号）。
- 输入相机索引选择要打开的相机。
- 输入发布话题名（回车使用默认 `/camera/image_raw`）。
- `Ctrl+C`：退出。
