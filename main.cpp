#include <iostream>
#include "utils/arg_parser.h"
#include "utils/compiler_utils.h"

using namespace Ryntra::Utils;

int main(int argc, char* argv[]) {
    // Parse command line arguments
    CompilerOptions options = ArgumentParser::parseArguments(argc, argv);
    
    // Handle help request
    if (options.helpRequested) {
        ArgumentParser::printHelp(argv[0]);
        return 0;
    }
    
    // Check if arguments are valid
    if (!options.valid) {
        return 1;
    }
    
    // Compile the file
    bool success = compile(
        options.inputFile,
        options.outputFile,
        options.showTokens,
        options.showAST,
        options.showLLVMIR
    );
    
    return success ? 0 : 1;
}