#include "ascii_art.h"
#include "edge_detection.h"
#include "output.h"
#include "image.h"
#include <algorithm>
#include <cmath>
#include <iostream>
#include <vector>
#include <opencv2/opencv.hpp>
#include <chrono>
#include <thread>

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

// Main function to process an image and generate ASCII art
// Handles loading, resizing, edge detection, text generation, and output
void processImage(const AsciiArtParams &params)
{
    // Load the image from the specified input path
    Image img = loadImage(params.input_path);

    // --- Image Resizing and Aspect Ratio Adjustment ---
    // Check if auto-fit to terminal is enabled
    if (params.auto_fit)
    {
        // Resize image to fit terminal dimensions while respecting aspect ratio
        img = resizeImageToTerminal(img, params.aspect_ratio, true);
    }
    // Otherwise, apply scaling based on the scale parameter
    else
    {
        // Original size mode with optional scaling
        if (params.scale != 1.0f)
        {
            // Calculate target dimensions preserving original aspect ratio
            int target_width = static_cast<int>(img.width * params.scale);
            int target_height = static_cast<int>(img.height * params.scale / params.aspect_ratio);

            // Safety bounds checking
            if (target_width > 1000 || target_height > 1000)
            {
                std::cerr << "Error: Scaled dimensions too large. Maximum 1000x1000." << std::endl;
                return;
            }

            if (target_width < 10 || target_height < 10)
            {
                std::cerr << "Error: Scaled dimensions too small. Minimum 10x10." << std::endl;
                return;
            }

            // Create new image structure with target dimensions
            Image scaled_img;
            scaled_img.width = target_width;
            scaled_img.height = target_height;
            scaled_img.channels = img.channels;
            scaled_img.data.resize(target_width * target_height * img.channels);

            // Use OpenCV for reliable resizing
            cv::Mat src_mat(img.height, img.width,
                            img.channels == 3 ? CV_8UC3 : CV_8UC1, img.data.data());
            cv::Mat dst_mat;

            cv::resize(src_mat, dst_mat, cv::Size(target_width, target_height), 0, 0, cv::INTER_LINEAR);

            memcpy(scaled_img.data.data(), dst_mat.data,
                   target_width * target_height * img.channels);

            img = scaled_img;
        }
    }

    // --- Edge Detection ---
    // Perform edge detection ONCE on the resized image if requested by parameters
    std::vector<float> edge_magnitudes;
    // Use a pointer to the edge magnitudes vector. It will be nullptr if edges are not detected.
    const std::vector<float> *edge_magnitudes_ptr = nullptr;

    if (params.detect_edges)
    {
        // Call the edge detection function
        edge_magnitudes = detectEdges(img);
        // Point the pointer to the calculated magnitudes vector
        edge_magnitudes_ptr = &edge_magnitudes;
    }
    // --- End Edge Detection ---

    // Generate the ASCII text representation of the image
    // Pass the image data, parameters, and the optional edge magnitudes pointer
    std::string ascii_text = generateAsciiText(img, params, edge_magnitudes_ptr);

    // --- Output Handling ---
    // If an output path is specified, save the ASCII text to a file
    if (!params.output_path.empty())
    {
        saveOutputText(ascii_text, params.output_path);
    }
    // If no output path is specified, print the ASCII text to the console
    else
    {
        std::cout << std::endl
                  << ascii_text << std::endl;
    }

    // Note: Image data (img.data) is automatically freed when img goes out of scope
    // Edge magnitudes vector is also freed when edge_magnitudes goes out of scope
    // OpenCV Mat objects are automatically cleaned up by their destructors
}

