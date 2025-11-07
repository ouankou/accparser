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

/**
 * @brief C API for external parser implementations
 *
 * These functions allow non-C++ parsers to integrate with accparser.
 * External implementations should provide parseOpenACC() and setLang().
 */
extern "C" {
    /**
     * @brief Set language mode for parsing
     * @param lang Language enum (ACC_Lang_C, ACC_Lang_Fortran, etc.)
     */
    void setLang(OpenACCBaseLang lang);

    /**
     * @brief Parse OpenACC directive with custom expression parser
     * @param input Directive string
     * @param exprParse Optional custom expression parser callback
     * @return Parsed directive AST, or nullptr on parse error
     */
    OpenACCDirective* parseOpenACC(const char* input, void* exprParse(const char* expr));
}
