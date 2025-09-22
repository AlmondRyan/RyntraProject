#ifndef ARG_PARSER_H
#define ARG_PARSER_H

#include <string>
#include <vector>

namespace Ryntra {
namespace Utils {

struct CompilerOptions {
    std::string inputFile;
    std::string outputFile;
    bool showTokens = false;
    bool showAST = false;
    bool showLLVMIR = false;
    bool helpRequested = false;
    bool valid = false;
};

class ArgumentParser {
public:
    static CompilerOptions parseArguments(int argc, char* argv[]);
    static void printUsage(const std::string& programName);
    static void printHelp(const std::string& programName);

private:
    static bool isOption(const std::string& arg);
    static std::string getFileNameWithoutExtension(const std::string& filePath);
};

} // namespace Utils
} // namespace Ryntra

#endif // ARG_PARSER_H