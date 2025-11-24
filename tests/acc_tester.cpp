#include "OpenACCParser.h"
#include <iostream>
#include <regex>

extern std::vector<std::pair<std::string, int>>
preProcess(std::ifstream &input_file);

int openFile(std::ifstream &file, const char *filename) {
  file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
  try {
    file.open(filename);
  } catch (std::ifstream::failure const &) {
    std::cerr << "Exception caused by opening the given file\n";
    return -1;
  }

  return 0;
}

void savePragmaList(std::vector<OpenACCDirective *> *acc_ast_list,
                    std::string filename) {

  std::string output_filename = filename + ".output";
  std::ofstream output_file(output_filename.c_str(), std::ofstream::trunc);

  if (acc_ast_list != NULL) {
    for (unsigned int i = 0; i < acc_ast_list->size(); i++) {
      if (acc_ast_list->at(i) != NULL) {
        output_file << acc_ast_list->at(i)->generatePragmaString() << std::endl;
      } else {
        output_file << "NULL" << std::endl;
      };
    };
  };

  output_file.close();
}

int main(int argc, char **argv) {

  const char *filename = NULL;
  int result = 0;

  // Built-in tests expect normalized/merged clause output; enable merging here
  // while keeping the round-trip tool (acc_roundtrip) in non-merging mode.
  OpenACCDirective::setClauseMerging(true);

  if (argc > 1) {
    filename = argv[1];
  };
  std::ifstream input_file;

  if (filename != NULL) {
    result = openFile(input_file, filename);
  };
  if (result) {
    std::cout << "No testing file is available.\n";
    return -1;
  };

  std::vector<OpenACCDirective *> *acc_ast_list =
      new std::vector<OpenACCDirective *>();
  OpenACCDirective *acc_ast = NULL;

  std::string filename_string = std::string(filename);
  filename_string = filename_string.substr(filename_string.rfind("/") + 1);

  if (result) {
    std::cout << "No output file is available.\n";
    return -1;
  };

  auto acc_pragmas = preProcess(input_file);

  for (const auto &pragma : acc_pragmas) {
    acc_ast = parseOpenACC(pragma.first);
    acc_ast_list->push_back(acc_ast);
  }

  savePragmaList(acc_ast_list, filename_string);

  for (OpenACCDirective *directive : *acc_ast_list) {
    delete directive;
  }
  delete acc_ast_list;

  input_file.close();

  return 0;
}
