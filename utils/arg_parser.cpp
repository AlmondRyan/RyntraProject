#include "arg_parser.h"
#include <filesystem>
#include <iostream>

namespace Ryntra::Utils {
    CompilerOptions ArgumentParser::parseArguments(int argc, char *argv[]) {
        CompilerOptions options;
        if (argc < 2) {
            std::cerr << "Error: No arguments provided." << std::endl;
            printUsage(argv[0]);
            return options;
        }

        for (int i = 1; i < argc; i++) {
            std::string arg = argv[i];

            if (arg == "-h" || arg == "--help") {
                options.helpRequested = true;
                return options;
                // I don't know why ReSharper Engine told me this stupid warning...
                // ReSharper disable once CppRedundantElseKeywordInsideCompoundStatement
            } else if (arg == "-i") {
                if (i + 1 < argc) {
                    options.inputFile = argv[++i];
                } else {
                    std::cerr << "Error: -i option requires a filename." << std::endl;
                    return options;
                }
            } else if (arg == "-o") {
                if (i + 1 < argc) {
                    options.outputFile = argv[++i];
                } else {
                    std::cerr << "Error: -o option requires a filename." << std::endl;
                    return options;
                }
            } else if (arg == "--show-token") {
                options.showTokens = true;
            } else if (arg == "--show-ast") {
                options.showAST = true;
            } else if (arg == "--show-llvm-ir") {
                options.showLLVMIR = true;
            } else {
                std::cerr << "Error: Unknown option '" << arg << "'" << std::endl;
                printUsage(argv[0]);
                return options;
            }
        }

        if (options.inputFile.empty()) {
            std::cerr << "Error: Input file (-i) is required." << std::endl;
            printUsage(argv[0]);
            return options;
        }

        if (options.outputFile.empty()) {
            std::cerr << "Error: Output file (-o) is required." << std::endl;
            printUsage(argv[0]);
            return options;
        }

        if (!std::filesystem::exists(options.inputFile)) {
            std::cerr << "Error: Input file '" << options.inputFile << "' does not exist." << std::endl;
            return options;
        }

        options.valid = true;
        return options;
    }

    void ArgumentParser::printUsage(const std::string &programName) {
        std::cout << "Usage: " << programName << " -i <InputFile> -o <OutputFile> [options]" << std::endl;
        std::cout << "Use -h or --help for more information." << std::endl;
    }

    void ArgumentParser::printHelp(const std::string &programName) {
        std::cout << "Ryntra Compiler (rycc)" << std::endl;
        std::cout << "Copyright (c) 2025 Remimwen, all rights reserved." << std::endl;
        std::cout << std::endl;

        std::cout << "Usage: " << programName << " -i <input_file> -o <output_file> [options]" << std::endl;
        std::cout << std::endl;
        std::cout << "Required Arguments:" << std::endl;
        std::cout << "  -i <file>          Input Ryntra source file (.rynt)" << std::endl;
        std::cout << "  -o <file>          Output executable file name" << std::endl;
        std::cout << std::endl;
        std::cout << "Optional Arguments:" << std::endl;
        std::cout << "  --show-token       Display lexical analysis tokens" << std::endl;
        std::cout << "  --show-ast         Display abstract syntax tree" << std::endl;
        std::cout << "  --show-llvm-ir     Display generated LLVM IR code" << std::endl;
        std::cout << "  -h, --help         Show this help message" << std::endl;
        std::cout << std::endl;
    }

    bool ArgumentParser::isOption(const std::string &arg) {
        return !arg.empty() && arg[0] == '-';
    }

    std::string ArgumentParser::getFileNameWithoutExtension(const std::string &filePath) {
        const std::filesystem::path path(filePath);
        return path.stem().string();
    }
} // namespace Ryntra::Utils