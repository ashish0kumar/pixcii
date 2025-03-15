#include "ascii_art.h"
#include "edge_detection.h"
#include "output.h"
#include <algorithm>
#include <cmath>
#include <iostream>

// Main function to process an image
void processImage(const AsciiArtParams &params)
{
    // Load the image
    Image img = loadImage(params.input_path);

    // Resize the image if needed, accounting for aspect ratio
    if (params.scale != 1.0f)
    {
        img = resizeImage(img, params.scale, params.aspect_ratio);
    }
    else
    {
        // Even if not explicitly resizing, we may need to adjust for aspect ratio
        img = resizeImage(img, 1.0f, params.aspect_ratio);
    }

    // Generate ASCII text
    std::string ascii_text = generateAsciiText(img, params);

    // Print to console if no output path
    if (params.output_path.empty())
    {
        std::cout << ascii_text << std::endl;
    }
    else
    {
        // Save to file
        saveOutputText(ascii_text, params.output_path);
    }
}

// Calculate block information for ASCII character selection
BlockInfo calculateBlockInfo(const Image &img, int x, int y, const AsciiArtParams &params)
{
    BlockInfo info;

    // Initialize sum_color with 3 zeros
    info.sum_color = {0, 0, 0};

    // Calculate edge magnitudes if edge detection is enabled
    static std::vector<float> edge_magnitudes;
    static bool edges_calculated = false;

    if (params.detect_edges && !edges_calculated)
    {
        edge_magnitudes = detectEdges(img);
        edges_calculated = true;
    }

    int ix = x;
    int iy = y;

    if (ix >= img.width || iy >= img.height)
    {
        return info;
    }

    int pixel_index = (iy * img.width + ix) * img.channels;
    if (pixel_index + 2 >= img.width * img.height * img.channels)
    {
        return info;
    }

    uint8_t r = img.data[pixel_index];
    uint8_t g = img.data[pixel_index + 1];
    uint8_t b = img.data[pixel_index + 2];

    uint64_t gray = static_cast<uint64_t>(0.3f * r + 0.59f * g + 0.11f * b);

    // If edge detection is enabled, use edge magnitude instead of brightness
    if (params.detect_edges)
    {
        info.sum_mag += edge_magnitudes[iy * img.width + ix];
    }
    else
    {
        info.sum_brightness += gray;
    }

    if (params.color)
    {
        info.sum_color[0] += r;
        info.sum_color[1] += g;
        info.sum_color[2] += b;
    }

    info.pixel_count += 1;

    return info;
}

// Select an ASCII character based on block brightness
char selectAsciiChar(const BlockInfo &block_info, const AsciiArtParams &params)
{
    if (block_info.pixel_count == 0)
    {
        return ' ';
    }

    uint64_t value;
    if (params.detect_edges)
    {
        // Use edge magnitude
        float avg_mag = block_info.sum_mag / block_info.pixel_count;
        value = static_cast<uint64_t>(std::min(avg_mag * params.brightness_boost, 255.0f));
    }
    else
    {
        // Use brightness
        uint64_t avg_brightness = block_info.sum_brightness / block_info.pixel_count;
        float boosted_brightness = static_cast<float>(avg_brightness) * params.brightness_boost;
        value = std::min(static_cast<uint64_t>(std::max(boosted_brightness, 0.0f)), static_cast<uint64_t>(255));
    }

    if (value == 0)
    {
        return ' ';
    }

    size_t char_index = (value * params.ascii_chars.size()) / 256;
    char_index = std::min(char_index, params.ascii_chars.size() - 1);

    return params.ascii_chars[params.invert_color ? params.ascii_chars.size() - 1 - char_index : char_index];
}

// Generate ASCII art as text
std::string generateAsciiText(const Image &img, const AsciiArtParams &params)
{
    std::string ascii_text;
    bool use_color = params.color && img.channels >= 3;

    for (int y = 0; y < img.height; y++)
    {
        for (int x = 0; x < img.width; x++)
        {
            BlockInfo block_info = calculateBlockInfo(img, x, y, params);
            char ascii_char = selectAsciiChar(block_info, params);

            if (use_color)
            {
                // Add ANSI color escape codes
                uint8_t r = block_info.sum_color[0] / block_info.pixel_count;
                uint8_t g = block_info.sum_color[1] / block_info.pixel_count;
                uint8_t b = block_info.sum_color[2] / block_info.pixel_count;

                // ANSI color escape code for RGB
                ascii_text += "\033[38;2;" + std::to_string(r) + ";" + std::to_string(g) + ";" + std::to_string(b) + "m";
                ascii_text += ascii_char;
            }
            else
            {
                ascii_text += ascii_char;
            }
        }

        // Reset color at the end of each line
        if (use_color)
        {
            ascii_text += "\033[0m";
        }

        ascii_text += '\n';
    }

    return ascii_text;
}