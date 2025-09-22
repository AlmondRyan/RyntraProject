#include "compiler.h"
#include <fstream>
#include <iostream>
#include <sstream>

namespace Ryntra::Compiler {
    RyntraCompiler::RyntraCompiler(const std::string &source) : sourceCode(source) {}

    std::unique_ptr<Program> RyntraCompiler::compile() {
        try {
            // Lexical analysis
            Lexer lexer(sourceCode);
            auto  tokens = lexer.tokenize();

            // Syntax analysis
            Parser parser(tokens);
            auto   ast = parser.parse();

            return ast;
        } catch (const std::exception &e) {
            std::cerr << "Compilation error: " << e.what() << std::endl;
            return nullptr;
        }
    }

    void RyntraCompiler::printTokens() {
        Lexer lexer(sourceCode);
        auto  tokens = lexer.tokenize();

        std::cout << "Tokens:\n";
        for (const auto &token : tokens) {
            std::cout << "  " << TokenUtils::tokenTypeToString(token.type)
                      << ": '" << token.value << "' at line " << token.line
                      << ", column " << token.column << std::endl;
        }
    }

    void RyntraCompiler::printAST() {
        auto ast = compile();
        if (ast) {
            Parser parser({});
            parser.displayAST(ast);
        }
    }

    void RyntraCompiler::generateLLVMIR() {
        auto ast = compile();
        if (ast) {
            LLVMCodeGenerator codegen("RyntraModule");
            codegen.generateCode(*ast);
            
            std::cout << "\n=== Generated LLVM IR ===" << std::endl;
            codegen.printIR();
        }
    }

    std::unique_ptr<LLVMCodeGenerator> RyntraCompiler::compileLLVM() {
        auto ast = compile();
        if (ast) {
            auto codegen = std::make_unique<LLVMCodeGenerator>("RyntraModule");
            codegen->generateCode(*ast);
            return codegen;
        }
        return nullptr;
    }

    bool RyntraCompiler::compileToExecutable(const std::string& outputName) {
        auto codegen = compileLLVM();
        if (codegen) {
            return codegen->compileToExecutable(outputName);
        }
        return false;
    }

    std::unique_ptr<Program> RyntraCompiler::compileFromFile(const std::string &filename) {
        std::ifstream file(filename);
        if (!file.is_open()) {
            std::cerr << "Error: Could not open file " << filename << std::endl;
            return nullptr;
        }

        std::stringstream buffer;
        buffer << file.rdbuf();
        std::string source = buffer.str();

        RyntraCompiler compiler(source);
        return compiler.compile();
    }

} // namespace Ryntra::Compiler
