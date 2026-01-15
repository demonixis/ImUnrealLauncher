# ImUnrealLauncher

A cross-platform launcher for managing and running Unreal Engine projects.

## Features

- **Engine Management**: Register multiple Unreal Engine installations with custom names
- **Project Management**: Add individual projects or scan folders for multiple projects
- **Master-Detail View**: Browse projects with icons and see detailed information
- **Project Operations**:
  - Clean: Remove generated folders (Binaries, Intermediate, Saved, etc.)
  - Generate: Generate project files for your IDE
  - Build: Compile the project
  - Run: Launch the Unreal Editor with the project
  - Package: Create builds for Windows, Linux, Mac, or Android

## Requirements

- CMake 3.20 or higher
- C++20 compatible compiler:
  - GCC 10+ (Linux)
  - Clang 12+ (macOS)
  - MSVC 2019+ (Windows)
- OpenGL 3.3+

### Platform-specific

**Linux:**
```bash
sudo apt install build-essential cmake libgl1-mesa-dev libxrandr-dev libxinerama-dev libxcursor-dev libxi-dev
```

**macOS:**
```bash
xcode-select --install
brew install cmake
```

**Windows:**
- Visual Studio 2019 or later with C++ workload
- CMake (included with Visual Studio or install separately)

## Building

```bash
# Clone the repository
git clone <repository-url>
cd unreal-launcher

# Create build directory
mkdir build && cd build

# Configure
cmake ..

# Build
cmake --build . --config Release
```

## Usage

1. **Configure Engine Versions**: Go to Settings > Engine Versions and add your Unreal Engine installations
2. **Add Projects**: Use File > Add Project or File > Add Projects from Folder
3. **Select a Project**: Click on a project in the list to see its details
4. **Run Operations**: Use the buttons to clean, generate, build, or run the project
5. **Package**: Select a target platform and click Package to create a distributable build

## Configuration

Configuration files are stored next to the executable:
- `engines.json`: Registered Unreal Engine versions
- `projects.json`: Added projects

## Project Icon

Place a PNG image with the same name as your `.uproject` file in the project directory to display a custom icon in the launcher.

Example: For `MyGame.uproject`, create `MyGame.png` in the same directory.

## License

MIT License - See LICENSE file for details.
