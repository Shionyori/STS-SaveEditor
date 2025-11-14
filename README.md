用于对 Slay The Spire 的 `.autosave` 文件进行解码和加密的简单程序，涉及Base64的编码/解码和XOR变换加密。

使用的工具链为 CMake + MinGW + Ninja，使用 `cmake --build .` 进行构建。

解码 `.autosave` 到 `.json`：

```powershell
STSSaveEditor.exe encode <infile|- for stdin> [outfile|- for stdout]
```

从 `.json` 编回 `.autosave`：

```powershell
STSSaveEditor.exe decode <infile|- for stdin> [outfile|- for stdout]
```