// Calculate relevant information (brightness, color, edge_magnitude) for a single pixel
// This function processes one pixel at the given (x, y) coordinates
PixelInfo getPixelInfo(const Image &img, int x, int y, const AsciiArtParams &params, const std::vector<float> *edge_magnitudes)
{
    PixelInfo info; // Create a struct to hold pixel information

    // Initialize color vector with zeros
    info.color = {0, 0, 0};

    // --- Bounds Check ---
    // Basic bounds check to ensure coordinates are within image dimensions
    // Although calling code should ideally provide valid coordinates, this adds safety
    if (x < 0 || x >= img.width || y < 0 || y >= img.height)
    {
        // If out of bounds, return default (zero-initialized) info
        return info;
    }

    // Calculate the starting index of the pixel's data in the image vector
    int pixel_index = (y * img.width + x) * img.channels;

    // --- Data Bounds Check ---
    // More robust check to ensure accessing pixel_index and subsequent channels
    // does not go beyond the allocated data vector size
    if (static_cast<size_t>(pixel_index + (img.channels > 0 ? img.channels - 1 : 0)) >= img.data.size())
    {
        // If index is out of bounds for the data vector, return default info
        return info;
    }
    // --- End Bounds Checks ---

    // Get the color components based on the number of channels
    // Safely access channels if they exist
    uint8_t r = (img.channels >= 1) ? img.data[pixel_index] : 0;
    uint8_t g = (img.channels >= 2) ? img.data[pixel_index + 1] : 0;
    uint8_t b = (img.channels >= 3) ? img.data[pixel_index + 2] : 0;

    // Calculate grayscale brightness using standard luminance weights
    // These weights are defined as constants in image.cpp (conceptually)
    uint64_t gray = static_cast<uint64_t>(
        constants::GRAYSCALE_WEIGHT_R * r +
        constants::GRAYSCALE_WEIGHT_G * g +
        constants::GRAYSCALE_WEIGHT_B * b);

    // --- Edge Detection Data Access ---
    // If edge detection was enabled and the edge magnitudes vector was provided
    if (params.detect_edges && edge_magnitudes != nullptr)
    {
        // Check if the current pixel's index is within the bounds of the edge magnitudes vector
        size_t edge_index = y * img.width + x;
        if (edge_index < edge_magnitudes->size())
        {
            // Assign the pre-calculated edge magnitude for this pixel
            info.edge_magnitude = (*edge_magnitudes)[edge_index];
        }
        else
        {
            // Should not happen if edge_magnitudes vector size matches image size, but safer check
            info.edge_magnitude = 0.0f; // Default if index is somehow out of bounds
        }
    }
    else
    {
        // If edge detection is not enabled, use the calculated brightness value
        info.brightness = gray;
    }
    // --- End Edge Detection Data Access ---

    // If color output is enabled and the image has enough channels (at least 3 for RGB)
    if (params.color && img.channels >= 3)
    {
        // Store the pixel's color values directly
        info.color[0] = r;
        info.color[1] = g;
        info.color[2] = b;
    }

    // The struct now holds info for this specific pixel
    return info;
}

// Select an ASCII character from the character set based on the pixel information (brightness or edge magnitude)
// The character is chosen based on mapping the pixel value (0-255) to the character set size
char selectAsciiChar(const PixelInfo &pixel_info, const AsciiArtParams &params)
{
    uint64_t value; // Value used for character selection (brightness or edge magnitude)

    // Determine which value to use based on parameters
    if (params.detect_edges)
    {
        // Use edge magnitude if edge detection is enabled
        float mag = pixel_info.edge_magnitude;
        // Apply brightness boost and clamp between 0 and 255
        value = static_cast<uint64_t>(std::min(std::max(mag * params.brightness_boost, 0.0f), 255.0f));
    }
    else
    {
        // Use brightness if edge detection is not enabled
        uint64_t brightness = pixel_info.brightness;
        // Apply brightness boost and clamp between 0 and 255
        float boosted_brightness = static_cast<float>(brightness) * params.brightness_boost;
        value = std::min(static_cast<uint64_t>(std::max(boosted_brightness, 0.0f)), static_cast<uint64_t>(255));
    }

    // Handle the case where the value is 0. Map to the first character unless inverted.
    // The first character is typically space for brightness.
    if (value == 0 && !params.invert_color)
    {
        return params.ascii_chars.front(); // Return the first character (lowest brightness)
    }
    // Handle the case where the value is 0 and inverted. Map to the last character.
    else if (value == 0 && params.invert_color)
    {
        return params.ascii_chars.back(); // Return the last character (highest brightness in inverted scale)
    }

    // Map the value (0-255) to an index in the ASCII character set
    // The index is proportional to the value relative to the 0-256 range
    size_t char_index = (value * params.ascii_chars.size()) / 256;

    // Ensure the calculated index is within the bounds of the character set vector
    char_index = std::min(char_index, params.ascii_chars.size() - 1);

    // Return the character at the selected index
    // If invert_color is true, select from the end of the character set
    return params.ascii_chars[params.invert_color ? params.ascii_chars.size() - 1 - char_index : char_index];
}

