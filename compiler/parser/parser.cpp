#include "parser.h"
#include <iostream>
#include <stdexcept>

namespace Ryntra::Compiler {
    Parser::Parser(const std::vector<Token> &tokenList) : tokens(tokenList), current(0) {}

    Token &Parser::currentToken() {
        if (current >= tokens.size()) {
            static Token eofToken(TokenType::EOF_TOKEN);
            return eofToken;
        }
        return tokens[current];
    }

    Token &Parser::peekToken(int offset) {
        size_t pos = current + offset;
        if (pos >= tokens.size()) {
            static Token eofToken(TokenType::EOF_TOKEN);
            return eofToken;
        }
        return tokens[pos];
    }

    bool Parser::match(TokenType type) {
        if (check(type)) {
            advance();
            return true;
        }
        return false;
    }

    bool Parser::check(TokenType type) {
        return currentToken().type == type;
    }

    Token Parser::advance() {
        if (current < tokens.size()) {
            return tokens[current++];
        }
        return Token(TokenType::EOF_TOKEN);
    }

    Token Parser::consume(TokenType type, const std::string &message) {
        if (check(type)) {
            return advance();
        }
        throw std::runtime_error("Parse error: " + message + " at line " +
                                 std::to_string(currentToken().line));
    }

    std::unique_ptr<Program> Parser::parse() {
        return parseProgram();
    }

    std::unique_ptr<Program> Parser::parseProgram() {
        auto program = std::make_unique<Program>();

        while (!check(TokenType::EOF_TOKEN)) {
            auto decl = parseDeclaration();
            if (decl) {
                program->declarations.push_back(std::move(decl));
            }
        }

        return program;
    }

    std::unique_ptr<ASTNode> Parser::parseDeclaration() {
        if (match(TokenType::DECLARE)) {
            consume(TokenType::LPAREN, "Expected '(' after 'declare'");

            if (match(TokenType::IDENTIFIER)) {
                std::string declType = tokens[current - 1].value;
                consume(TokenType::RPAREN, "Expected ')' after declare type");

                if (declType == "package") {
                    return parsePackageDeclaration();
                }
            }
            throw std::runtime_error("Unsupported declare type");
        }

        if (match(TokenType::IMPORT)) {
            return parseImportDeclaration();
        }

        if (check(TokenType::PUBLIC) || check(TokenType::PRIVATE)) {
            return parseClassDeclaration();
        }

        throw std::runtime_error("Unexpected token in declaration");
    }

    std::unique_ptr<PackageDeclaration> Parser::parsePackageDeclaration() {
        std::string packageName;

        // Parse package name (e.g., org.remimwen.test)
        Token firstPart = consume(TokenType::IDENTIFIER, "Expected package name");
        packageName = firstPart.value;

        // Handle dotted package name
        while (check(TokenType::DOT)) {
            advance(); // consume '.'
            packageName += ".";

            Token part = consume(TokenType::IDENTIFIER, "Expected identifier after '.' in package name");
            packageName += part.value;
        }

        consume(TokenType::SEMICOLON, "Expected ';' after package declaration");

        return std::make_unique<PackageDeclaration>(packageName);
    }

    std::unique_ptr<ImportDeclaration> Parser::parseImportDeclaration() {
        std::string importPath;

        // Parse import path (e.g., ryntra.io.*)
        Token firstPart = consume(TokenType::IDENTIFIER, "Expected import path");
        importPath = firstPart.value;

        // Handle dotted path and wildcard
        while (check(TokenType::DOT)) {
            advance(); // consume '.'
            importPath += ".";

            if (check(TokenType::STAR)) {
                advance(); // consume '*'
                importPath += "*";
                break;
            } else if (check(TokenType::IDENTIFIER)) {
                Token part = advance();
                importPath += part.value;
            } else {
                throw std::runtime_error("Expected identifier or '*' after '.' in import path");
            }
        }

        consume(TokenType::SEMICOLON, "Expected ';' after import declaration");

        return std::make_unique<ImportDeclaration>(importPath);
    }

    std::unique_ptr<ClassDeclaration> Parser::parseClassDeclaration() {
        Token accessModifier = advance(); // public or private
        consume(TokenType::CLASS, "Expected 'class' keyword");
        Token className = consume(TokenType::IDENTIFIER, "Expected class name");

        auto classDecl = std::make_unique<ClassDeclaration>(accessModifier.value, className.value);

        consume(TokenType::LBRACE, "Expected '{' after class name");

        while (!check(TokenType::RBRACE) && !check(TokenType::EOF_TOKEN)) {
            auto member = parseMethodDeclaration();
            if (member) {
                classDecl->members.push_back(std::move(member));
            }
        }

        consume(TokenType::RBRACE, "Expected '}' after class body");

        return classDecl;
    }

