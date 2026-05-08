# capture

实时预览并手动抓图保存 PNG。

## 构建

```bash
cmake --build build --target capture
```

## 运行

```bash
./build/capture
```

## 交互

- `Gain x0.1` 与 `Exposure` 轨迹条：调节增益与曝光。
- `s`：保存当前帧到 `CaptureResult/<serial>/capture_###.png`。
- `q` / `ESC`：退出。

## 输出

- `CaptureResult/<serial>/capture_###.png`
