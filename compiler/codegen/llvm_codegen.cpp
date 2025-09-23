#include "llvm_codegen.h"
#include "../../utils/ErrorHandler.h"
#include "integrated_linker.h"
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <lld/Common/Driver.h>
#include <llvm/Bitcode/BitcodeWriter.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/GlobalVariable.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/IRReader/IRReader.h>
#include <llvm/MC/TargetRegistry.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/Program.h>
#include <llvm/Support/SourceMgr.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Target/TargetMachine.h>
#include <llvm/TargetParser/Host.h>
#include <vector>

namespace Ryntra::Compiler {
    LLVMCodeGenerator::LLVMCodeGenerator(const std::string &moduleName)
        : lastValue(nullptr), currentFunction(nullptr) {
        context = std::make_unique<llvm::LLVMContext>();
        module = std::make_unique<llvm::Module>(moduleName, *context);
        builder = std::make_unique<llvm::IRBuilder<>>(*context);
    }

    llvm::Type *LLVMCodeGenerator::getLLVMType(const std::string &typeName) {
        if (typeName == "int") {
            return llvm::Type::getInt32Ty(*context);
        } else if (typeName == "string" || typeName == "string[]") {
            llvm::Type        *Int8Type = llvm::Type::getInt8Ty(*context);
            llvm::PointerType *Int8PtrType = llvm::PointerType::getUnqual(Int8Type);
            return Int8PtrType;
        } else if (typeName == "void") {
            return llvm::Type::getVoidTy(*context);
        }

        // Default to i32 for unknown types
        return llvm::Type::getInt32Ty(*context);
    }

    llvm::Function *LLVMCodeGenerator::createPrintFunction() {
        // Create platform-specific print function
        std::string hostTriple = llvm::sys::getDefaultTargetTriple();

        llvm::Type        *Int32Type = llvm::Type::getInt32Ty(*context);
        llvm::Type        *Int8Type = llvm::Type::getInt8Ty(*context);
        llvm::PointerType *Int8PtrType = llvm::PointerType::getUnqual(Int8Type);

        if (hostTriple.find("windows") != std::string::npos ||
            hostTriple.find("win32") != std::string::npos ||
            hostTriple.find("msvc") != std::string::npos) {

            // Windows: Use WriteConsoleA
            return createWindowsPrintFunction();

        } else if (hostTriple.find("linux") != std::string::npos) {

            // Linux: Use write syscall
            return createLinuxPrintFunction();

        } else if (hostTriple.find("darwin") != std::string::npos) {

            // macOS: Use write syscall
            return createMacOSPrintFunction();

        } else {

            // Fallback: Create a simple printf declaration
            std::vector<llvm::Type *> printfArgs;
            printfArgs.push_back(Int8PtrType);

            llvm::FunctionType *printfType = llvm::FunctionType::get(
                Int32Type, printfArgs, true);

            llvm::Function *printfFunc = llvm::Function::Create(
                printfType, llvm::Function::ExternalLinkage, "printf", module.get());

            functions["printf"] = printfFunc;
            return printfFunc;
        }
    }

    llvm::Function *LLVMCodeGenerator::createWindowsPrintFunction() {
        llvm::Type        *Int32Type = llvm::Type::getInt32Ty(*context);
        llvm::Type        *Int8Type = llvm::Type::getInt8Ty(*context);
        llvm::PointerType *Int8PtrType = llvm::PointerType::getUnqual(Int8Type);
        llvm::PointerType *Int32PtrType = llvm::PointerType::getUnqual(Int32Type);

        // Create WriteConsoleA function declaration
        std::vector<llvm::Type *> writeConsoleArgs;
        writeConsoleArgs.push_back(Int32Type);    // HANDLE hConsoleOutput
        writeConsoleArgs.push_back(Int8PtrType);  // LPCSTR lpBuffer
        writeConsoleArgs.push_back(Int32Type);    // DWORD nNumberOfCharsToWrite
        writeConsoleArgs.push_back(Int32PtrType); // LPDWORD lpNumberOfCharsWritten
        writeConsoleArgs.push_back(Int8PtrType);  // LPVOID lpReserved

        llvm::FunctionType *writeConsoleType = llvm::FunctionType::get(
            Int32Type, writeConsoleArgs, false);

        llvm::Function *writeConsoleFunc = llvm::Function::Create(
            writeConsoleType, llvm::Function::ExternalLinkage, "WriteConsoleA", module.get());

        // Create GetStdHandle function declaration
        std::vector<llvm::Type *> getStdHandleArgs;
        getStdHandleArgs.push_back(Int32Type);

        llvm::FunctionType *getStdHandleType = llvm::FunctionType::get(
            Int32Type, getStdHandleArgs, false);

        llvm::Function *getStdHandleFunc = llvm::Function::Create(
            getStdHandleType, llvm::Function::ExternalLinkage, "GetStdHandle", module.get());

        // Create our print function
        std::vector<llvm::Type *> printArgs;
        printArgs.push_back(Int8PtrType);

        llvm::FunctionType *printType = llvm::FunctionType::get(
            Int32Type, printArgs, false);

        llvm::Function *printFunc = llvm::Function::Create(
            printType, llvm::Function::ExternalLinkage, "ryntra_print", module.get());

        functions["WriteConsoleA"] = writeConsoleFunc;
        functions["GetStdHandle"] = getStdHandleFunc;
        functions["ryntra_print"] = printFunc;

        return printFunc;
    }

