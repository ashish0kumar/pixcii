#pragma once
#include "image.h"
#include <string>
#include <vector>
#include <cstdint>

struct AsciiArtParams
{
    std::string input_path;
    std::string output_path;
    std::string ascii_chars = " .:-=+*#%@";
    bool color = false;
    bool invert_color = false;
    float brightness_boost = 1.0f;
    float scale = 1.0f;
    int block_width = 8;   // Block width (horizontal)
    int block_height = 16; // Block height (vertical)
    bool detect_edges = false;
    float aspect_ratio = 2.0f; // Default aspect ratio (character height / width)
};

struct BlockInfo
{
    uint64_t sum_brightness = 0;
    std::vector<uint64_t> sum_color = {0, 0, 0};
    uint64_t pixel_count = 0;
    float sum_mag = 0.0f;
    float sum_dir = 0.0f;
};

// Core ASCII art generation functions
void processImage(const AsciiArtParams &params);
BlockInfo calculateBlockInfo(const Image &img, int x, int y, int block_width, int block_height, const AsciiArtParams &params);
char selectAsciiChar(const BlockInfo &block_info, const AsciiArtParams &params);
std::vector<uint8_t> generateAsciiArt(const Image &img, const AsciiArtParams &params);
std::string generateAsciiText(const Image &img, const AsciiArtParams &params);