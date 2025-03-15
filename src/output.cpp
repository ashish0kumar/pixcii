#include "output.h"
#include "stb_image_wrapper.h"
#include <fstream>
#include <stdexcept>

// Save output as image
void saveOutputImage(const std::vector<uint8_t> &ascii_img, int width, int height, const std::string &output_path)
{
    stbi_write_png(output_path.c_str(), width, height, 3, ascii_img.data(), width * 3);
}

// Save output as text
void saveOutputText(const std::string &ascii_text, const std::string &output_path)
{
    std::ofstream file(output_path);
    if (!file.is_open())
    {
        throw std::runtime_error("Failed to open output file: " + output_path);
    }

    file << ascii_text;
    file.close();
}