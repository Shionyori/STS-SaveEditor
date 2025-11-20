# 介绍

用于对 Slay The Spire 的 `.autosave` 文件进行解码和加密的简单程序，涉及Base64的编码/解码和XOR变换加密。

使用的工具链为 CMake + MinGW + Ninja，使用 `cmake --build build --config [Debug/Release]` 进行编译。

## 命令行

解码 `.autosave` 到 `.json`：

```powershell
STSSaveEditor.exe encode <infile|- for stdin> [outfile|- for stdout]
```

从 `.json` 编回 `.autosave`：

```powershell
STSSaveEditor.exe decode <infile|- for stdin> [outfile|- for stdout]
```

## Qt 图形界面
运行 `STSSaveEditor-GUI.exe`，在界面中选择输入文件和输出文件，点击“编码”或“解码”按钮进行操作；

但是基于 Qt 实现的 GUI 程序暂时有问题，在点击按钮后一段时间内程序会奔溃，亟待解决。

## Web 界面
通过 HTML + JS + CSS 重写的静态网页可以正常运行，打开 web/index.html 即可使用；

可以自定义XOR密钥，但默认值为 "key"；

允许文件拖放和剪贴板操作，推荐使用现代浏览器（如 Chrome、Edge、Firefox 等）打开以获得最佳体验。