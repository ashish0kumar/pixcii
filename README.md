<h1 align="center">pixcii</h1>

<p align="center">
A fast and feature-rich image-to-ASCII converter written in C++
</p>

<div align="center">
<img src="./assets/doom_color.png" width="75%" />
</div>

## Features

- Convert **images to ASCII art** with **customizable character sets**
- Supports **grayscale** and **ANSI-colored** output
- **Auto-fit** to terminal size for optimal viewing experience
- Adjustable **brightness**, **scaling**, and **aspect ratio** correction
- **Edge detection** using the **Sobel filter** for enhanced details
- Option to **invert brightness** for different visual effects
- **Fast** processing with **efficient** image handling

## Installation

### Prerequisites

Ensure you have the following installed:

- **C++11** or later
- **CMake** (>= 3.10)
- `stb_image.h` and `stb_image_write.h` (already included)

### Build Instructions

```sh
# Clone the repository
git clone https://github.com/ashish0kumar/pixcii.git
cd pixcii

# Create a build directory
mkdir build && cd build

# Configure and compile
cmake ..
make

# Run the program
./pixcii --help
```

**Move to a Directory in `$PATH`** <br> This allows you to run the program from
anywhere

```sh
mv pixcii ~/.local/bin/

# If ~/.local/bin/ is not in `$PATH:
export PATH="$HOME/.local/bin:$PATH"

# Usage
pixcii --help
```

## Usage

> [!NOTE]
> You **may need to zoom in or out** in your terminal (`Ctrl +/-`) to **properly
> view the default large-size ASCII art** <br> To avoid manual zooming, you can
> use the **optional -t flag to auto-adjust** the art to fit your terminal size.

### Command-Line Options

| Option                       | Description                                                   |
| ---------------------------- | ------------------------------------------------------------- |
| `-i, --input <path>`         | Path to the input image (required)                            |
| `-o, --output <path>`        | Path to save output ASCII art (optional)                      |
| `-t, --terminal-fit`         | Auto-fit image to current terminal size                       |
| `-c, --color`                | Enable colored ASCII output using ANSI escape codes           |
| `-s, --scale <float>`        | Resize image before conversion (default: 1.0)                 |
| `-a, --aspect-ratio <float>` | Adjust character aspect ratio (default: 2.0)                  |
| `-b, --brightness <float>`   | Adjust brightness multiplier (default: 1.0)                   |
| `-n, --invert`               | Invert brightness levels                                      |
| `-e, --edges`                | Use edge detection instead of brightness for ASCII conversion |
| `-m, --chars <string>`       | Custom ASCII character set (default: " .:-=+*#%@")            |
| `-h, --help`                 | Show help message                                             |

### Example Commands

```sh
# Convert an image to ASCII art and save to a file
pixcii -i input.jpg -o output.txt

# Auto-fit to terminal size
pixcii -i input.jpg -t

# Enable colored ASCII output
pixcii -i input.jpg -c

# Resize image before conversion
pixcii -i input.jpg -s 0.8

# Adjust character aspect ratio
pixcii -i input.jpg -a 3.0

# Adjust brightness multiplier
pixcii -i input.jpg -b 1.5

# Invert brightness levels
pixcii -i input.jpg -n

# Use edge detection
pixcii -i input.jpg -e

# Use a custom ASCII character set
pixcii -i input.jpg -m " .:-=+*#%@"

# Show help message
pixcii -h
```

### Example Outputs

| Normal Output                | Colored Output             |
| ---------------------------- | -------------------------- |
| ![Normal](./assets/norm.png) | ![Color](./assets/col.png) |

| Negative Output               | Edge Detection Output    |
| ----------------------------- | ------------------------ |
| ![Negative](./assets/neg.png) | ![Edge](./assets/ed.png) |

## Roadmap

- [x] Automatically detect the terminal size and scale the output to fit.
- [ ] Implement different character sets optimized for different scenarios (one
      for density, one for clarity, etc)
- [ ] Allow users to save and load parameter presets via config files.
- [ ] Optimize performance for larger images
- [ ] Video support

## Contributions

Contributions are welcome! If you'd like to improve the project, follow these
steps:

1. Fork the repository.
2. Create a new branch for your feature or bug fix.
3. Commit your changes with a clear message.
4. Push your branch and create a pull request.

Before submitting, ensure your code follows the project's coding style and is
well documented.

## License

[MIT](LICENSE)
