#include "codec.h"
#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <cctype>
#include <array>
#include <stdexcept>

// Robust, well-tested Base64 table
static constexpr char B64_CHARS[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "abcdefghijklmnopqrstuvwxyz"
    "0123456789+/";

// base64_encode: encode arbitrary binary data represented as std::string
std::string base64_encode(const std::string &in) {
    const unsigned char *data = reinterpret_cast<const unsigned char*>(in.data());
    size_t len = in.size();
    std::string out;
    out.reserve(((len + 2) / 3) * 4);

    size_t i = 0;
    while (i + 2 < len) {
        unsigned int triple = (data[i] << 16) | (data[i+1] << 8) | data[i+2];
        out.push_back(B64_CHARS[(triple >> 18) & 0x3F]);
        out.push_back(B64_CHARS[(triple >> 12) & 0x3F]);
        out.push_back(B64_CHARS[(triple >> 6) & 0x3F]);
        out.push_back(B64_CHARS[triple & 0x3F]);
        i += 3;
    }

    if (i < len) {
        unsigned int triple = 0;
        int pad = 0;
        triple |= (data[i] << 16);
        if (i + 1 < len) {
            triple |= (data[i+1] << 8);
        } else {
            pad = 1; // one '=' padding
        }

        out.push_back(B64_CHARS[(triple >> 18) & 0x3F]);
        out.push_back(B64_CHARS[(triple >> 12) & 0x3F]);
        if (pad == 1) {
            out.push_back(B64_CHARS[(triple >> 6) & 0x3F]);
            out.push_back('=');
        } else {
            out.push_back(B64_CHARS[(triple >> 6) & 0x3F]);
            out.push_back(B64_CHARS[triple & 0x3F]);
        }

        if (i + 1 >= len) { // only one input byte -> two '='
            out[out.size()-2] = '=';
        }
    }

    // Ensure length multiple of 4 (should already be)
    while (out.size() % 4) out.push_back('=');

    return out;
}

// base64_decode: tolerate whitespace and ignore non-base64 characters, stop at padding
std::string base64_decode(const std::string &in) {
    // build reverse table once (static local with constexpr-like init)
    static std::array<int, 256> rev = []{
        std::array<int,256> a;
        a.fill(-1);
        for (size_t i = 0; i < sizeof(B64_CHARS)-1; ++i) {
            a[static_cast<unsigned char>(B64_CHARS[i])] = static_cast<int>(i);
        }
        a[static_cast<unsigned char>('=')] = 0;
        return a;
    }();

    // collect only meaningful base64 chars (ignore whitespace and other rubbish)
    std::string filtered;
    filtered.reserve(in.size());
    for (unsigned char ch : in) {
        if (rev[ch] != -1 || ch == '=') filtered.push_back(static_cast<char>(ch));
    }

    std::string out;
    out.reserve((filtered.size() * 3) / 4);

    size_t i = 0;
    unsigned int val = 0;
    int valb = -8;
    for (unsigned char ch : filtered) {
        if (ch == '=') break; // stop at padding - remaining characters (if any) ignored
        int d = rev[ch];
        if (d == -1) continue;
        val = (val << 6) | static_cast<unsigned int>(d);
        valb += 6;
        if (valb >= 0) {
            unsigned char byte = static_cast<unsigned char>((val >> valb) & 0xFF);
            out.push_back(static_cast<char>(byte));
            valb -= 8;
        }
    }

    return out;
}

// XOR transform: defensive and well-defined for arbitrary bytes.
// Throws std::invalid_argument if key is empty to avoid UB (modulo 0).
std::string xor_transform(const std::string &data, const std::string &key) {
    if (key.empty()) {
        throw std::invalid_argument("xor_transform: key must not be empty");
    }
    std::string out;
    out.resize(data.size());
    const unsigned char *d = reinterpret_cast<const unsigned char*>(data.data());
    const unsigned char *k = reinterpret_cast<const unsigned char*>(key.data());
    size_t ksz = key.size();
    for (size_t i = 0; i < data.size(); ++i) {
        out[i] = static_cast<char>(d[i] ^ k[i % ksz]);
    }
    return out;
}

// Read file content (binary safe). '-' means stdin.
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

// Write content to file (binary safe). '-' means stdout.
bool write_file(const std::string &path, const std::string &content) {
    if (path == "-") {
        std::cout << content;
        return true;
    }
    std::ofstream ofs(path, std::ios::binary);
    if (!ofs) return false;
    ofs.write(content.data(), static_cast<std::streamsize>(content.size()));
    return ofs.good();
}

// trim whitespace from both ends
std::string trim(const std::string &s) {
    const std::string ws = " \t\r\n";
    const auto a = s.find_first_not_of(ws);
    if (a == std::string::npos) return "";
    const auto b = s.find_last_not_of(ws);
    return s.substr(a, b - a + 1);
}

// high-level wrappers: validate input and propagate helpful exceptions
std::string encode_autosave(const std::string &json_content, const std::string &key) {
    if (key.empty()) throw std::invalid_argument("encode_autosave: key must not be empty");
    // operate on raw bytes
    std::string xored = xor_transform(json_content, key);
    return base64_encode(xored);
}

std::string decode_autosave(const std::string &autosave_content, const std::string &key) {
    if (key.empty()) throw std::invalid_argument("decode_autosave: key must not be empty");
    std::string bdec = base64_decode(autosave_content);
    return xor_transform(bdec, key);
}
