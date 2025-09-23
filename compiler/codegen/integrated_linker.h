#ifndef RYNTRA_COMPILER_CODEGEN_INTEGRATED_LINKER_H
#define RYNTRA_COMPILER_CODEGEN_INTEGRATED_LINKER_H

#include <string>
#include <vector>

namespace Ryntra::Compiler {
    class IntegratedLinker {
    public:
        // Link object file to executable using integrated LLD
        static bool linkExecutable(
            const std::string              &objFilename,
            const std::string              &exeFilename,
            const std::vector<std::string> &libraries = {},
            const std::vector<std::string> &libraryPaths = {});

    private:
        // Internal LLD linking function
        static bool linkWithLLD(
            const std::vector<std::string> &args,
            std::string                    &errorOutput);

        // Fallback system linker
        static bool linkWithSystemLinker(
            const std::string &objFilename,
            const std::string &exeFilename,
            std::string       &errorOutput);
    };

} // namespace Ryntra::Compiler

#endif // RYNTRA_COMPILER_CODEGEN_INTEGRATED_LINKER_H