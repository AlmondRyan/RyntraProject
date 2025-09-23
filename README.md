# Ryntra Project

A full development toolchain for The Ryntra Programming Language.

## Features

### Compiler (`compiler/`)
- **Lexer**: DFA/NFA-based tokenization with support for all Ryntra keywords and operators
- **Parser**: LALR(1) parser generating Abstract Syntax Trees (AST)
- **Code Generator**: LLVM IR generation from AST using visitor pattern
- **Namespace**: All components organized under `Ryntra::Compiler` namespace

### Linker (`rplink/`)
- **RPLink**: Custom linker for Ryntra object files
- **Object File Support**: ELF, COFF, and Mach-O format support
- **Symbol Resolution**: Global symbol table and relocation handling
- **Namespace**: All components organized under `Ryntra::RPLink` namespace

### Supported Language Features
- Package declarations: `declare(package) org.example;`
- Import statements: `import ryntra.io.*;`
- Class definitions: `public class ClassName { ... }`
- Method definitions: `public static int main(string[] args) { ... }`
- Stream output: `io.out << "Hello World!";`
- Return statements: `return 0;`

## Building

### Prerequisites
- CMake 3.31+
- C++23 compatible compiler
- LLVM development libraries

### Build Steps
```bash
mkdir build
cd build
cmake ..

# Build specific target (recommended)
cmake --build . --target RyntraCompiler

# Or build all targets
cmake --build .
```

### Environment Variables
Set one of these for LLVM detection:
- `LLVM_BUILD`: Path to LLVM build directory
- `LLVM_HOME`: Path to LLVM installation

## Usage

### Compile and Run

#### Using the CLI Compiler (RyntraCompiler)
```bash
# Basic compilation
./RyntraCompiler -i compiler/test/HelloWorld.rynt -o HelloWorld

# With debug information
./RyntraCompiler -i source.rynt -o program --show-token --show-ast --show-llvm-ir

# Show help
./RyntraCompiler --help
```

#### Development Tests
```bash
./RyntraCompilerTest      # Basic compiler test
./RyntraLLVMTest         # LLVM IR generation test  
./RyntraFullCompileTest  # Complete compilation pipeline test
```

### Example Output
The compiler will process `compiler/test/HelloWorld.rynt` and show:
1. **Lexical Analysis**: All tokens found
2. **Syntax Analysis**: AST structure  
3. **Code Generation**: Generated LLVM IR
4. **Complete Compilation**: .ll → .bc → .obj → .exe files

## Project Structure
```
compiler/
├── lexer/          # Tokenization
├── parser/         # AST generation  
├── codegen/        # LLVM IR generation
├── test/           # Test files
└── README.md       # Detailed documentation
```

## Example

**Input (HelloWorld.rynt):**
```rynt
declare(package) org.remimwen.test;
import ryntra.io.*;
public class main {
    public static int main(string[] args) {
        io.out << "Hello World! \n";
        return 0;
    }
}
```

**Output (Generated Files):**
- `HelloWorld.ll` - LLVM IR text format
- `HelloWorld.bc` - LLVM bytecode  
- `HelloWorld.obj` - Windows x64 object file
- `HelloWorld.exe` - Executable file

**LLVM IR Sample:**
```llvm
define i32 @main(i32 %0, i8** %1) {
entry:
  %str = getelementptr inbounds [15 x i8], [15 x i8]* @.str, i32 0, i32 0
  %printf_call = call i32 (i8*, ...) @printf(i8* %str)
  ret i32 0
}
```

**CLI Usage:**
```bash
# Compile HelloWorld program
./RyntraCompiler -i compiler/test/HelloWorld.rynt -o HelloWorld

# Run the generated executable
./HelloWorld.exe
# Output: Hello World!
```

### Build Options
```bash
# Build only the compiler
cmake -DBUILD_COMPILER=ON -DBUILD_RPLINK=OFF -DBUILD_TESTS=OFF ..

# Build only the linker
cmake -DBUILD_RPLINK=ON -DBUILD_COMPILER=OFF -DBUILD_TESTS=OFF ..

# Build compiler and linker
cmake -DBUILD_COMPILER=ON -DBUILD_RPLINK=ON -DBUILD_TESTS=OFF ..

# Build everything (default)
cmake ..
```