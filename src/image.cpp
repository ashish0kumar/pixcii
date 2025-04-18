#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image.h"
#include "stb_image_write.h"
#include "image.h"
#include <algorithm>
#include <stdexcept>
#include <cmath>
#include <iostream>

// For terminal size detection
#ifdef _WIN32
#include <windows.h>
#else
#include <sys/ioctl.h>
#include <unistd.h>
#endif

// Load an image using stb_image
Image loadImage(const std::string &path)
{
    Image img;
    int width, height, channels;

    // Load the image
    uint8_t *data = stbi_load(path.c_str(), &width, &height, &channels, 0);
    if (!data)
    {
        throw std::runtime_error("Failed to load image: " + path);
    }

    // Create our image structure
    img.width = width;
    img.height = height;
    img.channels = channels;
    img.data.assign(data, data + width * height * channels);

    // Free the stb_image data
    stbi_image_free(data);

    return img;
}

// Resize an image with aspect ratio correction
Image resizeImage(const Image &img, float scale, float aspect_ratio)
{
    Image resized;

    // Calculate new dimensions with aspect ratio correction
    // For text output, we need to make the image wider to compensate for character aspect ratio
    int new_width = static_cast<int>(img.width / scale);
    int new_height = static_cast<int>((img.height / scale) / aspect_ratio);

    new_width = std::max(new_width, 1);
    new_height = std::max(new_height, 1);

    resized.width = new_width;
    resized.height = new_height;
    resized.channels = img.channels;

    // Use stb_image_resize (this is simplified and would need actual implementation)
    // For now, we'll use a simple nearest-neighbor approach
    resized.data.resize(new_width * new_height * img.channels);

    for (int y = 0; y < new_height; y++)
    {
        for (int x = 0; x < new_width; x++)
        {
            int src_x = static_cast<int>(x * scale);
            int src_y = static_cast<int>(y * scale * aspect_ratio);

            // Keep coordinates in bounds
            src_x = std::min(src_x, img.width - 1);
            src_y = std::min(src_y, img.height - 1);

            for (int c = 0; c < img.channels; c++)
            {
                resized.data[(y * new_width + x) * img.channels + c] =
                    img.data[(src_y * img.width + src_x) * img.channels + c];
            }
        }
    }

    return resized;
}

// Get current terminal size
TerminalSize getTerminalSize()
{
    TerminalSize size = {80, 24}; // Default fallback size

#ifdef _WIN32
    // Windows implementation
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi))
    {
        size.width = csbi.srWindow.Right - csbi.srWindow.Left + 1;
        size.height = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
    }
#else
    // Unix implementation
    struct winsize w;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) != -1)
    {
        size.width = w.ws_col - 5;
        size.height = w.ws_row - 5;
    }
#endif

    return size;
}

// Resize image to fit terminal dimensions
Image resizeImageToTerminal(const Image &img, float aspect_ratio, bool auto_fit)
{
    if (!auto_fit)
    {
        // If auto_fit is false, just return the original image
        return img;
    }

    // Get terminal size
    TerminalSize term = getTerminalSize();

    // Account for one line potentially being used for status/prompt
    int terminal_height = term.height - 1;

    // Calculate scale factor
    float scale_width = static_cast<float>(img.width) / term.width;
    float scale_height = static_cast<float>(img.height) / (terminal_height * aspect_ratio);

    // Use the larger scale factor to ensure the image fits
    float scale = std::max(scale_width, scale_height);

    // Ensure scale is positive and not too small
    if (scale <= 0.01f)
        scale = 0.01f;

    // Use the existing resize with the calculated scale
    return resizeImage(img, scale, aspect_ratio);
}

// Convert RGB image to grayscale
std::vector<uint8_t> rgbToGrayscale(const Image &img)
{
    std::vector<uint8_t> grayscale(img.width * img.height);

    for (int y = 0; y < img.height; y++)
    {
        for (int x = 0; x < img.width; x++)
        {
            int i = (y * img.width + x) * img.channels;
            uint8_t r = img.data[i];
            uint8_t g = img.data[i + 1];
            uint8_t b = img.data[i + 2];

            // Convert to grayscale using standard weights
            grayscale[y * img.width + x] = static_cast<uint8_t>(0.299f * r + 0.587f * g + 0.114f * b);
        }
    }

    return grayscale;
}