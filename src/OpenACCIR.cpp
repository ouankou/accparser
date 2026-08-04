//===----------------------------------------------------------------------===//
//
// Part of accparser, under the BSD 3-Clause License.
// See LICENSE for license information.
// SPDX-License-Identifier: BSD-3-Clause
//
//===----------------------------------------------------------------------===//

#include "OpenACCIR.h"
#include "OpenACCParser.h"

#include <cstdlib>
#include <type_traits>

namespace openacc {
namespace {

template <class... Ts> struct Overloaded : Ts... {
  using Ts::operator()...;
};
template <class... Ts> Overloaded(Ts...) -> Overloaded<Ts...>;

ClauseKind getFlagClauseKind(FlagClauseKind Kind) {
  switch (Kind) {
  case FlagClauseKind::Auto:
    return ClauseKind::Auto;
  case FlagClauseKind::Capture:
    return ClauseKind::Capture;
  case FlagClauseKind::Finalize:
    return ClauseKind::Finalize;
  case FlagClauseKind::IfPresent:
    return ClauseKind::IfPresent;
  case FlagClauseKind::Independent:
    return ClauseKind::Independent;
  case FlagClauseKind::NoHost:
    return ClauseKind::NoHost;
  case FlagClauseKind::Read:
    return ClauseKind::Read;
  case FlagClauseKind::Seq:
    return ClauseKind::Seq;
  case FlagClauseKind::Update:
    return ClauseKind::Update;
  case FlagClauseKind::Write:
    return ClauseKind::Write;
  }
  return ClauseKind::Auto;
}

ClauseKind getVarListClauseKind(VarListClauseKind Kind) {
  switch (Kind) {
  case VarListClauseKind::Attach:
    return ClauseKind::Attach;
  case VarListClauseKind::Delete:
    return ClauseKind::Delete;
  case VarListClauseKind::Detach:
    return ClauseKind::Detach;
  case VarListClauseKind::Device:
    return ClauseKind::Device;
  case VarListClauseKind::DeviceResident:
    return ClauseKind::DeviceResident;
  case VarListClauseKind::DevicePtr:
    return ClauseKind::DevicePtr;
  case VarListClauseKind::FirstPrivate:
    return ClauseKind::FirstPrivate;
  case VarListClauseKind::Host:
    return ClauseKind::Host;
  case VarListClauseKind::Link:
    return ClauseKind::Link;
  case VarListClauseKind::NoCreate:
    return ClauseKind::NoCreate;
  case VarListClauseKind::Present:
    return ClauseKind::Present;
  case VarListClauseKind::Private:
    return ClauseKind::Private;
  case VarListClauseKind::Self:
    return ClauseKind::Self;
  case VarListClauseKind::UseDevice:
    return ClauseKind::UseDevice;
  }
  return ClauseKind::Attach;
}

bool equalGangArgument(const GangArgument &LHS, const GangArgument &RHS) {
  if ((std::holds_alternative<PositionalGangArgument>(LHS) &&
       std::holds_alternative<NumGangArgument>(RHS)) ||
      (std::holds_alternative<NumGangArgument>(LHS) &&
       std::holds_alternative<PositionalGangArgument>(RHS))) {
    const IntegerExpr &Left =
        std::holds_alternative<PositionalGangArgument>(LHS)
            ? std::get<PositionalGangArgument>(LHS).value
            : std::get<NumGangArgument>(LHS).value;
    const IntegerExpr &Right =
        std::holds_alternative<PositionalGangArgument>(RHS)
            ? std::get<PositionalGangArgument>(RHS).value
            : std::get<NumGangArgument>(RHS).value;
    return Left == Right;
  }
  if (LHS.index() != RHS.index())
    return false;
  return std::visit(
      [&RHS](const auto &Left) {
        using T = std::decay_t<decltype(Left)>;
        const T &Right = std::get<T>(RHS);
        return Left.value == Right.value;
      },
      LHS);
}

bool equalGangArguments(const std::vector<GangArgument> &LHS,
                        const std::vector<GangArgument> &RHS) {
  if (LHS.size() != RHS.size())
    return false;
  for (std::size_t I = 0; I < LHS.size(); ++I)
    if (!equalGangArgument(LHS[I], RHS[I]))
      return false;
  return true;
}

bool equalClause(const Clause &LHS, const Clause &RHS) {
  if (LHS.index() != RHS.index())
    return false;
  return std::visit(
      [&RHS](const auto &Left) {
        using T = std::decay_t<decltype(Left)>;
        const T &Right = std::get<T>(RHS);
        if constexpr (std::is_same_v<T, FlagClause>) {
          return Left.kind == Right.kind;
        } else if constexpr (std::is_same_v<T, AsyncClause>) {
          return Left.argument == Right.argument;
        } else if constexpr (std::is_same_v<T, BindClause>) {
          return Left.target == Right.target;
        } else if constexpr (std::is_same_v<T, CollapseClause>) {
          return Left.force == Right.force && Left.count == Right.count;
        } else if constexpr (std::is_same_v<T, CopyClause> ||
                             std::is_same_v<T, CopyInClause> ||
                             std::is_same_v<T, CopyOutClause> ||
                             std::is_same_v<T, CreateClause>) {
          return Left.modifiers == Right.modifiers &&
                 Left.variables == Right.variables;
        } else if constexpr (std::is_same_v<T, DefaultClause>) {
          return Left.value == Right.value;
        } else if constexpr (std::is_same_v<T, DefaultAsyncClause>) {
          return Left.argument == Right.argument;
        } else if constexpr (std::is_same_v<T, DeviceNumClause>) {
          return Left.value == Right.value;
        } else if constexpr (std::is_same_v<T, DeviceTypeClause>) {
          return Left.selectors == Right.selectors;
        } else if constexpr (std::is_same_v<T, GangClause>) {
          return equalGangArguments(Left.arguments, Right.arguments);
        } else if constexpr (std::is_same_v<T, IfClause>) {
          return Left.condition == Right.condition;
        } else if constexpr (std::is_same_v<T, NumGangsClause>) {
          return Left.values == Right.values;
        } else if constexpr (std::is_same_v<T, NumWorkersClause>) {
          return Left.value == Right.value;
        } else if constexpr (std::is_same_v<T, ReductionClause>) {
          return Left.op == Right.op && Left.variables == Right.variables;
        } else if constexpr (std::is_same_v<T, SelfConditionClause>) {
          return Left.condition == Right.condition;
        } else if constexpr (std::is_same_v<T, TileClause>) {
          return Left.sizes == Right.sizes;
        } else if constexpr (std::is_same_v<T, VarListClause>) {
          return Left.kind == Right.kind && Left.variables == Right.variables;
        } else if constexpr (std::is_same_v<T, VectorClause>) {
          if (Left.argument.has_value() != Right.argument.has_value())
            return false;
          return !Left.argument ||
                 Left.argument->value == Right.argument->value;
        } else if constexpr (std::is_same_v<T, VectorLengthClause>) {
          return Left.value == Right.value;
        } else if constexpr (std::is_same_v<T, WaitClause>) {
          return Left.argument == Right.argument;
        } else if constexpr (std::is_same_v<T, WorkerClause>) {
          if (Left.argument.has_value() != Right.argument.has_value())
            return false;
          return !Left.argument ||
                 Left.argument->value == Right.argument->value;
        }
        return false;
      },
      LHS);
}

bool equalClauses(const std::vector<Clause> &LHS,
                  const std::vector<Clause> &RHS) {
  if (LHS.size() != RHS.size())
    return false;
  for (std::size_t I = 0; I < LHS.size(); ++I)
    if (!equalClause(LHS[I], RHS[I]))
      return false;
  return true;
}

bool equalGeneralDirective(const GeneralDirective &LHS,
                           const GeneralDirective &RHS) {
  if (LHS.kind != RHS.kind || !(LHS.routineName == RHS.routineName) ||
      !(LHS.waitArgument == RHS.waitArgument) ||
      !equalClauses(LHS.defaultClauses, RHS.defaultClauses) ||
      LHS.deviceGroups.size() != RHS.deviceGroups.size())
    return false;
  for (std::size_t I = 0; I < LHS.deviceGroups.size(); ++I) {
    const DeviceClauseGroup &Left = LHS.deviceGroups[I];
    const DeviceClauseGroup &Right = RHS.deviceGroups[I];
    if (!(Left.selectors == Right.selectors) ||
        !equalClauses(Left.clauses, Right.clauses))
      return false;
  }
  return true;
}

} // namespace

bool ParseResult::succeeded() const {
  if (!directive)
    return false;
  for (const Diagnostic &Item : diagnostics)
    if (Item.severity == DiagnosticSeverity::Error)
      return false;
  return true;
}

DirectiveKind Directive::kind() const {
  return std::visit(
      Overloaded{
          [](const GeneralDirective &Value) { return Value.kind; },
          [](const CacheDirective &) { return DirectiveKind::Cache; },
          [](const EndDirective &) { return DirectiveKind::End; },
      },
      Payload);
}

bool Directive::semanticEquals(const Directive &Other) const {
  if (language() != Other.language() ||
      Payload.index() != Other.Payload.index())
    return false;
  return std::visit(
      [&Other](const auto &Left) {
        using T = std::decay_t<decltype(Left)>;
        const T &Right = std::get<T>(Other.Payload);
        if constexpr (std::is_same_v<T, GeneralDirective>)
          return equalGeneralDirective(Left, Right);
        if constexpr (std::is_same_v<T, CacheDirective>)
          return Left.readOnly == Right.readOnly &&
                 Left.variables == Right.variables;
        if constexpr (std::is_same_v<T, EndDirective>)
          return Left.kind == Right.kind;
        return false;
      },
      Payload);
}

void visitHostFragments(const Directive &Value,
                        const HostFragmentVisitor &Visitor) {
  if (!Visitor)
    std::abort();

  auto emitExpression = [&Visitor](const auto &Fragment) {
    Visitor(
        {HostFragmentKind::Expression, Fragment.spelling(), Fragment.range()});
  };
  auto emitVariable = [&Visitor](const VariableRef &Fragment) {
    Visitor(
        {HostFragmentKind::Variable, Fragment.spelling(), Fragment.range()});
  };
  auto visitVariables = [&emitVariable](const auto &Variables) {
    for (const VariableRef &Variable : Variables.values())
      emitVariable(Variable);
  };
  auto visitSize = [&emitExpression](const SizeExpr &Size) {
    if (const IntegerExpr *Expression = std::get_if<IntegerExpr>(&Size))
      emitExpression(*Expression);
  };
  auto visitWaitArgument = [&emitExpression](const WaitArgument &Argument) {
    if (Argument.deviceNumber)
      emitExpression(*Argument.deviceNumber);
    if (Argument.queues)
      for (const AsyncArgument &Queue : Argument.queues->values())
        emitExpression(Queue);
  };
  auto visitClause = [&](const Clause &Value) {
    std::visit(
        Overloaded{
            [](const FlagClause &) {},
            [&emitExpression](const AsyncClause &Clause) {
              if (Clause.argument)
                emitExpression(*Clause.argument);
            },
            [](const BindClause &) {},
            [&emitExpression](const CollapseClause &Clause) {
              emitExpression(Clause.count);
            },
            [&visitVariables](const CopyClause &Clause) {
              visitVariables(Clause.variables);
            },
            [&visitVariables](const CopyInClause &Clause) {
              visitVariables(Clause.variables);
            },
            [&visitVariables](const CopyOutClause &Clause) {
              visitVariables(Clause.variables);
            },
            [&visitVariables](const CreateClause &Clause) {
              visitVariables(Clause.variables);
            },
            [](const DefaultClause &) {},
            [&emitExpression](const DefaultAsyncClause &Clause) {
              emitExpression(Clause.argument);
            },
            [&emitExpression](const DeviceNumClause &Clause) {
              emitExpression(Clause.value);
            },
            [](const DeviceTypeClause &) {},
            [&emitExpression, &visitSize](const GangClause &Clause) {
              for (const GangArgument &Argument : Clause.arguments) {
                std::visit(
                    Overloaded{
                        [&emitExpression](const PositionalGangArgument &Item) {
                          emitExpression(Item.value);
                        },
                        [&emitExpression](const NumGangArgument &Item) {
                          emitExpression(Item.value);
                        },
                        [&emitExpression](const DimGangArgument &Item) {
                          emitExpression(Item.value);
                        },
                        [&visitSize](const StaticGangArgument &Item) {
                          visitSize(Item.value);
                        },
                    },
                    Argument);
              }
            },
            [&emitExpression](const IfClause &Clause) {
              emitExpression(Clause.condition);
            },
            [&emitExpression](const NumGangsClause &Clause) {
              for (const IntegerExpr &Expression : Clause.values.values())
                emitExpression(Expression);
            },
            [&emitExpression](const NumWorkersClause &Clause) {
              emitExpression(Clause.value);
            },
            [&visitVariables](const ReductionClause &Clause) {
              visitVariables(Clause.variables);
            },
            [&emitExpression](const SelfConditionClause &Clause) {
              if (Clause.condition)
                emitExpression(*Clause.condition);
            },
            [&visitSize](const TileClause &Clause) {
              for (const SizeExpr &Size : Clause.sizes.values())
                visitSize(Size);
            },
            [&visitVariables](const VarListClause &Clause) {
              visitVariables(Clause.variables);
            },
            [&emitExpression](const VectorClause &Clause) {
              if (Clause.argument)
                emitExpression(Clause.argument->value);
            },
            [&emitExpression](const VectorLengthClause &Clause) {
              emitExpression(Clause.value);
            },
            [&visitWaitArgument](const WaitClause &Clause) {
              if (Clause.argument)
                visitWaitArgument(*Clause.argument);
            },
            [&emitExpression](const WorkerClause &Clause) {
              if (Clause.argument)
                emitExpression(Clause.argument->value);
            },
        },
        Value);
  };

  std::visit(Overloaded{
                 [&](const GeneralDirective &Directive) {
                   if (Directive.waitArgument)
                     visitWaitArgument(*Directive.waitArgument);
                   for (const Clause &Clause : Directive.defaultClauses)
                     visitClause(Clause);
                   for (const DeviceClauseGroup &Group : Directive.deviceGroups)
                     for (const Clause &Clause : Group.clauses)
                       visitClause(Clause);
                 },
                 [&](const CacheDirective &Directive) {
                   visitVariables(Directive.variables);
                 },
                 [](const EndDirective &) {},
             },
             Value.payload());
}

ClauseKind getClauseKind(const Clause &Value) {
  return std::visit(
      Overloaded{
          [](const FlagClause &Item) { return getFlagClauseKind(Item.kind); },
          [](const AsyncClause &) { return ClauseKind::Async; },
          [](const BindClause &) { return ClauseKind::Bind; },
          [](const CollapseClause &) { return ClauseKind::Collapse; },
          [](const CopyClause &) { return ClauseKind::Copy; },
          [](const CopyInClause &) { return ClauseKind::CopyIn; },
          [](const CopyOutClause &) { return ClauseKind::CopyOut; },
          [](const CreateClause &) { return ClauseKind::Create; },
          [](const DefaultClause &) { return ClauseKind::Default; },
          [](const DefaultAsyncClause &) { return ClauseKind::DefaultAsync; },
          [](const DeviceNumClause &) { return ClauseKind::DeviceNum; },
          [](const DeviceTypeClause &) { return ClauseKind::DeviceType; },
          [](const GangClause &) { return ClauseKind::Gang; },
          [](const IfClause &) { return ClauseKind::If; },
          [](const NumGangsClause &) { return ClauseKind::NumGangs; },
          [](const NumWorkersClause &) { return ClauseKind::NumWorkers; },
          [](const ReductionClause &) { return ClauseKind::Reduction; },
          [](const SelfConditionClause &) { return ClauseKind::Self; },
          [](const TileClause &) { return ClauseKind::Tile; },
          [](const VarListClause &Item) {
            return getVarListClauseKind(Item.kind);
          },
          [](const VectorClause &) { return ClauseKind::Vector; },
          [](const VectorLengthClause &) { return ClauseKind::VectorLength; },
          [](const WaitClause &) { return ClauseKind::Wait; },
          [](const WorkerClause &) { return ClauseKind::Worker; },
      },
      Value);
}

const SourceRange &getClauseRange(const Clause &Value) {
  return std::visit(
      [](const auto &Item) -> const SourceRange & { return Item.range; },
      Value);
}

} // namespace openacc
