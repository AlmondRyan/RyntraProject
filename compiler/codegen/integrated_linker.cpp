#include "integrated_linker.h"
#include "../../utils/ErrorHandler.h"
#include <cstdlib>
#include <iostream>
#include <lld/Common/CommonLinkerContext.h>
#include <lld/Common/Driver.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/Program.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/TargetParser/Host.h>

namespace Ryntra::Compiler {
    struct TargetInfo {
        enum Platform { Windows,
                        Linux,
                        MacOS,
                        Unknown };

        enum Architecture { X86_64,
                            X86,
                            ARM64,
                            ARM,
                            Unknown_Arch };

        enum ABI { MSVC,
                   GNU,
                   Unknown_ABI };

        Platform     platform = Unknown;
        Architecture arch = Unknown_Arch;
        ABI          abi = Unknown_ABI;

        [[nodiscard]] bool needsWindowsAPI() const { return platform == Windows; }
        [[nodiscard]] bool needsMSVCRuntime() const { return platform == Windows && abi == MSVC; }
        [[nodiscard]] bool needsGNURuntime() const { return abi == GNU; }
        [[nodiscard]] bool needsUnixLibc() const { return platform == Linux || platform == MacOS; }
    };

    static TargetInfo parseTargetTriple(const std::string &triple) {
        TargetInfo info;

        std::cout << "Analyzing target triple: " << triple << std::endl;

        if (triple.find("x86_64") != std::string::npos || triple.find("amd64") != std::string::npos) {
            info.arch = TargetInfo::X86_64;
        } else if (triple.find("i386") != std::string::npos || triple.find("i686") != std::string::npos) {
            info.arch = TargetInfo::X86;
        } else if (triple.find("aarch64") != std::string::npos || triple.find("arm64") != std::string::npos) {
            info.arch = TargetInfo::ARM64;
        } else if (triple.find("arm") != std::string::npos) {
            info.arch = TargetInfo::ARM;
        }

        if (triple.find("windows") != std::string::npos || triple.find("win32") != std::string::npos) {
            info.platform = TargetInfo::Windows;
        } else if (triple.find("linux") != std::string::npos) {
            info.platform = TargetInfo::Linux;
        } else if (triple.find("darwin") != std::string::npos || triple.find("macos") != std::string::npos) {
            info.platform = TargetInfo::MacOS;
        }

        if (triple.find("msvc") != std::string::npos) {
            info.abi = TargetInfo::MSVC;
        } else if (triple.find("gnu") != std::string::npos || triple.find("mingw") != std::string::npos) {
            info.abi = TargetInfo::GNU;
        }

        std::cout << "Target analysis:" << std::endl;
        std::cout << "  Platform: " << (info.platform == TargetInfo::Windows ? "Windows" : info.platform == TargetInfo::Linux ? "Linux"
                                                                                       : info.platform == TargetInfo::MacOS   ? "macOS"
                                                                                                                              : "Unknown")
                  << std::endl;
        std::cout << "  Architecture: " << (info.arch == TargetInfo::X86_64 ? "x86_64" : info.arch == TargetInfo::X86 ? "x86"
                                                                                                                      : "Other")
                  << std::endl;
        std::cout << "  ABI: " << (info.abi == TargetInfo::MSVC ? "MSVC" : info.abi == TargetInfo::GNU ? "GNU"
                                                                                                       : "Unknown")
                  << std::endl;

        std::cout << "Required libraries:" << std::endl;
        if (info.needsWindowsAPI())
            std::cout << "  - Windows API (kernel32)" << std::endl;
        if (info.needsMSVCRuntime())
            std::cout << "  - MSVC Runtime (msvcrt)" << std::endl;
        if (info.needsUnixLibc())
            std::cout << "  - Unix libc" << std::endl;

        return info;
    }

    static std::vector<std::string> getPlatformLibraries() {
        std::vector<std::string> libs;

        std::string hostTriple = llvm::sys::getDefaultTargetTriple();

        if (hostTriple.find("windows") != std::string::npos ||
            hostTriple.find("win32") != std::string::npos ||
            hostTriple.find("msvc") != std::string::npos) {
            libs.emplace_back("kernel32.lib");
            libs.emplace_back("user32.lib");
        } else if (hostTriple.find("linux") != std::string::npos) {
            libs.emplace_back("-lc");
            libs.emplace_back("-ldl");
        } else if (hostTriple.find("darwin") != std::string::npos ||
                   hostTriple.find("macos") != std::string::npos) {
            libs.emplace_back("-lSystem");
        }

        return libs;
    }

