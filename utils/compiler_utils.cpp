#include "compiler_utils.h"
#include "../compiler/compiler.h"
#include <iostream>
#include <fstream>
#include <sstream>

using namespace Ryntra::Compiler;

namespace Ryntra {
namespace Utils {

bool compile(const std::string& inputFile, 
             const std::string& outputFile,
             bool showTokens, 
             bool showAST, 
             bool showIR,
             bool delTempFileAfterCompile) {
    
    printCompilerHeader();
    
    std::cout << "Input file:  " << inputFile << std::endl;
    std::cout << "Output file: " << outputFile << std::endl;
    std::cout << std::endl;
    
    // Read source file
    printCompilationStep("Reading source file");
    std::ifstream file(inputFile);
    if (!file.is_open()) {
        printError("Could not open input file: " + inputFile);
        return false;
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string sourceCode = buffer.str();
    file.close();
    
    printSuccess("Source file read successfully");
    
    // Create compiler instance
    RyntraCompiler compiler(sourceCode);
    
    // Lexical and Syntax Analysis
    printCompilationStep("Performing lexical and syntax analysis");
    auto ast = compiler.compile();
    if (!ast) {
        printError("Compilation failed during parsing");
        return false;
    }
    printSuccess("Parsing completed successfully");
    
    // Show tokens if requested
    if (showTokens) {
        std::cout << "\n=== LEXICAL ANALYSIS (TOKENS) ===" << std::endl;
        compiler.printTokens();
    }
    
    // Show AST if requested
    if (showAST) {
        std::cout << "\n=== SYNTAX ANALYSIS (AST) ===" << std::endl;
        compiler.printAST();
    }
    
    // Show LLVM IR if requested
    if (showIR) {
        std::cout << "\n=== LLVM IR GENERATION ===" << std::endl;
        compiler.generateLLVMIR();
    }
    
    // Complete compilation to executable
    printCompilationStep("Generating executable");
    bool success = compiler.compileToExecutable(outputFile);
    
    if (success) {
        printSuccess("Compilation completed successfully!");
        std::cout << "\nGenerated files:" << std::endl;
        std::cout << "  - " << outputFile << ".ll  (LLVM IR)" << std::endl;
        std::cout << "  - " << outputFile << ".bc  (Bytecode)" << std::endl;
        std::cout << "  - " << outputFile << ".obj (Object file)" << std::endl;
        std::cout << "  - " << outputFile << ".exe (Executable)" << std::endl;

        std::cout << "\nRun with: ./" << outputFile << std::endl;
        return true;
    } else {
        printError("Failed to generate executable");
        return false;
    }
}

void printCompilerHeader() {
    std::cout << "========================================" << std::endl;
    std::cout << "    Ryntra Language Compiler (rycc)    " << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "Target: x86_64-windows-msvc" << std::endl;
    std::cout << "Linker: clang" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << std::endl;
}

void printCompilationStep(const std::string& step) {
    std::cout << "[INFO] " << step << "..." << std::endl;
}

void printSuccess(const std::string& message) {
    std::cout << "[✓] " << message << std::endl;
}

void printError(const std::string& message) {
    std::cerr << "[✗] " << message << std::endl;
}

} // namespace Utils
} // namespace Ryntra