#include "lexer.h"
#include <cctype>

namespace Ryntra::Compiler {
    Lexer::Lexer(const std::string &source)
        : input(source), position(0), current_line(1), current_column(1) {}

    char Lexer::currentChar() {
        if (position >= input.length()) {
            return '\0';
        }
        return input[position];
    }

    char Lexer::peekChar(int offset) {
        size_t peek_pos = position + offset;
        if (peek_pos >= input.length()) {
            return '\0';
        }
        return input[peek_pos];
    }

    void Lexer::advance() {
        if (position < input.length()) {
            if (input[position] == '\n') {
                current_line++;
                current_column = 1;
            } else {
                current_column++;
            }
            position++;
        }
    }

    void Lexer::skipWhitespace() {
        while (currentChar() != '\0' && std::isspace(currentChar()) && currentChar() != '\n') {
            advance();
        }
    }

    void Lexer::skipComment() {
        if (currentChar() == '/' && peekChar() == '*') {
            advance(); // skip '/'
            advance(); // skip '*'

            while (currentChar() != '\0') {
                if (currentChar() == '*' && peekChar() == '/') {
                    advance(); // skip '*'
                    advance(); // skip '/'
                    break;
                }
                advance();
            }
        }
    }

    Token Lexer::readString() {
        std::string value;
        int         start_line = current_line;
        int         start_column = current_column;

        advance(); // skip opening quote

        while (currentChar() != '\0' && currentChar() != '"') {
            if (currentChar() == '\\') {
                advance();
                switch (currentChar()) {
                case 'n':
                    value += '\n';
                    break;
                case 't':
                    value += '\t';
                    break;
                case 'r':
                    value += '\r';
                    break;
                case '\\':
                    value += '\\';
                    break;
                case '"':
                    value += '"';
                    break;
                default:
                    value += currentChar();
                    break;
                }
            } else {
                value += currentChar();
            }
            advance();
        }

        if (currentChar() == '"') {
            advance(); // skip closing quote
        }

        return Token(TokenType::STRING_LITERAL, value, start_line, start_column);
    }

    Token Lexer::readNumber() {
        std::string value;
        int         start_line = current_line;
        int         start_column = current_column;

        while (currentChar() != '\0' && std::isdigit(currentChar())) {
            value += currentChar();
            advance();
        }

        return Token(TokenType::NUMBER, value, start_line, start_column);
    }

    Token Lexer::readIdentifier() {
        std::string value;
        int         start_line = current_line;
        int         start_column = current_column;

        while (currentChar() != '\0' &&
               (std::isalnum(currentChar()) || currentChar() == '_')) {
            value += currentChar();
            advance();
        }

        TokenType type = TokenUtils::isKeyword(value) ? TokenUtils::getKeywordType(value) : TokenType::IDENTIFIER;

        return Token(type, value, start_line, start_column);
    }

    Token Lexer::nextToken() {
        while (currentChar() != '\0') {
            int start_line = current_line;
            int start_column = current_column;

            // Skip whitespace
            if (std::isspace(currentChar()) && currentChar() != '\n') {
                skipWhitespace();
                continue;
            }

            // Handle newlines
            if (currentChar() == '\n') {
                advance();
                return Token(TokenType::NEWLINE, "\\n", start_line, start_column);
            }

            // Handle comments
            if (currentChar() == '/' && peekChar() == '*') {
                skipComment();
                continue;
            }

            // Handle string literals
            if (currentChar() == '"') {
                return readString();
            }

            // Handle numbers
            if (std::isdigit(currentChar())) {
                return readNumber();
            }

            // Handle identifiers and keywords
            if (std::isalpha(currentChar()) || currentChar() == '_') {
                return readIdentifier();
            }

            // Handle operators and delimiters
            char ch = currentChar();
            advance();

            switch (ch) {
            case '=':
                return Token(TokenType::ASSIGN, "=", start_line, start_column);
            case '<':
                if (currentChar() == '<') {
                    advance();
                    return Token(TokenType::LSHIFT, "<<", start_line, start_column);
                }
                return Token(TokenType::LT, "<", start_line, start_column);
            case '>':
                return Token(TokenType::GT, ">", start_line, start_column);
            case '.':
                return Token(TokenType::DOT, ".", start_line, start_column);
            case ';':
                return Token(TokenType::SEMICOLON, ";", start_line, start_column);
            case ',':
                return Token(TokenType::COMMA, ",", start_line, start_column);
            case ':':
                return Token(TokenType::COLON, ":", start_line, start_column);
            case '(':
                return Token(TokenType::LPAREN, "(", start_line, start_column);
            case ')':
                return Token(TokenType::RPAREN, ")", start_line, start_column);
            case '{':
                return Token(TokenType::LBRACE, "{", start_line, start_column);
            case '}':
                return Token(TokenType::RBRACE, "}", start_line, start_column);
            case '[':
                return Token(TokenType::LBRACKET, "[", start_line, start_column);
            case ']':
                return Token(TokenType::RBRACKET, "]", start_line, start_column);
            case '*':
                return Token(TokenType::STAR, "*", start_line, start_column);
            default:
                return Token(TokenType::UNKNOWN, std::string(1, ch), start_line, start_column);
            }
        }

        return Token(TokenType::EOF_TOKEN, "", current_line, current_column);
    }

    std::vector<Token> Lexer::tokenize() {
        std::vector<Token> tokens;
        Token              token;

        do {
            token = nextToken();
            if (token.type != TokenType::WHITESPACE && token.type != TokenType::NEWLINE) {
                tokens.push_back(token);
            }
        } while (token.type != TokenType::EOF_TOKEN);

        return tokens;
    }

} // namespace Ryntra::Compiler
