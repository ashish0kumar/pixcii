// Define the implementations for the stb image libraries in this file
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STB_IMAGE_RESIZE2_IMPLEMENTATION

#include "stb_image.h"
#include "stb_image_write.h"
#include "stb_image_resize2.h"

#include "image.h"
#include <algorithm>
#include <stdexcept>
#include <cmath>
#include <iostream>
#include <vector>
#include <string>

// For terminal size detection - platform specific includes
#ifdef _WIN32
#include <windows.h>
#else
#include <sys/ioctl.h>
#include <unistd.h>
#endif

// --- Constants ---
namespace constants
{
    // Standard luminance weights for RGB to Grayscale conversion (Rec. 601 standard)
    // These weights are used when converting a color image to a single grayscale value per pixel.
    const float GRAYSCALE_WEIGHT_R = 0.299f;
    const float GRAYSCALE_WEIGHT_G = 0.587f;
    const float GRAYSCALE_WEIGHT_B = 0.114f;
}
// --- End Constants ---

// Load an image from a file path using the stb_image library.
// Handles common image formats (like JPG, PNG, TGA, BMP, GIF, PSD, PIC).
Image loadImage(const std::string &path)
{
    Image img;                   // Create an Image struct to store the result
    int width, height, channels; // Variables to receive image dimensions and channel count

    // Load the image data. Pass 0 for desired_channels to keep original channels.
    // stb_image loads data into a byte array (uint8_t*).
    uint8_t *data = stbi_load(path.c_str(), &width, &height, &channels, 0);

    // Check if image loading failed
    if (!data)
    {
        // Throw a runtime_error with a descriptive message if loading fails
        throw std::runtime_error("Failed to load image: " + path + " - " + stbi_failure_reason());
    }

    // Populate our Image struct with the loaded data
    img.width = width;
    img.height = height;
    img.channels = channels;

    // Copy the loaded data into a std::vector owned by our Image struct.
    // This allows automatic memory management (vector will delete data when img goes out of scope).
    // Use static_cast<size_t> for multiplication result to avoid potential overflow before casting to size_t for size_t.
    img.data.assign(data, data + static_cast<size_t>(width) * static_cast<size_t>(height) * static_cast<size_t>(channels));

    // Free the data allocated by stb_image_load, as our vector now holds the copy
    stbi_image_free(data);

    // Return the populated Image struct
    return img;
}

// Resize an image using a scale factor and adjust height based on character aspect ratio.
// This implementation now uses stb_image_resize2 for high-quality interpolation.
// img: The input Image struct.
// scale: A scaling factor (e.g., 1.0 for no scaling, 2.0 to make output size ~half of input pixel dimensions).
// aspect_ratio: The aspect ratio (width / height) of characters used for output. Vertical dimension is adjusted by this.
// Returns: A new Image struct with the resized image data.
Image resizeImage(const Image &img, float scale, float aspect_ratio)
{
    Image resized; // Create a new Image struct for the resized data

    // Calculate new dimensions based on the scale factor and aspect ratio.
    // Note: scale > 1.0 typically means the resulting ASCII art has fewer characters (smaller output).
    // The resulting dimensions are integer values due to casting, which can cause slight inaccuracies.
    int new_width = static_cast<int>(static_cast<float>(img.width) / scale);
    int new_height = static_cast<int>(static_cast<float>(img.height) / scale / aspect_ratio);

    // Ensure dimensions are at least 1x1 pixel
    new_width = std::max(new_width, 1);
    new_height = std::max(new_height, 1);

    resized.width = new_width;
    resized.height = new_height;
    resized.channels = img.channels;
    // Allocate memory for the resized image data vector. Use size_t for size calculation.
    resized.data.resize(static_cast<size_t>(new_width) * static_cast<size_t>(new_height) * static_cast<size_t>(img.channels));

    // --- Use stb_image_resize2 for Interpolation ---

    // Call the appropriate resize function from stb_image_resize2.h (stbir_resize_uint8_linear)
    // We need to map img.channels (int) to stbir_pixel_layout enum.
    // Common values for stbir_pixel_layout are STBIR_LUMINANCE (1), STBIR_RGB (3), STBIR_RGBA (4).
    // We'll cast the int channel count to the enum type. This relies on the enum values matching channel counts.
    stbir_pixel_layout layout = static_cast<stbir_pixel_layout>(img.channels);

    unsigned char *result_ptr = stbir_resize_uint8_linear(
        img.data.data(),              // input_pixels (const unsigned char*) - Pass const pointer to source data
        img.width,                    // input_w (int)
        img.height,                   // input_h (int)
        img.width * img.channels,     // input_stride_bytes (int)
        resized.data.data(),          // output_pixels (unsigned char*) - Pass non-const pointer to destination data
        new_width,                    // output_w (int)
        new_height,                   // output_h (int)
        new_width * resized.channels, // output_stride_bytes (int)
        layout                        // pixel_layout (stbir_pixel_layout) - Pass the cast enum value
    );

    // Check for success - stb_image_resize2 linear functions return the output pointer on success, NULL on failure.
    if (result_ptr == NULL)
    {
        // Handle error - throw an exception to be caught by the main try-catch block.
        throw std::runtime_error("Image resizing failed using stb_image_resize2 (linear). Returned NULL.");
    }
    // --- End stb_image_resize2 ---

    return resized; // Return the resized image struct
}

