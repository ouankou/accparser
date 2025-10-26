#include "OpenACCASTConstructor.h"
#include "acclexer.h"
#include "accparser.h"
#include <antlr4-runtime.h>
#include <iostream>
#include <string>

extern OpenACCDirective *current_directive;

int main(int argc, char *argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " \"<OpenACC directive>\"" << std::endl;
        return 1;
    }

    std::string input = argv[1];

    // Add newline if not present
    if (input.back() != '\n') {
        input += '\n';
    }

    antlr4::ANTLRInputStream antlr_input(input);
    acclexer lexer(&antlr_input);
    antlr4::CommonTokenStream tokens(&lexer);
    tokens.fill();
    accparser parser(&tokens);
    parser.setBuildParseTree(true);

    current_directive = NULL;
    antlr4::tree::ParseTree *tree = parser.acc();

    antlr4::tree::ParseTreeWalker *walker = new antlr4::tree::ParseTreeWalker();
    walker->walk(new OpenACCIRConstructor(), tree);

    if (current_directive != nullptr) {
        std::string output = current_directive->generatePragmaString();
        std::cout << output << std::endl;
        delete walker;
        return 0;
    } else {
        std::cerr << "Parse error" << std::endl;
        delete walker;
        return 1;
    }
}
