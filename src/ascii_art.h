#pragma once

#include "image.h"
#include <string>
#include <vector>
#include <cstdint>
#include <opencv2/opencv.hpp>

// Structure to hold parameters for ASCII art generation
struct AsciiArtParams
{
    std::string input_path;
    std::string output_path;
    std::string ascii_chars = " .:-=+*#%@"; // Default character set
    bool color = false;                     // Enable color output using ANSI escape codes
    bool invert_color = false;              // Invert brightness mapping
    float brightness_boost = 1.0f;          // Multiplier for brightness
    float scale = 1.0f;                     // Scale factor for resizing the image
    bool detect_edges = false;              // Use edge magnitude instead of brightness
    float aspect_ratio = 2.0f;              // Aspect ratio of ASCII characters (width / height)
    bool auto_fit = true;                   // Automatically resize to fit terminal
};

// Information about a single pixel for character selection
struct PixelInfo
{
    uint64_t brightness = 0;                 // Grayscale brightness value of the pixel
    float edge_magnitude = 0.0f;             // Edge magnitude if edge detection is enabled
    std::vector<uint64_t> color = {0, 0, 0}; // RGB color values of the pixel
};

// --- Function Declarations ---

// Process an image based on provided parameters and generate ASCII art output
// params: Configuration parameters for the ASCII art generation
void processImage(const AsciiArtParams &params);

// Generate ASCII art as a string based on the processed image and parameters
// img: The Image struct containing pixel data
// params: Configuration parameters
// edge_magnitudes: Optional pointer to a vector of pre-calculated edge magnitudes (used if params.detect_edges is true)
// Returns: A string containing the generated ASCII art
std::string generateAsciiText(const Image &img, const AsciiArtParams &params, const std::vector<float> *edge_magnitudes);

// Calculate relevant information (brightness, color, edge_magnitude) for a single pixel
// img: The source image
// x, y: Coordinates of the pixel
// params: Configuration parameters
// edge_magnitudes: Optional pointer to edge magnitudes
// Returns: A PixelInfo struct containing the calculated information for the pixel
PixelInfo getPixelInfo(const Image &img, int x, int y, const AsciiArtParams &params, const std::vector<float> *edge_magnitudes);

// Select an ASCII character from the character set based on the pixel information (brightness or edge magnitude)
// pixel_info: Information about the pixel
// params: Configuration parameters (chars, invert_color)
// Returns: The selected ASCII character
char selectAsciiChar(const PixelInfo &pixel_info, const AsciiArtParams &params);

// Process video/GIF frame by frame using existing ASCII art pipeline
// videoFile: Path to video/GIF
// params: ASCII art parameters (same as used for static images)
// frameDelay: Delay between frames in milliseconds for animation effect
// Returns: true if processing successful, false otherwise
bool processVideo(const std::string &videoFile, const AsciiArtParams &params, int frameDelay = 100);

// Check if file is a supported video/GIF format
// filename: Input file path to check
// Returns: true if file extension matches supported video/GIF formats, false otherwise
bool isVideoFile(const std::string &filename);

// Convert OpenCV Mat to your Image structure for compatibility with existing pipeline
// mat: OpenCV Mat object containing frame data
// Returns: Image structure compatible with your existing ASCII art functions
Image matToImage(const cv::Mat &mat);