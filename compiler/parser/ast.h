#ifndef AST_H
#define AST_H

#include <memory>
#include <string>
#include <vector>

namespace Ryntra::Compiler {
    // Forward declarations
    class ASTVisitor;

    // Base AST Node
    class ASTNode {
    public:
        virtual ~ASTNode() = default;
        virtual void        accept(ASTVisitor &visitor) = 0;
        virtual std::string toString() const = 0;
    };

    // Program (root node)
    class Program : public ASTNode {
    public:
        std::vector<std::unique_ptr<ASTNode>> declarations;

        void        accept(ASTVisitor &visitor) override;
        std::string toString() const override;
    };

    // Package Declaration
    class PackageDeclaration : public ASTNode {
    public:
        std::string packageName;

        PackageDeclaration(const std::string &name) : packageName(name) {}
        void        accept(ASTVisitor &visitor) override;
        std::string toString() const override;
    };

    // Import Declaration
    class ImportDeclaration : public ASTNode {
    public:
        std::string importPath;

        ImportDeclaration(const std::string &path) : importPath(path) {}
        void        accept(ASTVisitor &visitor) override;
        std::string toString() const override;
    };

    // Class Declaration
    class ClassDeclaration : public ASTNode {
    public:
        std::string                           accessModifier;
        std::string                           className;
        std::vector<std::unique_ptr<ASTNode>> members;

        ClassDeclaration(const std::string &access, const std::string &name)
            : accessModifier(access), className(name) {}
        void        accept(ASTVisitor &visitor) override;
        std::string toString() const override;
    };

    // Method Declaration
    class MethodDeclaration : public ASTNode {
    public:
        std::string                                      accessModifier;
        bool                                             isStatic;
        std::string                                      returnType;
        std::string                                      methodName;
        std::vector<std::pair<std::string, std::string>> parameters; // type, name pairs
        std::vector<std::unique_ptr<ASTNode>>            body;

        MethodDeclaration(const std::string &access, bool stat, const std::string &retType,
                          const std::string &name)
            : accessModifier(access), isStatic(stat), returnType(retType), methodName(name) {}
        void        accept(ASTVisitor &visitor) override;
        std::string toString() const override;
    };

    // Expression Statement
    class ExpressionStatement : public ASTNode {
    public:
        std::unique_ptr<ASTNode> expression;

        ExpressionStatement(std::unique_ptr<ASTNode> expr) : expression(std::move(expr)) {}
        void        accept(ASTVisitor &visitor) override;
        std::string toString() const override;
    };

    // Binary Expression (for << operator)
    class BinaryExpression : public ASTNode {
    public:
        std::unique_ptr<ASTNode> left;
        std::string              operator_;
        std::unique_ptr<ASTNode> right;

        BinaryExpression(std::unique_ptr<ASTNode> l, const std::string &op, std::unique_ptr<ASTNode> r)
            : left(std::move(l)), operator_(op), right(std::move(r)) {}
        void        accept(ASTVisitor &visitor) override;
        std::string toString() const override;
    };

    // Identifier
    class Identifier : public ASTNode {
    public:
        std::string name;

        Identifier(const std::string &n) : name(n) {}
        void        accept(ASTVisitor &visitor) override;
        std::string toString() const override;
    };

    // String Literal
    class StringLiteral : public ASTNode {
    public:
        std::string value;

        StringLiteral(const std::string &val) : value(val) {}
        void        accept(ASTVisitor &visitor) override;
        std::string toString() const override;
    };

    // Number Literal
    class NumberLiteral : public ASTNode {
    public:
        std::string value;

        NumberLiteral(const std::string &val) : value(val) {}
        void        accept(ASTVisitor &visitor) override;
        std::string toString() const override;
    };

    // Return Statement
    class ReturnStatement : public ASTNode {
    public:
        std::unique_ptr<ASTNode> expression;

        ReturnStatement(std::unique_ptr<ASTNode> expr) : expression(std::move(expr)) {}
        void        accept(ASTVisitor &visitor) override;
        std::string toString() const override;
    };

    // Visitor pattern for AST traversal
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

#endif // AST_H