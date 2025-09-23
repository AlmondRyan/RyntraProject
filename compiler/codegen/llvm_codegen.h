#ifndef RYNTRA_COMPILER_CODEGEN_LLVM_CODEGEN_H
#define RYNTRA_COMPILER_CODEGEN_LLVM_CODEGEN_H

#include "../parser/ast.h"
#include <llvm/Bitcode/BitcodeWriter.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Value.h>
#include <llvm/IR/Verifier.h>
#include <llvm/IRReader/IRReader.h>
#include <llvm/MC/TargetRegistry.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/SourceMgr.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Target/TargetMachine.h>
#include <llvm/Target/TargetOptions.h>
#include <memory>
#include <string>
#include <unordered_map>

namespace Ryntra {
    namespace Compiler {

        class LLVMCodeGenerator : public ASTVisitor {
        private:
            std::unique_ptr<llvm::LLVMContext> context;
            std::unique_ptr<llvm::Module>      module;
            std::unique_ptr<llvm::IRBuilder<>> builder;

            // Symbol tables
            std::unordered_map<std::string, llvm::Value *>    namedValues;
            std::unordered_map<std::string, llvm::Function *> functions;

            // Current function being generated
            llvm::Function *currentFunction;

            // Helper methods
            llvm::Type     *getLLVMType(const std::string &typeName);
            llvm::Function *createPrintFunction();
            llvm::Function *createWindowsPrintFunction();
            llvm::Function *createLinuxPrintFunction();
            llvm::Function *createMacOSPrintFunction();
            llvm::Value    *createStringConstant(const std::string &str);

            // Platform-specific helpers
            std::string findWindowsSDK();

        public:
            LLVMCodeGenerator(const std::string &moduleName);
            ~LLVMCodeGenerator() = default;

            // Generate LLVM IR from AST
            void generateCode(Program &program);

            // Get the generated module
            llvm::Module *getModule() const { return module.get(); }

            // Print the generated IR
            void printIR();

            // Save IR to file
            bool saveIRToFile(const std::string &filename);

            // Generate bytecode from IR file
            bool generateBytecode(const std::string &llFilename, const std::string &bcFilename);

            // Generate object file from bytecode
            bool generateObjectFile(const std::string &bcFilename, const std::string &objFilename);

            // Link object file to executable
            bool linkExecutable(const std::string &objFilename, const std::string &exeFilename);

            // Complete compilation pipeline
            bool compileToExecutable(const std::string &baseName);

            // New linking pipeline
            std::vector<std::string> commandGenerate(const std::string &objFile, const std::string &exeFile);
            bool                     callLinker(const std::vector<std::string> &args);

            // Legacy method
            void compileIRToByteCode();

            // Visitor methods
            void visit(Program &node) override;
            void visit(PackageDeclaration &node) override;
            void visit(ImportDeclaration &node) override;
            void visit(ClassDeclaration &node) override;
            void visit(MethodDeclaration &node) override;
            void visit(ExpressionStatement &node) override;
            void visit(BinaryExpression &node) override;
            void visit(Identifier &node) override;
            void visit(StringLiteral &node) override;
            void visit(NumberLiteral &node) override;
            void visit(ReturnStatement &node) override;

        private:
            llvm::Value *lastValue; // Store the last generated value
        };

    } // namespace Compiler
} // namespace Ryntra

#endif // RYNTRA_COMPILER_CODEGEN_LLVM_CODEGEN_H