//===----------------------------------------------------------------------===//
//
// Part of accparser, under the BSD 3-Clause License.
// See LICENSE for license information.
// SPDX-License-Identifier: BSD-3-Clause
//
//===----------------------------------------------------------------------===//

#include "OpenACCASTConstructor.h"

#include "acclexer.h"
#include "accparser.h"

#include <antlr4-runtime.h>

#include <algorithm>
#include <cctype>
#include <memory>
#include <set>
#include <sstream>
#include <string>
#include <string_view>
#include <utility>

namespace openacc {
namespace {

bool isHorizontalSpace(char C) { return C == ' ' || C == '\t' || C == '\f'; }

bool isIdentifierStart(char C) {
  unsigned char UC = static_cast<unsigned char>(C);
  return std::isalpha(UC) || C == '_';
}

bool isIdentifierContinue(char C) {
  unsigned char UC = static_cast<unsigned char>(C);
  return std::isalnum(UC) || C == '_';
}

std::string lower(std::string_view Text) {
  std::string Result;
  Result.reserve(Text.size());
  for (char C : Text)
    Result.push_back(
        static_cast<char>(std::tolower(static_cast<unsigned char>(C))));
  return Result;
}

bool equalsKeyword(std::string_view Text, std::string_view Keyword,
                   Language Lang) {
  if (Lang == Language::Fortran)
    return lower(Text) == Keyword;
  return Text == Keyword;
}

class SourceManager {
public:
  explicit SourceManager(std::string_view Input) : Input(Input) {}

  SourcePosition position(std::size_t Offset) const {
    Offset = std::min(Offset, Input.size());
    SourcePosition Result;
    Result.byteOffset = Offset;
    for (std::size_t I = 0; I < Offset; ++I) {
      if (Input[I] == '\n') {
        ++Result.line;
        Result.column = 1;
      } else {
        ++Result.column;
      }
    }
    return Result;
  }

