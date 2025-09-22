#include "../compiler.h"
#include <iostream>
#include <fstream>
#include <sstream>

using namespace Ryntra::Compiler;

int main() {
    std::cout << "=== Ryntra Compiler Test ===" << std::endl;
    
    // Test with HelloWorld.rynt
    std::string filename = R"(E:\RyntraProj\compiler\test\HelloWorld.rynt)";
    
    std::cout << "\nCompiling: " << filename << std::endl;
    
    auto ast = RyntraCompiler::compileFromFile(filename);
    
    if (ast) {
        std::cout << "\n=== Compilation Successful ===" << std::endl;
        
        // Display tokens
        std::ifstream file(filename);
        if (file.is_open()) {
            std::stringstream buffer;
            buffer << file.rdbuf();
            std::string source = buffer.str();
            
            RyntraCompiler compiler(source);
            
            std::cout << "\n=== Tokens ===" << std::endl;
            compiler.printTokens();
            
            std::cout << "\n=== AST ===" << std::endl;
            compiler.printAST();
        }
    } else {
        std::cout << "\n=== Compilation Failed ===" << std::endl;
    }
    
    return 0;
}