//===----------------------------------------------------------------------===//
//
// Part of accparser, under the BSD 3-Clause License.
// See LICENSE for license information.
// SPDX-License-Identifier: BSD-3-Clause
//
//===----------------------------------------------------------------------===//

#ifndef ACCPARSER_OPENACCIR_H
#define ACCPARSER_OPENACCIR_H

#include "OpenACCKinds.h"

#include <cstddef>
#include <functional>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <variant>
#include <vector>

namespace openacc {

struct SourcePosition {
  std::size_t line = 1;
  std::size_t column = 1;
  std::size_t byteOffset = 0;

  friend bool operator==(const SourcePosition &LHS, const SourcePosition &RHS) {
    return LHS.line == RHS.line && LHS.column == RHS.column &&
           LHS.byteOffset == RHS.byteOffset;
  }
};

struct SourceRange {
  SourcePosition begin;
  SourcePosition end;

  friend bool operator==(const SourceRange &LHS, const SourceRange &RHS) {
    return LHS.begin == RHS.begin && LHS.end == RHS.end;
  }
};

struct Diagnostic {
  DiagnosticCode code;
  DiagnosticSeverity severity;
  std::string message;
  SourceRange range;
  std::optional<SourceRange> relatedRange;
};

template <typename Tag> class HostFragment {
public:
  static std::optional<HostFragment> create(std::string Spelling,
                                            SourceRange Range) {
    if (Spelling.empty())
      return std::nullopt;
    return HostFragment(std::move(Spelling), Range);
  }

  const std::string &spelling() const { return Spelling; }
  const SourceRange &range() const { return Range; }

  friend bool operator==(const HostFragment &LHS, const HostFragment &RHS) {
    return LHS.Spelling == RHS.Spelling;
  }

private:
  HostFragment(std::string Spelling, SourceRange Range)
      : Spelling(std::move(Spelling)), Range(Range) {}

  std::string Spelling;
  SourceRange Range;
};

struct AsyncArgumentTag;
struct ConditionTag;
struct IdentifierTag;
struct IntegerExprTag;
struct IntegralConstantExprTag;
struct RoutineNameTag;
struct VariableRefTag;

using AsyncArgument = HostFragment<AsyncArgumentTag>;
using Condition = HostFragment<ConditionTag>;
using Identifier = HostFragment<IdentifierTag>;
using IntegerExpr = HostFragment<IntegerExprTag>;
using IntegralConstantExpr = HostFragment<IntegralConstantExprTag>;
using RoutineName = HostFragment<RoutineNameTag>;
using VariableRef = HostFragment<VariableRefTag>;

class StringLiteral {
public:
  static std::optional<StringLiteral> create(std::string Spelling,
                                             SourceRange Range) {
    if (Spelling.empty())
      return std::nullopt;
    return StringLiteral(std::move(Spelling), Range);
  }

  const std::string &spelling() const { return Spelling; }
  const SourceRange &range() const { return Range; }

  friend bool operator==(const StringLiteral &LHS, const StringLiteral &RHS) {
    return LHS.Spelling == RHS.Spelling;
  }

private:
  StringLiteral(std::string Spelling, SourceRange Range)
      : Spelling(std::move(Spelling)), Range(Range) {}

  std::string Spelling;
  SourceRange Range;
};

template <typename T> class NonEmptyList {
public:
  static std::optional<NonEmptyList> create(std::vector<T> Values) {
    if (Values.empty())
      return std::nullopt;
    return NonEmptyList(std::move(Values));
  }

  const std::vector<T> &values() const { return Values; }
  std::size_t size() const { return Values.size(); }

  friend bool operator==(const NonEmptyList &LHS, const NonEmptyList &RHS) {
    return LHS.Values == RHS.Values;
  }

private:
  explicit NonEmptyList(std::vector<T> Values) : Values(std::move(Values)) {}

  std::vector<T> Values;
};

struct WildcardDevice {
  SourceRange range;

  friend bool operator==(const WildcardDevice &, const WildcardDevice &) {
    return true;
  }
};

using ArchitectureIdentifier = Identifier;
using DeviceSelector = std::variant<WildcardDevice, ArchitectureIdentifier>;
using DeviceSelectorList = NonEmptyList<DeviceSelector>;

struct StarSize {
  SourceRange range;

  friend bool operator==(const StarSize &, const StarSize &) { return true; }
};

using SizeExpr = std::variant<StarSize, IntegerExpr>;
using NameOrString = std::variant<Identifier, StringLiteral>;

struct WaitArgument {
  std::optional<IntegerExpr> deviceNumber;
  std::optional<NonEmptyList<AsyncArgument>> queues;
  bool hasQueuesKeyword = false;
  SourceRange range;

