#ifndef RYNTRA_COMPILER_LEXER_LEXER_H
#define RYNTRA_COMPILER_LEXER_LEXER_H

#include "token.h"
#include <string>
#include <vector>

namespace Ryntra::Compiler {
    class Lexer {
    private:
        std::string input;
        size_t      position;
        size_t      current_line;
        size_t      current_column;

        char  currentChar();
        char  peekChar(int offset = 1);
        void  advance();
        void  skipWhitespace();
        void  skipComment();
        Token readString();
        Token readNumber();
        Token readIdentifier();

    public:
        explicit Lexer(const std::string &source);
        Token              nextToken();
        std::vector<Token> tokenize();
    };

} // namespace Ryntra::Compiler

#endif // RYNTRA_COMPILER_LEXER_LEXER_H