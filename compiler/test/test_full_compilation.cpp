#include "../compiler.h"
#include <iostream>
#include <fstream>
#include <sstream>

using namespace Ryntra::Compiler;

int main() {
    std::cout << "=== Full Compilation Pipeline Test ===" << std::endl;
    
    // Test with HelloWorld.rynt
    std::string filename = R"(E:\RyntraProj\compiler\test\HelloWorld.rynt)";
    
    std::cout << "\nReading source file: " << filename << std::endl;
    
    // Read the file
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open file " << filename << std::endl;
        return 1;
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string source = buffer.str();
    
    RyntraCompiler compiler(source);
    
    // Show original source
    std::cout << "\n=== Source Code ===" << std::endl;
    std::cout << source << std::endl;
    
    // Complete compilation pipeline
    std::cout << "\n=== Starting Full Compilation ===" << std::endl;
    
    bool success = compiler.compileToExecutable("HelloWorldTest");
    
    if (success) {
        std::cout << "\n🎉 Full compilation successful!" << std::endl;
        std::cout << "\nGenerated files should include:" << std::endl;
        std::cout << "  - HelloWorldTest.ll (LLVM IR)" << std::endl;
        std::cout << "  - HelloWorldTest.bc (Bytecode)" << std::endl;
        std::cout << "  - HelloWorldTest.obj (Object file)" << std::endl;
        std::cout << "  - HelloWorldTest.exe (Executable)" << std::endl;
        
        std::cout << "\nTry running: ./HelloWorldTest.exe" << std::endl;
    } else {
        std::cout << "\n❌ Full compilation failed!" << std::endl;
        return 1;
    }
    
    return 0;
}