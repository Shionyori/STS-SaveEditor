#include <iostream>
#include <algorithm>
#include "../codec.h"

// 打印程序使用说明
void print_usage() {
    std::cout << "STSSaveEditor - simple encoder/decoder for Slay The Spire autosave format\n";
    std::cout << "[ Help Information ]:\n";
    std::cout << "STSSaveEditor.exe encode <infile|- for stdin> [outfile|- for stdout]\n";
    std::cout << "STSSaveEditor.exe decode <infile|- for stdin> [outfile|- for stdout]\n";
}

int main(int argc, char** argv) {
    
    if (argc < 3) { print_usage(); return 1; }
    std::string cmd = argv[1]; // 参数1：encode 或 decode
    std::string infile = argv[2]; // 参数2：输入文件路径或 '-' 表示从 stdin 读取
    std::string outfile = (argc >= 4) ? argv[3] : std::string("-"); // 参数3：输出文件或 '-' 表示写入 stdout

    const std::string key = "key"; // XOR 混淆的循环密钥

    // 读取输入内容（支持文件或 stdin）
    std::string input = read_file(infile);
    if (input.empty() && infile != "-") {
        std::cerr << "Failed to read input file: " << infile << "\n";
        return 2;
    }

    // 去除首尾空白，避免 base64 解码时被换行或空格干扰
    input = trim(input);

    // 若输出文件未指定，则根据命令和输入文件名自动生成
    if (outfile == "-" && infile != "-") {
        std::string lower = infile;
        std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
        if (cmd == "decode") {
            size_t pos = lower.rfind(".autosave");
            if (pos != std::string::npos) { 
                outfile = infile.substr(0, pos) + ".json";
            }
            else {
                std::cout << "Failed: Input file does not have .autosave extension for decoding.\n";
                print_usage();
                return 1;
            }
        } else if (cmd == "encode") {
            size_t pos = lower.rfind(".json");
            if (pos != std::string::npos) {
                outfile = infile.substr(0, pos) + ".autosave";
            } else {
                std::cout << "Failed: Input file does not have .json extension for encoding.\n";
                print_usage();
                return 1;
            }
        }
    }

    if (cmd == "decode") {
        std::string bdec = base64_decode(input);
        std::string dec = xor_transform(bdec, key);
        if (!write_file(outfile, dec)) {
            std::cerr << "Failed to write output file: " << outfile << "\n";
            return 3;
        }
        return 0;
    } else if (cmd == "encode") {
        std::string xored = xor_transform(input, key);
        std::string encoded = base64_encode(xored);
        if (!write_file(outfile, encoded)) {
            std::cerr << "Failed to write output file: " << outfile << "\n";
            return 4;
        }
        return 0;
    } else {
        print_usage();
        return 1;
    }
}
