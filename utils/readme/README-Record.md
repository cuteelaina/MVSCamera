# record

实时预览并录制视频文件（MJPG）。

## 构建

```bash
cmake --build build --target record
```

## 运行

```bash
./build/record [fps]
```

- `fps`：可选，默认 $125$。

## 交互

- `Gain x0.1` 与 `Exposure` 轨迹条：调节增益与曝光。
- `s`：开始/停止录制。
- `q` / `ESC`：退出。

## 输出

- `RecordResult/<serial>/record_###.avi`
