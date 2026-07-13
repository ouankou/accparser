//===----------------------------------------------------------------------===//
//
// Part of accparser, under the BSD 3-Clause License.
// See LICENSE for license information.
// SPDX-License-Identifier: BSD-3-Clause
//
//===----------------------------------------------------------------------===//

#include "OpenACCParser.h"

#include <string>
#include <string_view>
#include <type_traits>
#include <utility>

namespace openacc {
namespace {

template <class... Ts> struct Overloaded : Ts... {
  using Ts::operator()...;
};
template <class... Ts> Overloaded(Ts...) -> Overloaded<Ts...>;

std::string directiveName(DirectiveKind Kind) {
  switch (Kind) {
  case DirectiveKind::Atomic:
    return "atomic";
  case DirectiveKind::Cache:
    return "cache";
  case DirectiveKind::Data:
    return "data";
  case DirectiveKind::Declare:
    return "declare";
  case DirectiveKind::End:
    return "end";
  case DirectiveKind::EnterData:
    return "enter data";
  case DirectiveKind::ExitData:
    return "exit data";
  case DirectiveKind::HostData:
    return "host_data";
  case DirectiveKind::Init:
    return "init";
  case DirectiveKind::Kernels:
    return "kernels";
  case DirectiveKind::KernelsLoop:
    return "kernels loop";
  case DirectiveKind::Loop:
    return "loop";
  case DirectiveKind::Parallel:
    return "parallel";
  case DirectiveKind::ParallelLoop:
    return "parallel loop";
  case DirectiveKind::Routine:
    return "routine";
  case DirectiveKind::Serial:
    return "serial";
  case DirectiveKind::SerialLoop:
    return "serial loop";
  case DirectiveKind::Set:
    return "set";
  case DirectiveKind::Shutdown:
    return "shutdown";
  case DirectiveKind::Update:
    return "update";
  case DirectiveKind::Wait:
    return "wait";
  }
  return {};
}

std::string endDirectiveName(EndDirectiveKind Kind) {
  switch (Kind) {
  case EndDirectiveKind::Atomic:
    return "atomic";
  case EndDirectiveKind::Data:
    return "data";
  case EndDirectiveKind::HostData:
    return "host_data";
  case EndDirectiveKind::Kernels:
    return "kernels";
  case EndDirectiveKind::KernelsLoop:
    return "kernels loop";
  case EndDirectiveKind::Loop:
    return "loop";
  case EndDirectiveKind::Parallel:
    return "parallel";
  case EndDirectiveKind::ParallelLoop:
    return "parallel loop";
  case EndDirectiveKind::Serial:
    return "serial";
  case EndDirectiveKind::SerialLoop:
    return "serial loop";
  }
  return {};
}

std::string flagName(FlagClauseKind Kind) {
  switch (Kind) {
  case FlagClauseKind::Auto:
    return "auto";
  case FlagClauseKind::Capture:
    return "capture";
  case FlagClauseKind::Finalize:
    return "finalize";
  case FlagClauseKind::IfPresent:
    return "if_present";
  case FlagClauseKind::Independent:
    return "independent";
  case FlagClauseKind::NoHost:
    return "nohost";
  case FlagClauseKind::Read:
    return "read";
  case FlagClauseKind::Seq:
    return "seq";
  case FlagClauseKind::Update:
    return "update";
  case FlagClauseKind::Write:
    return "write";
  }
  return {};
}

std::string varListName(VarListClauseKind Kind) {
  switch (Kind) {
  case VarListClauseKind::Attach:
    return "attach";
  case VarListClauseKind::Delete:
    return "delete";
  case VarListClauseKind::Detach:
    return "detach";
  case VarListClauseKind::Device:
    return "device";
  case VarListClauseKind::DeviceResident:
    return "device_resident";
  case VarListClauseKind::DevicePtr:
    return "deviceptr";
  case VarListClauseKind::FirstPrivate:
    return "firstprivate";
  case VarListClauseKind::Host:
    return "host";
  case VarListClauseKind::Link:
    return "link";
  case VarListClauseKind::NoCreate:
    return "no_create";
  case VarListClauseKind::Present:
    return "present";
  case VarListClauseKind::Private:
    return "private";
  case VarListClauseKind::Self:
    return "self";
  case VarListClauseKind::UseDevice:
    return "use_device";
  }
  return {};
}

template <typename T, typename Printer>
std::string join(const std::vector<T> &Values, std::string_view Separator,
                 Printer Print) {
  std::string Result;
  for (std::size_t I = 0; I < Values.size(); ++I) {
    if (I != 0)
      Result += Separator;
    Result += Print(Values[I]);
  }
  return Result;
}

template <typename T> std::string joinFragments(const NonEmptyList<T> &Values) {
  return join(Values.values(), ", ",
              [](const T &Value) { return Value.spelling(); });
}

std::string printDeviceSelector(const DeviceSelector &Selector) {
  return std::visit(
      Overloaded{
          [](const WildcardDevice &) { return std::string("*"); },
          [](const ArchitectureIdentifier &Value) { return Value.spelling(); }},
      Selector);
}

std::string printDeviceSelectors(const DeviceSelectorList &Selectors) {
  return join(Selectors.values(), ", ", printDeviceSelector);
}

std::string printSize(const SizeExpr &Size) {
  return std::visit(
      Overloaded{[](const StarSize &) { return std::string("*"); },
                 [](const IntegerExpr &Value) { return Value.spelling(); }},
      Size);
}

std::string escapeCString(std::string_view Value) {
  std::string Result;
  for (char Ch : Value) {
    switch (Ch) {
    case '\\':
      Result += "\\\\";
      break;
    case '"':
      Result += "\\\"";
      break;
    case '\n':
      Result += "\\n";
      break;
    case '\r':
      Result += "\\r";
      break;
    case '\t':
      Result += "\\t";
      break;
    default:
      Result += Ch;
      break;
    }
  }
  return Result;
}

std::string printStringLiteral(const StringLiteral &Literal, Language Lang) {
  (void)Lang;
  return Literal.spelling();
}

std::string printNameOrString(const NameOrString &Value, Language Lang) {
  return std::visit(
      Overloaded{[](const Identifier &Name) { return Name.spelling(); },
                 [Lang](const StringLiteral &Literal) {
                   return printStringLiteral(Literal, Lang);
                 }},
      Value);
}

std::string printWaitArgument(const WaitArgument &Argument) {
  std::string Result;
  if (Argument.deviceNumber)
    Result = "devnum:" + Argument.deviceNumber->spelling();
  if (Argument.queues) {
    if (!Result.empty())
      Result += ':';
    if (Argument.hasQueuesKeyword || Argument.deviceNumber)
      Result += "queues:";
    Result += joinFragments(*Argument.queues);
  }
  return Result;
}

std::string reductionName(ReductionOperator Op) {
  switch (Op) {
  case ReductionOperator::Add:
    return "+";
  case ReductionOperator::Subtract:
    return "-";
  case ReductionOperator::Multiply:
    return "*";
  case ReductionOperator::Maximum:
    return "max";
  case ReductionOperator::Minimum:
    return "min";
  case ReductionOperator::BitAnd:
    return "&";
  case ReductionOperator::BitOr:
    return "|";
  case ReductionOperator::BitXor:
    return "^";
  case ReductionOperator::LogicalAnd:
    return "&&";
  case ReductionOperator::LogicalOr:
    return "||";
  case ReductionOperator::FortranAnd:
    return ".and.";
  case ReductionOperator::FortranOr:
    return ".or.";
  case ReductionOperator::FortranEqv:
    return ".eqv.";
  case ReductionOperator::FortranNeqv:
    return ".neqv.";
  case ReductionOperator::FortranIand:
    return "iand";
  case ReductionOperator::FortranIor:
    return "ior";
  case ReductionOperator::FortranIeor:
    return "ieor";
  }
  return {};
}

template <typename Modifier> std::string modifierName(Modifier Value) {
  if constexpr (std::is_same_v<Modifier, CopyModifier>) {
    switch (Value) {
    case CopyModifier::Always:
      return "always";
    case CopyModifier::AlwaysIn:
      return "alwaysin";
    case CopyModifier::AlwaysOut:
      return "alwaysout";
    case CopyModifier::Capture:
      return "capture";
    }
  } else if constexpr (std::is_same_v<Modifier, CopyInModifier>) {
    switch (Value) {
    case CopyInModifier::Always:
      return "always";
    case CopyInModifier::AlwaysIn:
      return "alwaysin";
    case CopyInModifier::Capture:
      return "capture";
    case CopyInModifier::ReadOnly:
      return "readonly";
    }
  } else if constexpr (std::is_same_v<Modifier, CopyOutModifier>) {
    switch (Value) {
    case CopyOutModifier::Always:
      return "always";
    case CopyOutModifier::AlwaysOut:
      return "alwaysout";
    case CopyOutModifier::Capture:
      return "capture";
    case CopyOutModifier::Zero:
      return "zero";
    }
  } else {
    switch (Value) {
    case CreateModifier::Capture:
      return "capture";
    case CreateModifier::Zero:
      return "zero";
    }
  }
  return {};
}

template <typename Modifier>
std::string printDataArguments(const std::vector<Modifier> &Modifiers,
                               const NonEmptyList<VariableRef> &Variables) {
  std::string Result;
  if (!Modifiers.empty()) {
    Result = join(Modifiers, ", ",
                  [](Modifier Value) { return modifierName(Value); });
    Result += ": ";
  }
  Result += joinFragments(Variables);
  return Result;
}

std::string printGangArgument(const GangArgument &Argument) {
  return std::visit(Overloaded{[](const PositionalGangArgument &Value) {
                                 return Value.value.spelling();
                               },
                               [](const NumGangArgument &Value) {
                                 return "num:" + Value.value.spelling();
                               },
                               [](const DimGangArgument &Value) {
                                 return "dim:" + Value.value.spelling();
                               },
                               [](const StaticGangArgument &Value) {
                                 return "static:" + printSize(Value.value);
                               }},
                    Argument);
}

std::string printClause(const Clause &Value, Language Lang) {
  return std::visit(
      Overloaded{
          [](const FlagClause &C) { return flagName(C.kind); },
          [](const AsyncClause &C) {
            return C.argument ? "async(" + C.argument->spelling() + ")"
                              : std::string("async");
          },
          [Lang](const BindClause &C) {
            return "bind(" + printNameOrString(C.target, Lang) + ")";
          },
          [](const CollapseClause &C) {
            return "collapse(" + std::string(C.force ? "force: " : "") +
                   C.count.spelling() + ")";
          },
          [](const CopyClause &C) {
            return "copy(" + printDataArguments(C.modifiers, C.variables) + ")";
          },
          [](const CopyInClause &C) {
            return "copyin(" + printDataArguments(C.modifiers, C.variables) +
                   ")";
          },
          [](const CopyOutClause &C) {
            return "copyout(" + printDataArguments(C.modifiers, C.variables) +
                   ")";
          },
          [](const CreateClause &C) {
            return "create(" + printDataArguments(C.modifiers, C.variables) +
                   ")";
          },
          [](const DefaultClause &C) {
            return std::string("default(") +
                   (C.value == DefaultKind::None ? "none" : "present") + ")";
          },
          [](const DefaultAsyncClause &C) {
            return "default_async(" + C.argument.spelling() + ")";
          },
          [](const DeviceNumClause &C) {
            return "device_num(" + C.value.spelling() + ")";
          },
          [](const DeviceTypeClause &C) {
            return "device_type(" + printDeviceSelectors(C.selectors) + ")";
          },
          [](const GangClause &C) {
            if (C.arguments.empty())
              return std::string("gang");
            return "gang(" + join(C.arguments, ", ", printGangArgument) + ")";
          },
          [](const IfClause &C) {
            return "if(" + C.condition.spelling() + ")";
          },
          [](const NumGangsClause &C) {
            return "num_gangs(" + joinFragments(C.values) + ")";
          },
          [](const NumWorkersClause &C) {
            return "num_workers(" + C.value.spelling() + ")";
          },
          [](const ReductionClause &C) {
            return "reduction(" + reductionName(C.op) + ": " +
                   joinFragments(C.variables) + ")";
          },
          [](const SelfConditionClause &C) {
            return C.condition ? "self(" + C.condition->spelling() + ")"
                               : std::string("self");
          },
          [](const TileClause &C) {
            return "tile(" + join(C.sizes.values(), ", ", printSize) + ")";
          },
          [](const VarListClause &C) {
            return varListName(C.kind) + "(" + joinFragments(C.variables) + ")";
          },
          [](const VectorClause &C) {
            if (!C.argument)
              return std::string("vector");
            return "vector(" +
                   std::string(C.argument->hasLengthKeyword ? "length: " : "") +
                   C.argument->value.spelling() + ")";
          },
          [](const VectorLengthClause &C) {
            return "vector_length(" + C.value.spelling() + ")";
          },
          [](const WaitClause &C) {
            return C.argument ? "wait(" + printWaitArgument(*C.argument) + ")"
                              : std::string("wait");
          },
          [](const WorkerClause &C) {
            if (!C.argument)
              return std::string("worker");
            return "worker(" +
                   std::string(C.argument->hasNumKeyword ? "num: " : "") +
                   C.argument->value.spelling() + ")";
          }},
      Value);
}

void appendClause(std::string &Result, const std::string &Clause) {
  if (Clause.empty())
    return;
  Result += ' ';
  Result += Clause;
}

std::string printGeneral(const GeneralDirective &Directive, Language Lang) {
  std::string Result = directiveName(Directive.kind);
  if (Directive.routineName)
    Result += '(' + Directive.routineName->spelling() + ')';
  if (Directive.waitArgument)
    Result += '(' + printWaitArgument(*Directive.waitArgument) + ')';

  for (const Clause &C : Directive.defaultClauses)
    appendClause(Result, printClause(C, Lang));
  for (const DeviceClauseGroup &Group : Directive.deviceGroups) {
    appendClause(Result,
                 "device_type(" + printDeviceSelectors(Group.selectors) + ")");
    for (const Clause &C : Group.clauses)
      appendClause(Result, printClause(C, Lang));
  }
  return Result;
}

std::string printBody(const Directive &Value) {
  return std::visit(Overloaded{[&Value](const GeneralDirective &D) {
                                 return printGeneral(D, Value.language());
                               },
                               [](const CacheDirective &D) {
                                 return "cache(" +
                                        std::string(D.readOnly ? "readonly: "
                                                               : "") +
                                        joinFragments(D.variables) + ")";
                               },
                               [](const EndDirective &D) {
                                 return "end " + endDirectiveName(D.kind);
                               }},
                    Value.payload());
}

InputForm defaultForm(Language Lang) {
  return Lang == Language::Fortran ? InputForm::FortranFree
                                   : InputForm::CPragma;
}

} // namespace

std::string formatDirective(const Directive &Value, PrintOptions Options) {
  std::string Body = printBody(Value);
  InputForm Form = Options.outputForm.value_or(defaultForm(Value.language()));
  switch (Form) {
  case InputForm::DirectiveBody:
    return Body;
  case InputForm::CPragma:
    return "#pragma acc " + Body;
  case InputForm::CPragmaOperator:
    return "_Pragma(\"acc " + escapeCString(Body) + "\")";
  case InputForm::FortranFree:
  case InputForm::FortranFixed:
    return "!$acc " + Body;
  }
  return {};
}

} // namespace openacc