// Convert an RGB image to grayscale using standard luminance weights.
// img: The input Image struct. Assumes at least 3 channels (RGB).
// Returns: A vector containing the grayscale value (0-255) for each pixel.
// Returns an empty vector or vector of zeros if the image doesn't have enough channels.
std::vector<uint8_t> rgbToGrayscale(const Image &img)
{
    // Check if the image has enough channels for RGB conversion
    if (img.channels < 3)
    {
        // If not, print a warning and return a default grayscale representation (all zeros)
        std::cerr << "Warning: Image does not have enough channels (" << img.channels << ") for RGB to Grayscale conversion. Expected 3 or more." << std::endl;
        return std::vector<uint8_t>(static_cast<size_t>(img.width) * static_cast<size_t>(img.height), 0); // Return black/zero grayscale, use static_cast
    }

    std::vector<uint8_t> grayscale(static_cast<size_t>(img.width) * static_cast<size_t>(img.height)); // Vector to store grayscale values, use static_cast

    // Iterate through each pixel of the image
    for (int y = 0; y < img.height; y++)
    {
        for (int x = 0; x < img.width; x++)
        {
            // Calculate the starting index of the current pixel's RGB data
            size_t i = static_cast<size_t>((y * img.width + x) * img.channels); // Use size_t for index calculation

            // Get the R, G, and B components
            uint8_t r = img.data[i];
            uint8_t g = img.data[i + 1];
            uint8_t b = img.data[i + 2];

            // Convert to grayscale using standard luminance weights (Rec. 601) defined as constants
            grayscale[static_cast<size_t>(y * img.width + x)] = static_cast<uint8_t>( // Use size_t for index calculation
                constants::GRAYSCALE_WEIGHT_R * r +
                constants::GRAYSCALE_WEIGHT_G * g +
                constants::GRAYSCALE_WEIGHT_B * b
            );
        }
    }

    // Return the vector containing the grayscale image data
    return grayscale;
}

// Get the current size of the terminal window using platform-specific APIs.
// Returns a default size (80x24) if determination fails.
TerminalSize getTerminalSize()
{
    TerminalSize size = {80, 24}; // Default fallback size (standard terminal dimensions)

#ifdef _WIN32
    // Windows implementation using GetConsoleScreenBufferInfo
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    // Get handle to the standard output console
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hConsole != INVALID_HANDLE_VALUE && GetConsoleScreenBufferInfo(hConsole, &csbi))
    {
        // Calculate width and height from the console window rectangle
        size.width = csbi.srWindow.Right - csbi.srWindow.Left + 1;
        size.height = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
    }
#else
    // Unix-like systems implementation using ioctl and TIOCGWINSZ
    struct winsize w;
    // Use ioctl to get window size for standard output file descriptor
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) != -1)
    {
        // Get columns (width) and rows (height) from winsize struct
        // Subtract a small margin (e.g., 5) to avoid issues with line wrapping/prompts
        size.width = w.ws_col > 5 ? w.ws_col - 5 : w.ws_col;
        size.height = w.ws_row > 5 ? w.ws_row - 5 : w.ws_row;
        // Ensure minimum size
        size.width = std::max(size.width, 1);
        size.height = std::max(size.height, 1);
    }
#endif

    // Return the determined (or default) terminal size
    return size;
}

// Resize image to fit the current terminal dimensions, adjusting for character aspect ratio.
// This function calculates the appropriate scale factor to fit the image within the terminal.
// img: The input Image struct.
// aspect_ratio: The aspect ratio of characters (width/height).
// auto_fit: If true, performs the resize; otherwise, returns the original image.
// Returns: The resized Image struct or the original if auto_fit is false.
Image resizeImageToTerminal(const Image &img, float aspect_ratio, bool auto_fit)
{
    // If auto-fitting is not requested, return the original image without resizing
    if (!auto_fit)
    {
        return img;
    }

    // Get the current terminal size
    TerminalSize term = getTerminalSize();

    // Calculate available terminal height, leaving a small margin for prompts/status
    int terminal_height = term.height > 1 ? term.height - 1 : term.height;
    terminal_height = std::max(terminal_height, 1); // Ensure at least 1

    // Calculate the required scale factors based on both width and height to fit within terminal.
    // Scale based on width: (Original width) / (Terminal width in characters)
    float scale_width = static_cast<float>(img.width) / term.width;
    // Scale based on height: (Original height) / (Terminal height in characters * character aspect ratio)
    float scale_height = static_cast<float>(img.height) / (static_cast<float>(terminal_height) * aspect_ratio);

    // Use the LARGER of the two scale factors. This ensures that the image, when scaled by this factor,
    // will fit entirely within the terminal dimensions (width and height).
    float scale = std::max(scale_width, scale_height);

    // Ensure the calculated scale is a reasonable positive value.
    // Avoid division by zero or near-zero, and prevent extreme scaling.
    if (scale <= 0.0001f)
        scale = 0.0001f; // Prevent scale from being too small (results in HUGE output)
    if (scale > 10000.0f)
        scale = 10000.0f; // Prevent scale from being too large (results in TINY output)

    // Use the generic resizeImage function with the calculated scale and aspect ratio.
    // Note that resizeImage will re-calculate the new dimensions based on this 'scale' value.
    return resizeImage(img, scale, aspect_ratio);
}