  SourceRange range(std::size_t Begin, std::size_t End) const {
    return {position(Begin), position(End)};
  }

private:
  std::string_view Input;
};

struct BodyText {
  std::string text;
  std::vector<std::size_t> sourceOffsets;
  std::size_t endOffset = 0;
};

SourceRange bodyRange(const BodyText &Body, const SourceManager &SM,
                      std::size_t Begin, std::size_t End) {
  Begin = std::min(Begin, Body.text.size());
  End = std::min(std::max(Begin, End), Body.text.size());
  std::size_t SourceBegin = Begin < Body.sourceOffsets.size()
                                ? Body.sourceOffsets[Begin]
                                : Body.endOffset;
  std::size_t SourceEnd = Body.endOffset;
  if (End < Body.sourceOffsets.size())
    SourceEnd = Body.sourceOffsets[End];
  else if (End > 0 && End - 1 < Body.sourceOffsets.size())
    SourceEnd = Body.sourceOffsets[End - 1] + 1;
  return SM.range(SourceBegin, SourceEnd);
}

void addDiagnostic(std::vector<Diagnostic> &Diagnostics, DiagnosticCode Code,
                   std::string Message, SourceRange Range) {
  Diagnostics.push_back({Code, DiagnosticSeverity::Error, std::move(Message),
                         Range, std::nullopt});
}

void appendMapped(BodyText &Body, char C, std::size_t SourceOffset) {
  Body.text.push_back(C);
  Body.sourceOffsets.push_back(SourceOffset);
  Body.endOffset = SourceOffset + 1;
}

void appendMappedRange(BodyText &Body, std::string_view Input,
                       std::size_t Begin, std::size_t End) {
  for (std::size_t I = Begin; I < End; ++I)
    appendMapped(Body, Input[I], I);
}

std::pair<std::size_t, std::size_t>
trimBounds(std::string_view Text, std::size_t Begin, std::size_t End) {
  while (Begin < End && std::isspace(static_cast<unsigned char>(Text[Begin])))
    ++Begin;
  while (End > Begin && std::isspace(static_cast<unsigned char>(Text[End - 1])))
    --End;
  return {Begin, End};
}

std::size_t skipHorizontalSpace(std::string_view Input, std::size_t Offset) {
  while (Offset < Input.size() && isHorizontalSpace(Input[Offset]))
    ++Offset;
  return Offset;
}

bool consumeWord(std::string_view Input, std::size_t &Offset,
                 std::string_view Word, bool CaseInsensitive = false) {
  if (Offset + Word.size() > Input.size())
    return false;
  std::string_view Candidate = Input.substr(Offset, Word.size());
  bool Matches =
      CaseInsensitive ? lower(Candidate) == lower(Word) : Candidate == Word;
  if (!Matches)
    return false;
  if (Offset + Word.size() < Input.size() &&
      isIdentifierContinue(Input[Offset + Word.size()]))
    return false;
  Offset += Word.size();
  return true;
}

std::optional<BodyText>
scanDirectiveBody(std::string_view Input, const SourceManager &SM,
                  std::vector<Diagnostic> &Diagnostics) {
  auto [Begin, End] = trimBounds(Input, 0, Input.size());
  BodyText Body;
  appendMappedRange(Body, Input, Begin, End);
  Body.endOffset = End;
  if (Body.text.empty()) {
    addDiagnostic(Diagnostics, DiagnosticCode::InvalidEnvelope,
                  "the directive body is empty", SM.range(Begin, End));
    return std::nullopt;
  }
  return Body;
}

std::optional<BodyText> scanCPragma(std::string_view Input,
                                    const SourceManager &SM,
                                    std::vector<Diagnostic> &Diagnostics) {
  std::size_t Offset = skipHorizontalSpace(Input, 0);
  std::size_t PrefixBegin = Offset;
  if (Offset >= Input.size() || Input[Offset] != '#') {
    addDiagnostic(Diagnostics, DiagnosticCode::InvalidEnvelope,
                  "expected '#' to begin an OpenACC pragma",
                  SM.range(Offset, std::min(Offset + 1, Input.size())));
    return std::nullopt;
  }
  ++Offset;
  Offset = skipHorizontalSpace(Input, Offset);
  if (!consumeWord(Input, Offset, "pragma")) {
    addDiagnostic(Diagnostics, DiagnosticCode::InvalidEnvelope,
                  "expected 'pragma' after '#'", SM.range(PrefixBegin, Offset));
    return std::nullopt;
  }
  if (Offset >= Input.size() || !isHorizontalSpace(Input[Offset])) {
    addDiagnostic(Diagnostics, DiagnosticCode::InvalidEnvelope,
                  "'pragma' and 'acc' must be separated by whitespace",
                  SM.range(PrefixBegin, Offset));
    return std::nullopt;
  }
  Offset = skipHorizontalSpace(Input, Offset);
  if (!consumeWord(Input, Offset, "acc")) {
    addDiagnostic(Diagnostics, DiagnosticCode::InvalidEnvelope,
                  "expected the OpenACC 'acc' pragma namespace",
                  SM.range(PrefixBegin, Offset));
    return std::nullopt;
  }
  if (Offset < Input.size() &&
      !std::isspace(static_cast<unsigned char>(Input[Offset]))) {
    addDiagnostic(
        Diagnostics, DiagnosticCode::InvalidEnvelope,
        "'acc' and the directive name must be separated by whitespace",
        SM.range(Offset, Offset + 1));
    return std::nullopt;
  }

  Offset = skipHorizontalSpace(Input, Offset);
  BodyText Body;
  bool SawTerminalNewline = false;
  for (std::size_t I = Offset; I < Input.size();) {
    if (Input[I] == '\\' && I + 1 < Input.size() && Input[I + 1] == '\n') {
      I += 2;
      continue;
    }
    if (Input[I] == '\\' && I + 2 < Input.size() && Input[I + 1] == '\r' &&
        Input[I + 2] == '\n') {
      I += 3;
      continue;
    }
    if (Input[I] == '\r' || Input[I] == '\n') {
      SawTerminalNewline = true;
      std::size_t Rest = I;
      while (Rest < Input.size() &&
             std::isspace(static_cast<unsigned char>(Input[Rest])))
        ++Rest;
      if (Rest != Input.size()) {
        addDiagnostic(Diagnostics, DiagnosticCode::InvalidLanguageForm,
                      "an uncontinued C/C++ pragma cannot span physical lines",
                      SM.range(I, Rest));
        return std::nullopt;
      }
      Body.endOffset = I;
      break;
    }
    appendMapped(Body, Input[I], I);
    ++I;
  }
  if (!SawTerminalNewline)
    Body.endOffset = Input.size();
  auto [Begin, End] = trimBounds(Body.text, 0, Body.text.size());
  if (Begin != 0 || End != Body.text.size()) {
    Body.text = Body.text.substr(Begin, End - Begin);
    Body.sourceOffsets = std::vector<std::size_t>(
        Body.sourceOffsets.begin() + static_cast<std::ptrdiff_t>(Begin),
        Body.sourceOffsets.begin() + static_cast<std::ptrdiff_t>(End));
  }
  if (Body.text.empty()) {
    addDiagnostic(Diagnostics, DiagnosticCode::InvalidEnvelope,
                  "the OpenACC pragma has no directive",
                  SM.range(Offset, Offset));
    return std::nullopt;
  }
  return Body;
}

std::optional<std::size_t> getCxxRawStringPrefixLength(std::string_view Text,
                                                       std::size_t Offset);

std::optional<std::size_t> findCxxRawStringEnd(std::string_view Text,
                                               std::size_t Begin,
                                               std::size_t PrefixLength);

bool decodePragmaStringLiteral(std::string_view Input, std::size_t &Offset,
                               Language Lang, std::string &Decoded,
                               std::vector<std::size_t> &SourceOffsets,
                               std::vector<Diagnostic> &Diagnostics,
                               const SourceManager &SM) {
  std::size_t LiteralBegin = Offset;
  if (Lang == Language::Cxx) {
    if (auto PrefixLength = getCxxRawStringPrefixLength(Input, Offset)) {
      auto End = findCxxRawStringEnd(Input, Offset, *PrefixLength);
      if (!End) {
        addDiagnostic(Diagnostics, DiagnosticCode::UnterminatedString,
                      "unterminated raw string literal in _Pragma",
                      SM.range(LiteralBegin, Input.size()));
        return false;
      }

      std::size_t DelimiterBegin = LiteralBegin + *PrefixLength;
      std::size_t Open = Input.find('(', DelimiterBegin);
      std::size_t DelimiterLength = Open - DelimiterBegin;
      std::size_t ContentEnd = *End - DelimiterLength - 2;
      for (std::size_t I = Open + 1; I < ContentEnd; ++I) {
        Decoded.push_back(Input[I]);
        SourceOffsets.push_back(I);
      }
      Offset = *End;
      return true;
    }
  }

  std::size_t PrefixLength = 0;
  for (std::string_view Prefix : {"u8\"", "L\"", "u\"", "U\""}) {
    if (Input.substr(Offset, Prefix.size()) == Prefix) {
      PrefixLength = Prefix.size() - 1;
      break;
    }
  }
  std::size_t QuoteBegin = Offset + PrefixLength;
  if (QuoteBegin >= Input.size() || Input[QuoteBegin] != '"') {
    addDiagnostic(
        Diagnostics, DiagnosticCode::InvalidEnvelope,
        "expected a string literal in _Pragma",
        SM.range(LiteralBegin, std::min(LiteralBegin + 1, Input.size())));
    return false;
  }
  Offset = QuoteBegin + 1;
  while (Offset < Input.size()) {
    char C = Input[Offset];
    if (C == '"') {
      ++Offset;
      return true;
    }
    if (C == '\n' || C == '\r')
      break;
    if (C != '\\') {
      Decoded.push_back(C);
      SourceOffsets.push_back(Offset++);
      continue;
    }
    std::size_t EscapeOffset = Offset++;
    if (Offset >= Input.size())
      break;
    char Escaped = Input[Offset++];
    switch (Escaped) {
    case '\\':
    case '"':
      Decoded.push_back(Escaped);
      SourceOffsets.push_back(EscapeOffset);
      break;
    default:
      Decoded.push_back('\\');
      SourceOffsets.push_back(EscapeOffset);
      Decoded.push_back(Escaped);
      SourceOffsets.push_back(EscapeOffset + 1);
      break;
    }
  }
  addDiagnostic(Diagnostics, DiagnosticCode::UnterminatedString,
                "unterminated string literal in _Pragma",
                SM.range(QuoteBegin, Offset));
  return false;
}

std::optional<BodyText>
scanCPragmaOperator(std::string_view Input, const SourceManager &SM,
                    std::vector<Diagnostic> &Diagnostics, Language Lang) {
  std::size_t Offset = skipHorizontalSpace(Input, 0);
  std::size_t Begin = Offset;
  if (!consumeWord(Input, Offset, "_Pragma")) {
    addDiagnostic(Diagnostics, DiagnosticCode::InvalidEnvelope,
                  "expected the C/C++ _Pragma operator",
                  SM.range(Begin, Offset));
    return std::nullopt;
  }
  Offset = skipHorizontalSpace(Input, Offset);
  if (Offset >= Input.size() || Input[Offset] != '(') {
    addDiagnostic(Diagnostics, DiagnosticCode::InvalidEnvelope,
                  "expected '(' after _Pragma", SM.range(Begin, Offset));
    return std::nullopt;
  }
  ++Offset;
  Offset = skipHorizontalSpace(Input, Offset);
  std::string Decoded;
  std::vector<std::size_t> Mapping;
  if (!decodePragmaStringLiteral(Input, Offset, Lang, Decoded, Mapping,
                                 Diagnostics, SM))
    return std::nullopt;
  Offset = skipHorizontalSpace(Input, Offset);
  if (Offset >= Input.size() || Input[Offset] != ')') {
    addDiagnostic(Diagnostics, DiagnosticCode::InvalidEnvelope,
                  "expected ')' after the _Pragma string",
                  SM.range(Begin, Offset));
    return std::nullopt;
  }
  ++Offset;
  Offset = skipHorizontalSpace(Input, Offset);
  while (Offset < Input.size() &&
         (Input[Offset] == '\r' || Input[Offset] == '\n'))
    ++Offset;
  Offset = skipHorizontalSpace(Input, Offset);
  if (Offset != Input.size()) {
    addDiagnostic(Diagnostics, DiagnosticCode::InvalidEnvelope,
                  "unexpected text after _Pragma",
                  SM.range(Offset, Input.size()));
    return std::nullopt;
  }

  std::size_t BodyOffset = 0;
  while (BodyOffset < Decoded.size() && isHorizontalSpace(Decoded[BodyOffset]))
    ++BodyOffset;
  if (Decoded.substr(BodyOffset, 3) != "acc" ||
      (BodyOffset + 3 < Decoded.size() &&
       isIdentifierContinue(Decoded[BodyOffset + 3]))) {
    addDiagnostic(Diagnostics, DiagnosticCode::InvalidEnvelope,
                  "the _Pragma string must begin with the 'acc' namespace",
                  SM.range(Begin, Offset));
    return std::nullopt;
  }
  BodyOffset += 3;
  if (BodyOffset >= Decoded.size() ||
      !std::isspace(static_cast<unsigned char>(Decoded[BodyOffset]))) {
    addDiagnostic(
        Diagnostics, DiagnosticCode::InvalidEnvelope,
        "'acc' and the directive name must be separated by whitespace",
        SM.range(Begin, Offset));
    return std::nullopt;
  }
  while (BodyOffset < Decoded.size() &&
         std::isspace(static_cast<unsigned char>(Decoded[BodyOffset])))
    ++BodyOffset;

  BodyText Body;
  for (std::size_t I = BodyOffset; I < Decoded.size(); ++I) {
    Body.text.push_back(Decoded[I]);
    Body.sourceOffsets.push_back(Mapping[I]);
  }
  Body.endOffset =
      Body.sourceOffsets.empty() ? Begin : Body.sourceOffsets.back() + 1;
  if (Body.text.empty()) {
    addDiagnostic(Diagnostics, DiagnosticCode::InvalidEnvelope,
                  "the _Pragma string has no OpenACC directive",
                  SM.range(Begin, Offset));
    return std::nullopt;
  }
  return Body;
}

std::size_t findFortranComment(std::string_view Line, std::size_t Begin) {
  char Quote = 0;
  for (std::size_t I = Begin; I < Line.size(); ++I) {
    char C = Line[I];
    if (Quote != 0) {
      if (C == Quote) {
        if (I + 1 < Line.size() && Line[I + 1] == Quote) {
          ++I;
          continue;
        }
        Quote = 0;
      }
      continue;
    }
    if (C == '\'' || C == '"') {
      Quote = C;
      continue;
    }
    if (C == '!')
      return I;
  }
  return Line.size();
}

bool matchFortranSentinel(std::string_view Input, std::size_t Offset,
                          bool Fixed, std::size_t &After) {
  if (Offset + 5 > Input.size())
    return false;
  char First = static_cast<char>(
      std::tolower(static_cast<unsigned char>(Input[Offset])));
  if ((!Fixed && First != '!') ||
      (Fixed && First != '!' && First != 'c' && First != '*'))
    return false;
  if (Input[Offset + 1] != '$' || lower(Input.substr(Offset + 2, 3)) != "acc")
    return false;
  After = Offset + 5;
  return true;
}

std::optional<BodyText> scanFortranFree(std::string_view Input,
                                        const SourceManager &SM,
                                        std::vector<Diagnostic> &Diagnostics) {
  BodyText Body;
  std::size_t LineBegin = 0;
  bool FirstLine = true;
  bool NeedContinuation = false;
  bool PreviousHadSeparator = false;
  while (LineBegin <= Input.size()) {
    std::size_t LineEnd = Input.find('\n', LineBegin);
    if (LineEnd == std::string_view::npos)
      LineEnd = Input.size();
    std::size_t ContentBegin = skipHorizontalSpace(Input, LineBegin);
    std::size_t AfterSentinel = 0;
    if (!matchFortranSentinel(Input, ContentBegin, false, AfterSentinel)) {
      addDiagnostic(
          Diagnostics, DiagnosticCode::InvalidEnvelope,
          "expected a free-form !$acc sentinel",
          SM.range(ContentBegin, std::min(ContentBegin + 5, LineEnd)));
      return std::nullopt;
    }
    if (AfterSentinel >= LineEnd ||
        (FirstLine && !isHorizontalSpace(Input[AfterSentinel]))) {
      addDiagnostic(Diagnostics, DiagnosticCode::InvalidEnvelope,
                    "the !$acc sentinel must be followed by whitespace",
                    SM.range(ContentBegin, AfterSentinel));
      return std::nullopt;
    }
    std::size_t UntrimmedContentBegin = AfterSentinel;
    ContentBegin = skipHorizontalSpace(Input, AfterSentinel);
    if (ContentBegin < LineEnd && Input[ContentBegin] == '!') {
      // An ignored sentinel comment does not alter continuation state.
      if (LineEnd == Input.size())
        break;
      LineBegin = LineEnd + 1;
      continue;
    }
    bool LeadingSeparator = ContentBegin != UntrimmedContentBegin;
    if (!FirstLine && ContentBegin < LineEnd && Input[ContentBegin] == '&') {
      ++ContentBegin;
      std::size_t AfterContinuationMarker = ContentBegin;
      ContentBegin = skipHorizontalSpace(Input, ContentBegin);
      LeadingSeparator = ContentBegin != AfterContinuationMarker;
    }

    std::size_t Comment = findFortranComment(
        Input.substr(LineBegin, LineEnd - LineBegin), ContentBegin - LineBegin);
    Comment += LineBegin;
    std::size_t ContentEnd = Comment;
    while (ContentEnd > ContentBegin &&
           isHorizontalSpace(Input[ContentEnd - 1]))
      --ContentEnd;
    bool Continues = ContentEnd > ContentBegin && Input[ContentEnd - 1] == '&';
    bool TrailingSeparator = Continues && ContentEnd > ContentBegin + 1 &&
                             isHorizontalSpace(Input[ContentEnd - 2]);
    if (Continues)
      --ContentEnd;
    while (ContentEnd > ContentBegin &&
           isHorizontalSpace(Input[ContentEnd - 1]))
      --ContentEnd;
    if (!Body.text.empty() && ContentBegin < ContentEnd &&
        (PreviousHadSeparator || LeadingSeparator))
      appendMapped(Body, ' ', ContentBegin);
    appendMappedRange(Body, Input, ContentBegin, ContentEnd);
    Body.endOffset = ContentEnd;
    NeedContinuation = Continues;
    PreviousHadSeparator = TrailingSeparator;
    FirstLine = false;

    if (LineEnd == Input.size())
      break;
    LineBegin = LineEnd + 1;
    if (!NeedContinuation) {
      std::size_t Rest = LineBegin;
      while (Rest < Input.size() &&
             std::isspace(static_cast<unsigned char>(Input[Rest])))
        ++Rest;
      if (Rest != Input.size()) {
        addDiagnostic(
            Diagnostics, DiagnosticCode::InvalidLanguageForm,
            "a free-form directive continuation requires a trailing '&'",
            SM.range(LineEnd, Rest));
        return std::nullopt;
      }
      break;
    }
  }
  if (NeedContinuation) {
    addDiagnostic(Diagnostics, DiagnosticCode::InvalidLanguageForm,
                  "the final free-form directive line cannot end in '&'",
                  SM.range(Body.endOffset, Body.endOffset));
    return std::nullopt;
  }
  if (Body.text.empty()) {
    addDiagnostic(Diagnostics, DiagnosticCode::InvalidEnvelope,
                  "the Fortran sentinel has no directive",
                  SM.range(0, Input.size()));
    return std::nullopt;
  }
  return Body;
}

std::optional<BodyText> scanFortranFixed(std::string_view Input,
                                         const SourceManager &SM,
                                         std::vector<Diagnostic> &Diagnostics) {
  BodyText Body;
  std::size_t LineBegin = 0;
  bool FirstLine = true;
  while (LineBegin <= Input.size()) {
    std::size_t LineEnd = Input.find('\n', LineBegin);
    if (LineEnd == std::string_view::npos)
      LineEnd = Input.size();
    std::size_t AfterSentinel = 0;
    if (!matchFortranSentinel(Input, LineBegin, true, AfterSentinel) ||
        AfterSentinel != LineBegin + 5) {
      addDiagnostic(Diagnostics, DiagnosticCode::InvalidEnvelope,
                    "a fixed-form OpenACC sentinel must occupy columns 1-5",
                    SM.range(LineBegin, std::min(LineBegin + 5, LineEnd)));
      return std::nullopt;
    }
    if (AfterSentinel >= LineEnd) {
      addDiagnostic(Diagnostics, DiagnosticCode::InvalidEnvelope,
                    "a fixed-form directive requires column 6",
                    SM.range(AfterSentinel, AfterSentinel));
      return std::nullopt;
    }
    char Continuation = Input[AfterSentinel];
    bool IsContinuation = Continuation != ' ' && Continuation != '0';
    if (FirstLine == IsContinuation) {
      addDiagnostic(
          Diagnostics, DiagnosticCode::InvalidLanguageForm,
          FirstLine ? "the first fixed-form line cannot be a continuation"
                    : "a continued fixed-form line needs a nonblank column 6",
          SM.range(AfterSentinel, AfterSentinel + 1));
      return std::nullopt;
    }
    std::size_t ContentBegin = AfterSentinel + 1;
    std::size_t Comment = findFortranComment(
        Input.substr(LineBegin, LineEnd - LineBegin), ContentBegin - LineBegin);
    Comment += LineBegin;
    std::size_t ContentEnd = Comment;
    while (ContentEnd > ContentBegin &&
           isHorizontalSpace(Input[ContentEnd - 1]))
      --ContentEnd;
    if (!Body.text.empty() && ContentBegin < ContentEnd)
      appendMapped(Body, ' ', ContentBegin);
    appendMappedRange(Body, Input, ContentBegin, ContentEnd);
    Body.endOffset = ContentEnd;
    FirstLine = false;
    if (LineEnd == Input.size())
      break;
    LineBegin = LineEnd + 1;
    std::size_t Rest = LineBegin;
    while (Rest < Input.size() && (Input[Rest] == '\r' || Input[Rest] == '\n'))
      ++Rest;
    if (Rest == Input.size())
      break;
  }
  auto [Begin, End] = trimBounds(Body.text, 0, Body.text.size());
  if (Begin != 0 || End != Body.text.size()) {
    Body.text = Body.text.substr(Begin, End - Begin);
    Body.sourceOffsets = std::vector<std::size_t>(
        Body.sourceOffsets.begin() + static_cast<std::ptrdiff_t>(Begin),
        Body.sourceOffsets.begin() + static_cast<std::ptrdiff_t>(End));
  }
  if (Body.text.empty()) {
    addDiagnostic(Diagnostics, DiagnosticCode::InvalidEnvelope,
                  "the fixed-form sentinel has no directive",
                  SM.range(0, Input.size()));
    return std::nullopt;
  }
  return Body;
}

std::optional<BodyText> scanEnvelope(std::string_view Input,
                                     ParseOptions Options,
                                     const SourceManager &SM,
                                     std::vector<Diagnostic> &Diagnostics) {
  switch (Options.inputForm) {
  case InputForm::DirectiveBody:
    return scanDirectiveBody(Input, SM, Diagnostics);
  case InputForm::CPragma:
    if (Options.language == Language::Fortran) {
      addDiagnostic(Diagnostics, DiagnosticCode::InvalidLanguageForm,
                    "a C pragma cannot be parsed in Fortran mode",
                    SM.range(0, Input.size()));
      return std::nullopt;
    }
    return scanCPragma(Input, SM, Diagnostics);
  case InputForm::CPragmaOperator:
    if (Options.language == Language::Fortran) {
      addDiagnostic(Diagnostics, DiagnosticCode::InvalidLanguageForm,
                    "_Pragma is not a Fortran directive form",
                    SM.range(0, Input.size()));
      return std::nullopt;
    }
    return scanCPragmaOperator(Input, SM, Diagnostics, Options.language);
  case InputForm::FortranFree:
    if (Options.language != Language::Fortran) {
      addDiagnostic(Diagnostics, DiagnosticCode::InvalidLanguageForm,
                    "a Fortran sentinel requires Fortran language mode",
                    SM.range(0, Input.size()));
      return std::nullopt;
    }
    return scanFortranFree(Input, SM, Diagnostics);
  case InputForm::FortranFixed:
    if (Options.language != Language::Fortran) {
      addDiagnostic(Diagnostics, DiagnosticCode::InvalidLanguageForm,
                    "a fixed-form sentinel requires Fortran language mode",
                    SM.range(0, Input.size()));
      return std::nullopt;
    }
    return scanFortranFixed(Input, SM, Diagnostics);
  }
  return std::nullopt;
}

struct PayloadRecord {
  std::size_t begin;
  std::size_t end;
  SourceRange range;
};

struct Occurrence {
  std::string word;
  SourceRange wordRange;
  SourceRange range;
  std::optional<std::size_t> payloadIndex;
};

struct StructuralInput {
  std::string normalized;
  std::vector<std::size_t> normalizedBodyOffsets;
  std::vector<PayloadRecord> payloads;
  std::vector<Occurrence> occurrences;
};

bool skipQuoted(std::string_view Text, std::size_t &Offset, char Quote,
                Language Lang) {
  ++Offset;
  while (Offset < Text.size()) {
    if (Text[Offset] == Quote) {
      if (Lang == Language::Fortran && Offset + 1 < Text.size() &&
          Text[Offset + 1] == Quote) {
        Offset += 2;
        continue;
      }
      ++Offset;
      return true;
    }
    if (Lang != Language::Fortran && Text[Offset] == '\\' &&
        Offset + 1 < Text.size()) {
      Offset += 2;
      continue;
    }
    ++Offset;
  }
  return false;
}

std::optional<std::size_t> getCxxRawStringPrefixLength(std::string_view Text,
                                                       std::size_t Offset) {
  if (Offset > 0 && isIdentifierContinue(Text[Offset - 1]))
    return std::nullopt;

  static constexpr std::string_view Prefixes[] = {"u8R\"", "uR\"", "UR\"",
                                                  "LR\"", "R\""};
  for (std::string_view Prefix : Prefixes)
    if (Text.substr(Offset, Prefix.size()) == Prefix)
      return Prefix.size();
  return std::nullopt;
}

std::optional<std::size_t> findCxxRawStringEnd(std::string_view Text,
                                               std::size_t Begin,
                                               std::size_t PrefixLength) {
  std::size_t DelimiterBegin = Begin + PrefixLength;
  std::size_t Open = Text.find('(', DelimiterBegin);
  if (Open == std::string_view::npos || Open - DelimiterBegin > 16)
    return std::nullopt;

  std::string_view Delimiter =
      Text.substr(DelimiterBegin, Open - DelimiterBegin);
  if (std::any_of(Delimiter.begin(), Delimiter.end(), [](char Ch) {
        return std::isspace(static_cast<unsigned char>(Ch)) || Ch == '(' ||
               Ch == ')' || Ch == '\\';
      }))
    return std::nullopt;

  std::string Closing = ")" + std::string(Delimiter) + "\"";
  std::size_t Close = Text.find(Closing, Open + 1);
  if (Close == std::string_view::npos)
    return std::nullopt;
  return Close + Closing.size();
}

bool hasAdjacentCxxTemplateName(std::string_view Text, std::size_t Open) {
  // Without host-language symbol information, angle brackets are ambiguous.
  // Restrict template recognition to the conventional adjacent-name spelling;
  // the closing-token check below rejects an obvious comparison right-hand
  // side.
  if (Open == 0 || !isIdentifierContinue(Text[Open - 1]))
    return false;
  std::size_t Begin = Open - 1;
  while (Begin > 0 && isIdentifierContinue(Text[Begin - 1]))
    --Begin;
  return isIdentifierStart(Text[Begin]);
}

std::optional<std::size_t> findCxxTemplateArgumentListEnd(std::string_view Text,
                                                          std::size_t Open) {
  if (!hasAdjacentCxxTemplateName(Text, Open) || Open + 1 >= Text.size() ||
      Text[Open + 1] == '<' || Text[Open + 1] == '=' || Text[Open + 1] == '>')
    return std::nullopt;

  int Angles = 1;
  int Parentheses = 0;
  int Brackets = 0;
  int Braces = 0;
  bool SawComma = false;
  for (std::size_t I = Open + 1; I < Text.size();) {
    if (auto PrefixLength = getCxxRawStringPrefixLength(Text, I)) {
      auto End = findCxxRawStringEnd(Text, I, *PrefixLength);
      if (!End)
        return std::nullopt;
      I = *End;
      continue;
    }
    if (Text[I] == '\'' || Text[I] == '"') {
      if (!skipQuoted(Text, I, Text[I], Language::Cxx))
        return std::nullopt;
      continue;
    }
    if (Text[I] == '/' && I + 1 < Text.size()) {
      if (Text[I + 1] == '*') {
        std::size_t Close = Text.find("*/", I + 2);
        if (Close == std::string_view::npos)
          return std::nullopt;
        I = Close + 2;
        continue;
      }
      if (Text[I + 1] == '/') {
        I = Text.find_first_of("\r\n", I + 2);
        if (I == std::string_view::npos)
          return std::nullopt;
        continue;
      }
    }

    char C = Text[I];
    if (C == '(') {
      ++Parentheses;
    } else if (C == ')') {
      if (Parentheses == 0)
        return std::nullopt;
      --Parentheses;
    } else if (C == '[') {
      ++Brackets;
    } else if (C == ']') {
      if (Brackets == 0)
        return std::nullopt;
      --Brackets;
    } else if (C == '{') {
      ++Braces;
    } else if (C == '}') {
      if (Braces == 0)
        return std::nullopt;
      --Braces;
    } else if (Parentheses == 0 && Brackets == 0 && Braces == 0) {
      if (C == '<' && hasAdjacentCxxTemplateName(Text, I) &&
          I + 1 < Text.size() && Text[I + 1] != '<' && Text[I + 1] != '=' &&
          Text[I + 1] != '>') {
        ++Angles;
      } else if (C == '>' && (I == 0 || Text[I - 1] != '-') &&
                 (I + 1 >= Text.size() || Text[I + 1] != '=')) {
        --Angles;
        if (Angles == 0) {
          if (!SawComma)
            return std::nullopt;
          std::size_t Next = I + 1;
          while (Next < Text.size() &&
                 std::isspace(static_cast<unsigned char>(Text[Next])))
            ++Next;
          if (Next < Text.size() &&
              (isIdentifierContinue(Text[Next]) || Text[Next] == '\'' ||
               Text[Next] == '"' || Text[Next] == '>'))
            return std::nullopt;
          return I;
        }
      } else if (C == ',') {
        SawComma = true;
      }
    }
    ++I;
  }
  return std::nullopt;
}

std::optional<std::size_t>
findClosingParenthesis(std::string_view Text, std::size_t Open, Language Lang) {
  int Depth = 1;
  for (std::size_t I = Open + 1; I < Text.size();) {
    char C = Text[I];
    if (Lang == Language::Cxx) {
      if (auto PrefixLength = getCxxRawStringPrefixLength(Text, I)) {
        auto End = findCxxRawStringEnd(Text, I, *PrefixLength);
        if (!End)
          return std::nullopt;
        I = *End;
        continue;
      }
    }
    if (C == '\'' || C == '"') {
      if (!skipQuoted(Text, I, C, Lang))
        return std::nullopt;
      continue;
    }
    if (Lang != Language::Fortran && C == '/' && I + 1 < Text.size() &&
        Text[I + 1] == '*') {
      std::size_t Close = Text.find("*/", I + 2);
      if (Close == std::string_view::npos)
        return std::nullopt;
      I = Close + 2;
      continue;
    }
    if (Lang != Language::Fortran && C == '/' && I + 1 < Text.size() &&
        Text[I + 1] == '/')
      return std::nullopt;
    if (C == '(')
      ++Depth;
    else if (C == ')' && --Depth == 0)
      return I;
    ++I;
  }
  return std::nullopt;
}

void appendNormalized(StructuralInput &Result, std::string_view Text,
                      std::size_t BodyOffset) {
  for (char C : Text) {
    Result.normalized.push_back(C);
    Result.normalizedBodyOffsets.push_back(BodyOffset);
  }
}

bool skipStructuralTrivia(const BodyText &Body, std::size_t &Offset,
                          Language Lang, const SourceManager &SM,
                          std::vector<Diagnostic> &Diagnostics) {
  while (Offset < Body.text.size()) {
    if (std::isspace(static_cast<unsigned char>(Body.text[Offset]))) {
      ++Offset;
      continue;
    }
    if (Lang == Language::Fortran || Body.text[Offset] != '/' ||
        Offset + 1 >= Body.text.size())
      break;
    if (Body.text[Offset + 1] == '/') {
      Offset = Body.text.size();
      break;
    }
    if (Body.text[Offset + 1] != '*')
      break;
    std::size_t Close = Body.text.find("*/", Offset + 2);
    if (Close == std::string::npos) {
      addDiagnostic(Diagnostics, DiagnosticCode::UnterminatedDelimiter,
                    "unterminated comment in OpenACC directive",
                    bodyRange(Body, SM, Offset, Body.text.size()));
      return false;
    }
    Offset = Close + 2;
  }
  return true;
}

std::optional<StructuralInput>
scanStructure(const BodyText &Body, Language Lang, const SourceManager &SM,
              std::vector<Diagnostic> &Diagnostics) {
  StructuralInput Result;
  for (std::size_t I = 0; I < Body.text.size();) {
    if (!skipStructuralTrivia(Body, I, Lang, SM, Diagnostics))
      return std::nullopt;
    if (I == Body.text.size())
      break;
    if (Body.text[I] == ',') {
      appendNormalized(Result, ",", I);
      ++I;
      continue;
    }
    if (!isIdentifierStart(Body.text[I])) {
      addDiagnostic(Diagnostics, DiagnosticCode::InvalidCharacter,
                    "invalid character in OpenACC directive structure",
                    bodyRange(Body, SM, I, I + 1));
      ++I;
      continue;
    }

    std::size_t WordBegin = I++;
    while (I < Body.text.size() && isIdentifierContinue(Body.text[I]))
      ++I;
    std::size_t WordEnd = I;
    std::string Word = Body.text.substr(WordBegin, WordEnd - WordBegin);
    std::string StructuralWord = Lang == Language::Fortran ? lower(Word) : Word;
    if (!Result.normalized.empty() && Result.normalized.back() != ',')
      appendNormalized(Result, " ", WordBegin);
    appendNormalized(Result, StructuralWord, WordBegin);

    if (!skipStructuralTrivia(Body, I, Lang, SM, Diagnostics))
      return std::nullopt;
    Occurrence Item{StructuralWord, bodyRange(Body, SM, WordBegin, WordEnd),
                    bodyRange(Body, SM, WordBegin, WordEnd), std::nullopt};
    if (I < Body.text.size() && Body.text[I] == '(') {
      std::size_t Open = I;
      auto Close = findClosingParenthesis(Body.text, Open, Lang);
      if (!Close) {
        addDiagnostic(Diagnostics, DiagnosticCode::UnterminatedDelimiter,
                      "unterminated parenthesized OpenACC argument",
                      bodyRange(Body, SM, Open, Body.text.size()));
        return std::nullopt;
      }
      PayloadRecord Payload{Open + 1, *Close,
                            bodyRange(Body, SM, Open + 1, *Close)};
      Item.payloadIndex = Result.payloads.size();
      Result.payloads.push_back(Payload);
      std::string Placeholder =
          "(__acc_payload_" + std::to_string(*Item.payloadIndex) + ")";
      appendNormalized(Result, Placeholder, Open);
      Item.range = bodyRange(Body, SM, WordBegin, *Close + 1);
      I = *Close + 1;
    }
    Result.occurrences.push_back(std::move(Item));
  }
  if (Result.occurrences.empty())
    return std::nullopt;
  return Result;
}

class CollectingErrorListener final : public antlr4::BaseErrorListener {
public:
  CollectingErrorListener(const StructuralInput &Structure,
                          const BodyText &Body, const SourceManager &SM,
                          std::vector<Diagnostic> &Diagnostics)
      : Structure(Structure), Body(Body), SM(SM), Diagnostics(Diagnostics) {}

