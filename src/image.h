#pragma once
#include <vector>
#include <string>
#include <cstdint>

// Structure to hold image data
struct Image
{
    std::vector<uint8_t> data; // Pixel data (e.g., RGBRGB...)
    int width;                 // Image width in pixels
    int height;                // Image height in pixels
    int channels;              // Number of color channels per pixel (e.g., 3 for RGB, 4 for RGBA)

    // Constructor to initialize members
    Image() : width(0), height(0), channels(0) {}

    // Destructor is implicitly handled by std::vector's destructor
};

// Structure to hold terminal dimensions
struct TerminalSize
{
    int width;  // Terminal width in characters
    int height; // Terminal height in characters
};

// --- Function Declarations ---

// Load an image from a file path using stb_image.
// path: The path to the image file.
// Returns: An Image struct containing the loaded image data and dimensions.
// Throws: std::runtime_error if the image fails to load.
Image loadImage(const std::string &path);

// Resize an image using a scale factor and adjust height based on character aspect ratio.
// img: The input Image struct.
// scale: A scaling factor (e.g., 1.0 for no scaling, 0.5 for half size).
// aspect_ratio: The aspect ratio (width/height) of characters used for output.
// Returns: A new Image struct with the resized image data.
// Note: Uses nearest-neighbor interpolation in the current implementation (see .cpp for details).
Image resizeImage(const Image &img, float scale, float aspect_ratio);

// Convert an RGB image to grayscale.
// img: The input Image struct (expected to have at least 3 channels).
// Returns: A vector of uint8_t containing the grayscale values (0-255) for each pixel.
// Returns an empty vector or vector of zeros if the image doesn't have enough channels.
std::vector<uint8_t> rgbToGrayscale(const Image &img);

// Get the current size of the terminal window.
// Returns: A TerminalSize struct with the width and height in characters.
// Provides a default size if terminal size cannot be determined.
TerminalSize getTerminalSize();

// Resize an image to fit the current terminal dimensions.
// img: The input Image struct.
// aspect_ratio: The aspect ratio of characters used for output.
// auto_fit: Boolean flag indicating if auto-fitting is enabled. If false, returns the original image.
// Returns: A new Image struct resized to fit the terminal, or the original image if auto_fit is false.
Image resizeImageToTerminal(const Image &img, float aspect_ratio, bool auto_fit);