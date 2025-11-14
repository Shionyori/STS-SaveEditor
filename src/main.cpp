#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>
#include <iterator>
#include <cctype>

// Base64 字符表
static const std::string b64_chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

// Base64 编码函数
std::string base64_encode(const std::string &in) {
    std::string out;
    int val=0, valb=-6;
    for (char ch : in) {
        unsigned char c = static_cast<unsigned char>(ch);
        val = (val<<8) + c;
        valb += 8;
        while (valb>=0) {
            out.push_back(b64_chars[(val>>valb)&0x3F]);
            valb -= 6;
        }
    }
    if (valb>-6) out.push_back(b64_chars[((val<<8)>>(valb+8))&0x3F]);
    while (out.size()%4) out.push_back('=');
    return out;
}

// Base64 解码函数
std::string base64_decode(const std::string &in) {
    std::string s;
    s.reserve(in.size());
    for (char ch : in) {
        unsigned char c = static_cast<unsigned char>(ch);
        if (std::isalnum(c) || c == '+' || c == '/' || c == '=') s.push_back(static_cast<char>(c));
    }
    std::vector<int> T(256, -1);
    for (size_t i = 0; i < 64; ++i) T[static_cast<unsigned char>(b64_chars[i])] = static_cast<int>(i);
    std::string out;
    int val = 0;
    int valb = -8;
    for (char ch : s) {
        unsigned char c = static_cast<unsigned char>(ch);
        if (c == '=') {
            // padding: stop processing further input to avoid inserting zero bytes
            break;
        }
        int d = T[c];
        if (d == -1) continue; // skip any invalid characters
        val = (val << 6) + d;
        valb += 6;
        if (valb >= 0) {
            out.push_back(char((val >> valb) & 0xFF));
            valb -= 8;
        }
    }
    return out;
}

// XOR 变换函数（用于加密/解密）
std::string xor_transform(const std::string &data, const std::string &key) {
    std::string out = data;
    for (size_t i = 0; i < out.size(); ++i) {
        out[i] = out[i] ^ key[i % key.size()];
    }
    return out;
}

// 读取文件内容
std::string read_file(const std::string &path) {
    if (path == "-") {
        std::ostringstream ss; 
        ss << std::cin.rdbuf(); 
        return ss.str();
    }
    std::ifstream ifs(path, std::ios::binary);
    if (!ifs) return std::string();
    std::ostringstream ss; 
    ss << ifs.rdbuf(); 
    return ss.str();
}

// 写入内容到文件
bool write_file(const std::string &path, const std::string &content) {
    if (path == "-") {
        std::cout << content;
        return true;
    }
    std::ofstream ofs(path, std::ios::binary);
    if (!ofs) return false;
    ofs << content;
    return true;
}

// 去除字符串首尾的空白字符（空格、制表、回车、换行）
std::string trim(const std::string &s) {
    size_t a = s.find_first_not_of(" \t\r\n");
    if (a==std::string::npos) return "";
    size_t b = s.find_last_not_of(" \t\r\n");
    return s.substr(a, b-a+1);
}

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