  void syntaxError(antlr4::Recognizer *, antlr4::Token *, std::size_t,
                   std::size_t CharPosition, const std::string &Message,
                   std::exception_ptr) override {
    std::size_t BodyOffset = Body.text.size();
    if (CharPosition < Structure.normalizedBodyOffsets.size())
      BodyOffset = Structure.normalizedBodyOffsets[CharPosition];
    addDiagnostic(Diagnostics, DiagnosticCode::SyntaxError, Message,
                  bodyRange(Body, SM, BodyOffset,
                            std::min(BodyOffset + 1, Body.text.size())));
  }

private:
  const StructuralInput &Structure;
  const BodyText &Body;
  const SourceManager &SM;
  std::vector<Diagnostic> &Diagnostics;
};

bool validateStructureWithANTLR(const StructuralInput &Structure,
                                const BodyText &Body, const SourceManager &SM,
                                std::vector<Diagnostic> &Diagnostics) {
  antlr4::ANTLRInputStream Input(Structure.normalized);
  acclexer Lexer(&Input);
  antlr4::CommonTokenStream Tokens(&Lexer);
  accparser Parser(&Tokens);
  CollectingErrorListener Listener(Structure, Body, SM, Diagnostics);
  Lexer.removeErrorListeners();
  Parser.removeErrorListeners();
  Lexer.addErrorListener(&Listener);
  Parser.addErrorListener(&Listener);
  Parser.setBuildParseTree(true);
  try {
    Parser.directive();
  } catch (const std::exception &Error) {
    addDiagnostic(Diagnostics, DiagnosticCode::SyntaxError, Error.what(),
                  bodyRange(Body, SM, 0, Body.text.size()));
  } catch (...) {
    addDiagnostic(Diagnostics, DiagnosticCode::SyntaxError,
                  "unknown structural parser failure",
                  bodyRange(Body, SM, 0, Body.text.size()));
  }
  return Diagnostics.empty();
}

struct Slice {
  std::size_t begin;
  std::size_t end;
};

Slice trimSlice(const BodyText &Body, Slice Value) {
  auto Bounds = trimBounds(Body.text, Value.begin, Value.end);
  return {Bounds.first, Bounds.second};
}

std::string sliceText(const BodyText &Body, Slice Value) {
  return Body.text.substr(Value.begin, Value.end - Value.begin);
}

struct DelimiterState {
  int parentheses = 0;
  int brackets = 0;
  int braces = 0;
  char quote = 0;
  bool blockComment = false;
  bool lineComment = false;
};

bool advanceDelimiterState(std::string_view Text, std::size_t &Offset,
                           Language Lang, DelimiterState &State) {
  char C = Text[Offset];
  if (State.blockComment) {
    if (C == '*' && Offset + 1 < Text.size() && Text[Offset + 1] == '/') {
      ++Offset;
      State.blockComment = false;
    }
    return true;
  }
  if (State.lineComment) {
    if (C == '\n' || C == '\r')
      State.lineComment = false;
    return true;
  }
  if (State.quote != 0) {
    if (C == State.quote) {
      if (Lang == Language::Fortran && Offset + 1 < Text.size() &&
          Text[Offset + 1] == State.quote) {
        ++Offset;
        return true;
      }
      State.quote = 0;
      return true;
    }
    if (Lang != Language::Fortran && C == '\\' && Offset + 1 < Text.size()) {
      ++Offset;
      return true;
    }
    return true;
  }
  if (Lang == Language::Cxx) {
    if (auto PrefixLength = getCxxRawStringPrefixLength(Text, Offset)) {
      auto End = findCxxRawStringEnd(Text, Offset, *PrefixLength);
      if (!End)
        return false;
      Offset = *End - 1;
      return true;
    }
  }
  if (C == '\'' || C == '"') {
    State.quote = C;
    return true;
  }
  if (Lang != Language::Fortran && C == '/' && Offset + 1 < Text.size()) {
    if (Text[Offset + 1] == '*') {
      ++Offset;
      State.blockComment = true;
      return true;
    }
    if (Text[Offset + 1] == '/') {
      ++Offset;
      State.lineComment = true;
      return true;
    }
  }
  if (Lang == Language::Cxx && C == '<') {
    if (auto End = findCxxTemplateArgumentListEnd(Text, Offset)) {
      Offset = *End;
      return true;
    }
  }
  switch (C) {
  case '(':
    ++State.parentheses;
    break;
  case ')':
    --State.parentheses;
    break;
  case '[':
    ++State.brackets;
    break;
  case ']':
    --State.brackets;
    break;
  case '{':
    ++State.braces;
    break;
  case '}':
    --State.braces;
    break;
  default:
    break;
  }
  return State.parentheses >= 0 && State.brackets >= 0 && State.braces >= 0;
}

bool isTopLevel(const DelimiterState &State) {
  return State.quote == 0 && State.parentheses == 0 && State.brackets == 0 &&
         State.braces == 0 && !State.blockComment && !State.lineComment;
}

std::vector<Slice> splitTopLevel(const BodyText &Body, Slice Input,
                                 char Delimiter, Language Lang,
                                 const SourceManager &SM,
                                 std::vector<Diagnostic> &Diagnostics) {
  std::vector<Slice> Result;
  DelimiterState State;
  std::size_t ItemBegin = Input.begin;
  bool SawDelimiter = false;
  for (std::size_t I = Input.begin; I < Input.end; ++I) {
    if (!advanceDelimiterState(Body.text, I, Lang, State)) {
      addDiagnostic(Diagnostics, DiagnosticCode::UnterminatedDelimiter,
                    "unbalanced delimiter in OpenACC argument",
                    bodyRange(Body, SM, I, I + 1));
      return {};
    }
    if (Body.text[I] == Delimiter && isTopLevel(State)) {
      SawDelimiter = true;
      Slice Item = trimSlice(Body, {ItemBegin, I});
      if (Item.begin == Item.end) {
        addDiagnostic(Diagnostics, DiagnosticCode::EmptyListItem,
                      "OpenACC lists cannot contain empty items",
                      bodyRange(Body, SM, ItemBegin, I + 1));
      } else {
        Result.push_back(Item);
      }
      ItemBegin = I + 1;
    }
  }
  if (State.quote != 0 || State.parentheses != 0 || State.brackets != 0 ||
      State.braces != 0 || State.blockComment) {
    addDiagnostic(Diagnostics, DiagnosticCode::UnterminatedDelimiter,
                  "unbalanced delimiter in OpenACC argument",
                  bodyRange(Body, SM, Input.begin, Input.end));
    return {};
  }
  Slice Last = trimSlice(Body, {ItemBegin, Input.end});
  if (Last.begin == Last.end) {
    addDiagnostic(Diagnostics,
                  SawDelimiter ? DiagnosticCode::TrailingComma
                               : DiagnosticCode::MissingArgument,
                  SawDelimiter ? "OpenACC lists cannot end with a comma"
                               : "the OpenACC argument cannot be empty",
                  bodyRange(Body, SM, ItemBegin, Input.end));
  } else {
    Result.push_back(Last);
  }
  return Result;
}

std::optional<std::size_t> findTopLevelDelimiter(const BodyText &Body,
                                                 Slice Input, char Delimiter,
                                                 Language Lang) {
  DelimiterState State;
  for (std::size_t I = Input.begin; I < Input.end; ++I) {
    advanceDelimiterState(Body.text, I, Lang, State);
    if (Body.text[I] == Delimiter && isTopLevel(State))
      return I;
  }
  return std::nullopt;
}

std::vector<std::size_t> findTopLevelSingleColons(const BodyText &Body,
                                                  Slice Input, Language Lang) {
  std::vector<std::size_t> Result;
  DelimiterState State;
  for (std::size_t I = Input.begin; I < Input.end; ++I) {
    advanceDelimiterState(Body.text, I, Lang, State);
    if (Body.text[I] != ':' || !isTopLevel(State))
      continue;
    bool PreviousColon = I > Input.begin && Body.text[I - 1] == ':';
    bool NextColon = I + 1 < Input.end && Body.text[I + 1] == ':';
    if (!PreviousColon && !NextColon)
      Result.push_back(I);
  }
  return Result;
}

std::optional<std::size_t> findTopLevelWaitQueueSeparator(const BodyText &Body,
                                                          Slice Input,
                                                          Language Lang) {
  DelimiterState State;
  std::size_t PendingConditionals = 0;
  for (std::size_t I = Input.begin; I < Input.end; ++I) {
    advanceDelimiterState(Body.text, I, Lang, State);
    if (!isTopLevel(State))
      continue;
    if (Lang != Language::Fortran && Body.text[I] == '?') {
      ++PendingConditionals;
      continue;
    }
    if (Body.text[I] != ':')
      continue;
    bool PreviousColon = I > Input.begin && Body.text[I - 1] == ':';
    bool NextColon = I + 1 < Input.end && Body.text[I + 1] == ':';
    if (PreviousColon || NextColon)
      continue;
    // A top-level conditional colon belongs to the C or C++ integer
    // expression, not to the surrounding wait syntax.
    if (PendingConditionals != 0) {
      --PendingConditionals;
      continue;
    }
    return I;
  }
  return std::nullopt;
}

template <typename Fragment>
std::optional<Fragment>
makeFragment(const BodyText &Body, Slice Input, const SourceManager &SM,
             std::vector<Diagnostic> &Diagnostics, std::string_view Role) {
  Input = trimSlice(Body, Input);
  if (Input.begin == Input.end) {
    addDiagnostic(Diagnostics, DiagnosticCode::MissingArgument,
                  std::string(Role) + " cannot be empty",
                  bodyRange(Body, SM, Input.begin, Input.end));
    return std::nullopt;
  }
  auto Result = Fragment::create(sliceText(Body, Input),
                                 bodyRange(Body, SM, Input.begin, Input.end));
  if (!Result)
    addDiagnostic(Diagnostics, DiagnosticCode::InvalidValue,
                  std::string("invalid ") + std::string(Role),
                  bodyRange(Body, SM, Input.begin, Input.end));
  return Result;
}

template <typename Fragment>
std::optional<NonEmptyList<Fragment>>
parseFragmentList(const BodyText &Body, Slice Input, Language Lang,
                  const SourceManager &SM, std::vector<Diagnostic> &Diagnostics,
                  std::string_view Role) {
  std::vector<Slice> Parts =
      splitTopLevel(Body, Input, ',', Lang, SM, Diagnostics);
  std::vector<Fragment> Values;
  for (Slice Part : Parts) {
    auto Value = makeFragment<Fragment>(Body, Part, SM, Diagnostics, Role);
    if (Value)
      Values.push_back(std::move(*Value));
  }
  return NonEmptyList<Fragment>::create(std::move(Values));
}

std::optional<PayloadRecord> getPayload(const Occurrence &Item,
                                        const StructuralInput &Structure,
                                        std::vector<Diagnostic> &Diagnostics,
                                        bool Required) {
  if (!Item.payloadIndex) {
    if (Required)
      addDiagnostic(Diagnostics, DiagnosticCode::MissingArgument,
                    "the '" + Item.word + "' clause requires an argument",
                    Item.wordRange);
    return std::nullopt;
  }
  return Structure.payloads[*Item.payloadIndex];
}

bool rejectPayload(const Occurrence &Item,
                   std::vector<Diagnostic> &Diagnostics) {
  if (!Item.payloadIndex)
    return false;
  addDiagnostic(Diagnostics, DiagnosticCode::UnexpectedArgument,
                "the '" + Item.word + "' clause does not take an argument",
                Item.range);
  return true;
}

bool isIdentifier(std::string_view Text) {
  if (Text.empty() || !isIdentifierStart(Text.front()))
    return false;
  return std::all_of(Text.begin() + 1, Text.end(), isIdentifierContinue);
}

bool isRoutineName(std::string_view Text, Language Lang) {
  if (Lang == Language::Fortran)
    return isIdentifier(Text);
  std::size_t Offset = 0;
  if (Text.substr(0, 2) == "::")
    Offset = 2;
  while (Offset < Text.size()) {
    std::size_t Begin = Offset;
    if (!isIdentifierStart(Text[Offset]))
      return false;
    while (Offset < Text.size() && isIdentifierContinue(Text[Offset]))
      ++Offset;
    if (Begin == Offset)
      return false;
    if (Offset == Text.size())
      return true;
    if (Text.substr(Offset, 2) != "::")
      return false;
    Offset += 2;
  }
  return false;
}

std::optional<StringLiteral>
parseStringLiteral(const BodyText &Body, Slice Input, Language Lang,
                   const SourceManager &SM,
                   std::vector<Diagnostic> &Diagnostics) {
  Input = trimSlice(Body, Input);
  std::string Spelling = sliceText(Body, Input);
  std::size_t QuoteOffset = 0;
  bool Raw = false;
  if (Lang != Language::Fortran) {
    if (Spelling.rfind("u8", 0) == 0)
      QuoteOffset = 2;
    else if (!Spelling.empty() &&
             (Spelling[0] == 'L' || Spelling[0] == 'u' || Spelling[0] == 'U'))
      QuoteOffset = 1;
    if (Lang == Language::Cxx && QuoteOffset < Spelling.size() &&
        Spelling[QuoteOffset] == 'R') {
      Raw = true;
      ++QuoteOffset;
    }
  }
  if (QuoteOffset >= Spelling.size()) {
    addDiagnostic(Diagnostics, DiagnosticCode::InvalidValue,
                  "invalid string literal",
                  bodyRange(Body, SM, Input.begin, Input.end));
    return std::nullopt;
  }
  char Quote = Spelling[QuoteOffset];
  if (Spelling.back() != Quote || (Lang != Language::Fortran && Quote != '"') ||
      (Lang == Language::Fortran && Quote != '\'' && Quote != '"')) {
    addDiagnostic(Diagnostics, DiagnosticCode::InvalidValue,
                  Lang == Language::Fortran
                      ? "expected a Fortran character literal"
                      : "expected a C/C++ string literal",
                  bodyRange(Body, SM, Input.begin, Input.end));
    return std::nullopt;
  }

  if (Raw) {
    std::size_t Open = Spelling.find('(', QuoteOffset + 1);
    if (Open == std::string::npos || Open - QuoteOffset - 1 > 16) {
      addDiagnostic(Diagnostics, DiagnosticCode::InvalidValue,
                    "invalid C++ raw string delimiter",
                    bodyRange(Body, SM, Input.begin, Input.end));
      return std::nullopt;
    }
    std::string Delimiter =
        Spelling.substr(QuoteOffset + 1, Open - QuoteOffset - 1);
    if (std::any_of(Delimiter.begin(), Delimiter.end(), [](char Ch) {
          return std::isspace(static_cast<unsigned char>(Ch)) || Ch == '(' ||
                 Ch == ')' || Ch == '\\';
        })) {
      addDiagnostic(Diagnostics, DiagnosticCode::InvalidValue,
                    "invalid C++ raw string delimiter",
                    bodyRange(Body, SM, Input.begin, Input.end));
      return std::nullopt;
    }
    std::string Closing = ")" + Delimiter + "\"";
    if (Spelling.size() < Open + 1 + Closing.size() ||
        Spelling.compare(Spelling.size() - Closing.size(), Closing.size(),
                         Closing) != 0 ||
        Spelling.size() == Open + 1 + Closing.size()) {
      addDiagnostic(Diagnostics, DiagnosticCode::InvalidValue,
                    "invalid or empty C++ raw bind string",
                    bodyRange(Body, SM, Input.begin, Input.end));
      return std::nullopt;
    }
    return StringLiteral::create(std::move(Spelling),
                                 bodyRange(Body, SM, Input.begin, Input.end));
  }

  bool HasContent = false;
  for (std::size_t I = QuoteOffset + 1; I + 1 < Spelling.size(); ++I) {
    char C = Spelling[I];
    if (Lang == Language::Fortran && C == Quote &&
        I + 1 < Spelling.size() - 1 && Spelling[I + 1] == Quote) {
      HasContent = true;
      ++I;
      continue;
    }
    if (Lang != Language::Fortran && C == '\\' && I + 1 < Spelling.size() - 1) {
      HasContent = true;
      ++I;
      continue;
    }
    if (C == Quote) {
      addDiagnostic(Diagnostics, DiagnosticCode::InvalidValue,
                    "unescaped quote in string literal",
                    bodyRange(Body, SM, Input.begin + I, Input.begin + I + 1));
      return std::nullopt;
    }
    HasContent = true;
  }
  auto Result = StringLiteral::create(
      std::move(Spelling), bodyRange(Body, SM, Input.begin, Input.end));
  if (!HasContent)
    Result = std::nullopt;
  if (!Result)
    addDiagnostic(Diagnostics, DiagnosticCode::InvalidValue,
                  "a bind string cannot be empty",
                  bodyRange(Body, SM, Input.begin, Input.end));
  return Result;
}

std::optional<NameOrString>
parseNameOrString(const BodyText &Body, Slice Input, Language Lang,
                  const SourceManager &SM,
                  std::vector<Diagnostic> &Diagnostics) {
  Input = trimSlice(Body, Input);
  if (Input.begin == Input.end) {
    addDiagnostic(Diagnostics, DiagnosticCode::MissingArgument,
                  "a bind target cannot be empty",
                  bodyRange(Body, SM, Input.begin, Input.end));
    return std::nullopt;
  }
  std::string Text = sliceText(Body, Input);
  bool LooksLikeLiteral = Text.front() == '\'' || Text.front() == '"';
  if (Lang != Language::Fortran)
    for (std::string_view Prefix : {"L\"", "u\"", "U\"", "u8\""})
      LooksLikeLiteral |= Text.rfind(Prefix, 0) == 0;
  if (Lang == Language::Cxx)
    for (std::string_view Prefix : {"R\"", "LR\"", "uR\"", "UR\"", "u8R\""})
      LooksLikeLiteral |= Text.rfind(Prefix, 0) == 0;
  if (LooksLikeLiteral) {
    auto Literal = parseStringLiteral(Body, Input, Lang, SM, Diagnostics);
    if (Literal)
      return NameOrString(std::move(*Literal));
    return std::nullopt;
  }
  std::string Name = std::move(Text);
  if (!isIdentifier(Name)) {
    addDiagnostic(Diagnostics, DiagnosticCode::InvalidValue,
                  "expected an identifier or string literal",
                  bodyRange(Body, SM, Input.begin, Input.end));
    return std::nullopt;
  }
  auto Result = Identifier::create(std::move(Name),
                                   bodyRange(Body, SM, Input.begin, Input.end));
  if (!Result)
    return std::nullopt;
  return NameOrString(std::move(*Result));
}

std::optional<DeviceSelectorList>
parseDeviceSelectors(const BodyText &Body, Slice Input, Language Lang,
                     const SourceManager &SM,
                     std::vector<Diagnostic> &Diagnostics) {
  std::vector<Slice> Parts =
      splitTopLevel(Body, Input, ',', Lang, SM, Diagnostics);
  std::vector<DeviceSelector> Selectors;
  bool SawWildcard = false;
  for (Slice Part : Parts) {
    Part = trimSlice(Body, Part);
    std::string Text = sliceText(Body, Part);
    if (Text == "*") {
      SawWildcard = true;
      Selectors.push_back(
          WildcardDevice{bodyRange(Body, SM, Part.begin, Part.end)});
      continue;
    }
    if (!isIdentifier(Text)) {
      addDiagnostic(Diagnostics, DiagnosticCode::InvalidValue,
                    "device_type expects architecture identifiers or '*'",
                    bodyRange(Body, SM, Part.begin, Part.end));
      continue;
    }
    auto IdentifierValue = ArchitectureIdentifier::create(
        std::move(Text), bodyRange(Body, SM, Part.begin, Part.end));
    if (IdentifierValue)
      Selectors.push_back(std::move(*IdentifierValue));
  }
  if (SawWildcard && Selectors.size() != 1)
    addDiagnostic(Diagnostics, DiagnosticCode::InvalidCombination,
                  "the device_type wildcard '*' must appear alone",
                  bodyRange(Body, SM, Input.begin, Input.end));
  return DeviceSelectorList::create(std::move(Selectors));
}

std::optional<std::size_t> keyColon(const BodyText &Body, Slice Input,
                                    std::string_view Key, Language Lang) {
  for (std::size_t Colon : findTopLevelSingleColons(Body, Input, Lang)) {
    Slice Prefix = trimSlice(Body, {Input.begin, Colon});
    if (equalsKeyword(sliceText(Body, Prefix), Key, Lang))
      return Colon;
    return std::nullopt;
  }
  return std::nullopt;
}

std::optional<WaitArgument>
parseWaitArgument(const BodyText &Body, const PayloadRecord &Payload,
                  Language Lang, const SourceManager &SM,
                  std::vector<Diagnostic> &Diagnostics) {
  Slice Whole = trimSlice(Body, {Payload.begin, Payload.end});
  if (Whole.begin == Whole.end) {
    addDiagnostic(Diagnostics, DiagnosticCode::MissingArgument,
                  "wait() requires a device or queue argument", Payload.range);
    return std::nullopt;
  }

  std::optional<IntegerExpr> DeviceNumber;
  std::optional<NonEmptyList<AsyncArgument>> Queues;
  bool HasQueuesKeyword = false;
  Slice Remaining = Whole;

  if (auto DevnumColon = keyColon(Body, Remaining, "devnum", Lang)) {
    Slice AfterDevnum{*DevnumColon + 1, Remaining.end};

    Slice DeviceSlice = AfterDevnum;
    std::optional<Slice> QueueSlice;
    std::optional<std::size_t> FirstComma =
        findTopLevelDelimiter(Body, AfterDevnum, ',', Lang);
    std::optional<std::size_t> FirstColon =
        findTopLevelWaitQueueSeparator(Body, AfterDevnum, Lang);
    if (FirstComma && (!FirstColon || *FirstComma < *FirstColon)) {
      DeviceSlice = {AfterDevnum.begin, *FirstComma};
      QueueSlice = Slice{*FirstComma + 1, AfterDevnum.end};
    } else if (FirstColon) {
      DeviceSlice = {AfterDevnum.begin, *FirstColon};
      QueueSlice = Slice{*FirstColon + 1, AfterDevnum.end};
    }
    DeviceNumber = makeFragment<IntegerExpr>(Body, DeviceSlice, SM, Diagnostics,
                                             "wait device number");
    if (QueueSlice)
      Remaining = trimSlice(Body, *QueueSlice);
    else
      Remaining = {Whole.end, Whole.end};
  }

  if (Remaining.begin != Remaining.end) {
    if (auto QueuesColon = keyColon(Body, Remaining, "queues", Lang)) {
      HasQueuesKeyword = true;
      Remaining = trimSlice(Body, {*QueuesColon + 1, Remaining.end});
    }
    Queues = parseFragmentList<AsyncArgument>(
        Body, Remaining, Lang, SM, Diagnostics, "wait queue expression");
  }

  if (!DeviceNumber && !Queues)
    return std::nullopt;
  return WaitArgument{std::move(DeviceNumber), std::move(Queues),
                      HasQueuesKeyword, Payload.range};
}

std::optional<ReductionOperator> parseReductionOperator(std::string_view Text,
                                                        Language Lang) {
  bool IsFortran = Lang == Language::Fortran;
  std::string Key = IsFortran ? lower(Text) : std::string(Text);
  if (Key == "+")
    return ReductionOperator::Add;
  // OpenACC-VV uses C/C++ subtraction reductions as a compatibility extension.
  if (!IsFortran && Key == "-")
    return ReductionOperator::Subtract;
  if (Key == "*")
    return ReductionOperator::Multiply;
  if (Key == "max")
    return ReductionOperator::Maximum;
  if (Key == "min")
    return ReductionOperator::Minimum;
  if (!IsFortran && Key == "&")
    return ReductionOperator::BitAnd;
  if (!IsFortran && Key == "|")
    return ReductionOperator::BitOr;
  if (!IsFortran && Key == "^")
    return ReductionOperator::BitXor;
  if (!IsFortran && Key == "&&")
    return ReductionOperator::LogicalAnd;
  if (!IsFortran && Key == "||")
    return ReductionOperator::LogicalOr;
  if (IsFortran && Key == ".and.")
    return ReductionOperator::FortranAnd;
  if (IsFortran && Key == ".or.")
    return ReductionOperator::FortranOr;
  if (IsFortran && Key == ".eqv.")
    return ReductionOperator::FortranEqv;
  if (IsFortran && Key == ".neqv.")
    return ReductionOperator::FortranNeqv;
  if (IsFortran && Key == "iand")
    return ReductionOperator::FortranIand;
  if (IsFortran && Key == "ior")
    return ReductionOperator::FortranIor;
  if (IsFortran && Key == "ieor")
    return ReductionOperator::FortranIeor;
  return std::nullopt;
}

std::optional<ReductionClause>
parseReductionClause(const BodyText &Body, const PayloadRecord &Payload,
                     Language Lang, const SourceManager &SM,
                     std::vector<Diagnostic> &Diagnostics,
                     SourceRange ClauseRange) {
  Slice Whole{Payload.begin, Payload.end};
  std::vector<std::size_t> Colons = findTopLevelSingleColons(Body, Whole, Lang);
  if (Colons.empty()) {
    addDiagnostic(Diagnostics, DiagnosticCode::MissingArgument,
                  "reduction requires an operator followed by ':'",
                  Payload.range);
    return std::nullopt;
  }
  std::size_t Colon = Colons.front();
  Slice OperatorSlice = trimSlice(Body, {Whole.begin, Colon});
  Slice VariablesSlice = trimSlice(Body, {Colon + 1, Whole.end});
  auto Operator = parseReductionOperator(sliceText(Body, OperatorSlice), Lang);
  if (!Operator) {
    addDiagnostic(Diagnostics, DiagnosticCode::InvalidValue,
                  "unsupported OpenACC reduction operator",
                  bodyRange(Body, SM, OperatorSlice.begin, OperatorSlice.end));
    return std::nullopt;
  }
  auto Variables = parseFragmentList<VariableRef>(
      Body, VariablesSlice, Lang, SM, Diagnostics, "reduction variable");
  if (!Variables)
    return std::nullopt;
  return ReductionClause{*Operator, std::move(*Variables), ClauseRange};
}

std::optional<GangClause>
parseGangClause(const BodyText &Body,
                const std::optional<PayloadRecord> &Payload, Language Lang,
                const SourceManager &SM, std::vector<Diagnostic> &Diagnostics,
                SourceRange ClauseRange) {
  if (!Payload)
    return GangClause{{}, ClauseRange};
  std::vector<Slice> Parts = splitTopLevel(Body, {Payload->begin, Payload->end},
                                           ',', Lang, SM, Diagnostics);
  std::vector<GangArgument> Arguments;
  for (Slice Part : Parts) {
    std::vector<std::size_t> Colons =
        findTopLevelSingleColons(Body, Part, Lang);
    std::optional<std::size_t> NamedColon;
    std::string Key;
    if (!Colons.empty()) {
      Slice KeySlice = trimSlice(Body, {Part.begin, Colons.front()});
      Key = Lang == Language::Fortran ? lower(sliceText(Body, KeySlice))
                                      : sliceText(Body, KeySlice);
      if (Key == "num" || Key == "dim" || Key == "static")
        NamedColon = Colons.front();
    }
    if (!NamedColon) {
      auto Value = makeFragment<IntegerExpr>(Body, Part, SM, Diagnostics,
                                             "gang num expression");
      if (Value)
        Arguments.push_back(PositionalGangArgument{std::move(*Value)});
      continue;
    }
    Slice ValueSlice = trimSlice(Body, {*NamedColon + 1, Part.end});
    if (Key == "num" || Key == "dim") {
      auto Value = makeFragment<IntegerExpr>(
          Body, ValueSlice, SM, Diagnostics,
          Key == "num" ? "gang num expression" : "gang dim expression");
      if (!Value)
        continue;
      if (Key == "num")
        Arguments.push_back(NumGangArgument{std::move(*Value)});
      else
        Arguments.push_back(DimGangArgument{std::move(*Value)});
      continue;
    }
    if (Key == "static") {
      std::string ValueText = sliceText(Body, ValueSlice);
      if (ValueText == "*") {
        Arguments.push_back(StaticGangArgument{
            StarSize{bodyRange(Body, SM, ValueSlice.begin, ValueSlice.end)}});
      } else {
        auto Value = makeFragment<IntegerExpr>(Body, ValueSlice, SM,
                                               Diagnostics, "gang static size");
        if (Value)
          Arguments.push_back(StaticGangArgument{SizeExpr(std::move(*Value))});
      }
      continue;
    }
  }
  return GangClause{std::move(Arguments), ClauseRange};
}

template <typename Modifier>
bool addUniqueModifier(std::vector<Modifier> &Modifiers, Modifier Value,
                       const BodyText &Body, const SourceManager &SM,
                       Slice ModifierSlice,
                       std::vector<Diagnostic> &Diagnostics) {
  if (std::find(Modifiers.begin(), Modifiers.end(), Value) != Modifiers.end()) {
    addDiagnostic(Diagnostics, DiagnosticCode::DuplicateModifier,
                  "a data-clause modifier cannot be repeated",
                  bodyRange(Body, SM, ModifierSlice.begin, ModifierSlice.end));
    return false;
  }
  Modifiers.push_back(Value);
  return true;
}

struct DataPayloadSlices {
  std::vector<Slice> modifiers;
  Slice variables;
};

DataPayloadSlices splitDataPayload(const BodyText &Body,
                                   const PayloadRecord &Payload, Language Lang,
                                   const SourceManager &SM,
                                   std::vector<Diagnostic> &Diagnostics) {
  Slice Whole{Payload.begin, Payload.end};
  std::vector<std::size_t> Colons = findTopLevelSingleColons(Body, Whole, Lang);
  if (Colons.empty())
    return {{}, Whole};
  std::size_t Colon = Colons.front();
  Slice ModifierPart{Whole.begin, Colon};
  std::vector<Slice> Modifiers =
      splitTopLevel(Body, ModifierPart, ',', Lang, SM, Diagnostics);
  return {std::move(Modifiers), {Colon + 1, Whole.end}};
}

std::optional<Clause> parseDataClause(const Occurrence &Item,
                                      const PayloadRecord &Payload,
                                      const BodyText &Body, Language Lang,
                                      const SourceManager &SM,
                                      std::vector<Diagnostic> &Diagnostics) {
  DataPayloadSlices Parts =
      splitDataPayload(Body, Payload, Lang, SM, Diagnostics);
  auto Variables = parseFragmentList<VariableRef>(
      Body, Parts.variables, Lang, SM, Diagnostics, "data variable");
  if (!Variables)
    return std::nullopt;

  auto ModifierName = [&](Slice Value) {
    std::string Text = sliceText(Body, trimSlice(Body, Value));
    return Lang == Language::Fortran ? lower(Text) : Text;
  };
  std::string Word = Item.word;
  if (Word == "pcopy" || Word == "present_or_copy")
    Word = "copy";
  if (Word == "pcopyin" || Word == "present_or_copyin")
    Word = "copyin";
  if (Word == "pcopyout" || Word == "present_or_copyout")
    Word = "copyout";
  if (Word == "pcreate" || Word == "present_or_create")
    Word = "create";

  if (Word == "copy") {
    std::vector<CopyModifier> Modifiers;
    for (Slice ModifierSlice : Parts.modifiers) {
      std::string Name = ModifierName(ModifierSlice);
      std::optional<CopyModifier> Value;
      if (Name == "always")
        Value = CopyModifier::Always;
      else if (Name == "alwaysin")
        Value = CopyModifier::AlwaysIn;
      else if (Name == "alwaysout")
        Value = CopyModifier::AlwaysOut;
      else if (Name == "capture")
        Value = CopyModifier::Capture;
      if (!Value) {
        addDiagnostic(
            Diagnostics, DiagnosticCode::InvalidModifier,
            "modifier is not permitted on copy",
            bodyRange(Body, SM, ModifierSlice.begin, ModifierSlice.end));
      } else {
        addUniqueModifier(Modifiers, *Value, Body, SM, ModifierSlice,
                          Diagnostics);
      }
    }
    return Clause(
        CopyClause{std::move(Modifiers), std::move(*Variables), Item.range});
  }
  if (Word == "copyin") {
    std::vector<CopyInModifier> Modifiers;
    for (Slice ModifierSlice : Parts.modifiers) {
      std::string Name = ModifierName(ModifierSlice);
      std::optional<CopyInModifier> Value;
      if (Name == "always")
        Value = CopyInModifier::Always;
      else if (Name == "alwaysin")
        Value = CopyInModifier::AlwaysIn;
      else if (Name == "capture")
        Value = CopyInModifier::Capture;
      else if (Name == "readonly")
        Value = CopyInModifier::ReadOnly;
      if (!Value) {
        addDiagnostic(
            Diagnostics, DiagnosticCode::InvalidModifier,
            "modifier is not permitted on copyin",
            bodyRange(Body, SM, ModifierSlice.begin, ModifierSlice.end));
      } else {
        addUniqueModifier(Modifiers, *Value, Body, SM, ModifierSlice,
                          Diagnostics);
      }
    }
    return Clause(
        CopyInClause{std::move(Modifiers), std::move(*Variables), Item.range});
  }
  if (Word == "copyout") {
    std::vector<CopyOutModifier> Modifiers;
    for (Slice ModifierSlice : Parts.modifiers) {
      std::string Name = ModifierName(ModifierSlice);
      std::optional<CopyOutModifier> Value;
      if (Name == "always")
        Value = CopyOutModifier::Always;
      else if (Name == "alwaysout")
        Value = CopyOutModifier::AlwaysOut;
      // The OpenACC 3.4 erratum changes line 1928 from alwaysin to
      // alwaysout. Line 1929 separately permits capture on structured data
      // and compute constructs; validate that context after construction.
      else if (Name == "capture")
        Value = CopyOutModifier::Capture;
      else if (Name == "zero")
        Value = CopyOutModifier::Zero;
      if (!Value) {
        addDiagnostic(
            Diagnostics, DiagnosticCode::InvalidModifier,
            "modifier is not permitted on copyout",
            bodyRange(Body, SM, ModifierSlice.begin, ModifierSlice.end));
      } else {
        addUniqueModifier(Modifiers, *Value, Body, SM, ModifierSlice,
                          Diagnostics);
      }
    }
    return Clause(
        CopyOutClause{std::move(Modifiers), std::move(*Variables), Item.range});
  }

  std::vector<CreateModifier> Modifiers;
  for (Slice ModifierSlice : Parts.modifiers) {
    std::string Name = ModifierName(ModifierSlice);
    std::optional<CreateModifier> Value;
    if (Name == "capture")
      Value = CreateModifier::Capture;
    else if (Name == "zero")
      Value = CreateModifier::Zero;
    if (!Value) {
      addDiagnostic(
          Diagnostics, DiagnosticCode::InvalidModifier,
          "modifier is not permitted on create",
          bodyRange(Body, SM, ModifierSlice.begin, ModifierSlice.end));
    } else {
      addUniqueModifier(Modifiers, *Value, Body, SM, ModifierSlice,
                        Diagnostics);
    }
  }
  return Clause(
      CreateClause{std::move(Modifiers), std::move(*Variables), Item.range});
}

std::optional<VarListClauseKind> getVarListKind(std::string_view Word,
                                                DirectiveKind Directive) {
  if (Word == "attach")
    return VarListClauseKind::Attach;
  if (Word == "delete")
    return VarListClauseKind::Delete;
  if (Word == "detach")
    return VarListClauseKind::Detach;
  if (Word == "device")
    return VarListClauseKind::Device;
  if (Word == "device_resident")
    return VarListClauseKind::DeviceResident;
  if (Word == "deviceptr")
    return VarListClauseKind::DevicePtr;
  if (Word == "firstprivate")
    return VarListClauseKind::FirstPrivate;
  if (Word == "host")
    return VarListClauseKind::Host;
  if (Word == "link")
    return VarListClauseKind::Link;
  if (Word == "no_create")
    return VarListClauseKind::NoCreate;
  if (Word == "present")
    return VarListClauseKind::Present;
  if (Word == "private")
    return VarListClauseKind::Private;
  if (Word == "self" && Directive == DirectiveKind::Update)
    return VarListClauseKind::Self;
  if (Word == "use_device")
    return VarListClauseKind::UseDevice;
  return std::nullopt;
}

std::optional<FlagClauseKind> getFlagKind(std::string_view Word) {
  if (Word == "auto")
    return FlagClauseKind::Auto;
  if (Word == "capture")
    return FlagClauseKind::Capture;
  if (Word == "finalize")
    return FlagClauseKind::Finalize;
  if (Word == "if_present")
    return FlagClauseKind::IfPresent;
  if (Word == "independent")
    return FlagClauseKind::Independent;
  if (Word == "nohost")
    return FlagClauseKind::NoHost;
  if (Word == "read")
    return FlagClauseKind::Read;
  if (Word == "seq")
    return FlagClauseKind::Seq;
  if (Word == "update")
    return FlagClauseKind::Update;
  if (Word == "write")
    return FlagClauseKind::Write;
  return std::nullopt;
}

std::optional<Clause> parseClause(const Occurrence &Item,
                                  DirectiveKind Directive,
                                  const StructuralInput &Structure,
                                  const BodyText &Body, Language Lang,
                                  const SourceManager &SM,
                                  std::vector<Diagnostic> &Diagnostics) {
  const std::string &Word = Item.word;
  if (Word == "indirect") {
    addDiagnostic(Diagnostics, DiagnosticCode::UnsupportedExtension,
                  "'indirect' is not an OpenACC 3.4 clause", Item.wordRange);
    return std::nullopt;
  }

  if (auto Flag = getFlagKind(Word)) {
    rejectPayload(Item, Diagnostics);
    return Clause(FlagClause{*Flag, Item.range});
  }

  if (Word == "async") {
    std::optional<AsyncArgument> Argument;
    if (auto Payload = getPayload(Item, Structure, Diagnostics, false))
      Argument =
          makeFragment<AsyncArgument>(Body, {Payload->begin, Payload->end}, SM,
                                      Diagnostics, "async argument");
    return Clause(AsyncClause{std::move(Argument), Item.range});
  }

  if (Word == "bind") {
    auto Payload = getPayload(Item, Structure, Diagnostics, true);
    if (!Payload)
      return std::nullopt;
    auto Target = parseNameOrString(Body, {Payload->begin, Payload->end}, Lang,
                                    SM, Diagnostics);
    if (!Target)
      return std::nullopt;
    return Clause(BindClause{std::move(*Target), Item.range});
  }

  if (Word == "collapse") {
    auto Payload = getPayload(Item, Structure, Diagnostics, true);
    if (!Payload)
      return std::nullopt;
    Slice ValueSlice = trimSlice(Body, {Payload->begin, Payload->end});
    bool Force = false;
    if (auto Colon = keyColon(Body, ValueSlice, "force", Lang)) {
      Force = true;
      ValueSlice = trimSlice(Body, {*Colon + 1, ValueSlice.end});
    }
    auto Count = makeFragment<IntegralConstantExpr>(
        Body, ValueSlice, SM, Diagnostics, "collapse count");
    if (!Count)
      return std::nullopt;
    return Clause(CollapseClause{Force, std::move(*Count), Item.range});
  }

  if (Word == "copy" || Word == "pcopy" || Word == "present_or_copy" ||
      Word == "copyin" || Word == "pcopyin" || Word == "present_or_copyin" ||
      Word == "copyout" || Word == "pcopyout" || Word == "present_or_copyout" ||
      Word == "create" || Word == "pcreate" || Word == "present_or_create") {
    auto Payload = getPayload(Item, Structure, Diagnostics, true);
    if (!Payload)
      return std::nullopt;
    return parseDataClause(Item, *Payload, Body, Lang, SM, Diagnostics);
  }

  if (Word == "default") {
    auto Payload = getPayload(Item, Structure, Diagnostics, true);
    if (!Payload)
      return std::nullopt;
    Slice ValueSlice = trimSlice(Body, {Payload->begin, Payload->end});
    std::string Value = sliceText(Body, ValueSlice);
    if (Lang == Language::Fortran)
      Value = lower(Value);
    if (Value == "none")
      return Clause(DefaultClause{DefaultKind::None, Item.range});
    if (Value == "present")
      return Clause(DefaultClause{DefaultKind::Present, Item.range});
    addDiagnostic(Diagnostics, DiagnosticCode::InvalidValue,
                  "default expects 'none' or 'present'", Payload->range);
    return std::nullopt;
  }

  if (Word == "default_async") {
    auto Payload = getPayload(Item, Structure, Diagnostics, true);
    if (!Payload)
      return std::nullopt;
    auto Argument =
        makeFragment<AsyncArgument>(Body, {Payload->begin, Payload->end}, SM,
                                    Diagnostics, "default_async argument");
    if (!Argument)
      return std::nullopt;
    return Clause(DefaultAsyncClause{std::move(*Argument), Item.range});
  }

  if (Word == "device_num") {
    auto Payload = getPayload(Item, Structure, Diagnostics, true);
    if (!Payload)
      return std::nullopt;
    auto Value =
        makeFragment<IntegerExpr>(Body, {Payload->begin, Payload->end}, SM,
                                  Diagnostics, "device_num expression");
    if (!Value)
      return std::nullopt;
    return Clause(DeviceNumClause{std::move(*Value), Item.range});
  }

  if (Word == "device_type" || Word == "dtype") {
    auto Payload = getPayload(Item, Structure, Diagnostics, true);
    if (!Payload)
      return std::nullopt;
    auto Selectors = parseDeviceSelectors(Body, {Payload->begin, Payload->end},
                                          Lang, SM, Diagnostics);
    if (!Selectors)
      return std::nullopt;
    return Clause(DeviceTypeClause{std::move(*Selectors), Item.range});
  }

  if (Word == "gang") {
    std::optional<PayloadRecord> Payload =
        getPayload(Item, Structure, Diagnostics, false);
    auto Result =
        parseGangClause(Body, Payload, Lang, SM, Diagnostics, Item.range);
    if (!Result)
      return std::nullopt;
    return Clause(std::move(*Result));
  }

  if (Word == "if") {
    auto Payload = getPayload(Item, Structure, Diagnostics, true);
    if (!Payload)
      return std::nullopt;
    auto Value = makeFragment<Condition>(Body, {Payload->begin, Payload->end},
                                         SM, Diagnostics, "if condition");
    if (!Value)
      return std::nullopt;
    return Clause(IfClause{std::move(*Value), Item.range});
  }

  if (Word == "num_gangs") {
    auto Payload = getPayload(Item, Structure, Diagnostics, true);
    if (!Payload)
      return std::nullopt;
    auto Values = parseFragmentList<IntegerExpr>(
        Body, {Payload->begin, Payload->end}, Lang, SM, Diagnostics,
        "num_gangs expression");
    if (!Values)
      return std::nullopt;
    return Clause(NumGangsClause{std::move(*Values), Item.range});
  }

  if (Word == "num_workers") {
    auto Payload = getPayload(Item, Structure, Diagnostics, true);
    if (!Payload)
      return std::nullopt;
    auto Value =
        makeFragment<IntegerExpr>(Body, {Payload->begin, Payload->end}, SM,
                                  Diagnostics, "num_workers expression");
    if (!Value)
      return std::nullopt;
    return Clause(NumWorkersClause{std::move(*Value), Item.range});
  }

  if (Word == "reduction") {
    auto Payload = getPayload(Item, Structure, Diagnostics, true);
    if (!Payload)
      return std::nullopt;
    auto Result =
        parseReductionClause(Body, *Payload, Lang, SM, Diagnostics, Item.range);
    if (!Result)
      return std::nullopt;
    return Clause(std::move(*Result));
  }

  if (Word == "self" && Directive != DirectiveKind::Update) {
    std::optional<Condition> Value;
    if (auto Payload = getPayload(Item, Structure, Diagnostics, false))
      Value = makeFragment<Condition>(Body, {Payload->begin, Payload->end}, SM,
                                      Diagnostics, "self condition");
    return Clause(SelfConditionClause{std::move(Value), Item.range});
  }

  if (Word == "tile") {
    auto Payload = getPayload(Item, Structure, Diagnostics, true);
    if (!Payload)
      return std::nullopt;
    std::vector<Slice> Parts = splitTopLevel(
        Body, {Payload->begin, Payload->end}, ',', Lang, SM, Diagnostics);
    std::vector<SizeExpr> Sizes;
    for (Slice Part : Parts) {
      Part = trimSlice(Body, Part);
      if (sliceText(Body, Part) == "*") {
        Sizes.push_back(StarSize{bodyRange(Body, SM, Part.begin, Part.end)});
        continue;
      }
      auto Value =
          makeFragment<IntegerExpr>(Body, Part, SM, Diagnostics, "tile size");
      if (Value)
        Sizes.push_back(std::move(*Value));
    }
    auto List = NonEmptyList<SizeExpr>::create(std::move(Sizes));
    if (!List)
      return std::nullopt;
    return Clause(TileClause{std::move(*List), Item.range});
  }

  if (auto VarKind = getVarListKind(Word, Directive)) {
    auto Payload = getPayload(Item, Structure, Diagnostics, true);
    if (!Payload)
      return std::nullopt;
    auto Variables =
        parseFragmentList<VariableRef>(Body, {Payload->begin, Payload->end},
                                       Lang, SM, Diagnostics, "variable");
    if (!Variables)
      return std::nullopt;
    return Clause(VarListClause{*VarKind, std::move(*Variables), Item.range});
  }

  if (Word == "vector" || Word == "worker") {
    std::optional<PayloadRecord> Payload =
        getPayload(Item, Structure, Diagnostics, false);
    if (!Payload) {
      if (Word == "vector")
        return Clause(VectorClause{std::nullopt, Item.range});
      return Clause(WorkerClause{std::nullopt, Item.range});
    }
    Slice ValueSlice = trimSlice(Body, {Payload->begin, Payload->end});
    bool HasKeyword = false;
    std::string_view Keyword = Word == "vector" ? "length" : "num";
    if (auto Colon = keyColon(Body, ValueSlice, Keyword, Lang)) {
      HasKeyword = true;
      ValueSlice = trimSlice(Body, {*Colon + 1, ValueSlice.end});
    }
    auto Value = makeFragment<IntegerExpr>(Body, ValueSlice, SM, Diagnostics,
                                           Word == "vector" ? "vector length"
                                                            : "worker count");
    if (!Value)
      return std::nullopt;
    if (Word == "vector")
      return Clause(VectorClause{VectorArgument{HasKeyword, std::move(*Value)},
                                 Item.range});
    return Clause(WorkerClause{WorkerArgument{HasKeyword, std::move(*Value)},
                               Item.range});
  }

  if (Word == "vector_length") {
    auto Payload = getPayload(Item, Structure, Diagnostics, true);
    if (!Payload)
      return std::nullopt;
    auto Value =
        makeFragment<IntegerExpr>(Body, {Payload->begin, Payload->end}, SM,
                                  Diagnostics, "vector_length expression");
    if (!Value)
      return std::nullopt;
    return Clause(VectorLengthClause{std::move(*Value), Item.range});
  }

  if (Word == "wait") {
    std::optional<WaitArgument> Argument;
    if (auto Payload = getPayload(Item, Structure, Diagnostics, false))
      Argument = parseWaitArgument(Body, *Payload, Lang, SM, Diagnostics);
    return Clause(WaitClause{std::move(Argument), Item.range});
  }

  // Names beginning with two underscores are reserved for implementation
  // clauses. An implementation clause that is not otherwise recognized has no
  // effect on the portable OpenACC directive.
  if (Word.rfind("__", 0) == 0)
    return std::nullopt;

  addDiagnostic(Diagnostics, DiagnosticCode::UnknownClause,
                "unknown OpenACC clause '" + Word + "'", Item.wordRange);
  return std::nullopt;
}

bool usesDeviceGroups(DirectiveKind Kind) {
  switch (Kind) {
  case DirectiveKind::Data:
  case DirectiveKind::Kernels:
  case DirectiveKind::KernelsLoop:
  case DirectiveKind::Loop:
  case DirectiveKind::Parallel:
  case DirectiveKind::ParallelLoop:
  case DirectiveKind::Routine:
  case DirectiveKind::Serial:
  case DirectiveKind::SerialLoop:
  case DirectiveKind::Update:
    return true;
  default:
    return false;
  }
}

std::optional<DirectiveKind>
parseDirectiveHead(const StructuralInput &Structure, std::size_t &ClauseBegin,
                   std::vector<Diagnostic> &Diagnostics) {
  const std::vector<Occurrence> &Items = Structure.occurrences;
  if (Items.empty())
    return std::nullopt;
  const std::string &First = Items.front().word;
  ClauseBegin = 1;
  if (First == "atomic")
    return DirectiveKind::Atomic;
  if (First == "cache")
    return DirectiveKind::Cache;
  if (First == "data")
    return DirectiveKind::Data;
  if (First == "declare")
    return DirectiveKind::Declare;
  if (First == "end")
    return DirectiveKind::End;
  if (First == "host_data")
    return DirectiveKind::HostData;
  if (First == "init")
    return DirectiveKind::Init;
  if (First == "loop")
    return DirectiveKind::Loop;
  if (First == "routine")
    return DirectiveKind::Routine;
  if (First == "set")
    return DirectiveKind::Set;
  if (First == "shutdown")
    return DirectiveKind::Shutdown;
  if (First == "update")
    return DirectiveKind::Update;
  if (First == "wait")
    return DirectiveKind::Wait;
  if (First == "enter" || First == "exit") {
    if (Items.size() < 2 || Items[1].word != "data" || Items[1].payloadIndex) {
      addDiagnostic(Diagnostics, DiagnosticCode::UnknownDirective,
                    "expected 'data' after '" + First + "'",
                    Items.front().range);
      return std::nullopt;
    }
    ClauseBegin = 2;
    return First == "enter" ? DirectiveKind::EnterData
                            : DirectiveKind::ExitData;
  }
  if (First == "kernels" || First == "parallel" || First == "serial") {
    bool Combined =
        Items.size() > 1 && Items[1].word == "loop" && !Items[1].payloadIndex;
    ClauseBegin = Combined ? 2 : 1;
    if (First == "kernels")
      return Combined ? DirectiveKind::KernelsLoop : DirectiveKind::Kernels;
    if (First == "parallel")
      return Combined ? DirectiveKind::ParallelLoop : DirectiveKind::Parallel;
    return Combined ? DirectiveKind::SerialLoop : DirectiveKind::Serial;
  }
  addDiagnostic(Diagnostics, DiagnosticCode::UnknownDirective,
                "unknown OpenACC directive '" + First + "'",
                Items.front().wordRange);
  return std::nullopt;
}

std::optional<EndDirectiveKind>
parseEndKind(const StructuralInput &Structure,
             std::vector<Diagnostic> &Diagnostics) {
  const auto &Items = Structure.occurrences;
  if (Items.size() < 2) {
    addDiagnostic(Diagnostics, DiagnosticCode::MissingArgument,
                  "the end directive requires a construct name",
                  Items.front().range);
    return std::nullopt;
  }
  if (Items[1].payloadIndex) {
    addDiagnostic(Diagnostics, DiagnosticCode::UnexpectedArgument,
                  "an end marker cannot have an argument", Items[1].range);
    return std::nullopt;
  }
  bool Combined =
      Items.size() > 2 && Items[2].word == "loop" && !Items[2].payloadIndex;
  std::size_t Consumed = Combined ? 3 : 2;
  if (Items.size() != Consumed) {
    addDiagnostic(Diagnostics, DiagnosticCode::UnexpectedClause,
                  "Fortran end markers cannot contain clauses",
                  Items[Consumed].range);
    return std::nullopt;
  }
  const std::string &Kind = Items[1].word;
  if (Kind == "atomic" && !Combined)
    return EndDirectiveKind::Atomic;
  if (Kind == "data" && !Combined)
    return EndDirectiveKind::Data;
  if (Kind == "host_data" && !Combined)
    return EndDirectiveKind::HostData;
  if (Kind == "kernels")
    return Combined ? EndDirectiveKind::KernelsLoop : EndDirectiveKind::Kernels;
  // Preserve the legacy parser's standalone Fortran loop terminator as a
  // compatibility extension.
  if (Kind == "loop" && !Combined)
    return EndDirectiveKind::Loop;
  if (Kind == "parallel")
    return Combined ? EndDirectiveKind::ParallelLoop
                    : EndDirectiveKind::Parallel;
  if (Kind == "serial")
    return Combined ? EndDirectiveKind::SerialLoop : EndDirectiveKind::Serial;
  addDiagnostic(Diagnostics, DiagnosticCode::InvalidValue,
                "unsupported OpenACC end marker", Items[1].wordRange);
  return std::nullopt;
}

std::optional<GeneralDirective>
buildGeneralDirective(DirectiveKind Kind, std::size_t ClauseBegin,
                      const StructuralInput &Structure, const BodyText &Body,
                      Language Lang, const SourceManager &SM,
                      std::vector<Diagnostic> &Diagnostics) {
  GeneralDirective Result{Kind, std::nullopt, std::nullopt, {}, {}};
  const std::vector<Occurrence> &Items = Structure.occurrences;

  if (Kind == DirectiveKind::Routine && Items.front().payloadIndex) {
    const PayloadRecord &Payload =
        Structure.payloads[*Items.front().payloadIndex];
    Slice NameSlice = trimSlice(Body, {Payload.begin, Payload.end});
    std::string Name = sliceText(Body, NameSlice);
    if (!isRoutineName(Name, Lang)) {
      addDiagnostic(Diagnostics, DiagnosticCode::InvalidValue,
                    "routine expects a base-language routine name",
                    Payload.range);
    } else {
      Result.routineName = RoutineName::create(
          std::move(Name), bodyRange(Body, SM, NameSlice.begin, NameSlice.end));
    }
  } else if (Kind != DirectiveKind::Wait && Items.front().payloadIndex) {
    addDiagnostic(Diagnostics, DiagnosticCode::UnexpectedArgument,
                  "the directive name does not take an argument",
                  Items.front().range);
  }

  if (Kind == DirectiveKind::Wait && Items.front().payloadIndex) {
    const PayloadRecord &Payload =
        Structure.payloads[*Items.front().payloadIndex];
    Result.waitArgument =
        parseWaitArgument(Body, Payload, Lang, SM, Diagnostics);
  }

  DeviceClauseGroup *CurrentGroup = nullptr;
  for (std::size_t I = ClauseBegin; I < Items.size(); ++I) {
    std::optional<Clause> Parsed =
        parseClause(Items[I], Kind, Structure, Body, Lang, SM, Diagnostics);
    if (!Parsed)
      continue;
    if (usesDeviceGroups(Kind) &&
        getClauseKind(*Parsed) == ClauseKind::DeviceType) {
      DeviceTypeClause Marker = std::get<DeviceTypeClause>(std::move(*Parsed));
      Result.deviceGroups.push_back(
          {std::move(Marker.selectors), Marker.range, {}});
      CurrentGroup = &Result.deviceGroups.back();
      continue;
    }
    if (CurrentGroup)
      CurrentGroup->clauses.push_back(std::move(*Parsed));
    else
      Result.defaultClauses.push_back(std::move(*Parsed));
  }
  return Result;
}

bool contains(const std::set<ClauseKind> &Values, ClauseKind Value) {
  return Values.find(Value) != Values.end();
}

std::set<ClauseKind> allowedClauses(DirectiveKind Kind) {
  using CK = ClauseKind;
  switch (Kind) {
  case DirectiveKind::Atomic:
    return {CK::Read, CK::Write, CK::Update, CK::Capture, CK::If};
  case DirectiveKind::Data:
    return {CK::If,      CK::Async,     CK::Wait,   CK::Copy,
            CK::CopyIn,  CK::CopyOut,   CK::Create, CK::NoCreate,
            CK::Present, CK::DevicePtr, CK::Attach, CK::Default};
  case DirectiveKind::Declare:
    return {CK::Copy,    CK::CopyIn,    CK::CopyOut,        CK::Create,
            CK::Present, CK::DevicePtr, CK::DeviceResident, CK::Link};
  case DirectiveKind::EnterData:
    return {CK::If, CK::Async, CK::Wait, CK::CopyIn, CK::Create, CK::Attach};
  case DirectiveKind::ExitData:
    return {CK::If,     CK::Async,  CK::Wait,    CK::CopyOut,
            CK::Delete, CK::Detach, CK::Finalize};
  case DirectiveKind::HostData:
    return {CK::UseDevice, CK::If, CK::IfPresent};
  case DirectiveKind::Init:
  case DirectiveKind::Shutdown:
    return {CK::DeviceType, CK::DeviceNum, CK::If};
  case DirectiveKind::Kernels:
    return {CK::Async,        CK::Wait,      CK::NumGangs, CK::NumWorkers,
            CK::VectorLength, CK::If,        CK::Self,     CK::Copy,
            CK::CopyIn,       CK::CopyOut,   CK::Create,   CK::NoCreate,
            CK::Present,      CK::DevicePtr, CK::Attach,   CK::Default};
  case DirectiveKind::Loop:
    return {CK::Collapse,    CK::Gang, CK::Worker, CK::Vector,  CK::Seq,
            CK::Independent, CK::Auto, CK::Tile,   CK::Private, CK::Reduction};
  case DirectiveKind::Parallel:
    return {CK::Async,        CK::Wait,         CK::NumGangs,  CK::NumWorkers,
            CK::VectorLength, CK::If,           CK::Self,      CK::Reduction,
            CK::Copy,         CK::CopyIn,       CK::CopyOut,   CK::Create,
            CK::NoCreate,     CK::Present,      CK::DevicePtr, CK::Attach,
            CK::Private,      CK::FirstPrivate, CK::Default};
  case DirectiveKind::Routine:
    return {CK::Gang, CK::Worker, CK::Vector, CK::Seq, CK::Bind, CK::NoHost};
  case DirectiveKind::Serial:
    return {CK::Async,     CK::Wait,     CK::If,           CK::Self,
            CK::Reduction, CK::Copy,     CK::CopyIn,       CK::CopyOut,
            CK::Create,    CK::NoCreate, CK::Present,      CK::DevicePtr,
            CK::Attach,    CK::Private,  CK::FirstPrivate, CK::Default};
  case DirectiveKind::Set:
    return {CK::DefaultAsync, CK::DeviceNum, CK::DeviceType, CK::If};
  case DirectiveKind::Update:
    return {CK::Async, CK::Wait, CK::If,    CK::IfPresent,
            CK::Self,  CK::Host, CK::Device};
  case DirectiveKind::Wait:
    return {CK::Async, CK::If};
  case DirectiveKind::ParallelLoop: {
    auto Result = allowedClauses(DirectiveKind::Parallel);
    auto Loop = allowedClauses(DirectiveKind::Loop);
    Result.insert(Loop.begin(), Loop.end());
    return Result;
  }
  case DirectiveKind::SerialLoop: {
    auto Result = allowedClauses(DirectiveKind::Serial);
    auto Loop = allowedClauses(DirectiveKind::Loop);
    Result.insert(Loop.begin(), Loop.end());
    return Result;
  }
  case DirectiveKind::KernelsLoop: {
    auto Result = allowedClauses(DirectiveKind::Kernels);
    auto Loop = allowedClauses(DirectiveKind::Loop);
    Result.insert(Loop.begin(), Loop.end());
    return Result;
  }
  case DirectiveKind::Cache:
  case DirectiveKind::End:
    return {};
  }
  return {};
}

std::set<ClauseKind> allowedDeviceSpecificClauses(DirectiveKind Kind) {
  using CK = ClauseKind;
  switch (Kind) {
  case DirectiveKind::Parallel:
  case DirectiveKind::Kernels:
    return {CK::Async, CK::Wait, CK::NumGangs, CK::NumWorkers,
            CK::VectorLength};
  case DirectiveKind::Serial:
  case DirectiveKind::Data:
    return {CK::Async, CK::Wait};
  case DirectiveKind::Loop:
    return {CK::Collapse, CK::Gang,        CK::Worker, CK::Vector,
            CK::Seq,      CK::Independent, CK::Auto,   CK::Tile};
  case DirectiveKind::ParallelLoop:
  case DirectiveKind::KernelsLoop:
    return {CK::Async,        CK::Wait,     CK::NumGangs,    CK::NumWorkers,
            CK::VectorLength, CK::Collapse, CK::Gang,        CK::Worker,
            CK::Vector,       CK::Seq,      CK::Independent, CK::Auto,
            CK::Tile};
  case DirectiveKind::SerialLoop:
    return {CK::Async,  CK::Wait, CK::Collapse,    CK::Gang, CK::Worker,
            CK::Vector, CK::Seq,  CK::Independent, CK::Auto, CK::Tile};
  case DirectiveKind::Routine:
    return {CK::Gang, CK::Worker, CK::Vector, CK::Seq, CK::Bind};
  case DirectiveKind::Update:
    return {CK::Async, CK::Wait};
  default:
    return {};
  }
}

std::size_t countClause(const std::vector<Clause> &Clauses, ClauseKind Kind) {
  return static_cast<std::size_t>(std::count_if(
      Clauses.begin(), Clauses.end(),
      [Kind](const Clause &Value) { return getClauseKind(Value) == Kind; }));
}

bool hasAnyClause(const std::vector<Clause> &Clauses,
                  const std::set<ClauseKind> &Kinds) {
  return std::any_of(Clauses.begin(), Clauses.end(), [&](const Clause &Value) {
    return contains(Kinds, getClauseKind(Value));
  });
}

void validateAllowedClauses(const GeneralDirective &Directive,
                            std::vector<Diagnostic> &Diagnostics) {
  std::set<ClauseKind> Allowed = allowedClauses(Directive.kind);
  for (const Clause &Value : Directive.defaultClauses) {
    if (!contains(Allowed, getClauseKind(Value)))
      addDiagnostic(Diagnostics, DiagnosticCode::UnexpectedClause,
                    "clause is not permitted on this OpenACC directive",
                    getClauseRange(Value));
  }
  std::set<ClauseKind> DeviceAllowed =
      allowedDeviceSpecificClauses(Directive.kind);
  for (const DeviceClauseGroup &Group : Directive.deviceGroups) {
    for (const Clause &Value : Group.clauses) {
      if (!contains(Allowed, getClauseKind(Value)))
        addDiagnostic(Diagnostics, DiagnosticCode::UnexpectedClause,
                      "clause is not permitted on this OpenACC directive",
                      getClauseRange(Value));
      else if (!contains(DeviceAllowed, getClauseKind(Value)))
        addDiagnostic(Diagnostics, DiagnosticCode::InvalidClauseOrder,
                      "clause may not follow device_type on this directive",
                      getClauseRange(Value));
    }
  }
}

std::vector<const Clause *> allClauses(const GeneralDirective &Directive) {
  std::vector<const Clause *> Result;
  std::size_t Size = Directive.defaultClauses.size();
  for (const DeviceClauseGroup &Group : Directive.deviceGroups)
    Size += Group.clauses.size();
  Result.reserve(Size);
  for (const Clause &Value : Directive.defaultClauses)
    Result.push_back(&Value);
  for (const DeviceClauseGroup &Group : Directive.deviceGroups)
    for (const Clause &Value : Group.clauses)
      Result.push_back(&Value);
  return Result;
}

std::vector<Clause>
effectiveClauses(const std::vector<Clause> &Defaults,
                 const std::vector<Clause> &DeviceSpecificClauses) {
  std::set<ClauseKind> DeviceKinds;
  for (const Clause &Value : DeviceSpecificClauses)
    DeviceKinds.insert(getClauseKind(Value));

  std::vector<Clause> Result;
  for (const Clause &Value : Defaults)
    if (!contains(DeviceKinds, getClauseKind(Value)))
      Result.push_back(Value);
  Result.insert(Result.end(), DeviceSpecificClauses.begin(),
                DeviceSpecificClauses.end());
  return Result;
}

SourceRange deviceSelectorRange(const DeviceSelector &Selector) {
  if (const auto *Wildcard = std::get_if<WildcardDevice>(&Selector))
    return Wildcard->range;
  return std::get<ArchitectureIdentifier>(Selector).range();
}

std::string deviceSelectorKey(const DeviceSelector &Selector, Language Lang) {
  if (std::holds_alternative<WildcardDevice>(Selector))
    return "*";
  std::string Key = std::get<ArchitectureIdentifier>(Selector).spelling();
  return Lang == Language::Fortran ? lower(Key) : Key;
}

bool groupContainsSelector(const DeviceClauseGroup &Group, std::string_view Key,
                           Language Lang) {
  return std::any_of(Group.selectors.values().begin(),
                     Group.selectors.values().end(),
                     [&](const DeviceSelector &Selector) {
                       return deviceSelectorKey(Selector, Lang) == Key;
                     });
}

struct EffectiveDeviceClauseSet {
  SourceRange selectorRange;
  std::vector<Clause> clauses;
};

std::vector<EffectiveDeviceClauseSet>
effectiveDeviceClauseSets(const GeneralDirective &Directive, Language Lang) {
  struct SelectorReference {
    std::string key;
    SourceRange range;
  };

  std::vector<SelectorReference> Selectors;
  std::set<std::string> SeenSelectors;
  for (const DeviceClauseGroup &Group : Directive.deviceGroups) {
    for (const DeviceSelector &Selector : Group.selectors.values()) {
      std::string Key = deviceSelectorKey(Selector, Lang);
      if (SeenSelectors.insert(Key).second)
        Selectors.push_back({std::move(Key), deviceSelectorRange(Selector)});
    }
  }

  std::vector<std::vector<std::size_t>> SeenGroupSets;
  std::vector<EffectiveDeviceClauseSet> Result;
  Result.reserve(Selectors.size());
  for (const SelectorReference &Selector : Selectors) {
    std::vector<std::size_t> GroupSet;
    std::vector<Clause> DeviceSpecificClauses;
    for (std::size_t I = 0; I < Directive.deviceGroups.size(); ++I) {
      const DeviceClauseGroup &Group = Directive.deviceGroups[I];
      if (!groupContainsSelector(Group, Selector.key, Lang))
        continue;
      GroupSet.push_back(I);
      DeviceSpecificClauses.insert(DeviceSpecificClauses.end(),
                                   Group.clauses.begin(), Group.clauses.end());
    }
    if (std::find(SeenGroupSets.begin(), SeenGroupSets.end(), GroupSet) !=
        SeenGroupSets.end())
      continue;
    SeenGroupSets.push_back(std::move(GroupSet));
    Result.push_back({Selector.range, effectiveClauses(Directive.defaultClauses,
                                                       DeviceSpecificClauses)});
  }
  return Result;
}

bool isStructuredDataOrCompute(DirectiveKind Kind) {
  switch (Kind) {
  case DirectiveKind::Data:
  case DirectiveKind::Kernels:
  case DirectiveKind::KernelsLoop:
  case DirectiveKind::Parallel:
  case DirectiveKind::ParallelLoop:
  case DirectiveKind::Serial:
  case DirectiveKind::SerialLoop:
    return true;
  default:
    return false;
  }
}

void validateDataModifierContexts(const GeneralDirective &Directive,
                                  std::vector<Diagnostic> &Diagnostics) {
  bool Structured = isStructuredDataOrCompute(Directive.kind);
  for (const Clause *Value : allClauses(Directive)) {
    if (const auto *Copy = std::get_if<CopyClause>(Value)) {
      if (!Structured &&
          std::find(Copy->modifiers.begin(), Copy->modifiers.end(),
                    CopyModifier::Capture) != Copy->modifiers.end())
        addDiagnostic(Diagnostics, DiagnosticCode::InvalidModifier,
                      "capture on copy is limited to structured data and "
                      "compute constructs",
                      Copy->range);
    } else if (const auto *CopyIn = std::get_if<CopyInClause>(Value)) {
      if (!Structured &&
          std::find(CopyIn->modifiers.begin(), CopyIn->modifiers.end(),
                    CopyInModifier::Capture) != CopyIn->modifiers.end())
        addDiagnostic(Diagnostics, DiagnosticCode::InvalidModifier,
                      "capture on copyin is limited to structured data and "
                      "compute constructs",
                      CopyIn->range);
    } else if (const auto *CopyOut = std::get_if<CopyOutClause>(Value)) {
      bool Capture =
          std::find(CopyOut->modifiers.begin(), CopyOut->modifiers.end(),
                    CopyOutModifier::Capture) != CopyOut->modifiers.end();
      bool Zero =
          std::find(CopyOut->modifiers.begin(), CopyOut->modifiers.end(),
                    CopyOutModifier::Zero) != CopyOut->modifiers.end();
      if (!Structured && (Capture || Zero))
        addDiagnostic(Diagnostics, DiagnosticCode::InvalidModifier,
                      "capture and zero on copyout are limited to structured "
                      "data and compute constructs",
                      CopyOut->range);
    } else if (const auto *Create = std::get_if<CreateClause>(Value)) {
      if (!Structured &&
          std::find(Create->modifiers.begin(), Create->modifiers.end(),
                    CreateModifier::Capture) != Create->modifiers.end())
        addDiagnostic(Diagnostics, DiagnosticCode::InvalidModifier,
                      "capture on create is limited to structured data and "
                      "compute constructs",
                      Create->range);
    }
  }
}

void validateSingleton(const GeneralDirective &Directive, ClauseKind Kind,
                       std::string_view Name,
                       std::vector<Diagnostic> &Diagnostics) {
  const Clause *First = nullptr;
  for (const Clause *Value : allClauses(Directive)) {
    if (getClauseKind(*Value) != Kind)
      continue;
    if (!First) {
      First = Value;
      continue;
    }
    addDiagnostic(Diagnostics, DiagnosticCode::DuplicateClause,
                  "at most one '" + std::string(Name) + "' clause may appear",
                  getClauseRange(*Value));
  }
}

void validateLoopSequence(const std::vector<Clause> &Clauses,
                          std::vector<Diagnostic> &Diagnostics) {
  std::size_t Seq = countClause(Clauses, ClauseKind::Seq);
  std::size_t Independent = countClause(Clauses, ClauseKind::Independent);
  std::size_t Auto = countClause(Clauses, ClauseKind::Auto);
  if (Seq + Independent + Auto > 1) {
    for (const Clause &Value : Clauses)
      if (getClauseKind(Value) == ClauseKind::Seq ||
          getClauseKind(Value) == ClauseKind::Independent ||
          getClauseKind(Value) == ClauseKind::Auto) {
        addDiagnostic(Diagnostics, DiagnosticCode::InvalidCombination,
                      "only one of seq, independent, and auto may appear",
                      getClauseRange(Value));
        break;
      }
  }
  if (Seq > 0 && (countClause(Clauses, ClauseKind::Gang) > 0 ||
                  countClause(Clauses, ClauseKind::Worker) > 0 ||
                  countClause(Clauses, ClauseKind::Vector) > 0)) {
    for (const Clause &Value : Clauses)
      if (getClauseKind(Value) == ClauseKind::Seq) {
        addDiagnostic(Diagnostics, DiagnosticCode::InvalidCombination,
                      "seq cannot be combined with gang, worker, or vector",
                      getClauseRange(Value));
        break;
      }
  }
  if (countClause(Clauses, ClauseKind::Gang) > 1)
    for (const Clause &Value : Clauses)
      if (getClauseKind(Value) == ClauseKind::Gang) {
        addDiagnostic(Diagnostics, DiagnosticCode::DuplicateClause,
                      "at most one gang clause may appear on a loop",
                      getClauseRange(Value));
        break;
      }

  for (const Clause &Value : Clauses) {
    const auto *Gang = std::get_if<GangClause>(&Value);
    if (!Gang)
      continue;
    std::size_t Positional = 0;
    std::size_t Num = 0;
    std::size_t Dim = 0;
    std::size_t Static = 0;
    for (const GangArgument &Argument : Gang->arguments) {
      Positional += std::holds_alternative<PositionalGangArgument>(Argument);
      Num += std::holds_alternative<NumGangArgument>(Argument);
      Dim += std::holds_alternative<DimGangArgument>(Argument);
      Static += std::holds_alternative<StaticGangArgument>(Argument);
    }
    if (Positional + Num > 1 || Dim > 1 || Static > 1)
      addDiagnostic(Diagnostics, DiagnosticCode::DuplicateClause,
                    "gang may have at most one num, dim, and static argument",
                    Gang->range);
  }
}

using DiagnosticKey = std::pair<DiagnosticCode, SourceRange>;

void validateLoopParentRestrictions(
    DirectiveKind Kind, const std::vector<Clause> &Clauses,
    std::vector<DiagnosticKey> &ReportedDiagnostics,
    std::vector<Diagnostic> &Diagnostics) {
  bool HasParallelParent = Kind == DirectiveKind::ParallelLoop;
  bool HasSerialParent = Kind == DirectiveKind::SerialLoop;
  bool HasKernelsParent = Kind == DirectiveKind::KernelsLoop;

  // A standalone loop's lexical parent is not available to this
  // per-directive API. Combined constructs encode that parent in their kind.
  if (!HasParallelParent && !HasSerialParent && !HasKernelsParent)
    return;

  auto Report = [&](DiagnosticCode Code, std::string Message,
                    SourceRange Range) {
    DiagnosticKey Key{Code, Range};
    if (std::find(ReportedDiagnostics.begin(), ReportedDiagnostics.end(),
                  Key) != ReportedDiagnostics.end())
      return;
    addDiagnostic(Diagnostics, Code, std::move(Message), Range);
    ReportedDiagnostics.push_back(Key);
  };

  bool HasNumGangs = countClause(Clauses, ClauseKind::NumGangs) != 0;
  bool HasNumWorkers = countClause(Clauses, ClauseKind::NumWorkers) != 0;
  bool HasVectorLength = countClause(Clauses, ClauseKind::VectorLength) != 0;
  std::string ParentName = HasSerialParent ? "serial" : "parallel";

  for (const Clause &Value : Clauses) {
    if (const auto *Gang = std::get_if<GangClause>(&Value)) {
      for (const GangArgument &Argument : Gang->arguments) {
        const IntegerExpr *Num = nullptr;
        if (const auto *Positional =
                std::get_if<PositionalGangArgument>(&Argument))
          Num = &Positional->value;
        else if (const auto *Named = std::get_if<NumGangArgument>(&Argument))
          Num = &Named->value;

        if (Num && (HasParallelParent || HasSerialParent)) {
          Report(DiagnosticCode::UnexpectedArgument,
                 "gang num argument is not allowed when the parent compute "
                 "construct is " +
                     ParentName,
                 Num->range());
        } else if (Num && HasNumGangs) {
          Report(DiagnosticCode::InvalidCombination,
                 "gang num argument cannot be combined with num_gangs on "
                 "the parent kernels construct",
                 Num->range());
        }

        if (const auto *Dim = std::get_if<DimGangArgument>(&Argument))
          if (HasKernelsParent)
            Report(DiagnosticCode::UnexpectedArgument,
                   "gang dim argument is not allowed when the parent compute "
                   "construct is kernels",
                   Dim->value.range());
      }
      continue;
    }

    if (const auto *Worker = std::get_if<WorkerClause>(&Value)) {
      if (!Worker->argument)
        continue;
      if (HasParallelParent || HasSerialParent)
        Report(DiagnosticCode::UnexpectedArgument,
               "worker argument is not allowed when the parent compute "
               "construct is " +
                   ParentName,
               Worker->argument->value.range());
      else if (HasNumWorkers)
        Report(DiagnosticCode::InvalidCombination,
               "worker argument cannot be combined with num_workers on the "
               "parent kernels construct",
               Worker->argument->value.range());
      continue;
    }

    if (const auto *Vector = std::get_if<VectorClause>(&Value)) {
      if (!Vector->argument)
        continue;
      if (HasParallelParent || HasSerialParent)
        Report(DiagnosticCode::UnexpectedArgument,
               "vector argument is not allowed when the parent compute "
               "construct is " +
                   ParentName,
               Vector->argument->value.range());
      else if (HasVectorLength)
        Report(DiagnosticCode::InvalidCombination,
               "vector argument cannot be combined with vector_length on "
               "the parent kernels construct",
               Vector->argument->value.range());
    }
  }
}

void validateNumGangs(
    const GeneralDirective &Directive,
    const std::vector<EffectiveDeviceClauseSet> &EffectiveDeviceSets,
    std::vector<Diagnostic> &Diagnostics) {
  const std::vector<const Clause *> Clauses = allClauses(Directive);
  bool HasParallelComponent = Directive.kind == DirectiveKind::Parallel ||
                              Directive.kind == DirectiveKind::ParallelLoop;
  bool IsKernels = Directive.kind == DirectiveKind::Kernels ||
                   Directive.kind == DirectiveKind::KernelsLoop;
  for (const Clause *Value : Clauses) {
    const auto *NumGangs = std::get_if<NumGangsClause>(Value);
    if (!NumGangs)
      continue;
    std::size_t Size = NumGangs->values.size();
    if ((HasParallelComponent && Size > 3) || (IsKernels && Size != 1))
      addDiagnostic(Diagnostics, DiagnosticCode::InvalidValue,
                    HasParallelComponent
                        ? "parallel num_gangs expects one to three arguments"
                        : "kernels num_gangs expects exactly one argument",
                    NumGangs->range);
  }
  if (!HasParallelComponent)
    return;

  std::vector<SourceRange> ReportedRanges;
  auto ValidateReduction = [&](const std::vector<Clause> &EffectiveClauses) {
    bool HasReduction =
        countClause(EffectiveClauses, ClauseKind::Reduction) != 0;
    bool NeedsGang = Directive.kind == DirectiveKind::ParallelLoop;
    bool HasGang = countClause(EffectiveClauses, ClauseKind::Gang) != 0;
    if (!HasReduction || (NeedsGang && !HasGang))
      return;
    for (const Clause &Value : EffectiveClauses) {
      const auto *NumGangs = std::get_if<NumGangsClause>(&Value);
      if (NumGangs && NumGangs->values.size() > 1 &&
          std::find(ReportedRanges.begin(), ReportedRanges.end(),
                    NumGangs->range) == ReportedRanges.end()) {
        addDiagnostic(
            Diagnostics, DiagnosticCode::InvalidCombination,
            "reduction cannot be combined with multi-dimensional num_gangs",
            NumGangs->range);
        ReportedRanges.push_back(NumGangs->range);
      }
    }
  };
  ValidateReduction(Directive.defaultClauses);
  for (const EffectiveDeviceClauseSet &Set : EffectiveDeviceSets)
    ValidateReduction(Set.clauses);
}

std::size_t parallelismCount(const std::vector<Clause> &Clauses) {
  return countClause(Clauses, ClauseKind::Gang) +
         countClause(Clauses, ClauseKind::Worker) +
         countClause(Clauses, ClauseKind::Vector) +
         countClause(Clauses, ClauseKind::Seq);
}

void validateRoutine(
    const GeneralDirective &Directive,
    const std::vector<EffectiveDeviceClauseSet> &EffectiveDeviceSets,
    std::vector<Diagnostic> &Diagnostics) {
  auto ValidateRoutineArguments = [&](const std::vector<Clause> &Clauses) {
    for (const Clause &Value : Clauses) {
      if (const auto *Gang = std::get_if<GangClause>(&Value)) {
        for (const GangArgument &Argument : Gang->arguments)
          if (!std::holds_alternative<DimGangArgument>(Argument))
            addDiagnostic(Diagnostics, DiagnosticCode::InvalidValue,
                          "routine gang accepts only a dim argument",
                          Gang->range);
      } else if (const auto *Vector = std::get_if<VectorClause>(&Value)) {
        if (Vector->argument)
          addDiagnostic(Diagnostics, DiagnosticCode::UnexpectedArgument,
                        "routine vector does not take an argument",
                        Vector->range);
      } else if (const auto *Worker = std::get_if<WorkerClause>(&Value)) {
        if (Worker->argument)
          addDiagnostic(Diagnostics, DiagnosticCode::UnexpectedArgument,
                        "routine worker does not take an argument",
                        Worker->range);
      }
    }
  };
  ValidateRoutineArguments(Directive.defaultClauses);
  for (const DeviceClauseGroup &Group : Directive.deviceGroups)
    ValidateRoutineArguments(Group.clauses);

  // OpenACC 3.4 permits an embedding compiler to infer routine parallelism
  // from surrounding source context. This directive-only parser can reject
  // conflicting explicit clauses, but must preserve the zero-explicit-clause
  // case for contextual analysis.
  if (Directive.deviceGroups.empty()) {
    if (parallelismCount(Directive.defaultClauses) > 1)
      addDiagnostic(
          Diagnostics, DiagnosticCode::InvalidCombination,
          "routine accepts at most one of gang, worker, vector, or seq",
          getClauseRange(Directive.defaultClauses.front()));
  }
  for (const EffectiveDeviceClauseSet &Set : EffectiveDeviceSets) {
    if (parallelismCount(Set.clauses) > 1)
      addDiagnostic(Diagnostics, DiagnosticCode::InvalidCombination,
                    "each effective routine device group accepts at most one "
                    "explicit parallelism clause",
                    Set.selectorRange);
  }
}

void validateGeneralDirective(const GeneralDirective &Directive,
                              SourceRange DirectiveRange, Language Lang,
                              std::vector<Diagnostic> &Diagnostics) {
  std::vector<EffectiveDeviceClauseSet> EffectiveDeviceSets =
      effectiveDeviceClauseSets(Directive, Lang);
  validateAllowedClauses(Directive, Diagnostics);
  validateSingleton(Directive, ClauseKind::If, "if", Diagnostics);
  validateSingleton(Directive, ClauseKind::Default, "default", Diagnostics);
  validateDataModifierContexts(Directive, Diagnostics);
  validateNumGangs(Directive, EffectiveDeviceSets, Diagnostics);

  if (Directive.kind == DirectiveKind::Atomic) {
    std::size_t AtomicClauses = 0;
    for (ClauseKind Kind : {ClauseKind::Read, ClauseKind::Write,
                            ClauseKind::Update, ClauseKind::Capture})
      AtomicClauses += countClause(Directive.defaultClauses, Kind);
    if (AtomicClauses > 1)
      addDiagnostic(
          Diagnostics, DiagnosticCode::InvalidCombination,
          "atomic accepts at most one of read, write, update, and capture",
          DirectiveRange);
  }

  const std::set<ClauseKind> DataActions = {
      ClauseKind::Copy,      ClauseKind::CopyIn,   ClauseKind::CopyOut,
      ClauseKind::Create,    ClauseKind::NoCreate, ClauseKind::Present,
      ClauseKind::DevicePtr, ClauseKind::Attach,   ClauseKind::Default};
  if (Directive.kind == DirectiveKind::Data &&
      !hasAnyClause(Directive.defaultClauses, DataActions))
    addDiagnostic(Diagnostics, DiagnosticCode::MissingClause,
                  "data requires a data or default clause", DirectiveRange);
  if (Directive.kind == DirectiveKind::EnterData &&
      !hasAnyClause(
          Directive.defaultClauses,
          {ClauseKind::CopyIn, ClauseKind::Create, ClauseKind::Attach}))
    addDiagnostic(Diagnostics, DiagnosticCode::MissingClause,
                  "enter data requires copyin, create, or attach",
                  DirectiveRange);
  if (Directive.kind == DirectiveKind::ExitData &&
      !hasAnyClause(
          Directive.defaultClauses,
          {ClauseKind::CopyOut, ClauseKind::Delete, ClauseKind::Detach}))
    addDiagnostic(Diagnostics, DiagnosticCode::MissingClause,
                  "exit data requires copyout, delete, or detach",
                  DirectiveRange);
  if (Directive.kind == DirectiveKind::HostData &&
      countClause(Directive.defaultClauses, ClauseKind::UseDevice) == 0)
    addDiagnostic(Diagnostics, DiagnosticCode::MissingClause,
                  "host_data requires a use_device clause", DirectiveRange);
  if (Directive.kind == DirectiveKind::Declare &&
      Directive.defaultClauses.empty())
    addDiagnostic(Diagnostics, DiagnosticCode::MissingClause,
                  "declare requires at least one clause", DirectiveRange);
  if (Directive.kind == DirectiveKind::Update &&
      !hasAnyClause(Directive.defaultClauses,
                    {ClauseKind::Self, ClauseKind::Host, ClauseKind::Device}))
    addDiagnostic(Diagnostics, DiagnosticCode::MissingClause,
                  "update requires self, host, or device", DirectiveRange);
  if (Directive.kind == DirectiveKind::Set) {
    if (!hasAnyClause(Directive.defaultClauses,
                      {ClauseKind::DefaultAsync, ClauseKind::DeviceNum,
                       ClauseKind::DeviceType}))
      addDiagnostic(Diagnostics, DiagnosticCode::MissingClause,
                    "set requires default_async, device_num, or device_type",
                    DirectiveRange);
    for (ClauseKind Kind : {ClauseKind::DefaultAsync, ClauseKind::DeviceNum,
                            ClauseKind::DeviceType})
      if (countClause(Directive.defaultClauses, Kind) > 1)
        for (const Clause &Value : Directive.defaultClauses)
          if (getClauseKind(Value) == Kind) {
            addDiagnostic(Diagnostics, DiagnosticCode::DuplicateClause,
                          "set clauses cannot be repeated",
                          getClauseRange(Value));
            break;
          }
  }

  bool HasLoopComponent = Directive.kind == DirectiveKind::Loop ||
                          Directive.kind == DirectiveKind::ParallelLoop ||
                          Directive.kind == DirectiveKind::SerialLoop ||
                          Directive.kind == DirectiveKind::KernelsLoop;
  if (HasLoopComponent) {
    std::vector<DiagnosticKey> ReportedParentDiagnostics;
    validateLoopSequence(Directive.defaultClauses, Diagnostics);
    validateLoopParentRestrictions(Directive.kind, Directive.defaultClauses,
                                   ReportedParentDiagnostics, Diagnostics);
    for (const EffectiveDeviceClauseSet &Set : EffectiveDeviceSets) {
      validateLoopSequence(Set.clauses, Diagnostics);
      validateLoopParentRestrictions(Directive.kind, Set.clauses,
                                     ReportedParentDiagnostics, Diagnostics);
    }
  }
  if (Directive.kind == DirectiveKind::Routine)
    validateRoutine(Directive, EffectiveDeviceSets, Diagnostics);
}

std::optional<CacheDirective>
buildCacheDirective(const StructuralInput &Structure, const BodyText &Body,
                    Language Lang, const SourceManager &SM,
                    std::vector<Diagnostic> &Diagnostics) {
  const Occurrence &Item = Structure.occurrences.front();
  auto Payload = getPayload(Item, Structure, Diagnostics, true);
  if (!Payload)
    return std::nullopt;
  Slice Variables{Payload->begin, Payload->end};
  bool ReadOnly = false;
  std::vector<std::size_t> Colons =
      findTopLevelSingleColons(Body, Variables, Lang);
  if (!Colons.empty()) {
    std::size_t Colon = Colons.front();
    Slice Prefix = trimSlice(Body, {Variables.begin, Colon});
    if (!equalsKeyword(sliceText(Body, Prefix), "readonly", Lang)) {
      addDiagnostic(Diagnostics, DiagnosticCode::InvalidModifier,
                    "cache supports only the readonly modifier",
                    bodyRange(Body, SM, Prefix.begin, Prefix.end));
      return std::nullopt;
    }
    ReadOnly = true;
    Variables = {Colon + 1, Variables.end};
  }
  auto ParsedVariables = parseFragmentList<VariableRef>(
      Body, Variables, Lang, SM, Diagnostics, "cache variable");
  if (!ParsedVariables)
    return std::nullopt;
  return CacheDirective{ReadOnly, std::move(*ParsedVariables)};
}

bool hasError(const std::vector<Diagnostic> &Diagnostics) {
  return std::any_of(Diagnostics.begin(), Diagnostics.end(),
                     [](const Diagnostic &Item) {
                       return Item.severity == DiagnosticSeverity::Error;
                     });
}

} // namespace

ParseResult parseDirective(std::string_view Input, ParseOptions Options) {
  ParseResult Result;
  SourceManager SM(Input);
  std::optional<BodyText> Body =
      scanEnvelope(Input, Options, SM, Result.diagnostics);
  if (!Body)
    return Result;
  std::optional<StructuralInput> Structure =
      scanStructure(*Body, Options.language, SM, Result.diagnostics);
  if (!Structure || hasError(Result.diagnostics))
    return Result;

  std::size_t ClauseBegin = 0;
  std::optional<DirectiveKind> Kind =
      parseDirectiveHead(*Structure, ClauseBegin, Result.diagnostics);
  if (!Kind || hasError(Result.diagnostics))
    return Result;
  if (!validateStructureWithANTLR(*Structure, *Body, SM, Result.diagnostics) ||
      hasError(Result.diagnostics))
    return Result;
  SourceRange DirectiveRange = SM.range(0, Input.size());

  if (*Kind == DirectiveKind::End) {
    if (Options.language != Language::Fortran) {
      addDiagnostic(Result.diagnostics, DiagnosticCode::InvalidLanguageForm,
                    "OpenACC end markers are Fortran-only", DirectiveRange);
      return Result;
    }
    auto EndKind = parseEndKind(*Structure, Result.diagnostics);
    if (!EndKind)
      return Result;
    Result.directive.emplace(Options.language, Options.inputForm,
                             DirectiveRange, EndDirective{*EndKind});
    return Result;
  }

  if (*Kind == DirectiveKind::Cache) {
    auto Cache = buildCacheDirective(*Structure, *Body, Options.language, SM,
                                     Result.diagnostics);
    if (!Cache || hasError(Result.diagnostics))
      return Result;
    Result.directive.emplace(Options.language, Options.inputForm,
                             DirectiveRange, std::move(*Cache));
    return Result;
  }

  auto General =
      buildGeneralDirective(*Kind, ClauseBegin, *Structure, *Body,
                            Options.language, SM, Result.diagnostics);
  if (!General)
    return Result;
  validateGeneralDirective(*General, DirectiveRange, Options.language,
                           Result.diagnostics);
  if (hasError(Result.diagnostics))
    return Result;
  Result.directive.emplace(Options.language, Options.inputForm, DirectiveRange,
                           std::move(*General));
  return Result;
}

} // namespace openacc

