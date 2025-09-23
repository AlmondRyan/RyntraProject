#ifndef ERROR_HANDLER_H
#define ERROR_HANDLER_H

#include "ConsoleTextColorManager.h"
#include <iostream>
#include <string>
#include <vector>

namespace Ryntra {
namespace Utils {
    struct ErrorObject {
        enum Level {
            L_ERROR,
            L_WARNING,
            L_NOTE
        } level;

        std::string file;
        std::string description;
        int         row;
        int         col;

        ErrorObject(const Level _level, std::string _file, std::string _desc, int _row, int _col)
            : level(_level), file(_file), description(_desc), row(_row), col(_col) {}
    };

    class ErrorHandler {
    private:
        std::vector<ErrorObject> errorLists;

    public:
        void makeError(std::string file, std::string desc, int row, int col) {
            errorLists.emplace_back(ErrorObject::Level::L_ERROR, file, desc, row, col);
        }

        void makeWarning(std::string file, std::string desc, int row, int col) {
            errorLists.emplace_back(ErrorObject::Level::L_WARNING, file, desc, row, col);
        }

        void makeNote(std::string file, std::string desc, int row, int col) {
            errorLists.emplace_back(ErrorObject::Level::L_NOTE, file, desc, row, col);
        }

        void printErrorList() {
            for (auto i : errorLists) {
                if (i.level == ErrorObject::L_ERROR) {
                    std::cout << CTCM_RED << "[Error]"
                              << CTCM_DEFAULT << " | " << i.row << ":" << i.col << " in " << i.file << ": " << i.description << std::endl;
                } else if (i.level == ErrorObject::L_WARNING) {
                    std::cout << CTCM_YELLOW << "[Warning]"
                              << CTCM_DEFAULT << " | " << i.row << ":" << i.col << " in " << i.file << ": " << i.description << std::endl;
                } else if (i.level == ErrorObject::L_NOTE) {
                    std::cout << CTCM_CYAN << "[Note]"
                              << CTCM_DEFAULT << " | " << i.row << ":" << i.col << " in " << i.file << ": " << i.description << std::endl;
                }
            }
        }

        int getErrorListSize() {
            return errorLists.size();
        }
    };

    // Global error handler instance
    extern ErrorHandler globalErrorHandler;

} // namespace Utils
} // namespace Ryntra

#endif