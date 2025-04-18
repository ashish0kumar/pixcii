#pragma once
#include <vector>
#include <string>
#include <cstdint>

struct Image
{
    std::vector<uint8_t> data;
    int width;
    int height;
    int channels;
};

struct TerminalSize
{
    int width;
    int height;
};

// Image loading and manipulation functions
Image loadImage(const std::string &path);
Image resizeImage(const Image &img, float scale, float aspect_ratio);
std::vector<uint8_t> rgbToGrayscale(const Image &img);
TerminalSize getTerminalSize();
Image resizeImageToTerminal(const Image &img, float aspect_ratio, bool auto_fit);