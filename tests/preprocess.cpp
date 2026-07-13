//===----------------------------------------------------------------------===//
//
// Part of accparser, under the BSD 3-Clause License.
// See LICENSE for license information.
// SPDX-License-Identifier: BSD-3-Clause
//
//===----------------------------------------------------------------------===//

#include "preprocess.h"

#include <algorithm>
#include <cctype>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace {

std::size_t firstNonblank(std::string_view Line) {
  std::size_t Offset = 0;
  while (Offset < Line.size() && (Line[Offset] == ' ' || Line[Offset] == '\t'))
    ++Offset;
  return Offset;
}

bool equalsIgnoringCase(std::string_view LHS, std::string_view RHS) {
  if (LHS.size() != RHS.size())
    return false;
  for (std::size_t I = 0; I < LHS.size(); ++I)
    if (std::tolower(static_cast<unsigned char>(LHS[I])) !=
        std::tolower(static_cast<unsigned char>(RHS[I])))
      return false;
  return true;
}

bool isCPragma(std::string_view Line) {
  std::size_t Offset = firstNonblank(Line);
  if (Offset == Line.size() || Line[Offset] != '#')
    return false;
  ++Offset;
  Offset = firstNonblank(Line.substr(Offset)) + Offset;
  if (Line.substr(Offset, 6) != "pragma")
    return false;
  Offset += 6;
  if (Offset == Line.size() ||
      !std::isspace(static_cast<unsigned char>(Line[Offset])))
    return false;
  Offset += firstNonblank(Line.substr(Offset));
  if (Line.substr(Offset, 3) != "acc")
    return false;
  Offset += 3;
  return Offset == Line.size() ||
         std::isspace(static_cast<unsigned char>(Line[Offset]));
}

bool hasFortranSentinel(std::string_view Line, std::size_t Offset, bool Fixed) {
  if (Offset + 5 > Line.size())
    return false;
  char Sentinel =
      static_cast<char>(std::tolower(static_cast<unsigned char>(Line[Offset])));
  if ((!Fixed && Sentinel != '!') ||
      (Fixed && Sentinel != '!' && Sentinel != 'c' && Sentinel != '*'))
    return false;
  return Line[Offset + 1] == '$' &&
         equalsIgnoringCase(Line.substr(Offset + 2, 3), "acc");
}

bool isFortranFreeDirective(std::string_view Line) {
  std::size_t Offset = firstNonblank(Line);
  if (!hasFortranSentinel(Line, Offset, false))
    return false;
  Offset += 5;
  return Offset == Line.size() ||
         std::isspace(static_cast<unsigned char>(Line[Offset]));
}

bool isFortranFixedInitial(std::string_view Line) {
  if (!hasFortranSentinel(Line, 0, true))
    return false;
  return Line.size() == 5 || Line[5] == ' ' || Line[5] == '0';
}

bool isFortranFixedContinuation(std::string_view Line) {
  return hasFortranSentinel(Line, 0, true) && Line.size() > 5 &&
         Line[5] != ' ' && Line[5] != '0';
}

bool isIgnoredFortranSentinelComment(std::string_view Line) {
  std::size_t Offset = firstNonblank(Line);
  if (!hasFortranSentinel(Line, Offset, false))
    return false;
  Offset += 5;
  Offset += firstNonblank(Line.substr(Offset));
  return Offset < Line.size() && Line[Offset] == '!';
}

bool cContinues(std::string_view Line) {
  if (!Line.empty() && Line.back() == '\\')
    return true;
  return Line.size() >= 2 && Line[Line.size() - 2] == '\\' &&
         Line.back() == '\r';
}

bool fortranContinues(std::string_view Line) {
  std::size_t Begin = firstNonblank(Line);
  Begin = std::min(Begin + 5, Line.size());
  char Quote = 0;
  std::size_t End = Line.size();
  for (std::size_t I = Begin; I < Line.size(); ++I) {
    char Ch = Line[I];
    if (Quote != 0) {
      if (Ch == Quote) {
        if (I + 1 < Line.size() && Line[I + 1] == Quote) {
          ++I;
          continue;
        }
        Quote = 0;
      }
    } else if (Ch == '\'' || Ch == '"') {
      Quote = Ch;
    } else if (Ch == '!') {
      End = I;
      break;
    }
  }
  while (End > Begin && std::isspace(static_cast<unsigned char>(Line[End - 1])))
    --End;
  return End > Begin && Line[End - 1] == '&';
}

} // namespace

std::vector<PreprocessedDirective> preProcess(std::istream &InputFile) {
  std::vector<std::string> Lines;
  std::string Line;
  while (std::getline(InputFile, Line))
    Lines.push_back(std::move(Line));

  std::vector<PreprocessedDirective> Pragmas;
  for (std::size_t I = 0; I < Lines.size(); ++I) {
    Line = Lines[I];
    bool IsC = isCPragma(Line);
    bool IsFortranFree = isFortranFreeDirective(Line);
    bool IsFortranFixed = isFortranFixedInitial(Line);
    if (!IsC && !IsFortranFree && !IsFortranFixed)
      continue;
    if (IsFortranFree && isIgnoredFortranSentinelComment(Line))
      continue;

    int StartLine = static_cast<int>(I + 1);
    std::string Directive = Line;
    if (IsC) {
      while (cContinues(Lines[I]) && I + 1 < Lines.size()) {
        Directive += '\n';
        Directive += Lines[++I];
      }
      Pragmas.push_back(
          {std::move(Directive), StartLine, PreprocessedForm::CPragma});
      continue;
    }

    bool UseFixedForm =
        IsFortranFixed &&
        (!IsFortranFree || (!fortranContinues(Line) && I + 1 < Lines.size() &&
                            isFortranFixedContinuation(Lines[I + 1])));
    if (UseFixedForm) {
      while (I + 1 < Lines.size() && isFortranFixedContinuation(Lines[I + 1])) {
        Directive += '\n';
        Directive += Lines[++I];
      }
      Pragmas.push_back(
          {std::move(Directive), StartLine, PreprocessedForm::FortranFixed});
      continue;
    }

    bool NeedsContinuation = fortranContinues(Line);
    while (NeedsContinuation && I + 1 < Lines.size()) {
      Line = Lines[++I];
      Directive += '\n';
      Directive += Line;
      if (!isIgnoredFortranSentinelComment(Line))
        NeedsContinuation = fortranContinues(Line);
    }
    Pragmas.push_back(
        {std::move(Directive), StartLine, PreprocessedForm::FortranFree});
  }
  return Pragmas;
}
