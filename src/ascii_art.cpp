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

    // Check if we should output as text or image
    if (params.output_path.empty() || params.output_path.substr(params.output_path.size() - 4) == ".txt")
    {
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
    else
    {
        // Generate ASCII art as an image
        std::vector<uint8_t> ascii_img = generateAsciiArt(img, params);

        // Calculate output dimensions
        int out_w = (img.width / params.block_width) * params.block_width;
        int out_h = (img.height / params.block_height) * params.block_height;

        // Save the image
        saveOutputImage(ascii_img, out_w, out_h, params.output_path);
    }
}

// Calculate block information for ASCII character selection
BlockInfo calculateBlockInfo(const Image &img, int x, int y, int block_width, int block_height, const AsciiArtParams &params)
{
    BlockInfo info;

    // Initialize sum_color with 3 zeros
    info.sum_color = {0, 0, 0};

    int bw = std::min(block_width, img.width - x);
    int bh = std::min(block_height, img.height - y);

    // Calculate edge magnitudes if edge detection is enabled
    static std::vector<float> edge_magnitudes;
    static bool edges_calculated = false;

    if (params.detect_edges && !edges_calculated)
    {
        edge_magnitudes = detectEdges(img);
        edges_calculated = true;
    }

    for (int dy = 0; dy < bh; dy++)
    {
        for (int dx = 0; dx < bw; dx++)
        {
            int ix = x + dx;
            int iy = y + dy;

            if (ix >= img.width || iy >= img.height)
            {
                continue;
            }

            int pixel_index = (iy * img.width + ix) * img.channels;
            if (pixel_index + 2 >= img.width * img.height * img.channels)
            {
                continue;
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
        }
    }

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

// Generate ASCII art as an image
std::vector<uint8_t> generateAsciiArt(const Image &img, const AsciiArtParams &params)
{
    int out_w = (img.width / params.block_width) * params.block_width;
    int out_h = (img.height / params.block_height) * params.block_height;

    out_w = std::max(out_w, 1);
    out_h = std::max(out_h, 1);

    std::vector<uint8_t> ascii_img(out_w * out_h * 3, 0);

    // Generate ASCII art
    for (int y = 0; y < out_h; y += params.block_height)
    {
        for (int x = 0; x < out_w; x += params.block_width)
        {
            BlockInfo block_info = calculateBlockInfo(img, x, y, params.block_width, params.block_height, params);
            char ascii_char = selectAsciiChar(block_info, params);

            // Calculate average color for this block
            std::vector<uint8_t> avg_color = {255, 255, 255};
            if (params.color && block_info.pixel_count > 0)
            {
                avg_color[0] = static_cast<uint8_t>(block_info.sum_color[0] / block_info.pixel_count);
                avg_color[1] = static_cast<uint8_t>(block_info.sum_color[1] / block_info.pixel_count);
                avg_color[2] = static_cast<uint8_t>(block_info.sum_color[2] / block_info.pixel_count);

                if (params.invert_color)
                {
                    avg_color[0] = 255 - avg_color[0];
                    avg_color[1] = 255 - avg_color[1];
                    avg_color[2] = 255 - avg_color[2];
                }
            }

            // For now, just fill the block with a color based on the brightness
            for (int dy = 0; dy < params.block_height; dy++)
            {
                for (int dx = 0; dx < params.block_width; dx++)
                {
                    int ix = x + dx;
                    int iy = y + dy;

                    if (ix >= out_w || iy >= out_h)
                    {
                        continue;
                    }

                    int img_index = (iy * out_w + ix) * 3;

                    // For simplicity, just fill with the average color
                    // In a more complete implementation, we would draw the actual character here
                    if (ascii_char != ' ')
                    {
                        ascii_img[img_index] = avg_color[0];
                        ascii_img[img_index + 1] = avg_color[1];
                        ascii_img[img_index + 2] = avg_color[2];
                    }
                    else
                    {
                        ascii_img[img_index] = 0;
                        ascii_img[img_index + 1] = 0;
                        ascii_img[img_index + 2] = 0;
                    }
                }
            }
        }
    }

    return ascii_img;
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
            BlockInfo block_info = calculateBlockInfo(img, x, y, 1, 1, params);
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

                // Reset color after each character
                // ascii_text += "\033[0m";  // Uncomment this for reset after each char
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