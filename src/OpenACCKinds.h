//===----------------------------------------------------------------------===//
//
// Part of accparser, under the BSD 3-Clause License.
// See LICENSE for license information.
// SPDX-License-Identifier: BSD-3-Clause
//
//===----------------------------------------------------------------------===//

#ifndef ACCPARSER_OPENACCKINDS_H
#define ACCPARSER_OPENACCKINDS_H

#include <cstdint>

namespace openacc {

enum class Language : std::uint8_t { C, Cxx, Fortran };

enum class InputForm : std::uint8_t {
  DirectiveBody,
  CPragma,
  CPragmaOperator,
  FortranFree,
  FortranFixed,
};

enum class DirectiveKind : std::uint8_t {
  Atomic,
  Cache,
  Data,
  Declare,
  End,
  EnterData,
  ExitData,
  HostData,
  Init,
  Kernels,
  KernelsLoop,
  Loop,
  Parallel,
  ParallelLoop,
  Routine,
  Serial,
  SerialLoop,
  Set,
  Shutdown,
  Update,
  Wait,
};

enum class EndDirectiveKind : std::uint8_t {
  Atomic,
  Data,
  HostData,
  Kernels,
  KernelsLoop,
  Loop,
  Parallel,
  ParallelLoop,
  Serial,
  SerialLoop,
};

enum class ClauseKind : std::uint8_t {
  Async,
  Attach,
  Auto,
  Bind,
  Capture,
  Collapse,
  Copy,
  CopyIn,
  CopyOut,
  Create,
  DefaultAsync,
  Default,
  Delete,
  Detach,
  Device,
  DeviceNum,
  DeviceResident,
  DeviceType,
  DevicePtr,
  Finalize,
  FirstPrivate,
  Gang,
  Host,
  If,
  IfPresent,
  Independent,
  Link,
  NoHost,
  NoCreate,
  NumGangs,
  NumWorkers,
  Present,
  Private,
  Read,
  Reduction,
  Self,
  Seq,
  Tile,
  Update,
  UseDevice,
  Vector,
  VectorLength,
  Wait,
  Worker,
  Write,
};

enum class FlagClauseKind : std::uint8_t {
  Auto,
  Capture,
  Finalize,
  IfPresent,
  Independent,
  NoHost,
  Read,
  Seq,
  Update,
  Write,
};

enum class VarListClauseKind : std::uint8_t {
  Attach,
  Delete,
  Detach,
  Device,
  DeviceResident,
  DevicePtr,
  FirstPrivate,
  Host,
  Link,
  NoCreate,
  Present,
  Private,
  Self,
  UseDevice,
};

enum class CopyModifier : std::uint8_t {
  Always,
  AlwaysIn,
  AlwaysOut,
  Capture,
};

enum class CopyInModifier : std::uint8_t {
  Always,
  AlwaysIn,
  Capture,
  ReadOnly,
};

enum class CopyOutModifier : std::uint8_t {
  Always,
  AlwaysOut,
  Capture,
  Zero,
};

enum class CreateModifier : std::uint8_t {
  Capture,
  Zero,
};

enum class DefaultKind : std::uint8_t { None, Present };

enum class ReductionOperator : std::uint8_t {
  Add,
  Subtract,
  Multiply,
  Maximum,
  Minimum,
  BitAnd,
  BitOr,
  BitXor,
  LogicalAnd,
  LogicalOr,
  FortranAnd,
  FortranOr,
  FortranEqv,
  FortranNeqv,
  FortranIand,
  FortranIor,
  FortranIeor,
};

enum class GangArgumentKind : std::uint8_t {
  PositionalNum,
  Num,
  Dim,
  Static,
};

enum class DiagnosticSeverity : std::uint8_t { Warning, Error };

enum class DiagnosticCode : std::uint8_t {
  InvalidEnvelope,
  InvalidLanguageForm,
  InvalidCharacter,
  UnterminatedDelimiter,
  UnterminatedString,
  SyntaxError,
  UnknownDirective,
  UnknownClause,
  UnexpectedClause,
  MissingClause,
  DuplicateClause,
  InvalidClauseOrder,
  MissingArgument,
  UnexpectedArgument,
  EmptyListItem,
  TrailingComma,
  InvalidModifier,
  DuplicateModifier,
  InvalidValue,
  InvalidCombination,
  UnsupportedExtension,
  InternalError,
};

} // namespace openacc

#endif // ACCPARSER_OPENACCKINDS_H
