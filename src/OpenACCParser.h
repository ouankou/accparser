#pragma once

#include "OpenACCIR.h"
#include <string>

/**
 * @file OpenACCParser.h
 * @brief Standalone parser interface for OpenACC directives
 *
 * This header provides the parser API independent of ANTLR implementation details.
 * External projects can implement parseOpenACC() using alternative parsers while
 * maintaining compatibility with OpenACCIR AST classes.
 */

/**
 * @brief Parse OpenACC directive from string
 * @param input Directive string (with or without pragma prefix)
 * @return Parsed directive AST, or nullptr on parse error
 */
OpenACCDirective* parseOpenACC(std::string input);

/**
 * @brief Trim leading and trailing whitespace
 * @param input String to trim
 * @return Trimmed string
 */
std::string trimEnclosingWhiteSpace(std::string);