    llvm::Function *LLVMCodeGenerator::createLinuxPrintFunction() {
        // Use write syscall for Linux
        llvm::Type        *Int32Type = llvm::Type::getInt32Ty(*context);
        llvm::Type        *Int8Type = llvm::Type::getInt8Ty(*context);
        llvm::PointerType *Int8PtrType = llvm::PointerType::getUnqual(Int8Type);

        // Create write function declaration
        std::vector<llvm::Type *> writeArgs;
        writeArgs.push_back(Int32Type);   // fd
        writeArgs.push_back(Int8PtrType); // buf
        writeArgs.push_back(Int32Type);   // count

        llvm::FunctionType *writeType = llvm::FunctionType::get(
            Int32Type, writeArgs, false);

        llvm::Function *writeFunc = llvm::Function::Create(
            writeType, llvm::Function::ExternalLinkage, "write", module.get());

        // Create strlen function declaration
        std::vector<llvm::Type *> strlenArgs;
        strlenArgs.push_back(Int8PtrType);

        llvm::FunctionType *strlenType = llvm::FunctionType::get(
            Int32Type, strlenArgs, false);

        llvm::Function *strlenFunc = llvm::Function::Create(
            strlenType, llvm::Function::ExternalLinkage, "strlen", module.get());

        functions["write"] = writeFunc;
        functions["strlen"] = strlenFunc;

        return writeFunc;
    }

    llvm::Function *LLVMCodeGenerator::createMacOSPrintFunction() {
        // Similar to Linux, use write syscall
        return createLinuxPrintFunction();
    }

    std::string LLVMCodeGenerator::findWindowsSDK() {
        // Common Windows SDK paths
        std::vector<std::string> basePaths = {
            "C:\\Program Files (x86)\\Windows Kits\\10\\Lib",
            "C:\\Program Files\\Windows Kits\\10\\Lib"};

        // Common SDK versions (newest first)
        std::vector<std::string> versions = {
            "10.0.22621.0", "10.0.19041.0", "10.0.18362.0", "10.0.17763.0", "10.0.16299.0"};

        for (const auto &basePath : basePaths) {
            for (const auto &version : versions) {
                std::string fullPath = basePath + "\\" + version + "\\um\\x64";

                // Check if kernel32.lib exists in this path
                std::string testFile = fullPath + "\\kernel32.lib";
                if (llvm::sys::fs::exists(testFile)) {
                    return fullPath;
                }
            }
        }

        return ""; // Not found
    }

    llvm::Value *LLVMCodeGenerator::createStringConstant(const std::string &str) {
        // Create a global string constant
        llvm::Constant *strConstant = llvm::ConstantDataArray::getString(*context, str, true);

        llvm::GlobalVariable *globalStr = new llvm::GlobalVariable(
            *module, strConstant->getType(), true, llvm::GlobalValue::PrivateLinkage,
            strConstant, ".str");

        // Get pointer to the string
        std::vector<llvm::Value *> indices;
        indices.push_back(llvm::ConstantInt::get(llvm::Type::getInt32Ty(*context), 0));
        indices.push_back(llvm::ConstantInt::get(llvm::Type::getInt32Ty(*context), 0));

        return builder->CreateInBoundsGEP(globalStr->getValueType(), globalStr, indices, "str");
    }

