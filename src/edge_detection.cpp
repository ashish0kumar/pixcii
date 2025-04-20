#include "edge_detection.h"
#include "image.h"
#include <cmath>
#include <algorithm>
#include <vector>

// --- Constants for Sobel Operator ---
// Standard 3x3 Sobel kernel for detecting horizontal gradients
const int sobel_x[3][3] = {
    {-1, 0, 1},
    {-2, 0, 2},
    {-1, 0, 1}};

// Standard 3x3 Sobel kernel for detecting vertical gradients
const int sobel_y[3][3] = {
    {-1, -2, -1},
    {0, 0, 0},
    {1, 2, 1}};
// --- End Constants ---

// Perform Sobel edge detection on an image
// img: The input Image struct
// Returns: A vector of floats representing the magnitude of the gradient at each pixel
std::vector<float> detectEdges(const Image &img)
{
    // Convert the image to grayscale, as Sobel operates on single-channel images
    std::vector<uint8_t> gray = rgbToGrayscale(img);

    // Vector to store the calculated edge magnitude for each pixel
    // Initialized with size img.width * img.height and values 0.0f
    std::vector<float> edge_magnitude(img.width * img.height, 0.0f);

    // Iterate through the image pixels, excluding the border (1 pixel wide)
    // The Sobel operator is a 3x3 kernel, so it cannot be applied to the outermost pixels
    for (int y = 1; y < img.height - 1; y++)
    {
        for (int x = 1; x < img.width - 1; x++)
        {
            int gx = 0; // Horizontal gradient sum
            int gy = 0; // Vertical gradient sum

            // Apply the Sobel operator kernels (sobel_x and sobel_y) to the 3x3 neighborhood
            // around the current pixel (x, y)
            for (int ky = -1; ky <= 1; ky++) // Iterate over kernel rows relative to pixel
            {
                for (int kx = -1; kx <= 1; kx++) // Iterate over kernel columns relative to pixel
                {
                    // Get the grayscale value of the neighboring pixel
                    int pixel = gray[(y + ky) * img.width + (x + kx)];

                    // Accumulate weighted sums for horizontal and vertical gradients
                    gx += pixel * sobel_x[ky + 1][kx + 1];
                    gy += pixel * sobel_y[ky + 1][kx + 1];
                }
            }

            // Calculate the magnitude of the gradient using the Euclidean distance (sqrt(gx^2 + gy^2))
            edge_magnitude[y * img.width + x] = std::sqrt(static_cast<float>(gx * gx + gy * gy));
        }
    }

    // --- Normalization ---
    // Find the maximum edge magnitude in the result vector
    float max_mag = 0.0f;
    if (!edge_magnitude.empty())
    { // Prevent finding max on empty vector
        max_mag = *std::max_element(edge_magnitude.begin(), edge_magnitude.end());
    }

    // Normalize the edge magnitudes to the range [0, 255]
    // This scales the magnitudes so they can be used similarly to brightness values (0-255)
    if (max_mag > 0) // Avoid division by zero if no edges were detected
    {
        for (auto &mag : edge_magnitude)
        {
            mag = mag / max_mag * 255.0f;
        }
    }

    // Return the vector of normalized edge magnitudes
    return edge_magnitude;
}