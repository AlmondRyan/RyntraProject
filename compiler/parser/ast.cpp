#include "ast.h"

namespace Ryntra::Compiler {
    void Program::accept(ASTVisitor &visitor) {
        visitor.visit(*this);
    }

    std::string Program::toString() const {
        return "Program";
    }

    void PackageDeclaration::accept(ASTVisitor &visitor) {
        visitor.visit(*this);
    }

    std::string PackageDeclaration::toString() const {
        return "PackageDeclaration: " + packageName;
    }

    void ImportDeclaration::accept(ASTVisitor &visitor) {
        visitor.visit(*this);
    }

    std::string ImportDeclaration::toString() const {
        return "ImportDeclaration: " + importPath;
    }

    void ClassDeclaration::accept(ASTVisitor &visitor) {
        visitor.visit(*this);
    }

    std::string ClassDeclaration::toString() const {
        return accessModifier + " class " + className;
    }

    void MethodDeclaration::accept(ASTVisitor &visitor) {
        visitor.visit(*this);
    }

    std::string MethodDeclaration::toString() const {
        std::string result = accessModifier;
        if (isStatic)
            result += " static";
        result += " " + returnType + " " + methodName + "(";
        for (size_t i = 0; i < parameters.size(); ++i) {
            if (i > 0)
                result += ", ";
            result += parameters[i].first + " " + parameters[i].second;
        }
        result += ")";
        return result;
    }

    void ExpressionStatement::accept(ASTVisitor &visitor) {
        visitor.visit(*this);
    }

    std::string ExpressionStatement::toString() const {
        return "ExpressionStatement";
    }

    void BinaryExpression::accept(ASTVisitor &visitor) {
        visitor.visit(*this);
    }

    std::string BinaryExpression::toString() const {
        return "BinaryExpression: " + operator_;
    }

    void Identifier::accept(ASTVisitor &visitor) {
        visitor.visit(*this);
    }

    std::string Identifier::toString() const {
        return "Identifier: " + name;
    }

    void StringLiteral::accept(ASTVisitor &visitor) {
        visitor.visit(*this);
    }

    std::string StringLiteral::toString() const {
        return "StringLiteral: \"" + value + "\"";
    }

    void NumberLiteral::accept(ASTVisitor &visitor) {
        visitor.visit(*this);
    }

    std::string NumberLiteral::toString() const {
        return "NumberLiteral: " + value;
    }

    void ReturnStatement::accept(ASTVisitor &visitor) {
        visitor.visit(*this);
    }

    std::string ReturnStatement::toString() const {
        return "ReturnStatement";
    }
} // namespace Ryntra::Compiler
