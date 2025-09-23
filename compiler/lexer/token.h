#ifndef RYNTRA_COMPILER_LEXER_TOKEN_H
#define RYNTRA_COMPILER_LEXER_TOKEN_H

#include <string>
#include <unordered_map>

namespace Ryntra::Compiler {
    enum class TokenType {
        // Keywords
        DECLARE,
        IMPORT,
        PUBLIC,
        PRIVATE,
        CLASS,
        STATIC,
        INT,
        STRING,
        VAR,
        RETURN,

        // Operators
        ASSIGN,
        LSHIFT,
        DOT,
        SEMICOLON,
        COMMA,
        COLON,
        STAR,

        // Delimiters
        LPAREN,
        RPAREN,
        LBRACE,
        RBRACE,
        LBRACKET,
        RBRACKET,
        LT,
        GT,

        // Literals
        IDENTIFIER,
        STRING_LITERAL,
        NUMBER,

        // Special
        NEWLINE,
        WHITESPACE,
        COMMENT,
        EOF_TOKEN,
        UNKNOWN
    };

    struct Token {
        TokenType   type;
        std::string value;
        int         line;
        int         column;

        Token(TokenType t = TokenType::UNKNOWN, const std::string &v = "", int l = 1, int c = 1)
            : type(t), value(v), line(l), column(c) {}
    };

    class TokenUtils {
    public:
        static std::string tokenTypeToString(TokenType type);
        static bool        isKeyword(const std::string &str);
        static TokenType   getKeywordType(const std::string &str);

    private:
        static std::unordered_map<std::string, TokenType> keywords;
    };

} // namespace Ryntra::Compiler

#endif // RYNTRA_COMPILER_LEXER_TOKEN_H