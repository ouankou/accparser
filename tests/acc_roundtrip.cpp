#include "OpenACCASTConstructor.h"
#include <iostream>
#include <string>

int main(int argc, char *argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " \"<OpenACC directive>\"" << std::endl;
        return 1;
    }

    std::string input = argv[1];

    if (input.back() != '\n') {
        input += '\n';
    }

    OpenACCDirective::setClauseMerging(false);

    OpenACCDirective *directive = parseOpenACC(input);

    if (directive != nullptr) {
        std::string output = directive->generatePragmaString();
        std::cout << output << std::endl;
        delete directive;
        return 0;
    } else {
        std::cerr << "Parse error" << std::endl;
        return 1;
    }
}
