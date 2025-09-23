#include "../utils/arg_parser.h"
#include "../utils/compiler_utils.h"
#include <iostream>

using namespace Ryntra::Utils;

int main(int argc, char *argv[]) {
    const auto [inputFile,
                outputFile,
                showTokens,
                showAST,
                showLLVMIR,
                helpRequested,
                valid] = ArgumentParser::parseArguments(argc, argv);

    if (helpRequested) {
        ArgumentParser::printHelp(argv[0]);
        return 0;
    }

    if (!valid) {
        return 1;
    }

    const bool success = compile(
        inputFile,
        outputFile,
        showTokens,
        showAST,
        showLLVMIR);

    return success ? 0 : 1;
}