// Generate ASCII art as a text string
// Iterates through each pixel of the image, calculates its info, selects a character,
// and formats the output string, including ANSI color codes if enabled.
std::string generateAsciiText(const Image &img, const AsciiArtParams &params, const std::vector<float> *edge_magnitudes)
{
    std::string ascii_text; // String to build the ASCII output
    // Determine if color output should be used (requires color flag and enough image channels)
    bool use_color = params.color && img.channels >= 3;

    // Iterate through each row (height) of the image
    for (int y = 0; y < img.height; y++)
    {
        // Iterate through each column (width) of the image
        for (int x = 0; x < img.width; x++)
        {
            // Get the information for the current pixel
            // Pass the edge magnitudes pointer
            PixelInfo pixel_info = getPixelInfo(img, x, y, params, edge_magnitudes);

            // Select the ASCII character corresponding to this pixel's info
            char ascii_char = selectAsciiChar(pixel_info, params);

            // --- Color Formatting ---
            if (use_color)
            {
                // Get the color components from the pixel info
                uint8_t r = pixel_info.color[0];
                uint8_t g = pixel_info.color[1];
                uint8_t b = pixel_info.color[2];

                // Append ANSI color escape code for 24-bit color (RGB)
                // Format: \033[38;2;R;G;Bm
                ascii_text += "\033[38;2;" + std::to_string(r) + ";" + std::to_string(g) + ";" + std::to_string(b) + "m";
                // Append the selected ASCII character
                ascii_text += ascii_char;
            }
            else
            {
                // If color is not enabled, just append the ASCII character
                ascii_text += ascii_char;
            }
        }

        // --- Color Reset and Newline ---
        // Reset color at the end of each line to prevent bleeding into the next line or prompt
        if (use_color)
        {
            ascii_text += "\033[0m"; // ANSI reset code
        }

        // Add a newline character at the end of each row to move to the next line of ASCII art
        ascii_text += '\n';
    }

    // Return the complete generated ASCII text string
    return ascii_text;
}

// --- Video Processing Implementation ---

// Convert OpenCV Mat to your Image structure
Image matToImage(const cv::Mat &mat)
{
    Image img;

    // Set basic image properties
    img.width = mat.cols;
    img.height = mat.rows;
    img.channels = mat.channels();

    // Calculate total data size
    size_t data_size = img.width * img.height * img.channels;
    img.data.resize(data_size);

    // Copy pixel data from OpenCV Mat to your Image structure
    // OpenCV uses BGR format, so we need to convert to RGB
    if (img.channels == 3)
    {
        cv::Mat rgb_mat;
        cv::cvtColor(mat, rgb_mat, cv::COLOR_BGR2RGB);
        memcpy(img.data.data(), rgb_mat.data, data_size);
    }
    else
    {
        // For grayscale or other formats, copy directly
        memcpy(img.data.data(), mat.data, data_size);
    }

    return img;
}

// Check if file is a supported video/GIF format
// Supports common video formats and animated GIFs
bool isVideoFile(const std::string &filename)
{
    // Convert filename to lowercase for case-insensitive comparison
    std::string lower = filename;
    std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);

    // Check for supported video and GIF extensions
    return (lower.find(".mp4") != std::string::npos ||
            lower.find(".avi") != std::string::npos ||
            lower.find(".mov") != std::string::npos ||
            lower.find(".mkv") != std::string::npos ||
            lower.find(".webm") != std::string::npos ||
            lower.find(".gif") != std::string::npos ||
            lower.find(".m4v") != std::string::npos ||
            lower.find(".wmv") != std::string::npos ||
            lower.find(".flv") != std::string::npos);
}

