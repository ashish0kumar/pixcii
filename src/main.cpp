#include "image.h"
#include "ascii_art.h"
#include <iostream>
#include <string>
#include <stdexcept>
#include <vector>
#include <filesystem>
#include <cstdlib>
#include <fstream>

// check if a string is a URL
bool isURL(const std::string &input)
{
    return input.find("http://") == 0 || input.find("https://") == 0;
}

// Function to get file extension from URL or filename
std::string getFileExtension(const std::string &path)
{
    size_t dotPos = path.find_last_of('.');
    size_t slashPos = path.find_last_of('/');
    size_t questionPos = path.find_last_of('?');

    if (dotPos != std::string::npos && dotPos > slashPos)
    {
        std::string ext;
        if (questionPos != std::string::npos && questionPos > dotPos)
        {
            ext = path.substr(dotPos, questionPos - dotPos);
        }
        else
        {
            ext = path.substr(dotPos);
        }

        // Validate extension doesn't contain path separators
        if (ext.find("/") == std::string::npos && ext.length() <= 5)
        {
            return ext;
        }
    }

    return ""; // No valid extension found
}

// Function to get content-type from URL using HTTP HEAD request
std::string getContentTypeFromURL(const std::string &url)
{
    std::string cmd = "curl -s -I \"" + url + "\" | grep -i '^Content-Type:'";
    std::array<char, 256> buffer;
    std::string result;

    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd.c_str(), "r"), pclose);
    if (!pipe)
    {
        return "";
    }

    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr)
    {
        result += buffer.data();
    }

    // Convert to lowercase for case-insensitive comparison
    std::transform(result.begin(), result.end(), result.begin(), ::tolower);

    // Map content-types to extensions
    if (result.find("image/jpeg") != std::string::npos)
    {
        return ".jpg";
    }
    else if (result.find("image/png") != std::string::npos)
    {
        return ".png";
    }
    else if (result.find("image/gif") != std::string::npos)
    {
        return ".gif";
    }
    else if (result.find("image/webp") != std::string::npos)
    {
        return ".webp";
    }
    else if (result.find("image/bmp") != std::string::npos)
    {
        return ".bmp";
    }
    else if (result.find("video/mp4") != std::string::npos)
    {
        return ".mp4";
    }
    else if (result.find("video/quicktime") != std::string::npos)
    {
        return ".mov";
    }
    else if (result.find("video/x-msvideo") != std::string::npos)
    {
        return ".avi";
    }
    else if (result.find("video/x-matroska") != std::string::npos)
    {
        return ".mkv";
    }
    else if (result.find("video/webm") != std::string::npos)
    {
        return ".webm";
    }

    return ""; // Unknown or unsupported type
}

// Function to download file from URL
std::string downloadFromURL(const std::string &url)
{
    // Generate temporary filename
    std::string extension;

    // First try to get extension from URL path
    extension = getFileExtension(url);

    // If no valid extension found in URL, use HTTP HEAD request
    if (extension.empty() || extension.find("/") != std::string::npos)
    {
        extension = getContentTypeFromURL(url);
    }

    // Final fallback
    if (extension.empty())
    {
        extension = ".jpg"; // Default to JPG for unknown image types
    }

    // Create proper temp file path
    std::string tempFile = std::filesystem::temp_directory_path().string() + "/pixcii_temp" + extension;

    // Try curl first, then wget as fallback
    std::string curlCommand = "curl -L -s -o \"" + tempFile + "\" \"" + url + "\"";
    std::string wgetCommand = "wget -q -O \"" + tempFile + "\" \"" + url + "\"";

    int result = std::system(curlCommand.c_str());
    if (result != 0)
    {
        result = std::system(wgetCommand.c_str());
        if (result != 0)
        {
            throw std::runtime_error("Failed to download file. Please ensure curl or wget is installed.");
        }
    }

    // Check if file was actually downloaded and has content
    std::ifstream file(tempFile, std::ios::binary | std::ios::ate);
    if (!file.is_open() || file.tellg() == 0)
    {
        std::filesystem::remove(tempFile);
        throw std::runtime_error("Downloaded file is empty or failed to download.");
    }
    file.close();

    return tempFile;
}

