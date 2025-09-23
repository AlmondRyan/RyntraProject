#ifndef RYNTRA_COMPILER_COMPILER_H
#define RYNTRA_COMPILER_COMPILER_H

#include "codegen/llvm_codegen.h"
#include "lexer/lexer.h"
#include "parser/parser.h"
#include <memory>
#include <string>

namespace Ryntra::Compiler {
    class RyntraCompiler {
    private:
        std::string sourceCode;

    public:
        explicit RyntraCompiler(const std::string &source);

        [[nodiscard]] std::unique_ptr<Program> compile() const;

        void printTokens() const;
        void printAST();

        void                               generateLLVMIR();
        std::unique_ptr<LLVMCodeGenerator> compileLLVM();

        bool compileToExecutable(const std::string &outputName = "program");

        static std::unique_ptr<Program> compileFromFile(const std::string &filename);
    };

} // namespace Ryntra::Compiler

#endif // RYNTRA_COMPILER_COMPILER_H