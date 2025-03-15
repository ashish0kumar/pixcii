#include "image.h"
#include "ascii_art.h"
#include <iostream>
#include <string>
#include <stdexcept>

void displayHelp(const char *program_name)
{
    std::cout << "Usage: " << program_name << " [options]" << std::endl;
    std::cout << "Options:" << std::endl;
    std::cout << "  -i, --input <path>       Input image path" << std::endl;
    std::cout << "  -o, --output <path>      Output path (.txt)" << std::endl;
    std::cout << "  -c, --chars <string>     ASCII character set (default: \" .:-=+*#%@\")" << std::endl;
    std::cout << "  --color                  Use color in output (ANSI escape codes for terminal)" << std::endl;
    std::cout << "  --invert                 Invert brightness" << std::endl;
    std::cout << "  -b, --brightness <float> Brightness adjustment (default: 1.0)" << std::endl;
    std::cout << "  -s, --scale <float>      Scale factor (default: 1.0)" << std::endl;
    std::cout << "  --edges                  Detect edges instead of brightness" << std::endl;
    std::cout << "  --aspect-ratio <float>   Character aspect ratio (default: 0.5)" << std::endl;
    std::cout << "  -h, --help               Show this help message" << std::endl;
}

int main(int argc, char *argv[])
{
    try
    {
        AsciiArtParams params;

        // Simple command line parsing
        for (int i = 1; i < argc; i++)
        {
            std::string arg = argv[i];

            if (arg == "-i" || arg == "--input")
            {
                if (i + 1 < argc)
                {
                    params.input_path = argv[++i];
                }
            }
            else if (arg == "-o" || arg == "--output")
            {
                if (i + 1 < argc)
                {
                    params.output_path = argv[++i];
                }
            }
            else if (arg == "-c" || arg == "--chars")
            {
                if (i + 1 < argc)
                {
                    params.ascii_chars = argv[++i];
                }
            }
            else if (arg == "--color")
            {
                params.color = true;
            }
            else if (arg == "--invert")
            {
                params.invert_color = true;
            }
            else if (arg == "-b" || arg == "--brightness")
            {
                if (i + 1 < argc)
                {
                    params.brightness_boost = std::stof(argv[++i]);
                }
            }
            else if (arg == "-s" || arg == "--scale")
            {
                if (i + 1 < argc)
                {
                    params.scale = std::stof(argv[++i]);
                }
            }
            else if (arg == "--edges")
            {
                params.detect_edges = true;
            }
            else if (arg == "--aspect-ratio")
            {
                if (i + 1 < argc)
                {
                    params.aspect_ratio = std::stof(argv[++i]);
                }
            }
            else if (arg == "-h" || arg == "--help")
            {
                displayHelp(argv[0]);
                return 0;
            }
        }

        if (params.input_path.empty())
        {
            std::cerr << "Error: Input image path required" << std::endl;
            std::cerr << "Use -h or --help for usage information" << std::endl;
            return 1;
        }

        // Process the image
        processImage(params);
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}