// Function to display the command-line usage help message
// program_name: The name of the executable (argv[0])
void displayHelp(const char *program_name)
{
    std::cout << "Usage: " << program_name << " -i <input> [options]\n\n";
    std::cout << "Required:\n";
    std::cout << "  -i, --input <path|url>      Path to input media file or URL\n";
    std::cout << "\n";
    std::cout << "Options:\n";
    std::cout << "  -o, --output <path>         Path to save output ASCII art\n";
    std::cout << "  -c, --color                 Enable colored ASCII output using ANSI escape codes\n";
    std::cout << "  -g, --original              Display media at original resolution\n";
    std::cout << "  -s, --scale <float>         Scale media (default: 1.0) (ignored unless --original is used)\n";
    std::cout << "  -a, --aspect-ratio <float>  Set character aspect ratio (default: 2.0)\n";
    std::cout << "  -b, --brightness <float>    Adjust brightness multiplier (default: 1.0)\n";
    std::cout << "  -n, --invert                Invert brightness mapping\n";
    std::cout << "  -e, --edges                 Detect edges instead of brightness for character selection\n";
    std::cout << "  -m, --chars <string>        ASCII character set (default: \" .:-=+*#%@\")\n";
    std::cout << "  -d, --delay <ms>            Frame delay in milliseconds for videos (default: auto)\n";
    std::cout << "  -h, --help                  Show this help message\n";
    std::cout << "\n";
    std::cout << "Examples:\n";
    std::cout << "  " << program_name << " -i image.jpg -c\n";
    std::cout << "  " << program_name << " -i video.mp4\n";
    std::cout << "  " << program_name << " -i https://example.com/image.jpg -c\n";
    std::cout << "  " << program_name << " -i large_image.png -g -s 0.5\n";
}

