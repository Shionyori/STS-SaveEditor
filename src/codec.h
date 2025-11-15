#pragma once
#include <string>

std::string base64_encode(const std::string &in);
std::string base64_decode(const std::string &in);

std::string xor_transform(const std::string &input, const std::string &key);

std::string trim(const std::string &s);

std::string read_file(const std::string &path);
bool write_file(const std::string &path, const std::string &content);

// High-level encode/decode helpers used by both console and GUI
std::string encode_autosave(const std::string &json_content, const std::string &key = "key");
std::string decode_autosave(const std::string &autosave_content, const std::string &key = "key");

// File helpers
std::string read_file(const std::string &path);
bool write_file(const std::string &path, const std::string &content);
