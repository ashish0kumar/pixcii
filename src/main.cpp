#include "image.h"
#include "ascii_art.h"
#include <iostream>
#include <string>
#include <stdexcept>
#include <vector>

// Function to display the command-line usage help message
// program_name: The name of the executable (argv[0])
void displayHelp(const char *program_name)
{
    std::cout << "Usage: " << program_name << " -i <input> [options]\n\n";
    std::cout << "Required:\n";
    std::cout << "  -i, --input <path>          Path to input media file\n";
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
    std::cout << "  " << program_name << " -i image.jpg\n";
    std::cout << "  " << program_name << " -i video.mp4 -c\n";
    std::cout << "  " << program_name << " -i animation.gif -d 200\n";
    std::cout << "  " << program_name << " -i large_image.png -g -s 0.5\n";
}

// Main function - entry point of the program
// Handles command-line argument parsing and calls the appropriate processing function
int main(int argc, char *argv[])
{
    try
    {
        AsciiArtParams params; // Create a struct to hold parsed parameters
        int frameDelay = 100;

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
                    params.input_path = argv[++i]; // Assign the next argument as the input path and advance i
                }
                else
                {
                    // Error if option requires an argument but none is provided
                    std::cerr << "Error: Option '" << arg << "' requires an argument (input file path)." << std::endl;
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
                        return 1;
                    }
                    catch (const std::out_of_range &oor)
                    {
                        std::cerr << "Error: Argument for option '" << arg << "' out of float range." << std::endl;
                        displayHelp(argv[0]);
                        return 1;
                    }
                }
                else
                {
                    std::cerr << "Error: Option '" << arg << "' requires an argument (brightness multiplier)." << std::endl;
                    displayHelp(argv[0]);
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
                        return 1;
                    }
                    catch (const std::out_of_range &oor)
                    {
                        std::cerr << "Error: Argument for option '" << arg << "' out of float range." << std::endl;
                        displayHelp(argv[0]);
                        return 1;
                    }
                }
                else
                {
                    std::cerr << "Error: Option '" << arg << "' requires an argument (scale factor)." << std::endl;
                    displayHelp(argv[0]);
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
                        return 1;
                    }
                    catch (const std::out_of_range &oor)
                    {
                        std::cerr << "Error: Argument for option '" << arg << "' out of float range." << std::endl;
                        displayHelp(argv[0]);
                        return 1;
                    }
                }
                else
                {
                    std::cerr << "Error: Option '" << arg << "' requires an argument (aspect ratio value)." << std::endl;
                    displayHelp(argv[0]);
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
                            return 1;
                        }
                    }
                    catch (const std::invalid_argument &ia)
                    {
                        std::cerr << "Error: Invalid argument for option '" << arg << "'. Expected an integer." << std::endl;
                        displayHelp(argv[0]);
                        return 1;
                    }
                    catch (const std::out_of_range &oor)
                    {
                        std::cerr << "Error: Argument for option '" << arg << "' out of integer range." << std::endl;
                        displayHelp(argv[0]);
                        return 1;
                    }
                }
                else
                {
                    std::cerr << "Error: Option '" << arg << "' requires an argument (delay in milliseconds)." << std::endl;
                    displayHelp(argv[0]);
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
                return 1;
            }
        }
        // --- End Command Line Parsing ---

        // --- Parameter Validation ---
        // Check if the required input path was provided
        if (params.input_path.empty())
        {
            std::cerr << "Error: Input file path is required (--input or -i option)." << std::endl;
            std::cerr << "Use -h or --help for usage information." << std::endl;
            return 1;
        }
        // Check if the character set is empty
        if (params.ascii_chars.empty())
        {
            std::cerr << "Error: ASCII character set cannot be empty (--chars or -m option)." << std::endl;
            return 1;
        }
        // Check if scale or aspect ratio are non-positive
        if (params.scale <= 0)
        {
            std::cerr << "Error: Scale factor (--scale or -s) must be positive." << std::endl;
            return 1;
        }
        if (params.aspect_ratio <= 0)
        {
            std::cerr << "Error: Aspect ratio (--aspect-ratio or -a) must be positive." << std::endl;
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
                return 1;
            }
        }
        else
        {
            // Process static image using existing function
            processImage(params);
        }
        // --- End File Type Detection and Processing ---
    }
    catch (const std::exception &e)
    {
        // Catch and print any exceptions thrown during execution (e.g., file loading errors)
        std::cerr << "An error occurred: " << e.what() << std::endl;
        return 1;
    }
    catch (...)
    {
        // Catch any other unexpected exceptions
        std::cerr << "An unknown error occurred." << std::endl;
        return 1;
    }

    return 0;
}