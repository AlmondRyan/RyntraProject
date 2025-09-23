#include "compiler_utils.h"
#include "../compiler/compiler.h"
#include "ErrorHandler.h"
#include <fstream>
#include <iostream>
#include <sstream>

using namespace Ryntra::Compiler;

namespace Ryntra::Utils {
    bool compile(const std::string &inputFile,
                 const std::string &outputFile,
                 bool               showTokens,
                 bool               showAST,
                 bool               showIR) {

        printCompilerHeader();

        std::cout << "Input file:  " << inputFile << std::endl;
        std::cout << "Output file: " << outputFile << std::endl;
        std::cout << std::endl;

        globalErrorHandler = ErrorHandler();

        printCompilationStep("Reading source file");
        std::ifstream file(inputFile);
        if (!file.is_open()) {
            globalErrorHandler.makeError("compiler", "Could not open input file: " + inputFile, 0, 0);
            globalErrorHandler.printErrorList();
            return false;
        }

        std::stringstream buffer;
        buffer << file.rdbuf();
        std::string sourceCode = buffer.str();
        file.close();

        printSuccess("Source file read successfully");

        RyntraCompiler compiler(sourceCode);

        printCompilationStep("Performing lexical and syntax analysis");
        if (auto ast = compiler.compile(); !ast || globalErrorHandler.getErrorListSize() > 0) {
            if (globalErrorHandler.getErrorListSize() > 0) {
                globalErrorHandler.printErrorList();
            }
            printError("Compilation failed during parsing");
            return false;
        }
        printSuccess("Parsing completed successfully");

        if (showTokens) {
            std::cout << "\n=== LEXICAL ANALYSIS (TOKENS) ===" << std::endl;
            compiler.printTokens();
        }

        if (showAST) {
            std::cout << "\n=== SYNTAX ANALYSIS (AST) ===" << std::endl;
            compiler.printAST();
        }

        if (showIR) {
            std::cout << "\n=== LLVM IR GENERATION ===" << std::endl;
            compiler.generateLLVMIR();
        }

        printCompilationStep("Generating executable");
        bool success = compiler.compileToExecutable(outputFile);

        if (success && globalErrorHandler.getErrorListSize() == 0) {
            printSuccess("Compilation completed successfully!");
            std::cout << "\nGenerated files:" << std::endl;
            std::cout << "  - " << outputFile << ".ll  (LLVM IR)" << std::endl;
            std::cout << "  - " << outputFile << ".bc  (Bytecode)" << std::endl;
            std::cout << "  - " << outputFile << ".obj (Object file)" << std::endl;
            std::cout << "  - " << outputFile << ".exe (Executable)" << std::endl;

            std::cout << "\nRun with: ./" << outputFile << std::endl;
            return true;
            // ReSharper disable once CppRedundantElseKeywordInsideCompoundStatement
        } else {
            if (globalErrorHandler.getErrorListSize() > 0) {
                globalErrorHandler.printErrorList();
            }
            printError("Failed to generate executable");
            return false;
        }
    }

    void printCompilerHeader() {
        std::cout << "Ryntra Compiler (rycc)" << std::endl;
        std::cout << "Copyright (c) 2025 Remimwen. All rights reserved." << std::endl;
    }

    void printCompilationStep(const std::string &step) {
        std::cout << "[INFO] " << step << "..." << std::endl;
    }

    void printSuccess(const std::string &message) {
        std::cout << CTCM_GREEN << "[SUCCESS] " << CTCM_DEFAULT << message << std::endl;
    }

    void printError(const std::string &message) {
        std::cerr << CTCM_RED << "[Error] " << CTCM_DEFAULT << message << std::endl;
    }

} // namespace Ryntra::Utils