    std::unique_ptr<MethodDeclaration> Parser::parseMethodDeclaration() {
        Token accessModifier = advance(); // public or private

        bool isStatic = false;
        if (match(TokenType::STATIC)) {
            isStatic = true;
        }

        // Parse return type (can be keyword like 'int', 'string' or identifier)
        std::string returnTypeStr = parseType();

        Token methodName = consume(TokenType::IDENTIFIER, "Expected method name");

        auto methodDecl = std::make_unique<MethodDeclaration>(
            accessModifier.value, isStatic, returnTypeStr, methodName.value);

        consume(TokenType::LPAREN, "Expected '(' after method name");

        // Parse parameters
        if (!check(TokenType::RPAREN)) {
            do {
                // Parse parameter type (can be keyword like 'int', 'string' or identifier)
                std::string paramTypeStr = parseType();

                if (check(TokenType::LBRACKET)) {
                    advance(); // consume '['
                    consume(TokenType::RBRACKET, "Expected ']' after '['");
                    paramTypeStr += "[]";
                }

                Token paramName = consume(TokenType::IDENTIFIER, "Expected parameter name");
                methodDecl->parameters.emplace_back(paramTypeStr, paramName.value);
            } while (match(TokenType::COMMA));
        }

        consume(TokenType::RPAREN, "Expected ')' after parameters");
        consume(TokenType::LBRACE, "Expected '{' after method signature");

        // Parse method body
        while (!check(TokenType::RBRACE) && !check(TokenType::EOF_TOKEN)) {
            auto stmt = parseStatement();
            if (stmt) {
                methodDecl->body.push_back(std::move(stmt));
            }
        }

        consume(TokenType::RBRACE, "Expected '}' after method body");

        return methodDecl;
    }

    std::unique_ptr<ASTNode> Parser::parseStatement() {
        if (match(TokenType::RETURN)) {
            return parseReturnStatement();
        }

        return parseExpressionStatement();
    }

    std::unique_ptr<ASTNode> Parser::parseExpressionStatement() {
        auto expr = parseExpression();
        consume(TokenType::SEMICOLON, "Expected ';' after expression");
        return std::make_unique<ExpressionStatement>(std::move(expr));
    }

    std::unique_ptr<ReturnStatement> Parser::parseReturnStatement() {
        auto expr = parseExpression();
        consume(TokenType::SEMICOLON, "Expected ';' after return statement");
        return std::make_unique<ReturnStatement>(std::move(expr));
    }

    std::unique_ptr<ASTNode> Parser::parseExpression() {
        return parseBinaryExpression();
    }

    std::unique_ptr<ASTNode> Parser::parseBinaryExpression() {
        auto left = parsePrimary();

        while (check(TokenType::LSHIFT)) {
            Token op = advance();
            auto  right = parsePrimary();
            left = std::make_unique<BinaryExpression>(std::move(left), op.value, std::move(right));
        }

        return left;
    }

    std::unique_ptr<ASTNode> Parser::parsePrimary() {
        if (match(TokenType::STRING_LITERAL)) {
            return std::make_unique<StringLiteral>(tokens[current - 1].value);
        }

        if (match(TokenType::NUMBER)) {
            return std::make_unique<NumberLiteral>(tokens[current - 1].value);
        }

        if (match(TokenType::IDENTIFIER)) {
            std::string identifierName = tokens[current - 1].value;

            // Handle dotted identifiers (e.g., io.out)
            while (check(TokenType::DOT)) {
                advance(); // consume '.'
                identifierName += ".";

                Token part = consume(TokenType::IDENTIFIER, "Expected identifier after '.' in expression");
                identifierName += part.value;
            }

            return std::make_unique<Identifier>(identifierName);
        }

        throw std::runtime_error("Unexpected token in expression");
    }

    std::string Parser::parseType() {
        if (check(TokenType::INT) || check(TokenType::STRING) || check(TokenType::IDENTIFIER)) {
            Token typeToken = advance();
            return typeToken.value;
        } else {
            throw std::runtime_error("Parse error: Expected type at line " +
                                     std::to_string(currentToken().line));
        }
    }

    void Parser::displayAST(const std::unique_ptr<Program> &program, int indent) {
        std::cout << "AST Structure:\n";
        displayASTNode(program.get(), indent);
    }

    void Parser::displayASTNode(ASTNode *node, int indent) {
        if (!node)
            return;

        std::string indentStr(indent * 2, ' ');
        std::cout << indentStr << node->toString() << std::endl;

        // Handle specific node types to display children
        if (auto program = dynamic_cast<Program *>(node)) {
            for (const auto &decl : program->declarations) {
                displayASTNode(decl.get(), indent + 1);
            }
        } else if (auto classDecl = dynamic_cast<ClassDeclaration *>(node)) {
            for (const auto &member : classDecl->members) {
                displayASTNode(member.get(), indent + 1);
            }
        } else if (auto methodDecl = dynamic_cast<MethodDeclaration *>(node)) {
            for (const auto &stmt : methodDecl->body) {
                displayASTNode(stmt.get(), indent + 1);
            }
        } else if (auto exprStmt = dynamic_cast<ExpressionStatement *>(node)) {
            displayASTNode(exprStmt->expression.get(), indent + 1);
        } else if (auto binExpr = dynamic_cast<BinaryExpression *>(node)) {
            displayASTNode(binExpr->left.get(), indent + 1);
            displayASTNode(binExpr->right.get(), indent + 1);
        } else if (auto retStmt = dynamic_cast<ReturnStatement *>(node)) {
            displayASTNode(retStmt->expression.get(), indent + 1);
        }
    }

} // namespace Ryntra::Compiler
