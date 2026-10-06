#include "Cli/Arguments.h"
#include "Cli/Inputs.h"
#include "Convert/ScToFla.h"
#include "Core/Console.h"

#include <chrono>
#include <filesystem>
#include <iomanip>
#include <iostream>

namespace fs = std::filesystem;
using namespace sc2fla;

int main(int argc, char** argv) {
    std::vector<std::string> args(argv + 1, argv + argc);

    if (args.empty()) {
        PrintHelp();
        return 0;
    }

    const Arguments arguments = ParseArguments(args);
    if (arguments.help || arguments.input.empty()) {
        PrintHelp();
        return 0;
    }

    const auto start = std::chrono::steady_clock::now();

    const fs::path input = fs::absolute(arguments.input);

    ConvertOptions options;
    options.groups = arguments.groups;
    options.unpacked = arguments.xfl;
    if (!arguments.texture.empty())
        options.external_texture = fs::absolute(arguments.texture);
    if (!ParseFps(arguments.fps, options.fps))
        return 1;

    std::vector<fs::path> files;
    if (!CollectInputs(input, files))
        return 1;

    ConversionContext ctx;
    ctx.extend_single_frame_clips = arguments.extend_frames;
    ctx.animated_clips_play = !arguments.graphic_only;

    for (auto& file : files) {
        const fs::path out_dir = arguments.output.empty() ? file.parent_path() : fs::path(arguments.output);
        std::error_code ec;
        fs::create_directories(out_dir, ec);

        try {
            ScToFla(file, out_dir, ctx, options);
        } catch (const std::exception& e) {
            Console::Error("Failed to process " + file.filename().string() + ": " + e.what());
        }
    }

    const double elapsed = std::chrono::duration<double>(std::chrono::steady_clock::now() - start).count();
    std::cout << "Done in " << std::fixed << std::setprecision(3) << elapsed << " seconds." << std::endl;
    return 0;
}
