#include "output.h"
#include <fstream>
#include <stdexcept>

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