    void LLVMCodeGenerator::generateCode(Program &program) {
        // Create print function (simplified for now)
        llvm::Type        *Int32Type = llvm::Type::getInt32Ty(*context);
        llvm::Type        *Int8Type = llvm::Type::getInt8Ty(*context);
        llvm::PointerType *Int8PtrType = llvm::PointerType::getUnqual(Int8Type);

        std::vector<llvm::Type *> printArgs;
        printArgs.push_back(Int8PtrType);

        llvm::FunctionType *printType = llvm::FunctionType::get(
            Int32Type, printArgs, true);

        llvm::Function *printFunc = llvm::Function::Create(
            printType, llvm::Function::ExternalLinkage, "printf", module.get());

        functions["printf"] = printFunc;

        // Visit the program
        program.accept(*this);
    }

    void LLVMCodeGenerator::printIR() {
        module->print(llvm::outs(), nullptr);
    }

    void LLVMCodeGenerator::visit(Program &node) {
        for (auto &decl : node.declarations) {
            decl->accept(*this);
        }
    }

    void LLVMCodeGenerator::visit(PackageDeclaration &node) {
        // Package declarations don't generate code, just metadata
        // Could add module metadata here if needed
    }

    void LLVMCodeGenerator::visit(ImportDeclaration &node) {
        // Import declarations don't generate code in this simple implementation
        // In a full implementation, this would handle linking external modules
    }

    void LLVMCodeGenerator::visit(ClassDeclaration &node) {
        // For simplicity, we'll treat classes as namespaces for static methods
        // Visit all members
        for (auto &member : node.members) {
            member->accept(*this);
        }
    }

    void LLVMCodeGenerator::visit(MethodDeclaration &node) {
        // Create function type
        std::vector<llvm::Type *> paramTypes;
        for (const auto &param : node.parameters) {
            paramTypes.push_back(getLLVMType(param.first));
        }

        llvm::Type         *returnType = getLLVMType(node.returnType);
        llvm::FunctionType *funcType = llvm::FunctionType::get(returnType, paramTypes, false);

        // Create function
        std::string funcName = node.methodName;
        if (funcName == "main") {
            // Special handling for main function
            funcName = "main";
            // Override to match C main signature
            std::vector<llvm::Type *> mainParams;
            if (!node.parameters.empty()) {
                mainParams.push_back(llvm::Type::getInt32Ty(*context)); // argc
                // mainParams.push_back(llvm::Type::getInt8PtrTy(*context)->getPointerTo()); // argv
                llvm::Type        *Int8Type = llvm::Type::getInt8Ty(*context);
                llvm::PointerType *Int8PtrType = llvm::PointerType::getUnqual(Int8Type);
                mainParams.push_back(Int8PtrType->getPointerTo());
            }
            funcType = llvm::FunctionType::get(llvm::Type::getInt32Ty(*context), mainParams, false);
        }

        llvm::Function *function = llvm::Function::Create(
            funcType, llvm::Function::ExternalLinkage, funcName, module.get());

        functions[funcName] = function;
        currentFunction = function;

        // Create basic block
        llvm::BasicBlock *entry = llvm::BasicBlock::Create(*context, "entry", function);
        builder->SetInsertPoint(entry);

        // Set parameter names
        auto argIt = function->arg_begin();
        for (size_t i = 0; i < node.parameters.size() && argIt != function->arg_end(); ++i, ++argIt) {
            argIt->setName(node.parameters[i].second);
            namedValues[node.parameters[i].second] = &*argIt;
        }

        // Generate function body
        for (auto &stmt : node.body) {
            stmt->accept(*this);
        }

        // Verify function
        if (llvm::verifyFunction(*function, &llvm::errs())) {
            Ryntra::Utils::globalErrorHandler.makeError(
                "codegen",
                "Function verification failed for " + funcName,
                0, 0);
        }

        currentFunction = nullptr;
    }

    void LLVMCodeGenerator::visit(ExpressionStatement &node) {
        node.expression->accept(*this);
    }

    void LLVMCodeGenerator::visit(BinaryExpression &node) {
        // Handle << operator (stream output)
        if (node.operator_ == "<<") {
            // Left side should be io.out, right side is what to print
            node.right->accept(*this);
            llvm::Value *printValue = lastValue;

            if (printValue) {
                // Call printf function
                llvm::Function *printFunc = functions["printf"];
                if (printFunc) {
                    std::vector<llvm::Value *> args;
                    args.push_back(printValue);

                    lastValue = builder->CreateCall(printFunc, args, "printf_call");
                }
            }
            return;
        }

        // Handle other binary operators
        node.left->accept(*this);
        llvm::Value *left = lastValue;

        node.right->accept(*this);
        llvm::Value *right = lastValue;

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

    void LLVMCodeGenerator::visit(Identifier &node) {
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
            Ryntra::Utils::globalErrorHandler.makeWarning(
                "codegen",
                "Unknown variable: " + node.name,
                0, 0);
            lastValue = nullptr;
        }
    }

