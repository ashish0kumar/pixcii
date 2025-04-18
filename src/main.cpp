#include "image.h"
#include "ascii_art.h"
#include <iostream>
#include <string>
#include <stdexcept>

void displayHelp(const char *program_name)
{
    std::cout << "Usage: " << program_name << " [options]\n\n";
    std::cout << "Options:\n";
    std::cout << "  -i, --input <path>          Path to input image\n";
    std::cout << "  -o, --output <path>         Path to save output (.txt)\n";
    std::cout << "  -m, --chars <string>        ASCII character set (default: \" .:-=+*#%@\")\n";
    std::cout << "  -c, --color                 Enable colored output (ANSI escape codes)\n";
    std::cout << "  -n, --invert                Invert brightness\n";
    std::cout << "  -b, --brightness <float>    Adjust brightness (default: 1.0)\n";
    std::cout << "  -s, --scale <float>         Scale image (default: 1.0)\n";
    std::cout << "  -e, --edges                 Detect edges instead of brightness\n";
    std::cout << "  -a, --aspect-ratio <float>  Set character aspect ratio (default: 2.0)\n";
    std::cout << "  -t, --terminal-fit          Auto-fit image to terminal size\n";
    std::cout << "  -h, --help                  Show this help message\n";
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
            else if (arg == "-m" || arg == "--chars")
            {
                if (i + 1 < argc)
                {
                    params.ascii_chars = argv[++i];
                }
            }
            else if (arg == "-c" || arg == "--color")
            {
                params.color = true;
            }
            else if (arg == "-n" || arg == "--invert")
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
            else if (arg == "-e" || arg == "--edges")
            {
                params.detect_edges = true;
            }
            else if (arg == "-a" || arg == "--aspect-ratio")
            {
                if (i + 1 < argc)
                {
                    params.aspect_ratio = std::stof(argv[++i]);
                }
            }
            else if (arg == "-t" || arg == "--terminal-fit")
            {
                params.auto_fit = true;
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