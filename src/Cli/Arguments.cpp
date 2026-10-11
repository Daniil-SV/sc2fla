#include "Cli/Arguments.h"

#include "Core/Console.h"

#include <cstdlib>
#include <iostream>
#include <stdexcept>

namespace sc2fla {
    Arguments ParseArguments(const std::vector<std::string>& args) {
        Arguments result;

        for (size_t i = 0; i < args.size(); ++i) {
            const std::string& a = args[i];
            auto next = [&](const char* flag) -> std::string {
                if (i + 1 >= args.size()) {
                    Console::Error(std::string(flag) + " requires a value");
                    std::exit(1);
                }
                return args[++i];
            };

            if (a == "-h" || a == "--help")
                result.help = true;
            else if (a == "-t" || a == "--texture")
                result.texture = next("-t/--texture");
            else if (a == "-o" || a == "--output")
                result.output = next("-o/--output");
            else if (a == "-g" || a == "--group")
                result.groups.push_back(next("-g/--group"));
            else if (a == "-x" || a == "--xfl")
                result.xfl = true;
            else if (a == "-f" || a == "--fps")
                result.fps = next("-f/--fps");
            else if (!a.empty() && a[0] == '-')
                Console::Warning("Unknown argument: " + a);
            else if (result.input.empty())
                result.input = a;
            else
                Console::Warning("Unexpected extra argument: " + a);
        }

        return result;
    }

    bool ParseFps(const std::string& text, std::optional<int>& fps) {
        if (text.empty())
            return true;

        try {
            size_t consumed = 0;
            int parsed = std::stoi(text, &consumed);
            if (consumed != text.size() || parsed <= 0)
                throw std::invalid_argument("");
            fps = parsed;
        } catch (...) {
            Console::Error("-f/--fps expects a positive whole number, got: " + text);
            return false;
        }
        return true;
    }

    void PrintHelp() {
        std::cout << "usage: sc2fla input [args]\n\n"
                     "args:\n"
                     "  -h,  --help            show this message\n"
                     "  -g,  --group <name>    convert only matching exports\n"
                     "  -t,  --texture <file>  use an external texture\n"
                     "  -o,  --output <dir>    output directory\n"
                     "  -x,  --xfl             write an unpacked .xfl folder\n"
                     "  -f,  --fps <number>    set the frame rate\n";
    }
}
