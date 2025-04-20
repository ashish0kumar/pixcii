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
    std::cout << "Usage: " << program_name << " [options]\n\n";
    std::cout << "Options:\n";
    std::cout << "  -i, --input <path>          Path to input image (required)\n";
    std::cout << "  -o, --output <path>         Path to save output (.txt) (optional)\n";
    std::cout << "  -t, --terminal-fit          Auto-fit image to terminal size\n";
    std::cout << "  -c, --color                 Enable colored output (ANSI escape codes)\n";
    std::cout << "  -s, --scale <float>         Scale image (default: 1.0) (ignored if -t is used)\n";
    std::cout << "  -a, --aspect-ratio <float>  Set character aspect ratio (default: 2.0)\n";
    std::cout << "  -b, --brightness <float>    Adjust brightness multiplier (default: 1.0)\n";
    std::cout << "  -n, --invert                Invert brightness mapping\n";
    std::cout << "  -e, --edges                 Detect edges instead of brightness for character selection\n";
    std::cout << "  -m, --chars <string>        ASCII character set (default: \" .:-=+*#%@\")\n";
    std::cout << "  -h, --help                  Show this help message\n";
}

// Main function - entry point of the program
// Handles command-line argument parsing and calls the main processing function
int main(int argc, char *argv[])
{
    try
    {
        AsciiArtParams params; // Create a struct to hold parsed parameters

        // --- Simple Command Line Parsing ---
        // This is a basic manual implementation with checks for missing/unknown arguments.
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
            else if (arg == "-t" || arg == "--terminal-fit")
            {
                params.auto_fit = true; // Set the auto_fit flag
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
        // --- End Simple Command Line Parsing ---

        // --- Parameter Validation ---
        // Check if the required input path was provided
        if (params.input_path.empty())
        {
            std::cerr << "Error: Input image path is required (--input or -i option)." << std::endl;
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

        // Process the image using the parsed parameters
        processImage(params);
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