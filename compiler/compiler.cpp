#include "compiler.h"
#include "../utils/ErrorHandler.h"
#include <fstream>
#include <iostream>
#include <sstream>

namespace Ryntra::Compiler {
    RyntraCompiler::RyntraCompiler(const std::string &source) : sourceCode(source) {
    }

    std::unique_ptr<Program> RyntraCompiler::compile() const {
        Utils::globalErrorHandler = Ryntra::Utils::ErrorHandler();
        try {
            Lexer lexer(sourceCode);
            auto  tokens = lexer.tokenize();

            Parser parser(tokens);
            auto   ast = parser.parse();

            if (Utils::globalErrorHandler.getErrorListSize() > 0) {
                Utils::globalErrorHandler.printErrorList();
                return nullptr;
            }

            return ast;
        } catch (const std::exception &e) {
            Utils::globalErrorHandler.makeError(
                "compiler",
                "compilation error: " + std::string(e.what()),
                0,
                0);
            Utils::globalErrorHandler.printErrorList();
            return nullptr;
        }
    }

    void RyntraCompiler::printTokens() const {
        Lexer      lexer(sourceCode);
        const auto tokens = lexer.tokenize();

        std::cout << "Tokens:\n";
        for (const auto &token : tokens) {
            std::cout << "  " << TokenUtils::tokenTypeToString(token.type)
                      << ": '" << token.value << "' at line " << token.line
                      << ", column " << token.column << std::endl;
        }
    }

    void RyntraCompiler::printAST() {
        if (const auto ast = compile()) {
            Parser parser({});
            parser.displayAST(ast);
        }
    }

    void RyntraCompiler::generateLLVMIR() {
        if (const auto ast = compile()) {
            LLVMCodeGenerator codegen("RyntraModule");
            codegen.generateCode(*ast);
            std::cout << "\n=== Generated LLVM IR ===" << std::endl;
            codegen.printIR();
        }
    }

    std::unique_ptr<LLVMCodeGenerator> RyntraCompiler::compileLLVM() {
        if (const auto ast = compile()) {
            auto codegen = std::make_unique<LLVMCodeGenerator>("RyntraModule");
            codegen->generateCode(*ast);
            return codegen;
        }
        return nullptr;
    }

    bool RyntraCompiler::compileToExecutable(const std::string &outputName) {
        if (const auto codegen = compileLLVM()) {
            return codegen->compileToExecutable(outputName);
        }
        return false;
    }

    std::unique_ptr<Program> RyntraCompiler::compileFromFile(const std::string &filename) {
        std::ifstream file(filename);
        if (!file.is_open()) {
            Utils::globalErrorHandler.makeError(
                "compiler",
                "Could not open file " + filename,
                0, 0);
            Utils::globalErrorHandler.printErrorList();
            return nullptr;
        }

        std::stringstream buffer;
        buffer << file.rdbuf();
        const std::string source = buffer.str();
        const RyntraCompiler compiler(source);
        return compiler.compile();
    }
} // namespace Ryntra::Compiler