#ifndef COMPILER_UTILS_H
#define COMPILER_UTILS_H

#include <string>

namespace Ryntra::Utils {
    bool compile(const std::string &inputFile,
                 const std::string &outputFile,
                 bool               showTokens = false,
                 bool               showAST = false,
                 bool               showIR = false);

    void printCompilerHeader();
    void printCompilationStep(const std::string &step);
    void printSuccess(const std::string &message);
    void printError(const std::string &message);

} // namespace Ryntra::Utils

#endif // COMPILER_UTILS_H