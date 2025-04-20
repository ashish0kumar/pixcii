#include "output.h"
#include <fstream>
#include <stdexcept>
#include <string>

// Save the generated ASCII text to a file.
// ascii_text: The string containing the ASCII art.
// output_path: The path to the file where the text should be saved.
// Throws: std::runtime_error if the file cannot be opened for writing.
void saveOutputText(const std::string &ascii_text, const std::string &output_path)
{
    // Create an ofstream object to write to the file
    std::ofstream file(output_path);

    // Check if the file was successfully opened
    if (!file.is_open())
    {
        // If not, throw a runtime_error
        throw std::runtime_error("Failed to open output file for writing: " + output_path);
    }

    // Write the entire ASCII text string to the file
    file << ascii_text;

    // Close the file stream. This is important to ensure data is written and resources are released.
    // The file is also automatically closed when the ofstream object goes out of scope.
    file.close();
}