#pragma once
#include "image.h"
#include <vector>

// --- Function Declarations ---

// Performs edge detection on an image using the Sobel operator.
// img: The input Image struct.
// Returns: A vector of floats where each element is the edge magnitude for the corresponding pixel.
std::vector<float> detectEdges(const Image &img);