  friend bool operator==(const WaitArgument &LHS, const WaitArgument &RHS) {
    return LHS.deviceNumber == RHS.deviceNumber && LHS.queues == RHS.queues;
  }
};

struct PositionalGangArgument {
  IntegerExpr value;
};

struct NumGangArgument {
  IntegerExpr value;
};

struct DimGangArgument {
  IntegerExpr value;
};

struct StaticGangArgument {
  SizeExpr value;
};

using GangArgument = std::variant<PositionalGangArgument, NumGangArgument,
                                  DimGangArgument, StaticGangArgument>;

struct FlagClause {
  FlagClauseKind kind;
  SourceRange range;
};

struct AsyncClause {
  std::optional<AsyncArgument> argument;
  SourceRange range;
};

struct BindClause {
  NameOrString target;
  SourceRange range;
};

struct CollapseClause {
  bool force;
  IntegralConstantExpr count;
  SourceRange range;
};

struct CopyClause {
  std::vector<CopyModifier> modifiers;
  NonEmptyList<VariableRef> variables;
  SourceRange range;
};

struct CopyInClause {
  std::vector<CopyInModifier> modifiers;
  NonEmptyList<VariableRef> variables;
  SourceRange range;
};

struct CopyOutClause {
  std::vector<CopyOutModifier> modifiers;
  NonEmptyList<VariableRef> variables;
  SourceRange range;
};

struct CreateClause {
  std::vector<CreateModifier> modifiers;
  NonEmptyList<VariableRef> variables;
  SourceRange range;
};

struct DefaultClause {
  DefaultKind value;
  SourceRange range;
};

struct DefaultAsyncClause {
  AsyncArgument argument;
  SourceRange range;
};

struct DeviceNumClause {
  IntegerExpr value;
  SourceRange range;
};

struct DeviceTypeClause {
  DeviceSelectorList selectors;
  SourceRange range;
};

struct GangClause {
  std::vector<GangArgument> arguments;
  SourceRange range;
};

struct IfClause {
  Condition condition;
  SourceRange range;
};

struct NumGangsClause {
  NonEmptyList<IntegerExpr> values;
  SourceRange range;
};

struct NumWorkersClause {
  IntegerExpr value;
  SourceRange range;
};

struct ReductionClause {
  ReductionOperator op;
  NonEmptyList<VariableRef> variables;
  SourceRange range;
};

struct SelfConditionClause {
  std::optional<Condition> condition;
  SourceRange range;
};

struct TileClause {
  NonEmptyList<SizeExpr> sizes;
  SourceRange range;
};

struct VarListClause {
  VarListClauseKind kind;
  NonEmptyList<VariableRef> variables;
  SourceRange range;
};

struct VectorArgument {
  bool hasLengthKeyword;
  IntegerExpr value;
};

struct VectorClause {
  std::optional<VectorArgument> argument;
  SourceRange range;
};

struct VectorLengthClause {
  IntegerExpr value;
  SourceRange range;
};

struct WaitClause {
  std::optional<WaitArgument> argument;
  SourceRange range;
};

struct WorkerArgument {
  bool hasNumKeyword;
  IntegerExpr value;
};

struct WorkerClause {
  std::optional<WorkerArgument> argument;
  SourceRange range;
};

using Clause =
    std::variant<FlagClause, AsyncClause, BindClause, CollapseClause,
                 CopyClause, CopyInClause, CopyOutClause, CreateClause,
                 DefaultClause, DefaultAsyncClause, DeviceNumClause,
                 DeviceTypeClause, GangClause, IfClause, NumGangsClause,
                 NumWorkersClause, ReductionClause, SelfConditionClause,
                 TileClause, VarListClause, VectorClause, VectorLengthClause,
                 WaitClause, WorkerClause>;

struct DeviceClauseGroup {
  DeviceSelectorList selectors;
  SourceRange selectorRange;
  std::vector<Clause> clauses;
};

struct GeneralDirective {
  DirectiveKind kind;
  std::optional<RoutineName> routineName;
  std::optional<WaitArgument> waitArgument;
  std::vector<Clause> defaultClauses;
  std::vector<DeviceClauseGroup> deviceGroups;
};

struct CacheDirective {
  bool readOnly;
  NonEmptyList<VariableRef> variables;
};

struct EndDirective {
  EndDirectiveKind kind;
};

using DirectivePayload =
    std::variant<GeneralDirective, CacheDirective, EndDirective>;

class Directive {
public:
  Directive(Language Lang, InputForm Form, SourceRange Range,
            DirectivePayload Payload)
      : Lang(Lang), Form(Form), Range(Range), Payload(std::move(Payload)) {}

  Language language() const { return Lang; }
  InputForm inputForm() const { return Form; }
  const SourceRange &range() const { return Range; }
  const DirectivePayload &payload() const { return Payload; }
  DirectiveKind kind() const;

  bool semanticEquals(const Directive &Other) const;

private:
  Language Lang;
  InputForm Form;
  SourceRange Range;
  DirectivePayload Payload;
};

enum class HostFragmentKind { Expression, Variable };

struct HostFragmentView {
  HostFragmentKind kind;
  std::string_view spelling;
  SourceRange range;
};

using HostFragmentVisitor = std::function<void(const HostFragmentView &)>;

void visitHostFragments(const Directive &Value,
                        const HostFragmentVisitor &Visitor);

ClauseKind getClauseKind(const Clause &Value);
const SourceRange &getClauseRange(const Clause &Value);

} // namespace openacc

#endif // ACCPARSER_OPENACCIR_H
