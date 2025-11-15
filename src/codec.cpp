#include "codec.h"
#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>
#include <algorithm>
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

// 高级封装的编码函数
std::string encode_autosave(const std::string &json_content, const std::string &key) {
    std::string xored = xor_transform(json_content, key);
    return base64_encode(xored);
}

// 高级封装的解码函数
std::string decode_autosave(const std::string &autosave_content, const std::string &key) {
    std::string bdec = base64_decode(autosave_content);
    return xor_transform(bdec, key);
}