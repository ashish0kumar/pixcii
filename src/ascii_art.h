#pragma once

#include "image.h"
#include <string>
#include <vector>

// Parameters for ASCII art generation
struct AsciiArtParams
{
    std::string input_path;
    std::string output_path;
    std::string ascii_chars = " .:-=+*#%@";
    bool color = false;
    bool invert_color = false;
    float brightness_boost = 1.0f;
    float scale = 1.0f;
    bool detect_edges = false;
    float aspect_ratio = 2.0f;
    bool auto_fit = false;
};

// Information about a block of pixels
struct BlockInfo
{
    uint64_t sum_brightness = 0;
    float sum_mag = 0.0f;
    std::vector<uint64_t> sum_color = {0, 0, 0};
    int pixel_count = 0;
};

// Process an image and generate ASCII art
void processImage(const AsciiArtParams &params);

// Generate ASCII art as text
std::string generateAsciiText(const Image &img, const AsciiArtParams &params);

// Calculate block information
BlockInfo calculateBlockInfo(const Image &img, int x, int y, const AsciiArtParams &params);

// Select an ASCII character based on block brightness
char selectAsciiChar(const BlockInfo &block_info, const AsciiArtParams &params);