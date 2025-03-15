#pragma once
#include <string>
#include <vector>
#include <cstdint>

// Output functions
void saveOutputImage(const std::vector<uint8_t> &ascii_img, int width, int height, const std::string &output_path);
void saveOutputText(const std::string &ascii_text, const std::string &output_path);