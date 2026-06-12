# Reactor

## 项目简介

这是一个使用 CMake 管理的 C++ 项目，实现了一个简单的 Reactor 模式示例（含客户端/服务端）。仓库包含用于构建和演示的源代码和辅助脚本。

## 依赖

- C++17 支持的编译器（g++, clang++ 等）
- CMake 3.10+
- Ninja 或 Make（可选）

## 构建（推荐）

在项目根目录下运行：

```bash
mkdir -p build
cd build
cmake ..
cmake --build . -- -j$(nproc)
```

如果使用 Ninja：

```bash
mkdir -p build
cd build
cmake -G Ninja ..
ninja
```

构建产物默认在 `build` 目录下，具体可根据 `CMakeLists.txt` 中的设置找到可执行文件。

## 运行

构建完成后，在 `build` 目录中查找生成的可执行文件（例如 `Server`、`Client` 或 `reactor` 等），然后直接运行：

```bash
# 示例（在 build 目录下运行）
./Server
./Client
```

具体可执行文件名称请参考 CMake 输出或 `CMakeLists.txt`。

## 主要文件

- [src/Reactor.cpp](src/Reactor.cpp) — Reactor 实现
- [src/Reactor.h](src/Reactor.h) — Reactor 头文件
- [src/Server.cpp](src/Server.cpp) — 服务端示例
- [src/Client.cpp](src/Client.cpp) — 客户端示例
- [src/ThreadPool.cpp](src/ThreadPool.cpp) — 线程池实现

（路径基于项目根目录）

## 项目结构（概要）

- `CMakeLists.txt` — 顶层构建脚本
- `src/` 
- `csrc/`— 源代码
