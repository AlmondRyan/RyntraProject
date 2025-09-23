#include "token.h"

namespace Ryntra::Compiler {
    std::unordered_map<std::string, TokenType> TokenUtils::keywords = {
        {"declare", TokenType::DECLARE},
        {"import", TokenType::IMPORT},
        {"public", TokenType::PUBLIC},
        {"private", TokenType::PRIVATE},
        {"class", TokenType::CLASS},
        {"static", TokenType::STATIC},
        {"int", TokenType::INT},
        {"string", TokenType::STRING},
        {"var", TokenType::VAR},
        {"return", TokenType::RETURN}};

    std::string TokenUtils::tokenTypeToString(TokenType type) {
        switch (type) {
        case TokenType::DECLARE:
            return "DECLARE";
        case TokenType::IMPORT:
            return "IMPORT";
        case TokenType::PUBLIC:
            return "PUBLIC";
        case TokenType::PRIVATE:
            return "PRIVATE";
        case TokenType::CLASS:
            return "CLASS";
        case TokenType::STATIC:
            return "STATIC";
        case TokenType::INT:
            return "INT";
        case TokenType::STRING:
            return "STRING";
        case TokenType::VAR:
            return "VAR";
        case TokenType::RETURN:
            return "RETURN";
        case TokenType::ASSIGN:
            return "ASSIGN";
        case TokenType::LSHIFT:
            return "LSHIFT";
        case TokenType::DOT:
            return "DOT";
        case TokenType::SEMICOLON:
            return "SEMICOLON";
        case TokenType::COMMA:
            return "COMMA";
        case TokenType::COLON:
            return "COLON";
        case TokenType::STAR:
            return "STAR";
        case TokenType::LPAREN:
            return "LPAREN";
        case TokenType::RPAREN:
            return "RPAREN";
        case TokenType::LBRACE:
            return "LBRACE";
        case TokenType::RBRACE:
            return "RBRACE";
        case TokenType::LBRACKET:
            return "LBRACKET";
        case TokenType::RBRACKET:
            return "RBRACKET";
        case TokenType::LT:
            return "LT";
        case TokenType::GT:
            return "GT";
        case TokenType::IDENTIFIER:
            return "IDENTIFIER";
        case TokenType::STRING_LITERAL:
            return "STRING_LITERAL";
        case TokenType::NUMBER:
            return "NUMBER";
        case TokenType::NEWLINE:
            return "NEWLINE";
        case TokenType::WHITESPACE:
            return "WHITESPACE";
        case TokenType::COMMENT:
            return "COMMENT";
        case TokenType::EOF_TOKEN:
            return "EOF";
        default:
            return "UNKNOWN";
        }
    }

    bool TokenUtils::isKeyword(const std::string &str) {
        return keywords.find(str) != keywords.end();
    }

    TokenType TokenUtils::getKeywordType(const std::string &str) {
        auto it = keywords.find(str);
        return (it != keywords.end()) ? it->second : TokenType::IDENTIFIER;
    }

} // namespace Ryntra::Compiler