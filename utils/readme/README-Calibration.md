# calibration

使用棋盘格进行相机标定并保存内参与畸变系数。

## 构建

```bash
cmake --build build --target calibration
```

## 运行（在线采集）

```bash
./build/calibration
```

### 交互

- `Gain x0.1` 与 `Exposure` 轨迹条：调节增益与曝光。
- 当检测到棋盘格时，按 `s` 采集一帧。
- 建议采集至少 10 张后退出程序自动计算。
- `q` / `ESC`：退出。

## 运行（使用已采集图片）

```bash
./build/calibration captured
```

该模式会读取 `CaptureResult/<serial>/*.png` 并进行标定。

## 输出

- `CalibrationResult/<serial>.yaml`
- 控制台打印 RMS 与重投影误差。

## 棋盘格参数

- 内角点：$11 \times 8$
- 格子尺寸：$25\,\text{mm}$