    static std::string findWindowsSDKLibPath() {
        // TODO: Make finding sdk dynamically instead of make it static embedded.
        const std::vector<std::string> sdkPaths = {
            R"(C:\Program Files (x86)\Windows Kits\10\Lib)",
            R"(C:\Program Files\Windows Kits\10\Lib)",
            R"(C:\Program Files (x86)\Microsoft SDKs\Windows\v7.1A\Lib\x64)",
            R"(C:\Program Files\Microsoft SDKs\Windows\v7.1A\Lib\x64)"};

        for (const auto &basePath : sdkPaths) {
            std::error_code ec;
            if (llvm::sys::fs::exists(basePath)) {
                std::vector<std::string> versions = {"10.0.22621.0", "10.0.19041.0", "10.0.18362.0", "10.0.17763.0"};
                for (const auto &version : versions) {
                    if (std::string fullPath = basePath + "\\" + version + "\\um\\x64"; llvm::sys::fs::exists(fullPath + "\\kernel32.lib")) {
                        return fullPath;
                    }
                }
                std::string directPath = basePath + "\\x64";
                if (llvm::sys::fs::exists(directPath + "\\kernel32.lib")) {
                    return directPath;
                }
            }
        }

        return ""; // Not found
    }

    static std::vector<std::string> getPlatformLinkerArgs(const std::string &outputFile, const std::string &objFile) {
        std::vector<std::string> args;
        std::string              hostTriple = llvm::sys::getDefaultTargetTriple();
        TargetInfo               target = parseTargetTriple(hostTriple);

        if (target.platform == TargetInfo::Windows) {
            args.push_back("/OUT:" + outputFile);
            args.push_back(objFile);

            if (target.needsWindowsAPI() || target.needsMSVCRuntime()) {
                std::string sdkPath = findWindowsSDKLibPath();
                if (!sdkPath.empty()) {
                    args.push_back("/LIBPATH:\"" + sdkPath + "\"");
                    std::cout << "Using Windows SDK: " << sdkPath << std::endl;
                } else {
                    std::cout << "Warning: Windows SDK not found, using system default paths" << std::endl;
                }
            }

            if (target.needsWindowsAPI()) {
                args.emplace_back("kernel32.lib");
            }

            if (target.needsMSVCRuntime()) {
                args.push_back("msvcrt.lib");
            }

            // Windows-specific linker settings
            args.emplace_back("/SUBSYSTEM:CONSOLE");
            args.emplace_back("/ENTRY:mainCRTStartup");

            // Set machine type based on architecture
            if (target.arch == TargetInfo::X86_64) {
                args.push_back("/MACHINE:X64");
            } else if (target.arch == TargetInfo::X86) {
                args.push_back("/MACHINE:X86");
            }

        } else if (target.platform == TargetInfo::Linux) {
            // Linux linking
            args.push_back("-o");
            args.push_back(outputFile);
            args.push_back(objFile);

            if (target.needsUnixLibc()) {
                args.push_back("-lc");
            }

        } else if (target.platform == TargetInfo::MacOS) {
            // macOS linking
            args.push_back("-o");
            args.push_back(outputFile);
            args.push_back(objFile);
            args.push_back("-lSystem");

        } else {
            // Unknown platform - use generic approach
            std::cout << "Warning: Unknown target platform, using generic linking" << std::endl;
            args.push_back("-o");
            args.push_back(outputFile);
            args.push_back(objFile);
        }

        return args;
    }