    void LLVMCodeGenerator::visit(StringLiteral &node) {
        lastValue = createStringConstant(node.value);
    }

    void LLVMCodeGenerator::visit(NumberLiteral &node) {
        int value = std::stoi(node.value);
        lastValue = llvm::ConstantInt::get(llvm::Type::getInt32Ty(*context), value);
    }

    void LLVMCodeGenerator::visit(ReturnStatement &node) {
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

    bool LLVMCodeGenerator::saveIRToFile(const std::string &filename) {
        std::error_code      EC;
        llvm::raw_fd_ostream dest(filename, EC, llvm::sys::fs::OF_None);

        if (EC) {
            Ryntra::Utils::globalErrorHandler.makeError(
                "codegen",
                "Could not open file for IR output: " + EC.message(),
                0, 0);
            return false;
        }

        module->print(dest, nullptr);
        dest.close();

        std::cout << "LLVM IR saved to: " << filename << std::endl;
        return true;
    }

    bool LLVMCodeGenerator::generateBytecode(const std::string &llFilename, const std::string &bcFilename) {
        // Read the IR file
        llvm::SMDiagnostic            Err;
        std::unique_ptr<llvm::Module> M = llvm::parseIRFile(llFilename, Err, *context);

        if (!M) {
            Ryntra::Utils::globalErrorHandler.makeError(
                "codegen",
                "Error reading IR file: " + llFilename,
                0, 0);
            Err.print("llvm-as", llvm::errs());
            return false;
        }

        // Write bytecode
        std::error_code      EC;
        llvm::raw_fd_ostream dest(bcFilename, EC, llvm::sys::fs::OF_None);

        if (EC) {
            Ryntra::Utils::globalErrorHandler.makeError(
                "codegen",
                "Could not open file for bytecode output: " + EC.message(),
                0, 0);
            return false;
        }

        llvm::WriteBitcodeToFile(*M, dest);
        dest.close();

        std::cout << "Bytecode saved to: " << bcFilename << std::endl;
        return true;
    }

    bool LLVMCodeGenerator::generateObjectFile(const std::string &bcFilename, const std::string &objFilename) {
        // Initialize targets
        llvm::InitializeAllTargetInfos();
        llvm::InitializeAllTargets();
        llvm::InitializeAllTargetMCs();
        llvm::InitializeAllAsmParsers();
        llvm::InitializeAllAsmPrinters();

        // Read bytecode
        llvm::SMDiagnostic            Err;
        std::unique_ptr<llvm::Module> M = llvm::parseIRFile(bcFilename, Err, *context);

        if (!M) {
            Ryntra::Utils::globalErrorHandler.makeError(
                "codegen",
                "Error reading bytecode file: " + bcFilename,
                0, 0);
            Err.print("llc", llvm::errs());
            return false;
        }

        // Get target triple for Windows x64
        std::string TargetTriple = "x86_64-pc-windows-msvc";
        M->setTargetTriple(TargetTriple);

        std::string         Error;
        const llvm::Target *Target = llvm::TargetRegistry::lookupTarget(TargetTriple, Error);

        if (!Target) {
            Ryntra::Utils::globalErrorHandler.makeError(
                "codegen",
                "Target lookup error: " + Error,
                0, 0);
            return false;
        }

        std::string CPU = "generic";
        std::string Features = "";

        llvm::TargetOptions                  opt;
        std::unique_ptr<llvm::TargetMachine> TheTargetMachine(
            Target->createTargetMachine(TargetTriple, CPU, Features, opt, llvm::Reloc::PIC_));

        M->setDataLayout(TheTargetMachine->createDataLayout());

        // Open output file
        std::error_code      EC;
        llvm::raw_fd_ostream dest(objFilename, EC, llvm::sys::fs::OF_None);

        if (EC) {
            Ryntra::Utils::globalErrorHandler.makeError(
                "codegen",
                "Could not open file for object output: " + EC.message(),
                0, 0);
            return false;
        }

        // Generate object file
        llvm::legacy::PassManager pass;
        if (TheTargetMachine->addPassesToEmitFile(pass, dest, nullptr, llvm::CodeGenFileType::ObjectFile)) {
            Ryntra::Utils::globalErrorHandler.makeError(
                "codegen",
                "TheTargetMachine can't emit a file of this type",
                0, 0);
            return false;
        }

        pass.run(*M);
        dest.close();

        std::cout << "Object file saved to: " << objFilename << std::endl;
        return true;
    }

    std::vector<std::string> LLVMCodeGenerator::commandGenerate(const std::string &objFile, const std::string &exeFile) {
        std::vector<std::string> args;

        // Get target triple
        std::string targetTriple = llvm::sys::getDefaultTargetTriple();
        std::cout << "Target triple: " << targetTriple << std::endl;

        // Parse target triple components
        bool isWindows = (targetTriple.find("windows") != std::string::npos ||
                          targetTriple.find("win32") != std::string::npos ||
                          targetTriple.find("msvc") != std::string::npos);
        bool isLinux = targetTriple.find("linux") != std::string::npos;
        bool isMacOS = (targetTriple.find("darwin") != std::string::npos ||
                        targetTriple.find("macos") != std::string::npos);
        bool is64Bit = targetTriple.find("x86_64") != std::string::npos;
        bool isMSVC = targetTriple.find("msvc") != std::string::npos;

        std::cout << "Platform analysis:" << std::endl;
        std::cout << "  Windows: " << (isWindows ? "Yes" : "No") << std::endl;
        std::cout << "  Linux: " << (isLinux ? "Yes" : "No") << std::endl;
        std::cout << "  macOS: " << (isMacOS ? "Yes" : "No") << std::endl;
        std::cout << "  64-bit: " << (is64Bit ? "Yes" : "No") << std::endl;
        std::cout << "  MSVC: " << (isMSVC ? "Yes" : "No") << std::endl;

        if (isWindows) {
            // Windows linking with lld-link
            std::cout << "Generating Windows COFF linking command..." << std::endl;

            // Output file - ensure single .exe extension
            std::string outputFile = exeFile;
            if (outputFile.find(".exe") == std::string::npos) {
                outputFile += ".exe";
            }
            args.push_back("/OUT:" + outputFile);

            // Input object file
            args.push_back(objFile);

            // Find and add Windows SDK path
            std::string sdkPath = findWindowsSDK();
            if (!sdkPath.empty()) {
                // Don't add quotes here, let callLinker handle it
                args.push_back("/LIBPATH:" + sdkPath);
                std::cout << "Using Windows SDK: " << sdkPath << std::endl;
            }

            // Required libraries
            args.push_back("kernel32.lib");
            args.push_back("msvcrt.lib");
            args.push_back("legacy_stdio_definitions.lib"); // For printf support

            // Linker settings
            args.push_back("/SUBSYSTEM:CONSOLE");
            args.push_back("/ENTRY:mainCRTStartup");
            args.push_back(is64Bit ? "/MACHINE:X64" : "/MACHINE:X86");

        } else if (isLinux) {
            // Linux linking with ld.lld
            std::cout << "Generating Linux ELF linking command..." << std::endl;

            args.push_back("-o");
            args.push_back(exeFile);
            args.push_back(objFile);
            args.push_back("-lc");

        } else if (isMacOS) {
            // macOS linking with ld64.lld
            std::cout << "Generating macOS Mach-O linking command..." << std::endl;

            args.push_back("-o");
            args.push_back(exeFile);
            args.push_back(objFile);
            args.push_back("-lSystem");

        } else {
            // Unknown platform
            Ryntra::Utils::globalErrorHandler.makeError(
                "codegen",
                "Unsupported target platform: " + targetTriple,
                0, 0);
        }

        std::cout << "Generated linker arguments:" << std::endl;
        for (const auto &arg : args) {
            std::cout << "  " << arg << std::endl;
        }

        return args;
    }

    bool LLVMCodeGenerator::callLinker(const std::vector<std::string> &args) {
        if (args.empty()) {
            Ryntra::Utils::globalErrorHandler.makeError(
                "codegen",
                "No linker arguments generated",
                0, 0);
            return false;
        }

        // Determine which linker to use based on target triple
        std::string targetTriple = llvm::sys::getDefaultTargetTriple();
        std::string linkerName;

        if (targetTriple.find("windows") != std::string::npos ||
            targetTriple.find("win32") != std::string::npos ||
            targetTriple.find("msvc") != std::string::npos) {
            linkerName = "lld-link";
        } else if (targetTriple.find("linux") != std::string::npos) {
            linkerName = "ld.lld";
        } else if (targetTriple.find("darwin") != std::string::npos ||
                   targetTriple.find("macos") != std::string::npos) {
            linkerName = "ld64.lld";
        } else {
            linkerName = "lld-link"; // fallback
        }

        std::cout << "Using linker: " << linkerName << std::endl;

        // Try to find the linker
        llvm::ErrorOr<std::string> linkerPath = llvm::sys::findProgramByName(linkerName);
        if (!linkerPath) {
            Ryntra::Utils::globalErrorHandler.makeError(
                "codegen",
                "Linker not found: " + linkerName,
                0, 0);
            Ryntra::Utils::globalErrorHandler.makeNote(
                "codegen",
                "Please ensure LLVM with LLD is installed and in PATH",
                0, 0);
            return false;
        }

        std::cout << "Found linker at: " << linkerPath.get() << std::endl;

        // Prepare arguments for ExecuteAndWait
        std::vector<llvm::StringRef> execArgs;

        // Process arguments for LLVM ExecuteAndWait
        for (const auto &arg : args) {
            // Special handling for /LIBPATH: arguments
            if (arg.find("/LIBPATH:") == 0) {
                std::string path = arg.substr(9); // Remove "/LIBPATH:" prefix
                if (path.find(' ') != std::string::npos) {
                    execArgs.push_back("/LIBPATH:" + path); // ExecuteAndWait handles quotes automatically
                } else {
                    execArgs.push_back(arg);
                }
            } else {
                execArgs.push_back(arg);
            }
        }

        // Debug output
        std::cout << "Executing with LLVM ExecuteAndWait:" << std::endl;
        std::cout << "  Linker: " << linkerPath.get() << std::endl;
        std::cout << "  Arguments:" << std::endl;
        for (const auto &arg : execArgs) {
            std::cout << "    " << arg.str() << std::endl;
        }

        // Execute linker using LLVM's ExecuteAndWait
        std::string errorMsg;
        int         result = llvm::sys::ExecuteAndWait(
            linkerPath.get(), // Program path
            execArgs,         // Arguments
            std::nullopt,     // Environment (use default)
            {},               // Redirects (none)
            0,                // Timeout (no timeout)
            0,                // Memory limit (no limit)
            &errorMsg         // Error message output
        );

        if (result == 0) {
            std::cout << "Linking successful!" << std::endl;
            return true;
        } else {
            Ryntra::Utils::globalErrorHandler.makeError(
                "codegen",
                "Linking failed with exit code: " + std::to_string(result),
                0, 0);

            // Show error message from ExecuteAndWait if available
            if (!errorMsg.empty()) {
                Ryntra::Utils::globalErrorHandler.makeNote(
                    "codegen",
                    "Error details: " + errorMsg,
                    0, 0);
            }

            // Additional debugging info
            std::cout << "Debug info:" << std::endl;
            std::cout << "  Linker: " << linkerPath.get() << std::endl;
            std::cout << "  Exit code: " << result << std::endl;
            if (!errorMsg.empty()) {
                std::cout << "  Error message: " << errorMsg << std::endl;
            }

            // Try to provide helpful suggestions
            if (result == 1) {
                Ryntra::Utils::globalErrorHandler.makeNote(
                    "codegen",
                    "This usually indicates a command syntax error or missing files",
                    0, 0);
            }

            return false;
        }
    }

    bool LLVMCodeGenerator::linkExecutable(const std::string &objFilename, const std::string &exeFilename) {
        // Legacy method - redirect to new pipeline
        auto args = commandGenerate(objFilename, exeFilename);
        return callLinker(args);
    }

    bool LLVMCodeGenerator::compileToExecutable(const std::string &baseName) {
        std::string llFile = baseName + ".ll";
        std::string bcFile = baseName + ".bc";
        std::string objFile = baseName + ".obj";
        std::string exeFile = baseName; // Don't add .exe here, let commandGenerate handle it

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

        // Step 4: Generate linker command
        std::cout << "Step 4: Generating linker command..." << std::endl;
        std::vector<std::string> linkerArgs = commandGenerate(objFile, exeFile);

        // Step 5: Call linker
        std::cout << "Step 5: Calling linker..." << std::endl;
        if (!callLinker(linkerArgs)) {
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

} // namespace Ryntra::Compiler