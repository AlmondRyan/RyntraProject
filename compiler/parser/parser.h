#ifndef PARSER_H
#define PARSER_H

#include "../lexer/token.h"
#include "ast.h"
#include <memory>
#include <vector>

namespace Ryntra::Compiler {
    class Parser {
    private:
        std::vector<Token> tokens;
        size_t             current;

        Token &currentToken();
        Token &peekToken(int offset = 1);
        bool   match(TokenType type);
        bool   check(TokenType type);
        Token  advance();
        Token  consume(TokenType type, const std::string &message);

        // Parsing methods
        std::unique_ptr<Program>            parseProgram();
        std::unique_ptr<ASTNode>            parseDeclaration();
        std::unique_ptr<PackageDeclaration> parsePackageDeclaration();
        std::unique_ptr<ImportDeclaration>  parseImportDeclaration();
        std::unique_ptr<ClassDeclaration>   parseClassDeclaration();
        std::unique_ptr<MethodDeclaration>  parseMethodDeclaration();
        std::unique_ptr<ASTNode>            parseStatement();
        std::unique_ptr<ASTNode>            parseExpressionStatement();
        std::unique_ptr<ReturnStatement>    parseReturnStatement();
        std::unique_ptr<ASTNode>            parseExpression();
        std::unique_ptr<ASTNode>            parseBinaryExpression();
        std::unique_ptr<ASTNode>            parsePrimary();

        // Helper methods
        std::string parseType();

    public:
        Parser(const std::vector<Token> &tokenList);
        std::unique_ptr<Program> parse();
        void                     displayAST(const std::unique_ptr<Program> &program, int indent = 0);

    private:
        void displayASTNode(ASTNode *node, int indent);
    };

} // namespace Ryntra::Compiler

#endif // PARSER_H