    bool IntegratedLinker::linkExecutable(
        const std::string              &objFilename,
        const std::string              &exeFilename,
        const std::vector<std::string> &libraries,
        const std::vector<std::string> &libraryPaths) {

        std::cout << "Starting integrated linking process..." << std::endl;

        std::string outputFile = exeFilename;
        if (outputFile.find(".exe") == std::string::npos) {
            outputFile += ".exe";
        }

        std::string errorOutput;

        std::string hostTriple = llvm::sys::getDefaultTargetTriple();
        std::cout << "Target triple: " << hostTriple << std::endl;

        std::vector<std::string> lldArgs;

        if (hostTriple.find("windows") != std::string::npos ||
            hostTriple.find("win32") != std::string::npos ||
            hostTriple.find("msvc") != std::string::npos) {
            lldArgs.push_back("lld-link");
            lldArgs = getPlatformLinkerArgs(outputFile, objFilename);
            lldArgs.insert(lldArgs.begin(), "lld-link");
        } else {
            lldArgs.push_back("ld.lld");
            auto platformArgs = getPlatformLinkerArgs(outputFile, objFilename);
            lldArgs.insert(lldArgs.end(), platformArgs.begin(), platformArgs.end());
        }

        // Add custom libraries
        for (const auto &lib : libraries) {
            if (hostTriple.find("windows") != std::string::npos) {
                lldArgs.push_back("/DEFAULTLIB:" + lib);
            } else {
                lldArgs.push_back("-l" + lib);
            }
        }

        // Add library paths
        for (const auto &path : libraryPaths) {
            if (hostTriple.find("windows") != std::string::npos) {
                lldArgs.push_back("/LIBPATH:" + path);
            } else {
                lldArgs.push_back("-L" + path);
            }
        }

        if (linkWithLLD(lldArgs, errorOutput)) {
            std::cout << "Successfully linked with integrated LLD: " << outputFile << std::endl;
            return true;
        }

        std::cout << "Integrated LLD failed, trying system linker..." << std::endl;

        // Method 2: Fallback to system linker
        if (linkWithSystemLinker(objFilename, outputFile, errorOutput)) {
            std::cout << "Successfully linked with system linker: " << outputFile << std::endl;
            return true;
        }

        // Both methods failed
        Ryntra::Utils::globalErrorHandler.makeError(
            "linker",
            "All linking methods failed: " + errorOutput,
            0, 0);

        return false;
    }

    bool IntegratedLinker::linkWithLLD(
        const std::vector<std::string> &args,
        std::string                    &errorOutput) {

        try {
            std::cout << "Attempting LLD integration..." << std::endl;

            // Since we only have lld/Common, we'll try a different approach
            // Use the LLD driver interface through system call but with better error handling

            std::string command = "lld-link";
            for (size_t i = 1; i < args.size(); ++i) { // Skip program name
                command += " " + args[i];
            }

            std::cout << "LLD command: " << command << std::endl;

            // Try to execute lld-link
            int result = std::system(command.c_str());

            if (result == 0) {
                return true;
            } else {
                errorOutput = "lld-link failed with exit code " + std::to_string(result);
                return false;
            }

        } catch (const std::exception &e) {
            errorOutput = "LLD exception: " + std::string(e.what());
            return false;
        }
    }

    bool IntegratedLinker::linkWithSystemLinker(
        const std::string &objFilename,
        const std::string &exeFilename,
        std::string       &errorOutput) {

        std::string              hostTriple = llvm::sys::getDefaultTargetTriple();
        std::vector<std::string> linkers;

        // Choose linkers based on platform
        if (hostTriple.find("windows") != std::string::npos ||
            hostTriple.find("win32") != std::string::npos ||
            hostTriple.find("msvc") != std::string::npos) {
            linkers = {"lld-link", "link"};
        } else if (hostTriple.find("linux") != std::string::npos) {
            linkers = {"ld.lld", "ld", "gcc", "clang"};
        } else if (hostTriple.find("darwin") != std::string::npos) {
            linkers = {"ld.lld", "ld", "clang"};
        }

        for (const auto &linker : linkers) {
            llvm::ErrorOr<std::string> linkerPath = llvm::sys::findProgramByName(linker);
            if (!linkerPath) {
                continue;
            }

            std::cout << "Trying system linker: " << linker << std::endl;

            // Build platform-specific command
            std::string command = "\"" + linkerPath.get() + "\""; // Quote the linker path
            auto        args = getPlatformLinkerArgs(exeFilename, objFilename);

            for (const auto &arg : args) {
                // Check if argument contains spaces and needs quoting
                if (arg.find(' ') != std::string::npos && arg.front() != '"') {
                    command += " \"" + arg + "\"";
                } else {
                    command += " " + arg;
                }
            }

            std::cout << "Executing: " << command << std::endl;

            int result = std::system(command.c_str());
            if (result == 0) {
                return true;
                // ReSharper disable once CppRedundantElseKeywordInsideCompoundStatement
            } else {
                errorOutput += linker + " failed with exit code " + std::to_string(result) + "; ";
            }
        }

        return false;
    }

} // namespace Ryntra::Compiler
