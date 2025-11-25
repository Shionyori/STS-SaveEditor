# STS Save Editor

一个基于 **Rust + Tauri** 构建的 Slay The Spire 存档编辑器。

它提供了一个美观的图形界面，用于对 `.autosave` 文件进行解码（解密）和编码（加密）。核心算法（Base64 + XOR）已使用 Rust 重写，以提供更高的性能和安全性。

## 功能特性

- **高性能核心**：使用 Rust 实现的 Base64 编解码与 XOR 变换。
- **跨平台 GUI**：基于 Tauri 构建，支持 Windows, macOS 和 Linux。
- **实时编辑**：内置 JSON 编辑器，支持语法高亮。
- **兼容性**：完全兼容游戏原本的存档格式。

## 开发环境准备

在开始之前，请确保已安装以下环境：

1.  **Rust**: [安装 Rust](https://www.rust-lang.org/tools/install)
2.  **Tauri CLI**:
    ```bash
    cargo install tauri-cli --version "^2.0.0"
    ```
    *(注意：本项目使用 Tauri v2)*
3.  **Node.js** (可选): 如果你需要修改前端并使用 npm 包管理。

## 运行与构建

### 开发模式

启动开发服务器，支持热重载：

```bash
cargo tauri dev
```

### 构建发布版本

构建用于分发的应用程序（exe/msi/dmg/deb 等）：

```bash
cargo tauri build
```

构建产物将位于 `src-tauri/target/release/bundle/` 目录下。

## 项目结构

- `src-tauri/`: Rust 后端代码，处理文件加解密和系统交互。
- `web/`: 前端代码 (HTML/JS/CSS)，提供用户界面。
- `src/`: (旧版) C++ CLI 工具源码。

## 旧版 C++ CLI 工具

如果你仍然需要使用命令行的 C++ 版本：

使用 CMake + MinGW + Ninja 编译：
```bash
cmake --build build --config Release
```

用法：
```powershell
# 解码
STSSaveEditor.exe encode <infile> [outfile]
# 编码
STSSaveEditor.exe decode <infile> [outfile]
```