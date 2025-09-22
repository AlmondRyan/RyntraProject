#ifndef COMPILER_UTILS_H
#define COMPILER_UTILS_H

#include <string>

namespace Ryntra {
namespace Utils {

// Main compilation function
bool compile(const std::string& inputFile, 
             const std::string& outputFile,
             bool showTokens = false, 
             bool showAST = false, 
             bool showIR = false);

// Helper functions
void printCompilerHeader();
void printCompilationStep(const std::string& step);
void printSuccess(const std::string& message);
void printError(const std::string& message);

} // namespace Utils
} // namespace Ryntra

#endif // COMPILER_UTILS_H