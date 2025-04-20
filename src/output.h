#pragma once

#include <string>
#include <vector>

// --- Function Declarations ---

// Saves a string of text to a specified file path.
// ascii_text: The string content to save.
// output_path: The path to the output file.
// Throws: std::runtime_error if the file cannot be opened for writing.
void saveOutputText(const std::string &ascii_text, const std::string &output_path);