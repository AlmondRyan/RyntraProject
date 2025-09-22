#ifndef COMPILER_H
#define COMPILER_H

#include "lexer/lexer.h"
#include "parser/parser.h"
#include <memory>
#include <string>

namespace Ryntra::Compiler {
    class RyntraCompiler {
    private:
        std::string sourceCode;

    public:
        RyntraCompiler(const std::string &source);

        // Compile the source code and return AST
        std::unique_ptr<Program> compile();

        // Utility methods
        void printTokens();
        void printAST();

        // Static method to compile from file
        static std::unique_ptr<Program> compileFromFile(const std::string &filename);
    };

} // namespace Ryntra::Compiler

#endif // COMPILER_H