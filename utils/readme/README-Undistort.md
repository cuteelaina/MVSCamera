# undistort

加载标定结果并实时显示去畸变效果。

## 构建

```bash
cmake --build build --target undistort
```

## 运行

```bash
./build/undistort
```

## 前置条件

- 需要存在 `CalibrationResult/<serial>.yaml` 标定文件。

## 交互

- `s`：切换畸变/去畸变显示。
- `q` / `ESC`：退出。