openacc::Directive *parseOpenACC(std::string Input) {
  std::size_t Begin = 0;
  while (Begin < Input.size() &&
         std::isspace(static_cast<unsigned char>(Input[Begin])))
    ++Begin;

  openacc::ParseOptions Options{openacc::Language::C,
                                openacc::InputForm::DirectiveBody};
  bool AmbiguousFortranSentinel = false;
  if (Begin < Input.size() && Input[Begin] == '#') {
    Options.inputForm = openacc::InputForm::CPragma;
  } else if (Input.substr(Begin, 7) == "_Pragma") {
    Options.inputForm = openacc::InputForm::CPragmaOperator;
  } else if (Begin + 5 <= Input.size() && Input[Begin + 1] == '$' &&
             (Input[Begin] == '!' || Input[Begin] == '*' ||
              std::tolower(static_cast<unsigned char>(Input[Begin])) == 'c')) {
    std::string Prefix = Input.substr(Begin + 2, 3);
    std::transform(Prefix.begin(), Prefix.end(), Prefix.begin(),
                   [](unsigned char Ch) { return std::tolower(Ch); });
    if (Prefix != "acc")
      return nullptr;
    Options.language = openacc::Language::Fortran;
    char Sentinel = static_cast<char>(
        std::tolower(static_cast<unsigned char>(Input[Begin])));
    AmbiguousFortranSentinel = Sentinel == '!';
    Options.inputForm = Sentinel == 'c' || Sentinel == '*'
                            ? openacc::InputForm::FortranFixed
                            : openacc::InputForm::FortranFree;
  }

  openacc::ParseResult Result = openacc::parseDirective(Input, Options);
  if (!Result.succeeded() && AmbiguousFortranSentinel) {
    Options.inputForm = openacc::InputForm::FortranFixed;
    Result = openacc::parseDirective(Input, Options);
  }
  if (!Result.succeeded())
    return nullptr;
  return new openacc::Directive(std::move(*Result.directive));
}
