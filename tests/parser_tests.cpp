//===----------------------------------------------------------------------===//
//
// Part of accparser, under the BSD 3-Clause License.
// See LICENSE for license information.
// SPDX-License-Identifier: BSD-3-Clause
//
//===----------------------------------------------------------------------===//

#include "OpenACCParser.h"

#include <algorithm>
#include <atomic>
#include <iostream>
#include <string>
#include <thread>
#include <variant>
#include <vector>

namespace {

using openacc::DiagnosticCode;
using openacc::InputForm;
using openacc::Language;
using openacc::ParseOptions;
using openacc::ParseResult;

int Failures = 0;

void fail(const std::string &Message) {
  std::cerr << "FAIL: " << Message << '\n';
  ++Failures;
}

bool hasCode(const ParseResult &Result, DiagnosticCode Code) {
  return std::any_of(Result.diagnostics.begin(), Result.diagnostics.end(),
                     [Code](const openacc::Diagnostic &Diagnostic) {
                       return Diagnostic.code == Code;
                     });
}

std::size_t countCode(const ParseResult &Result, DiagnosticCode Code) {
  return static_cast<std::size_t>(
      std::count_if(Result.diagnostics.begin(), Result.diagnostics.end(),
                    [Code](const openacc::Diagnostic &Diagnostic) {
                      return Diagnostic.code == Code;
                    }));
}

ParseResult parse(std::string_view Input, Language Lang = Language::C,
                  InputForm Form = InputForm::CPragma) {
  return openacc::parseDirective(Input, {Lang, Form});
}

void expectValid(std::string_view Input, Language Lang = Language::C,
                 InputForm Form = InputForm::CPragma) {
  ParseResult First = parse(Input, Lang, Form);
  if (!First.succeeded()) {
    fail("expected valid directive: " + std::string(Input));
    for (const openacc::Diagnostic &Diagnostic : First.diagnostics)
      std::cerr << "  " << Diagnostic.message << '\n';
    return;
  }

  openacc::PrintOptions PrintOptions;
  PrintOptions.outputForm = Form;
  std::string Canonical =
      openacc::formatDirective(*First.directive, PrintOptions);
  ParseResult Second = parse(Canonical, Lang, Form);
  if (!Second.succeeded() ||
      !First.directive->semanticEquals(*Second.directive))
    fail("semantic round trip failed: " + std::string(Input));
  if (!Canonical.empty() && Canonical.back() == ' ')
    fail("canonical output has trailing whitespace");
}

void expectInvalid(std::string_view Input, DiagnosticCode Code,
                   Language Lang = Language::C,
                   InputForm Form = InputForm::CPragma) {
  ParseResult Result = parse(Input, Lang, Form);
  if (Result.succeeded() || Result.directive)
    fail("invalid directive produced an AST: " + std::string(Input));
  if (!hasCode(Result, Code))
    fail("invalid directive produced the wrong diagnostic: " +
         std::string(Input));
}

void testDirectiveFamilies() {
  const std::vector<std::string> Valid = {
      "#pragma acc atomic update",
      "#pragma acc cache(readonly: a[0:n])",
      "#pragma acc data copy(a)",
      "#pragma acc data copy(capture:a)",
      "#pragma acc declare create(a)",
      "#pragma acc enter data copyin(a)",
      "#pragma acc exit data delete(a)",
      "#pragma acc host_data use_device(a)",
      "#pragma acc init device_num(0)",
      "#pragma acc kernels num_gangs(4)",
      "#pragma acc kernels loop independent gang vector",
      "#pragma acc loop seq private(a)",
      "#pragma acc parallel async(1) copy(a)",
      "#pragma acc parallel loop num_gangs(2,2) reduction(+:x)",
      "#pragma acc parallel loop reduction(+:x) "
      "device_type(nvidia) num_gangs(2,2) device_type(radeon) gang",
      "#pragma acc parallel copyout(always,alwaysout,zero:a)",
      "#pragma acc parallel copyout(capture:a)",
      "#pragma acc parallel loop gang(dim:2, static:*) vector",
      "#pragma acc loop gang(cond ? 4 : 8)",
      "#pragma acc routine(foo)",
      "#pragma acc routine(foo) bind(backend) gang(dim:2)",
      "#pragma acc serial wait(queues:q) present(a)",
      "#pragma acc serial loop worker vector",
      "#pragma acc set device_type(nvidia) device_num(0)",
      "#pragma acc shutdown if(done) device_num(0)",
      "#pragma acc update self(a) async",
      "#pragma acc wait(devnum:d:queues:q1,q2) async(3)",
      "#pragma acc wait(devnum:use_gpu ? 0 : 1)",
      "#pragma acc wait(devnum:use_gpu ? 0 : 1:queues:q ? 2 : 3)",
  };
  for (const std::string &Input : Valid)
    expectValid(Input);
  expectValid("#pragma acc loop gang(num:cond ? 4 : 8, "
              "dim:which ? 1 : 2, static:chunk ? 4 : 8)");

  expectValid("!$ACC PARALLEL LOOP GANG VECTOR", Language::Fortran,
              InputForm::FortranFree);
  expectValid("!$acc routine(foo)", Language::Fortran, InputForm::FortranFree);
  expectValid("!$acc parallel copyout(capture:a)", Language::Fortran,
              InputForm::FortranFree);
  expectValid("!$acc parallel loop &\n!$acc& gang vector", Language::Fortran,
              InputForm::FortranFree);
  expectValid("!$acc parallel &\n!$acc ! ignored continuation comment\n"
              "!$acc& copy(a)",
              Language::Fortran, InputForm::FortranFree);
  expectValid("!$acc para&\n!$acc&llel", Language::Fortran,
              InputForm::FortranFree);
  expectValid("!$acc parallel", Language::Fortran, InputForm::FortranFixed);
  expectValid("!$acc parallel\n!$acc& copy(a)", Language::Fortran,
              InputForm::FortranFixed);
  expectValid("!$acc end parallel loop", Language::Fortran,
              InputForm::FortranFree);
  expectValid("!$acc end loop", Language::Fortran, InputForm::FortranFree);
  expectValid("!$acc end loop", Language::Fortran, InputForm::FortranFixed);
  expectValid("_Pragma(\"acc parallel copy(a[0:n])\")", Language::Cxx,
              InputForm::CPragmaOperator);
  expectValid("#pragma acc routine", Language::Cxx);
  expectValid("_Pragma(\"acc routine(foo) bind(\\\"\\\\x66oo\\\") seq\")",
              Language::Cxx, InputForm::CPragmaOperator);
  expectValid("_Pragma(u8\"acc parallel copy(a)\")", Language::Cxx,
              InputForm::CPragmaOperator);
  expectValid("_Pragma(L\"acc parallel copy(a)\")", Language::Cxx,
              InputForm::CPragmaOperator);
  expectValid("_Pragma(R\"(acc parallel copy(a))\")", Language::Cxx,
              InputForm::CPragmaOperator);
  expectValid("_Pragma(u8R\"tag(acc parallel copy(a))tag\")", Language::Cxx,
              InputForm::CPragmaOperator);
  expectValid("_Pragma(L\"acc parallel copy(a)\")", Language::C,
              InputForm::CPragmaOperator);
  expectValid("_Pragma(u8\"acc parallel copy(a)\")", Language::C,
              InputForm::CPragmaOperator);
  expectValid("_Pragma(u\"acc parallel copy(a)\")", Language::C,
              InputForm::CPragmaOperator);
  expectValid("_Pragma(U\"acc parallel copy(a)\")", Language::C,
              InputForm::CPragmaOperator);
}

void testTypedModel() {
  ParseResult Result = parse("#pragma acc parallel copyin(readonly:a) "
                             "device_type(nvidia) async(1)");
  if (!Result.succeeded()) {
    fail("typed-model specimen did not parse");
    return;
  }
  const auto *General =
      std::get_if<openacc::GeneralDirective>(&Result.directive->payload());
  if (!General || General->defaultClauses.size() != 1 ||
      General->deviceGroups.size() != 1) {
    fail("device-specific clauses were not structurally partitioned");
    return;
  }
  const auto *CopyIn =
      std::get_if<openacc::CopyInClause>(&General->defaultClauses.front());
  if (!CopyIn || CopyIn->modifiers.size() != 1 ||
      CopyIn->modifiers.front() != openacc::CopyInModifier::ReadOnly)
    fail("copyin modifier was not represented by its clause-specific enum");
  if (General->deviceGroups.front().selectors.size() != 1 ||
      General->deviceGroups.front().clauses.size() != 1)
    fail("device_type group lost typed data");

  ParseResult Conditional = parse("#pragma acc loop gang(cond ? 4 : 8)");
  const auto *ConditionalGeneral = Conditional.directive
                                       ? std::get_if<openacc::GeneralDirective>(
                                             &Conditional.directive->payload())
                                       : nullptr;
  const auto *Gang =
      ConditionalGeneral && ConditionalGeneral->defaultClauses.size() == 1
          ? std::get_if<openacc::GangClause>(
                &ConditionalGeneral->defaultClauses.front())
          : nullptr;
  const auto *Positional = Gang && Gang->arguments.size() == 1
                               ? std::get_if<openacc::PositionalGangArgument>(
                                     &Gang->arguments.front())
                               : nullptr;
  if (!Positional || Positional->value.spelling() != "cond ? 4 : 8")
    fail("conditional gang expression was not retained as a positional value");

  ParseResult Wait = parse("#pragma acc wait(devnum:use_gpu ? 0 : 1)");
  const auto *WaitGeneral =
      Wait.directive
          ? std::get_if<openacc::GeneralDirective>(&Wait.directive->payload())
          : nullptr;
  const openacc::WaitArgument *WaitArgument =
      WaitGeneral && WaitGeneral->waitArgument ? &*WaitGeneral->waitArgument
                                               : nullptr;
  if (!WaitArgument || !WaitArgument->deviceNumber || WaitArgument->queues ||
      WaitArgument->deviceNumber->spelling() != "use_gpu ? 0 : 1")
    fail("conditional wait device expression was split into a queue list");

  ParseResult WaitQueues =
      parse("#pragma acc wait(devnum:use_gpu ? 0 : 1:queues:q ? 2 : 3)");
  const auto *WaitQueuesGeneral = WaitQueues.directive
                                      ? std::get_if<openacc::GeneralDirective>(
                                            &WaitQueues.directive->payload())
                                      : nullptr;
  const openacc::WaitArgument *WaitQueuesArgument =
      WaitQueuesGeneral && WaitQueuesGeneral->waitArgument
          ? &*WaitQueuesGeneral->waitArgument
          : nullptr;
  if (!WaitQueuesArgument || !WaitQueuesArgument->deviceNumber ||
      WaitQueuesArgument->deviceNumber->spelling() != "use_gpu ? 0 : 1" ||
      !WaitQueuesArgument->queues || WaitQueuesArgument->queues->size() != 1 ||
      WaitQueuesArgument->queues->values().front().spelling() != "q ? 2 : 3")
    fail("wait queue separator did not preserve conditional expressions");

  ParseResult EndLoop =
      parse("!$acc end loop", Language::Fortran, InputForm::FortranFree);
  const auto *End =
      EndLoop.directive
          ? std::get_if<openacc::EndDirective>(&EndLoop.directive->payload())
          : nullptr;
  if (!EndLoop.succeeded() || !End ||
      End->kind != openacc::EndDirectiveKind::Loop)
    fail("Fortran end loop marker lost its typed construct kind");
}

void testHostFragmentsAndSpelling() {
  expectValid("#pragma acc parallel if(f(a, g(b, c))) "
              "copy(obj.member[lo:len], ptr[(i + 1)])");
  expectValid("#pragma acc parallel /* clause gap */ copy(a/*, ignored */)");
  expectValid("#pragma acc parallel copy /* payload gap */ (a)");
  expectValid("#pragma acc parallel copy/* ) ( ignored */(a)");
  expectValid("#pragma acc parallel // trailing source comment");
  expectValid("#pragma acc parallel wait(devnum:f(a,b):queues:q)");
  ParseResult Fortran = parse("!$ACC PARALLEL COPY(MyArray(Lo:Hi))",
                              Language::Fortran, InputForm::FortranFree);
  if (!Fortran.succeeded()) {
    fail("Fortran case-preservation specimen did not parse");
    return;
  }
  openacc::PrintOptions Options;
  Options.outputForm = InputForm::DirectiveBody;
  std::string Printed = openacc::formatDirective(*Fortran.directive, Options);
  if (Printed.find("MyArray") == std::string::npos)
    fail("Fortran payload spelling was case-folded with structural keywords");

  ParseResult Binding = parse("#pragma acc routine(foo) bind(\"\\x66oo\") seq");
  if (!Binding.succeeded()) {
    fail("escaped bind string specimen did not parse");
  } else if (openacc::formatDirective(*Binding.directive).find("\\x66oo") ==
             std::string::npos) {
    fail("bind string spelling was decoded and reconstructed lossily");
  }
  expectValid("#pragma acc routine(foo) bind(R\"tag(device_name)tag\") seq",
              Language::Cxx);
  expectValid("#pragma acc routine(foo) bind(R\"(gpu\"name)\") seq",
              Language::Cxx);
  expectValid(
      "#pragma acc routine(foo) bind(u8R\"tag(gpu)\"name,/*raw*/)tag\") seq",
      Language::Cxx);
  expectValid("#pragma acc routine(foo) bind(L\"gpu\") seq", Language::C);
  expectValid("#pragma acc routine(foo) bind(u8\"gpu\") seq", Language::C);
  for (std::string_view Prefix : {"R", "LR", "uR", "UR", "u8R"}) {
    std::string Input = "#pragma acc routine(foo) bind(" + std::string(Prefix) +
                        "\"(gpu)\") seq";
    expectValid(Input, Language::Cxx);
    expectInvalid(Input, DiagnosticCode::InvalidValue, Language::C);
  }
  expectValid("#pragma acc parallel if(check(R\"(value, ) quoted)\"))",
              Language::Cxx);
}

void testReductionOperators() {
  for (std::string_view Operator :
       {"+", "-", "*", "max", "min", "&", "|", "^", "&&", "||"})
    expectValid("#pragma acc parallel loop reduction(" + std::string(Operator) +
                ":x)");
  for (std::string_view Operator :
       {"iand", "ior", "ieor", ".and.", ".or.", ".eqv.", ".neqv."})
    expectInvalid("#pragma acc parallel loop reduction(" +
                      std::string(Operator) + ":x)",
                  DiagnosticCode::InvalidValue);

  for (std::string_view Operator : {"+", "*", "max", "min", "iand", "ior",
                                    "ieor", ".and.", ".or.", ".eqv.", ".neqv."})
    expectValid("!$acc parallel loop reduction(" + std::string(Operator) +
                    ":x)",
                Language::Fortran, InputForm::FortranFree);
  expectValid("!$acc parallel loop reduction(.AND.:x)", Language::Fortran,
              InputForm::FortranFree);
  for (std::string_view Operator : {"-", "&", "|", "^", "&&", "||"})
    expectInvalid("!$acc parallel loop reduction(" + std::string(Operator) +
                      ":x)",
                  DiagnosticCode::InvalidValue, Language::Fortran,
                  InputForm::FortranFree);
}

void testCxxTemplateCommas() {
  auto GetNumGangs = [](const ParseResult &Result) {
    const auto *General = Result.directive
                              ? std::get_if<openacc::GeneralDirective>(
                                    &Result.directive->payload())
                              : nullptr;
    if (!General)
      return static_cast<const openacc::NumGangsClause *>(nullptr);
    for (const openacc::Clause &Clause : General->defaultClauses)
      if (const auto *NumGangs = std::get_if<openacc::NumGangsClause>(&Clause))
        return NumGangs;
    return static_cast<const openacc::NumGangsClause *>(nullptr);
  };

  ParseResult Template =
      parse("#pragma acc kernels num_gangs(foo<int, int>())", Language::Cxx);
  const openacc::NumGangsClause *TemplateClause = GetNumGangs(Template);
  if (!Template.succeeded() || !TemplateClause ||
      TemplateClause->values.size() != 1 ||
      TemplateClause->values.values().front().spelling() != "foo<int, int>()")
    fail("a C++ template argument comma split an OpenACC expression list");

  ParseResult Nested =
      parse("#pragma acc kernels num_gangs(foo<std::pair<int, int>>())",
            Language::Cxx);
  const openacc::NumGangsClause *NestedClause = GetNumGangs(Nested);
  if (!Nested.succeeded() || !NestedClause || NestedClause->values.size() != 1)
    fail("a nested C++ template argument comma split an expression list");

  ParseResult Comparison =
      parse("#pragma acc parallel num_gangs(a < b, c > d)", Language::Cxx);
  const openacc::NumGangsClause *ComparisonClause = GetNumGangs(Comparison);
  if (!Comparison.succeeded() || !ComparisonClause ||
      ComparisonClause->values.size() != 2 ||
      ComparisonClause->values.values()[0].spelling() != "a < b" ||
      ComparisonClause->values.values()[1].spelling() != "c > d")
    fail("C++ comparison operators were mistaken for template delimiters");

  ParseResult Shift =
      parse("#pragma acc parallel num_gangs(x >> 1, y)", Language::Cxx);
  const openacc::NumGangsClause *ShiftClause = GetNumGangs(Shift);
  if (!Shift.succeeded() || !ShiftClause || ShiftClause->values.size() != 2)
    fail("a C++ shift operator was mistaken for a template delimiter");

  ParseResult CompactOperators =
      parse("#pragma acc parallel num_gangs(a<b, c>>d)", Language::Cxx);
  const openacc::NumGangsClause *CompactClause = GetNumGangs(CompactOperators);
  if (!CompactOperators.succeeded() || !CompactClause ||
      CompactClause->values.size() != 2)
    fail("adjacent C++ comparisons and shifts looked like a template-id");
}

void testImplementationClauses() {
  expectValid("#pragma acc parallel __vendor_hint(x)");
  expectValid("#pragma acc parallel __vendor_flag");
  expectValid("!$acc parallel __VENDOR_HINT(x)", Language::Fortran,
              InputForm::FortranFree);
  expectInvalid("#pragma acc parallel _vendor_hint(x)",
                DiagnosticCode::UnknownClause);
  expectInvalid("#pragma acc data __vendor_hint(x)",
                DiagnosticCode::MissingClause);
}

void testDiagnostics() {
  expectInvalid("#pragma omp parallel", DiagnosticCode::InvalidEnvelope);
  expectInvalid("#pragma acc mystery", DiagnosticCode::UnknownDirective);
  expectInvalid("#pragma acc parallel mystery(a)",
                DiagnosticCode::UnknownClause);
  expectInvalid("#pragma acc parallel indirect(a)",
                DiagnosticCode::UnsupportedExtension);
  expectInvalid("#pragma acc parallel copyin(zero:a)",
                DiagnosticCode::InvalidModifier);
  expectInvalid("#pragma acc declare copy(capture:a)",
                DiagnosticCode::InvalidModifier);
  expectInvalid("#pragma acc declare copyout(capture:a)",
                DiagnosticCode::InvalidModifier);
  expectInvalid("#pragma acc parallel copyout(alwaysin:a)",
                DiagnosticCode::InvalidModifier);
  expectInvalid("#pragma acc parallel copy(a,)", DiagnosticCode::TrailingComma);
  expectInvalid("#pragma acc parallel copy(a",
                DiagnosticCode::UnterminatedDelimiter);
  expectInvalid("#pragma acc parallel /* unterminated",
                DiagnosticCode::UnterminatedDelimiter);
  expectInvalid("#pragma acc loop seq vector",
                DiagnosticCode::InvalidCombination);
  expectInvalid("#pragma acc atomic read write",
                DiagnosticCode::InvalidCombination);
  expectInvalid("#pragma acc data", DiagnosticCode::MissingClause);
  expectInvalid("#pragma acc routine(foo) bind() seq",
                DiagnosticCode::MissingArgument);
  expectInvalid("#pragma acc routine gang worker",
                DiagnosticCode::InvalidCombination);
  expectInvalid("#pragma acc set device_num(0) device_num(1)",
                DiagnosticCode::DuplicateClause);
  expectInvalid("#pragma acc parallel device_type(nvidia) copy(a)",
                DiagnosticCode::InvalidClauseOrder);
  expectInvalid("#pragma acc parallel", DiagnosticCode::InvalidLanguageForm,
                Language::Fortran, InputForm::CPragma);
  expectInvalid("#pragma acc end loop", DiagnosticCode::InvalidLanguageForm);
  expectInvalid("!$acc parallel &\n!$acc ! ignored continuation comment",
                DiagnosticCode::InvalidLanguageForm, Language::Fortran,
                InputForm::FortranFree);
  expectInvalid("_Pragma(R\"tag(acc parallel)bad\")",
                DiagnosticCode::UnterminatedString, Language::Cxx,
                InputForm::CPragmaOperator);
  expectInvalid("_Pragma(R\"(acc parallel)\")", DiagnosticCode::InvalidEnvelope,
                Language::C, InputForm::CPragmaOperator);
  expectInvalid("#pragma acc parallel num_gangs(2,2) reduction(+:x)",
                DiagnosticCode::InvalidCombination);
  expectInvalid("#pragma acc parallel loop num_gangs(2,2) gang reduction(+:x)",
                DiagnosticCode::InvalidCombination);
  expectInvalid("#pragma acc parallel loop reduction(+:x) "
                "device_type(nvidia) num_gangs(2,2) gang",
                DiagnosticCode::InvalidCombination);

  ParseResult Result = parse("#pragma acc parallel copyin(zero:a)");
  if (Result.diagnostics.empty() ||
      Result.diagnostics.front().range.begin.line != 1 ||
      Result.diagnostics.front().range.begin.column == 0)
    fail("diagnostic did not carry a one-based source location");
}

void testCombinedLoopParentRestrictions() {
  expectValid("#pragma acc parallel loop gang(dim:2, static:*) worker vector");
  expectValid("#pragma acc parallel loop num_gangs(8) num_workers(4) "
              "vector_length(32) gang(dim:2) worker vector");
  expectInvalid("#pragma acc parallel loop gang(4)",
                DiagnosticCode::UnexpectedArgument);
  expectInvalid("#pragma acc parallel loop gang(num:4)",
                DiagnosticCode::UnexpectedArgument);
  expectInvalid("#pragma acc parallel loop worker(4)",
                DiagnosticCode::UnexpectedArgument);
  expectInvalid("#pragma acc parallel loop worker(num:4)",
                DiagnosticCode::UnexpectedArgument);
  expectInvalid("#pragma acc parallel loop vector(32)",
                DiagnosticCode::UnexpectedArgument);
  expectInvalid("#pragma acc parallel loop vector(length:32)",
                DiagnosticCode::UnexpectedArgument);
  expectInvalid("#pragma acc parallel loop device_type(nvidia) worker(4)",
                DiagnosticCode::UnexpectedArgument);

  expectValid("#pragma acc serial loop gang(dim:2) worker vector");
  expectInvalid("#pragma acc serial loop gang(num:4)",
                DiagnosticCode::UnexpectedArgument);
  expectInvalid("#pragma acc serial loop worker(4)",
                DiagnosticCode::UnexpectedArgument);
  expectInvalid("#pragma acc serial loop vector(32)",
                DiagnosticCode::UnexpectedArgument);
  expectInvalid("#pragma acc serial loop device_type(nvidia) vector(32)",
                DiagnosticCode::UnexpectedArgument);

  expectValid("#pragma acc kernels loop gang(4) worker(8) vector(32)");
  expectValid("#pragma acc kernels loop gang(num:4) worker(num:8) "
              "vector(length:32)");
  expectValid("#pragma acc kernels loop num_gangs(8) num_workers(4) "
              "vector_length(32) gang(static:*) worker vector");
  expectInvalid("#pragma acc kernels loop gang(dim:2)",
                DiagnosticCode::UnexpectedArgument);
  expectInvalid("#pragma acc kernels loop num_gangs(8) gang(4)",
                DiagnosticCode::InvalidCombination);
  expectInvalid("#pragma acc kernels loop num_workers(8) worker(4)",
                DiagnosticCode::InvalidCombination);
  expectInvalid("#pragma acc kernels loop vector_length(32) vector(16)",
                DiagnosticCode::InvalidCombination);

  expectValid("#pragma acc kernels loop device_type(nvidia) num_gangs(8) "
              "device_type(radeon) gang(4)");
  expectValid("#pragma acc kernels loop gang(4) device_type(nvidia) gang");
  expectInvalid("#pragma acc kernels loop num_gangs(8) "
                "device_type(nvidia) gang(4)",
                DiagnosticCode::InvalidCombination);
  expectInvalid("#pragma acc kernels loop gang(4) "
                "device_type(nvidia) num_gangs(8)",
                DiagnosticCode::InvalidCombination);
  expectInvalid("#pragma acc kernels loop device_type(nvidia) num_gangs(8) "
                "device_type(nvidia) gang(4)",
                DiagnosticCode::InvalidCombination);
  expectInvalid("#pragma acc parallel loop reduction(+:x) "
                "device_type(nvidia) num_gangs(2,2) "
                "device_type(nvidia) gang",
                DiagnosticCode::InvalidCombination);

  ParseResult RepeatedDefault = parse("#pragma acc parallel loop worker(4) "
                                      "device_type(nvidia) collapse(2) "
                                      "device_type(radeon) tile(2)");
  if (countCode(RepeatedDefault, DiagnosticCode::UnexpectedArgument) != 1)
    fail("an inherited invalid loop argument was diagnosed more than once");
}

void testDeviceSpecificOverride() {
  expectValid("#pragma acc loop gang device_type(nvidia) gang(num:4)");
  expectValid("#pragma acc loop device_type(nvidia) collapse(2)");
  expectValid("#pragma acc parallel loop device_type(nvidia) collapse(2)");
  expectValid("#pragma acc serial loop device_type(nvidia) collapse(2)");
  expectValid("#pragma acc kernels loop device_type(nvidia) collapse(2)");
  expectValid("!$acc loop device_type(nvidia) collapse(2)", Language::Fortran,
              InputForm::FortranFree);
  expectValid("#pragma acc routine device_type(nvidia) gang "
              "device_type(radeon) worker");
  expectValid("#pragma acc routine device_type(nvidia) gang "
              "device_type(*) worker");
  expectValid("#pragma acc loop device_type(nvidia) gang "
              "device_type(nvidia) vector");
  expectValid("#pragma acc routine device_type(nvidia) gang "
              "device_type(nvidia) bind(backend)");
  expectValid("!$acc routine device_type(nvidia) gang &\n"
              "!$acc& device_type(radeon) worker",
              Language::Fortran, InputForm::FortranFree);
  expectValid("#pragma acc routine device_type(nvidia) gang "
              "device_type(radeon)");
  expectInvalid("#pragma acc routine gang device_type(nvidia) worker",
                DiagnosticCode::InvalidCombination);
  expectInvalid("#pragma acc routine device_type(nvidia) gang "
                "device_type(nvidia) worker",
                DiagnosticCode::InvalidCombination);
  expectInvalid("#pragma acc routine device_type(nvidia, radeon) gang "
                "device_type(nvidia) worker",
                DiagnosticCode::InvalidCombination);
  expectInvalid("!$acc routine device_type(NVIDIA) gang &\n"
                "!$acc& device_type(nvidia) worker",
                DiagnosticCode::InvalidCombination, Language::Fortran,
                InputForm::FortranFree);
  expectInvalid("#pragma acc loop device_type(nvidia) seq "
                "device_type(nvidia) vector",
                DiagnosticCode::InvalidCombination);

  ParseResult Spelled = parse("#pragma acc loop gang(4) vector(32)");
  ParseResult Named = parse("#pragma acc loop gang(num:4) vector(length:32)");
  if (!Spelled.succeeded() || !Named.succeeded() || !Spelled.directive ||
      !Named.directive || !Spelled.directive->semanticEquals(*Named.directive))
    fail("optional argument keywords changed AST semantics");
}

void testNoCrossParseState() {
  ParseResult First = parse("#pragma acc parallel copy(a)");
  ParseResult Second = parse("#pragma acc parallel copy(b)");
  if (!First.succeeded() || !Second.succeeded()) {
    fail("independent parses failed");
    return;
  }
  openacc::PrintOptions Options;
  Options.outputForm = InputForm::DirectiveBody;
  std::string FirstText = openacc::formatDirective(*First.directive, Options);
  std::string SecondText = openacc::formatDirective(*Second.directive, Options);
  if (FirstText.find("b") != std::string::npos ||
      SecondText.find("a)") != std::string::npos)
    fail("parse results leaked clause data across calls");
}

void testConcurrency() {
  std::atomic<bool> Failed{false};
  std::vector<std::thread> Threads;
  for (int Thread = 0; Thread < 8; ++Thread) {
    Threads.emplace_back([Thread, &Failed] {
      for (int I = 0; I < 100; ++I) {
        std::string Input = "#pragma acc parallel loop gang vector private(v" +
                            std::to_string(Thread) + ")";
        ParseResult Result = parse(Input);
        if (!Result.succeeded()) {
          Failed = true;
          return;
        }
      }
    });
  }
  for (std::thread &Thread : Threads)
    Thread.join();
  if (Failed)
    fail("concurrent parse calls were not independent");
}

} // namespace

int main() {
  testDirectiveFamilies();
  testTypedModel();
  testHostFragmentsAndSpelling();
  testReductionOperators();
  testCxxTemplateCommas();
  testImplementationClauses();
  testDiagnostics();
  testCombinedLoopParentRestrictions();
  testDeviceSpecificOverride();
  testNoCrossParseState();
  testConcurrency();
  return Failures == 0 ? 0 : 1;
}