// Process video/GIF file with smooth playback and proper frame buffer management
bool processVideo(const std::string &videoFile,
                  const AsciiArtParams &params,
                  int frameDelay)
{
    cv::VideoCapture cap(videoFile);
    if (!cap.isOpened())
    {
        std::cerr << "Failed to open video/GIF file: " << videoFile << std::endl;
        return false;
    }

    double fps = cap.get(cv::CAP_PROP_FPS);
    int calculatedDelay = (fps > 0) ? static_cast<int>(1000.0 / fps) : 100;
    int actualDelay = (frameDelay == 100) ? calculatedDelay : frameDelay;

    // Terminal initialization
    std::cout << "\033c" << std::flush;                     // Full reset
    std::cout << "\033[?1049h" << std::flush;               // Enter alternate screen
    std::cout << "\033[2J\033[1;1H\033[?25l" << std::flush; // Clear screen, position cursor, hide cursor
    std::cout << "\033[?1000h" << std::flush;               // Capture mouse event

    int frameCount = 0;
    cv::Mat frame;
    auto lastFrameTime = std::chrono::steady_clock::now();
    int prevHeight = 0;
    int prevWidth = 0;

    while (true)
    {
        cap >> frame;
        if (frame.empty())
        {
            break;
        }

        // Convert and process frame
        Image img = matToImage(frame);

        // Apply existing processing pipeline
        if (params.auto_fit)
        {
            img = resizeImageToTerminal(img, params.aspect_ratio, true);
        }
        else if (params.scale != 1.0f)
        {
            img = resizeImage(img, params.scale, params.aspect_ratio);
        }
        else
        {
            img = resizeImage(img, 1.0f, params.aspect_ratio);
        }

        // Edge detection if enabled
        std::vector<float> edge_magnitudes;
        const std::vector<float> *edge_magnitudes_ptr = nullptr;

        if (params.detect_edges)
        {
            edge_magnitudes = detectEdges(img);
            edge_magnitudes_ptr = &edge_magnitudes;
        }

        // Generate ASCII text
        std::string ascii_text = generateAsciiText(img, params, edge_magnitudes_ptr);

        int currentHeight = img.height;
        int currentWidth = img.width;

        // Frame rendering with cleanup
        std::cout << "\033[1;1H" << std::flush; // Go to top-left

        // Clean up previous frame artifacts
        if (prevHeight > 0 || prevWidth > 0)
        {
            // Clear extra lines if frame got shorter
            if (currentHeight < prevHeight)
            {
                for (int i = currentHeight + 1; i <= prevHeight; i++)
                {
                    std::cout << "\033[" << i << ";1H\033[K";
                }
            }

            // Clear extra columns if frame got narrower
            if (currentWidth < prevWidth)
            {
                int maxHeight = std::max(currentHeight, prevHeight);
                for (int line = 1; line <= maxHeight; line++)
                {
                    std::cout << "\033[" << line << ";" << (currentWidth + 1) << "H\033[K";
                }
            }

            std::cout << "\033[1;1H";
        }

        // Display the frame
        std::cout << ascii_text << std::flush;

        // Update dimensions for next iteration
        prevHeight = currentHeight;
        prevWidth = currentWidth;
        frameCount++;

        // Timing control
        auto currentTime = std::chrono::steady_clock::now();
        auto elapsedTime = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - lastFrameTime).count();

        if (elapsedTime < actualDelay)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(actualDelay - elapsedTime));
        }

        lastFrameTime = std::chrono::steady_clock::now();
    }

    // Restore and exit
    std::cout << "\033[?1000l" << std::flush; // Disable mouse capture
    std::cout << "\033[?25h" << std::flush;   // Show cursor
    std::cout << "\033[?1049l" << std::flush; // Exit alternate screen

    cap.release();
    return true;
}