// Main function - entry point of the program
// Handles command-line argument parsing and calls the appropriate processing function
int main(int argc, char *argv[])
{
    AsciiArtParams params; // Create a struct to hold parsed parameters
    int frameDelay = 100;
    std::string originalInput;
    std::string tempFile;
    bool isTemporaryFile = false;

    try
    {
        // --- Command Line Parsing ---
        for (int i = 1; i < argc; i++) // Start from 1 to skip the program name (argv[0])
        {
            std::string arg = argv[i]; // Get the current argument as a string

            // Check for known options
            if (arg == "-i" || arg == "--input")
            {
                // Check if the next argument exists (the path)
                if (i + 1 < argc)
                {
                    originalInput = argv[++i]; // Store original input

                    // Check if input is a URL
                    if (isURL(originalInput))
                    {
                        try
                        {
                            tempFile = downloadFromURL(originalInput);
                            params.input_path = tempFile;
                            isTemporaryFile = true;
                        }
                        catch (const std::exception &e)
                        {
                            std::cerr << "Error downloading from URL: " << e.what() << std::endl;
                            return 1;
                        }
                    }
                    else
                    {
                        params.input_path = originalInput;
                    }
                }
                else
                {
                    // Error if option requires an argument but none is provided
                    std::cerr << "Error: Option '" << arg << "' requires an argument (input file path or URL)." << std::endl;
                    displayHelp(argv[0]);
                    return 1;
                }
            }
            else if (arg == "-o" || arg == "--output")
            {
                if (i + 1 < argc)
                {
                    params.output_path = argv[++i]; // Assign and advance
                }
                else
                {
                    std::cerr << "Error: Option '" << arg << "' requires an argument (output file path)." << std::endl;
                    displayHelp(argv[0]);
                    return 1;
                }
            }
            else if (arg == "-m" || arg == "--chars")
            {
                if (i + 1 < argc)
                {
                    params.ascii_chars = argv[++i]; // Assign and advance
                }
                else
                {
                    std::cerr << "Error: Option '" << arg << "' requires an argument (character set string)." << std::endl;
                    displayHelp(argv[0]);
                    return 1;
                }
            }
            else if (arg == "-b" || arg == "--brightness")
            {
                if (i + 1 < argc)
                {
                    try
                    {                                                   // Use try-catch for stof to handle invalid float input
                        params.brightness_boost = std::stof(argv[++i]); // Convert next arg to float and assign
                    }
                    catch (const std::invalid_argument &ia)
                    {
                        std::cerr << "Error: Invalid argument for option '" << arg << "'. Expected a number." << std::endl;
                        displayHelp(argv[0]);
                        if (isTemporaryFile && !tempFile.empty())
                        {
                            std::filesystem::remove(tempFile);
                        }
                        return 1;
                    }
                    catch (const std::out_of_range &oor)
                    {
                        std::cerr << "Error: Argument for option '" << arg << "' out of float range." << std::endl;
                        displayHelp(argv[0]);
                        if (isTemporaryFile && !tempFile.empty())
                        {
                            std::filesystem::remove(tempFile);
                        }
                        return 1;
                    }
                }
                else
                {
                    std::cerr << "Error: Option '" << arg << "' requires an argument (brightness multiplier)." << std::endl;
                    displayHelp(argv[0]);
                    if (isTemporaryFile && !tempFile.empty())
                    {
                        std::filesystem::remove(tempFile);
                    }
                    return 1;
                }
            }
            else if (arg == "-s" || arg == "--scale")
            {
                if (i + 1 < argc)
                {
                    try
                    {
                        params.scale = std::stof(argv[++i]); // Convert next arg to float and assign
                    }
                    catch (const std::invalid_argument &ia)
                    {
                        std::cerr << "Error: Argument for option '" << arg << "'. Expected a number." << std::endl;
                        displayHelp(argv[0]);
                        if (isTemporaryFile && !tempFile.empty())
                        {
                            std::filesystem::remove(tempFile);
                        }
                        return 1;
                    }
                    catch (const std::out_of_range &oor)
                    {
                        std::cerr << "Error: Argument for option '" << arg << "' out of float range." << std::endl;
                        displayHelp(argv[0]);
                        if (isTemporaryFile && !tempFile.empty())
                        {
                            std::filesystem::remove(tempFile);
                        }
                        return 1;
                    }
                }
                else
                {
                    std::cerr << "Error: Option '" << arg << "' requires an argument (scale factor)." << std::endl;
                    displayHelp(argv[0]);
                    if (isTemporaryFile && !tempFile.empty())
                    {
                        std::filesystem::remove(tempFile);
                    }
                    return 1;
                }
            }
            else if (arg == "-a" || arg == "--aspect-ratio")
            {
                if (i + 1 < argc)
                {
                    try
                    {
                        params.aspect_ratio = std::stof(argv[++i]); // Convert next arg to float and assign
                    }
                    catch (const std::invalid_argument &ia)
                    {
                        std::cerr << "Error: Argument for option '" << arg << "'. Expected a number." << std::endl;
                        displayHelp(argv[0]);
                        if (isTemporaryFile && !tempFile.empty())
                        {
                            std::filesystem::remove(tempFile);
                        }
                        return 1;
                    }
                    catch (const std::out_of_range &oor)
                    {
                        std::cerr << "Error: Argument for option '" << arg << "' out of float range." << std::endl;
                        displayHelp(argv[0]);
                        if (isTemporaryFile && !tempFile.empty())
                        {
                            std::filesystem::remove(tempFile);
                        }
                        return 1;
                    }
                }
                else
                {
                    std::cerr << "Error: Option '" << arg << "' requires an argument (aspect ratio value)." << std::endl;
                    displayHelp(argv[0]);
                    if (isTemporaryFile && !tempFile.empty())
                    {
                        std::filesystem::remove(tempFile);
                    }
                    return 1;
                }
            }
            else if (arg == "-d" || arg == "--delay")
            {
                if (i + 1 < argc)
                {
                    try
                    {
                        frameDelay = std::stoi(argv[++i]); // Convert next arg to int and assign
                        if (frameDelay < 0)
                        {
                            std::cerr << "Error: Frame delay must be non-negative." << std::endl;
                            displayHelp(argv[0]);
                            if (isTemporaryFile && !tempFile.empty())
                            {
                                std::filesystem::remove(tempFile);
                            }
                            return 1;
                        }
                    }
                    catch (const std::invalid_argument &ia)
                    {
                        std::cerr << "Error: Invalid argument for option '" << arg << "'. Expected an integer." << std::endl;
                        displayHelp(argv[0]);
                        if (isTemporaryFile && !tempFile.empty())
                        {
                            std::filesystem::remove(tempFile);
                        }
                        return 1;
                    }
                    catch (const std::out_of_range &oor)
                    {
                        std::cerr << "Error: Argument for option '" << arg << "' out of integer range." << std::endl;
                        displayHelp(argv[0]);
                        if (isTemporaryFile && !tempFile.empty())
                        {
                            std::filesystem::remove(tempFile);
                        }
                        return 1;
                    }
                }
                else
                {
                    std::cerr << "Error: Option '" << arg << "' requires an argument (delay in milliseconds)." << std::endl;
                    displayHelp(argv[0]);
                    if (isTemporaryFile && !tempFile.empty())
                    {
                        std::filesystem::remove(tempFile);
                    }
                    return 1;
                }
            }
            // Boolean flags
            else if (arg == "-g" || arg == "--original")
            {
                params.auto_fit = false; // Disable auto-fit for original sizing
            }
            else if (arg == "-c" || arg == "--color")
            {
                params.color = true; // Set the color flag
            }
            else if (arg == "-n" || arg == "--invert")
            {
                params.invert_color = true; // Set the invert flag
            }
            else if (arg == "-e" || arg == "--edges")
            {
                params.detect_edges = true; // Set the detect_edges flag
            }
            else if (arg == "-h" || arg == "--help")
            {
                displayHelp(argv[0]); // Display help and exit
                return 0;
            }
            else
            {
                // Handle unknown arguments
                std::cerr << "Error: Unknown argument '" << arg << "'." << std::endl;
                displayHelp(argv[0]);
                if (isTemporaryFile && !tempFile.empty())
                {
                    std::filesystem::remove(tempFile);
                }
                return 1;
            }
        }
        // --- End Command Line Parsing ---

        // --- Parameter Validation ---
        // Check if the required input path was provided
        if (params.input_path.empty())
        {
            std::cerr << "Error: Input file path or URL is required (--input or -i option)." << std::endl;
            std::cerr << "Use -h or --help for usage information." << std::endl;
            if (isTemporaryFile && !tempFile.empty())
            {
                std::filesystem::remove(tempFile);
            }
            return 1;
        }
        // Check if the character set is empty
        if (params.ascii_chars.empty())
        {
            std::cerr << "Error: ASCII character set cannot be empty (--chars or -m option)." << std::endl;
            if (isTemporaryFile && !tempFile.empty())
            {
                std::filesystem::remove(tempFile);
            }
            return 1;
        }
        // Check if scale or aspect ratio are non-positive
        if (params.scale <= 0)
        {
            std::cerr << "Error: Scale factor (--scale or -s) must be positive." << std::endl;
            if (isTemporaryFile && !tempFile.empty())
            {
                std::filesystem::remove(tempFile);
            }
            return 1;
        }
        if (params.aspect_ratio <= 0)
        {
            std::cerr << "Error: Aspect ratio (--aspect-ratio or -a) must be positive." << std::endl;
            if (isTemporaryFile && !tempFile.empty())
            {
                std::filesystem::remove(tempFile);
            }
            return 1;
        }
        // --- End Parameter Validation ---

        // --- File Type Detection and Processing ---
        // Determine if input is video/GIF or static image and process accordingly
        if (isVideoFile(params.input_path))
        {
            // Process video/GIF
            if (!processVideo(params.input_path, params, frameDelay))
            {
                std::cerr << "Error: Failed to process video file." << std::endl;
                if (isTemporaryFile && !tempFile.empty())
                {
                    std::filesystem::remove(tempFile);
                }
                return 1;
            }
        }
        else
        {
            // Process static image using existing function
            processImage(params);
        }
        // --- End File Type Detection and Processing ---

        // Clean up temporary file if it was downloaded
        if (isTemporaryFile && !tempFile.empty())
        {
            std::filesystem::remove(tempFile);
        }
    }
    catch (const std::exception &e)
    {
        // Catch and print any exceptions thrown during execution (e.g., file loading errors)
        std::cerr << "An error occurred: " << e.what() << std::endl;

        // Clean up temporary file if it exists
        if (isTemporaryFile && !tempFile.empty())
        {
            std::filesystem::remove(tempFile);
        }
        return 1;
    }
    catch (...)
    {
        // Catch any other unexpected exceptions
        std::cerr << "An unknown error occurred." << std::endl;

        // Clean up temporary file if it exists
        if (isTemporaryFile && !tempFile.empty())
        {
            std::filesystem::remove(tempFile);
        }
        return 1;
    }

    return 0;
}