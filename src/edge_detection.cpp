#include "edge_detection.h"
#include <cmath>
#include <algorithm>

// Sobel edge detection
std::vector<float> detectEdges(const Image &img)
{
    std::vector<uint8_t> gray = rgbToGrayscale(img);
    std::vector<float> edge_magnitude(img.width * img.height, 0.0f);

    // Simple 3x3 Sobel operator for edge detection
    const int sobel_x[3][3] = {
        {-1, 0, 1},
        {-2, 0, 2},
        {-1, 0, 1}};

    const int sobel_y[3][3] = {
        {-1, -2, -1},
        {0, 0, 0},
        {1, 2, 1}};

    for (int y = 1; y < img.height - 1; y++)
    {
        for (int x = 1; x < img.width - 1; x++)
        {
            int gx = 0, gy = 0;

            // Apply Sobel operator
            for (int ky = -1; ky <= 1; ky++)
            {
                for (int kx = -1; kx <= 1; kx++)
                {
                    int pixel = gray[(y + ky) * img.width + (x + kx)];
                    gx += pixel * sobel_x[ky + 1][kx + 1];
                    gy += pixel * sobel_y[ky + 1][kx + 1];
                }
            }

            // Calculate magnitude
            edge_magnitude[y * img.width + x] = std::sqrt(gx * gx + gy * gy);
        }
    }

    // Normalize edge magnitudes
    float max_mag = *std::max_element(edge_magnitude.begin(), edge_magnitude.end());
    if (max_mag > 0)
    {
        for (auto &mag : edge_magnitude)
        {
            mag = mag / max_mag * 255.0f;
        }
    }

    return edge_magnitude;
}