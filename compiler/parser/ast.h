#ifndef RYNTRA_COMPILER_PARSER_AST_H
#define RYNTRA_COMPILER_PARSER_AST_H

#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace Ryntra::Compiler {
    class ASTVisitor;

    class ASTNode {
    public:
        virtual ~ASTNode() = default;
        virtual void        accept(ASTVisitor &visitor) = 0;
        [[nodiscard]] virtual std::string toString() const = 0;
    };

    class Program final : public ASTNode {
    public:
        std::vector<std::unique_ptr<ASTNode>> declarations;

        void                      accept(ASTVisitor &visitor) override;
        [[nodiscard]] std::string toString() const override;
    };

    class PackageDeclaration final : public ASTNode {
    public:
        std::string packageName;

        explicit PackageDeclaration(std::string name) : packageName(std::move(name)) {}
        void                      accept(ASTVisitor &visitor) override;
        [[nodiscard]] std::string toString() const override;
    };

    class ImportDeclaration final : public ASTNode {
    public:
        std::string importPath;

        explicit ImportDeclaration(std::string path) : importPath(std::move(path)) {}
        void                      accept(ASTVisitor &visitor) override;
        [[nodiscard]] std::string toString() const override;
    };

    class ClassDeclaration final : public ASTNode {
    public:
        std::string                           accessModifier;
        std::string                           className;
        std::vector<std::unique_ptr<ASTNode>> members;

        ClassDeclaration(std::string access, std::string name)
            : accessModifier(std::move(access)), className(std::move(name)) {}
        void                      accept(ASTVisitor &visitor) override;
        [[nodiscard]] std::string toString() const override;
    };

    class MethodDeclaration final : public ASTNode {
    public:
        std::string                                      accessModifier;
        bool                                             isStatic;
        std::string                                      returnType;
        std::string                                      methodName;
        std::vector<std::pair<std::string, std::string>> parameters;
        std::vector<std::unique_ptr<ASTNode>>            body;

        MethodDeclaration(std::string access, const bool stat, std::string retType,
                          std::string name)
            : accessModifier(std::move(access)), isStatic(stat),
              returnType(std::move(retType)), methodName(std::move(name)) {}
        void                      accept(ASTVisitor &visitor) override;
        [[nodiscard]] std::string toString() const override;
    };

    class ExpressionStatement final : public ASTNode {
    public:
        std::unique_ptr<ASTNode> expression;

        explicit ExpressionStatement(std::unique_ptr<ASTNode> expr) : expression(std::move(expr)) {}
        void                      accept(ASTVisitor &visitor) override;
        [[nodiscard]] std::string toString() const override;
    };

    class BinaryExpression final : public ASTNode {
    public:
        std::unique_ptr<ASTNode> left;
        std::string              operator_;
        std::unique_ptr<ASTNode> right;

        BinaryExpression(std::unique_ptr<ASTNode> l, std::string op, std::unique_ptr<ASTNode> r)
            : left(std::move(l)), operator_(std::move(op)), right(std::move(r)) {}
        void                      accept(ASTVisitor &visitor) override;
        [[nodiscard]] std::string toString() const override;
    };

    class Identifier final : public ASTNode {
    public:
        std::string name;

        explicit Identifier(std::string n) : name(std::move(n)) {}
        void                      accept(ASTVisitor &visitor) override;
        [[nodiscard]] std::string toString() const override;
    };

    class StringLiteral final : public ASTNode {
    public:
        std::string value;

        explicit StringLiteral(std::string val) : value(std::move(val)) {}
        void                      accept(ASTVisitor &visitor) override;
        [[nodiscard]] std::string toString() const override;
    };

    class NumberLiteral final : public ASTNode {
    public:
        std::string value;

        explicit NumberLiteral(std::string val) : value(std::move(val)) {}
        void                      accept(ASTVisitor &visitor) override;
        [[nodiscard]] std::string toString() const override;
    };

    class ReturnStatement final : public ASTNode {
    public:
        std::unique_ptr<ASTNode> expression;

        explicit ReturnStatement(std::unique_ptr<ASTNode> expr) : expression(std::move(expr)) {}
        void                      accept(ASTVisitor &visitor) override;
        [[nodiscard]] std::string toString() const override;
    };

    class ASTVisitor {
    public:
        virtual ~ASTVisitor() = default;
        virtual void visit(Program &node) = 0;
        virtual void visit(PackageDeclaration &node) = 0;
        virtual void visit(ImportDeclaration &node) = 0;
        virtual void visit(ClassDeclaration &node) = 0;
        virtual void visit(MethodDeclaration &node) = 0;
        virtual void visit(ExpressionStatement &node) = 0;
        virtual void visit(BinaryExpression &node) = 0;
        virtual void visit(Identifier &node) = 0;
        virtual void visit(StringLiteral &node) = 0;
        virtual void visit(NumberLiteral &node) = 0;
        virtual void visit(ReturnStatement &node) = 0;
    };

} // namespace Ryntra::Compiler

#endif // RYNTRA_COMPILER_PARSER_AST_H