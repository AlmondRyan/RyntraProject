#include "llvm_codegen.h"
#include <llvm/IR/Constants.h>
#include <llvm/IR/GlobalVariable.h>
#include <llvm/IR/Instructions.h>
#include <llvm/Bitcode/BitcodeWriter.h>
#include <llvm/IRReader/IRReader.h>
#include <llvm/Support/SourceMgr.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Target/TargetMachine.h>
#include <llvm/MC/TargetRegistry.h>
#include <llvm/IR/LegacyPassManager.h>
#include <iostream>
#include <fstream>
#include <cstdlib>

namespace Ryntra {
namespace Compiler {

LLVMCodeGenerator::LLVMCodeGenerator(const std::string& moduleName) 
    : lastValue(nullptr), currentFunction(nullptr) {
    context = std::make_unique<llvm::LLVMContext>();
    module = std::make_unique<llvm::Module>(moduleName, *context);
    builder = std::make_unique<llvm::IRBuilder<>>(*context);
}

llvm::Type* LLVMCodeGenerator::getLLVMType(const std::string& typeName) {
    if (typeName == "int") {
        return llvm::Type::getInt32Ty(*context);
    } else if (typeName == "string" || typeName == "string[]") {
        llvm::Type *Int8Type = llvm::Type::getInt8Ty(*context);
        llvm::PointerType *Int8PtrType = llvm::PointerType::getUnqual(Int8Type);
        return Int8PtrType;
    } else if (typeName == "void") {
        return llvm::Type::getVoidTy(*context);
    }
    
    // Default to i32 for unknown types
    return llvm::Type::getInt32Ty(*context);
}

llvm::Function* LLVMCodeGenerator::createPrintFunction() {
    // Create printf function declaration
    std::vector<llvm::Type*> printfArgs;
    llvm::Type *Int8Type = llvm::Type::getInt8Ty(*context);
    llvm::PointerType *Int8PtrType = llvm::PointerType::getUnqual(Int8Type);

    // printfArgs.push_back(llvm::Type::getInt8PtrTy(*context));
    printfArgs.push_back(Int8PtrType);
    
    llvm::FunctionType* printfType = llvm::FunctionType::get(
        llvm::Type::getInt32Ty(*context), printfArgs, true);
    
    llvm::Function* printfFunc = llvm::Function::Create(
        printfType, llvm::Function::ExternalLinkage, "printf", module.get());
    
    return printfFunc;
}

llvm::Value* LLVMCodeGenerator::createStringConstant(const std::string& str) {
    // Create a global string constant
    llvm::Constant* strConstant = llvm::ConstantDataArray::getString(*context, str, true);
    
    llvm::GlobalVariable* globalStr = new llvm::GlobalVariable(
        *module, strConstant->getType(), true, llvm::GlobalValue::PrivateLinkage,
        strConstant, ".str");
    
    // Get pointer to the string
    std::vector<llvm::Value*> indices;
    indices.push_back(llvm::ConstantInt::get(llvm::Type::getInt32Ty(*context), 0));
    indices.push_back(llvm::ConstantInt::get(llvm::Type::getInt32Ty(*context), 0));
    
    return builder->CreateInBoundsGEP(globalStr->getValueType(), globalStr, indices, "str");
}

void LLVMCodeGenerator::generateCode(Program& program) {
    // Create printf function
    functions["printf"] = createPrintFunction();
    
    // Visit the program
    program.accept(*this);
}

void LLVMCodeGenerator::printIR() {
    module->print(llvm::outs(), nullptr);
}

void LLVMCodeGenerator::visit(Program& node) {
    for (auto& decl : node.declarations) {
        decl->accept(*this);
    }
}

void LLVMCodeGenerator::visit(PackageDeclaration& node) {
    // Package declarations don't generate code, just metadata
    // Could add module metadata here if needed
}

void LLVMCodeGenerator::visit(ImportDeclaration& node) {
    // Import declarations don't generate code in this simple implementation
    // In a full implementation, this would handle linking external modules
}

void LLVMCodeGenerator::visit(ClassDeclaration& node) {
    // For simplicity, we'll treat classes as namespaces for static methods
    // Visit all members
    for (auto& member : node.members) {
        member->accept(*this);
    }
}

void LLVMCodeGenerator::visit(MethodDeclaration& node) {
    // Create function type
    std::vector<llvm::Type*> paramTypes;
    for (const auto& param : node.parameters) {
        paramTypes.push_back(getLLVMType(param.first));
    }
    
    llvm::Type* returnType = getLLVMType(node.returnType);
    llvm::FunctionType* funcType = llvm::FunctionType::get(returnType, paramTypes, false);
    
    // Create function
    std::string funcName = node.methodName;
    if (funcName == "main") {
        // Special handling for main function
        funcName = "main";
        // Override to match C main signature
        std::vector<llvm::Type*> mainParams;
        if (!node.parameters.empty()) {
            mainParams.push_back(llvm::Type::getInt32Ty(*context)); // argc
            // mainParams.push_back(llvm::Type::getInt8PtrTy(*context)->getPointerTo()); // argv
            llvm::Type *Int8Type = llvm::Type::getInt8Ty(*context);
            llvm::PointerType *Int8PtrType = llvm::PointerType::getUnqual(Int8Type);
            mainParams.push_back(Int8PtrType->getPointerTo());
        }
        funcType = llvm::FunctionType::get(llvm::Type::getInt32Ty(*context), mainParams, false);
    }
    
    llvm::Function* function = llvm::Function::Create(
        funcType, llvm::Function::ExternalLinkage, funcName, module.get());
    
    functions[funcName] = function;
    currentFunction = function;
    
    // Create basic block
    llvm::BasicBlock* entry = llvm::BasicBlock::Create(*context, "entry", function);
    builder->SetInsertPoint(entry);
    
    // Set parameter names
    auto argIt = function->arg_begin();
    for (size_t i = 0; i < node.parameters.size() && argIt != function->arg_end(); ++i, ++argIt) {
        argIt->setName(node.parameters[i].second);
        namedValues[node.parameters[i].second] = &*argIt;
    }
    
    // Generate function body
    for (auto& stmt : node.body) {
        stmt->accept(*this);
    }
    
    // Verify function
    if (llvm::verifyFunction(*function, &llvm::errs())) {
        std::cerr << "Error: Function verification failed for " << funcName << std::endl;
    }
    
    currentFunction = nullptr;
}

void LLVMCodeGenerator::visit(ExpressionStatement& node) {
    node.expression->accept(*this);
}

void LLVMCodeGenerator::visit(BinaryExpression& node) {
    // Handle << operator (stream output)
    if (node.operator_ == "<<") {
        // Left side should be io.out, right side is what to print
        node.right->accept(*this);
        llvm::Value* printValue = lastValue;
        
        if (printValue) {
            // Call printf
            llvm::Function* printfFunc = functions["printf"];
            if (printfFunc) {
                std::vector<llvm::Value*> args;
                args.push_back(printValue);
                
                lastValue = builder->CreateCall(printfFunc, args, "printf_call");
            }
        }
        return;
    }
    
    // Handle other binary operators
    node.left->accept(*this);
    llvm::Value* left = lastValue;
    
    node.right->accept(*this);
    llvm::Value* right = lastValue;
    
    if (!left || !right) {
        lastValue = nullptr;
        return;
    }
    
    // For now, just handle basic arithmetic (not used in HelloWorld but good to have)
    if (node.operator_ == "+") {
        lastValue = builder->CreateAdd(left, right, "addtmp");
    } else if (node.operator_ == "-") {
        lastValue = builder->CreateSub(left, right, "subtmp");
    } else if (node.operator_ == "*") {
        lastValue = builder->CreateMul(left, right, "multmp");
    } else if (node.operator_ == "/") {
        lastValue = builder->CreateSDiv(left, right, "divtmp");
    } else {
        lastValue = nullptr;
    }
}

void LLVMCodeGenerator::visit(Identifier& node) {
    // Handle special identifiers
    if (node.name == "io.out") {
        // io.out doesn't generate a value by itself
        lastValue = nullptr;
        return;
    }
    
    // Look up variable
    auto it = namedValues.find(node.name);
    if (it != namedValues.end()) {
        lastValue = it->second;
    } else {
        std::cerr << "Unknown variable: " << node.name << std::endl;
        lastValue = nullptr;
    }
}

void LLVMCodeGenerator::visit(StringLiteral& node) {
    lastValue = createStringConstant(node.value);
}

void LLVMCodeGenerator::visit(NumberLiteral& node) {
    int value = std::stoi(node.value);
    lastValue = llvm::ConstantInt::get(llvm::Type::getInt32Ty(*context), value);
}

void LLVMCodeGenerator::visit(ReturnStatement& node) {
    if (node.expression) {
        node.expression->accept(*this);
        if (lastValue) {
            builder->CreateRet(lastValue);
        } else {
            // Return 0 if no value
            builder->CreateRet(llvm::ConstantInt::get(llvm::Type::getInt32Ty(*context), 0));
        }
    } else {
        builder->CreateRetVoid();
    }
}

bool LLVMCodeGenerator::saveIRToFile(const std::string& filename) {
    std::error_code EC;
    llvm::raw_fd_ostream dest(filename, EC, llvm::sys::fs::OF_None);
    
    if (EC) {
        std::cerr << "Could not open file: " << EC.message() << std::endl;
        return false;
    }
    
    module->print(dest, nullptr);
    dest.close();
    
    std::cout << "LLVM IR saved to: " << filename << std::endl;
    return true;
}

bool LLVMCodeGenerator::generateBytecode(const std::string& llFilename, const std::string& bcFilename) {
    // Read the IR file
    llvm::SMDiagnostic Err;
    std::unique_ptr<llvm::Module> M = llvm::parseIRFile(llFilename, Err, *context);
    
    if (!M) {
        std::cerr << "Error reading IR file: " << llFilename << std::endl;
        Err.print("llvm-as", llvm::errs());
        return false;
    }
    
    // Write bytecode
    std::error_code EC;
    llvm::raw_fd_ostream dest(bcFilename, EC, llvm::sys::fs::OF_None);
    
    if (EC) {
        std::cerr << "Could not open file: " << EC.message() << std::endl;
        return false;
    }
    
    llvm::WriteBitcodeToFile(*M, dest);
    dest.close();
    
    std::cout << "Bytecode saved to: " << bcFilename << std::endl;
    return true;
}

bool LLVMCodeGenerator::generateObjectFile(const std::string& bcFilename, const std::string& objFilename) {
    // Initialize targets
    llvm::InitializeAllTargetInfos();
    llvm::InitializeAllTargets();
    llvm::InitializeAllTargetMCs();
    llvm::InitializeAllAsmParsers();
    llvm::InitializeAllAsmPrinters();
    
    // Read bytecode
    llvm::SMDiagnostic Err;
    std::unique_ptr<llvm::Module> M = llvm::parseIRFile(bcFilename, Err, *context);
    
    if (!M) {
        std::cerr << "Error reading bytecode file: " << bcFilename << std::endl;
        Err.print("llc", llvm::errs());
        return false;
    }
    
    // Get target triple for Windows x64
    std::string TargetTriple = "x86_64-pc-windows-msvc";
    M->setTargetTriple(TargetTriple);
    
    std::string Error;
    const llvm::Target* Target = llvm::TargetRegistry::lookupTarget(TargetTriple, Error);
    
    if (!Target) {
        std::cerr << "Error: " << Error << std::endl;
        return false;
    }
    
    std::string CPU = "generic";
    std::string Features = "";
    
    llvm::TargetOptions opt;
    std::unique_ptr<llvm::TargetMachine> TheTargetMachine(
        Target->createTargetMachine(TargetTriple, CPU, Features, opt, llvm::Reloc::PIC_));
    
    M->setDataLayout(TheTargetMachine->createDataLayout());
    
    // Open output file
    std::error_code EC;
    llvm::raw_fd_ostream dest(objFilename, EC, llvm::sys::fs::OF_None);
    
    if (EC) {
        std::cerr << "Could not open file: " << EC.message() << std::endl;
        return false;
    }
    
    // Generate object file
    llvm::legacy::PassManager pass;
    if (TheTargetMachine->addPassesToEmitFile(pass, dest, nullptr, llvm::CodeGenFileType::ObjectFile)) {
        std::cerr << "TheTargetMachine can't emit a file of this type" << std::endl;
        return false;
    }
    
    pass.run(*M);
    dest.close();
    
    std::cout << "Object file saved to: " << objFilename << std::endl;
    return true;
}

bool LLVMCodeGenerator::linkExecutable(const std::string& objFilename, const std::string& exeFilename) {
    // Use clang to link the executable
    std::string linkCommand = "clang " + objFilename + " -o " + exeFilename;
    
    std::cout << "Running linker: " << linkCommand << std::endl;
    int result = std::system(linkCommand.c_str());
    
    if (result == 0) {
        std::cout << "Executable linked successfully: " << exeFilename << std::endl;
        return true;
    } else {
        std::cerr << "Linking failed with exit code: " << result << std::endl;
        std::cerr << "Make sure clang is installed and available in PATH" << std::endl;
        return false;
    }
}

bool LLVMCodeGenerator::compileToExecutable(const std::string& baseName) {
    std::string llFile = baseName + ".ll";
    std::string bcFile = baseName + ".bc";
    std::string objFile = baseName + ".obj";
    std::string exeFile = baseName + ".exe";
    
    std::cout << "\n=== Complete Compilation Pipeline ===" << std::endl;
    
    // Step 1: Save IR to .ll file
    std::cout << "Step 1: Generating LLVM IR file..." << std::endl;
    if (!saveIRToFile(llFile)) {
        return false;
    }
    
    // Step 2: Generate bytecode from IR
    std::cout << "Step 2: Generating bytecode..." << std::endl;
    if (!generateBytecode(llFile, bcFile)) {
        return false;
    }
    
    // Step 3: Generate object file
    std::cout << "Step 3: Generating object file..." << std::endl;
    if (!generateObjectFile(bcFile, objFile)) {
        return false;
    }
    
    // Step 4: Link executable
    std::cout << "Step 4: Linking executable..." << std::endl;
    if (!linkExecutable(objFile, exeFile)) {
        return false;
    }
    
    std::cout << "\n[Success] Compilation completed successfully!" << std::endl;
    std::cout << "Generated files:" << std::endl;
    std::cout << "  - " << llFile << " (LLVM IR)" << std::endl;
    std::cout << "  - " << bcFile << " (Bytecode)" << std::endl;
    std::cout << "  - " << objFile << " (Object file)" << std::endl;
    std::cout << "  - " << exeFile << " (Executable)" << std::endl;
    
    return true;
}

void LLVMCodeGenerator::compileIRToByteCode() {
    // Legacy method - use the new pipeline instead
    compileToExecutable("output");
}

} // namespace Compiler
} // namespace Ryntra