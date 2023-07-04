#include "parser/parser.h"

#include <sys/types.h>

#include <bit>
#include <cstdint>
#include <cstring>
#include <iterator>
#include <string>
#include <utility>
#include <variant>
#include <vector>

#include "absl/container/flat_hash_set.h"
#include "absl/strings/str_format.h"
#include "base/bytes.h"
#include "base/coro.h"
#include "base/stream.h"
#include "core/ast.h"
#include "core/instruction.h"
#include "leb128/leb128.h"
#include "parser/validator.h"

namespace wasmcc {

namespace {

constexpr size_t MAX_FUNCTIONS = 1U << 16U;
constexpr size_t MAX_FUNCTION_LOCALS = 1U << 8U;
// These are currently set so that we're always passing everything into
// registers.
constexpr size_t kMaxFunctionParameters = 6;
constexpr size_t kMaxFunctionResults = 1;

constexpr size_t kMaxFunctionSignatures = 1U << 17U;
constexpr size_t kMaxImports = 1U << 8U;
// NOTE: Tables and memories both have maximums for an individual entity, but
// those are validated at runtime.
constexpr size_t kMaxTables = 1U << 4U;
constexpr size_t kMaxMemories = 1;
constexpr size_t kMaxGlobals = 1U << 10U;
constexpr size_t kMaxExports = 1U << 8U;
constexpr size_t kMaxNameLength = 1U << 8U;

ValType ParseValType(Stream* parser) {
  auto type_id = parser->ReadByte();
  switch (type_id) {
    case uint8_t(ValType::kI32):
    case uint8_t(ValType::kI64):
    case uint8_t(ValType::kF32):
    case uint8_t(ValType::kF64):
    case uint8_t(ValType::kFuncRef):
    case uint8_t(ValType::kExternRef):
      return ValType(type_id);
    default:
      throw ParseException(absl::StrFormat("unknown valtype: %x", type_id));
  }
}

template <typename NamedInteger, typename Vector>
void ValidateInRange(std::string_view msg, NamedInteger idx, const Vector& v) {
  if (idx.value() < 0 || idx.value() >= v.size()) {
    throw ParseException(absl::StrFormat("%s out of range - %d ∉ [0, %d)", msg,
                                         idx.value(), v.size()));
  }
}

Name ParseName(Stream* parser) {
  auto str_len = leb128::decode<uint32_t>(parser);
  if (str_len > kMaxNameLength) {
    throw ParseException(absl::StrFormat("name too long: %d", str_len));
  }
  bytes b(str_len, 0);
  auto amt = parser->ReadBytes(b);
  if (amt != str_len) {
    throw EndOfStreamException();
  }
  // TODO: Validate UTF8
  std::string s(b.begin(), b.end());
  return Name(std::move(s));
}

TypeIdx ParseTypeIdx(Stream* parser) {
  return TypeIdx(leb128::decode<uint32_t>(parser));
}

FuncIdx ParseFuncIdx(Stream* parser) {
  return FuncIdx(leb128::decode<uint32_t>(parser));
}
TableIdx ParseTableIdx(Stream* parser) {
  return TableIdx(leb128::decode<uint32_t>(parser));
}
MemIdx ParseMemIdx(Stream* parser) {
  return MemIdx(leb128::decode<uint32_t>(parser));
}
GlobalIdx ParseGlobalIdx(Stream* parser) {
  return GlobalIdx(leb128::decode<uint32_t>(parser));
}

template <size_t kMax>
std::vector<ValType> ParseSignatureTypes(Stream* parser) {
  auto vector_size = leb128::decode<uint32_t>(parser);
  if (vector_size > kMax) {
    throw ModuleTooLargeException(
        absl::StrFormat("too many parameters to function: %d", vector_size));
  }
  std::vector<ValType> result_type;
  result_type.reserve(vector_size);
  for (uint32_t i = 0; i < vector_size; ++i) {
    result_type.push_back(ParseValType(parser));
  }
  return result_type;
}

FunctionSignature ParseSignature(Stream* parser) {
  auto magic = parser->ReadByte();
  // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers)
  if (magic != 0x60) {
    throw ParseException(
        absl::StrFormat("function type magic mismatch: %x", magic));
  }
  auto parameter_types = ParseSignatureTypes<kMaxFunctionParameters>(parser);
  auto result_types = ParseSignatureTypes<kMaxFunctionResults>(parser);
  return {.parameter_types = std::move(parameter_types),
          .result_types = std::move(result_types)};
}

Limits ParseLimits(Stream* parser) {
  if (parser->ReadByte()) {
    auto min = leb128::decode<uint32_t>(parser);
    auto max = leb128::decode<uint32_t>(parser);
    return {.min = min, .max = max};
  } else {
    auto min = leb128::decode<uint32_t>(parser);
    return {.min = min, .max = std::numeric_limits<uint32_t>::max()};
  }
}

TableType ParseTableType(Stream* parser) {
  auto reftype = ParseValType(parser);
  if (reftype != ValType::kExternRef && reftype != ValType::kFuncRef) {
    throw ParseException(
        absl::StrFormat("invalid tabletype type: %x", reftype));
  }
  auto limits = ParseLimits(parser);
  return {.limits = limits, .reftype = reftype};
}

MemType ParseMemType(Stream* parser) { return {.limits = ParseLimits(parser)}; }

GlobalType ParseGlobalType(Stream* parser) {
  auto valtype = ParseValType(parser);
  auto mut = parser->ReadByte();
  return {.valtype = valtype, .mut = bool(mut)};
}

struct Code {
  std::vector<ValType> locals;
  std::vector<Instruction> body;
};

/**
 * module_builder is responible for parsing the binary representation of a WASM
 * module and also enforcing various limits we've enforced.
 *
 * It also validates modules are well formed, although not all of that is done
 * directly. More complex validation is delegated to other helper classes.
 *
 * This class has "one and done" usage.
 *
 * Spec: https://webassembly.github.io/spec/core/binary/index.html
 */
class ModuleBuilder {
 public:
  ModuleBuilder() = default;
  ModuleBuilder(const ModuleBuilder&) = delete;
  ModuleBuilder& operator=(const ModuleBuilder&) = delete;
  ModuleBuilder(ModuleBuilder&&) = delete;
  ModuleBuilder& operator=(ModuleBuilder&&) = delete;
  ~ModuleBuilder() = default;

  co::Future<> Parse(Stream* parser);

  co::Future<ParsedModule> Build();

 private:
  // The main loop of parsing functions.
  //
  // At a high level from the spec. The sections are as follows and parsed in
  // order:
  // - type signatures
  // - imports
  // - function forward declarations
  // - tables
  // - memories
  // - globals
  // - exports
  // - start function
  // - elements (initializer code for tables)
  // - code (the bodies of functions that were forward declared)
  // - data (initializer code for memories)
  //
  // Around any of these sections can be a custom section, which we
  // currently ignore.
  co::Future<> ParseOneSection(Stream* parser);

  // Parses the first section, which is made up of function signatures.
  co::Future<> ParseSignatureSection(Stream*);

  // Parses the forward declarations of functions.
  co::Future<> ParseFunctionDeclarationSection(Stream*);

  // Parses the imports for this module.
  //
  // NOTE: this does not validate that the imports exist. That will need to be
  // done at a later phase (or maybe that should be inputs to this..?)
  ModuleImport ParseOneImport(Stream*);
  co::Future<> ParseImportSection(Stream*);

  co::Future<> ParseTableSection(Stream*);

  co::Future<> ParseMemoriesSection(Stream*);

  co::Future<> ParseGlobalsSection(Stream*);

  ModuleExport ParseOneExport(Stream*);
  co::Future<> ParseExportsSection(Stream*);

  // Parse a function body
  void ParseOneCode(Stream*, Function*);
  co::Future<> ParseCodeSection(Stream*);

  // In order to properly be able to stream parsing of modules, we need to
  // ensure everything is created in the correct order. The spec enforces that
  // modules are in order to achieve this usecase. This keeps track of that
  // bit so we can correctly enforce wellformed modules.
  //
  // NOTE: Custom sections are allowed to be anywhere.
  size_t _latest_section_read = 0;

  std::vector<FunctionSignature> _func_signatures;
  std::vector<ModuleImport> _imports;
  std::vector<Function> _functions;
  std::vector<Table> _tables;
  std::vector<Mem> _memories;
  std::vector<Global> _globals;
  std::vector<ModuleExport> _exports;
  std::optional<FuncIdx> _start;
};

co::Future<ParsedModule> ModuleBuilder::Build() {
  ParsedModule parsed;
  std::swap(_functions, parsed.functions);
  for (const auto& exprt : _exports) {
    if (std::holds_alternative<FuncIdx>(exprt.description)) {
      parsed.exported_functions.emplace(exprt.name,
                                        std::get<FuncIdx>(exprt.description));
    }
  }
  co_return parsed;
}

co::Future<> ModuleBuilder::ParseSignatureSection(Stream* parser) {
  auto vector_size = leb128::decode<uint32_t>(parser);
  if (vector_size > kMaxFunctionSignatures) {
    throw ModuleTooLargeException(
        absl::StrFormat("too large of type section: %d, max: %d", vector_size,
                        kMaxFunctionSignatures));
  }
  for (uint32_t i = 0; i < vector_size; ++i) {
    _func_signatures.push_back(ParseSignature(parser));
    co_await co::MaybeYield();
  }
}

co::Future<> ModuleBuilder::ParseFunctionDeclarationSection(Stream* parser) {
  auto vector_size = leb128::decode<uint32_t>(parser);
  if (vector_size > MAX_FUNCTIONS) {
    throw ModuleTooLargeException(absl::StrFormat(
        "too many functions: %d, max: %d", vector_size, MAX_FUNCTIONS));
  }
  for (uint32_t i = 0; i < vector_size; ++i) {
    auto funcidx = ParseFuncIdx(parser);
    ValidateInRange("unknown function signature", funcidx, _func_signatures);
    _functions.push_back({.meta = {
                              .signature = _func_signatures[funcidx.value()],
                          }});
    co_await co::MaybeYield();
  }
}

ModuleImport ModuleBuilder::ParseOneImport(Stream* parser) {
  auto module_name = ParseName(parser);
  auto name = ParseName(parser);
  auto type = parser->ReadByte();
  std::optional<ModuleImport::Description> desc;
  switch (type) {
    case 0x00: {  // func
      auto funcidx = ParseTypeIdx(parser);
      ValidateInRange("unknown import function signature", funcidx,
                      _func_signatures);
      desc = funcidx;
      break;
    }
    case 0x01:  // table
      desc = ParseTableType(parser);
      break;
    case 0x02:  // memory
      desc = ParseMemType(parser);
      break;
    case 0x03:  // global
      desc = ParseGlobalType(parser);
      break;
    default:
      throw ParseException(absl::StrFormat("unknown import type: %x", type));
  }
  // TODO: Validate the import
  return {.module_name = std::move(module_name),
          .name = std::move(name),
          .description = desc.value()};
}

co::Future<> ModuleBuilder::ParseImportSection(Stream* parser) {
  auto vector_size = leb128::decode<uint32_t>(parser);
  if (vector_size > kMaxImports) {
    throw ModuleTooLargeException(absl::StrFormat(
        "too many imports: %d, max: %d", vector_size, kMaxImports));
  }

  for (uint32_t i = 0; i < vector_size; ++i) {
    _imports.push_back(ParseOneImport(parser));
    co_await co::MaybeYield();
  }
}

Table parse_table(Stream* parser) { return {.type = ParseTableType(parser)}; }

co::Future<> ModuleBuilder::ParseTableSection(Stream* parser) {
  auto vector_size = leb128::decode<uint32_t>(parser);
  if (vector_size > kMaxTables) {
    throw ModuleTooLargeException(absl::StrFormat(
        "too many tables: %d, max: %d", vector_size, kMaxTables));
  }

  for (uint32_t i = 0; i < vector_size; ++i) {
    _tables.push_back(parse_table(parser));
    co_await co::MaybeYield();
  }
}

Mem parse_memory(Stream* parser) { return {.type = ParseMemType(parser)}; }

co::Future<> ModuleBuilder::ParseMemoriesSection(Stream* parser) {
  auto vector_size = leb128::decode<uint32_t>(parser);
  if (vector_size > kMaxMemories) {
    throw ModuleTooLargeException(absl::StrFormat(
        "too many memories: %d, max: %d", vector_size, kMaxMemories));
  }

  for (uint32_t i = 0; i < vector_size; ++i) {
    _memories.push_back(parse_memory(parser));
    co_await co::MaybeYield();
  }
}

Value parse_const_expr(Stream* parser) {
  auto opcode = parser->ReadByte();
  // NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers)
  switch (opcode) {
    case 0x41:
      return Value::U32(leb128::decode<uint32_t>(parser));
    case 0x42:
      return Value::U64(leb128::decode<uint64_t>(parser));
    case 0x43: {
      bytes b(sizeof(float), 0);
      size_t amt = parser->ReadBytes(b);
      if (amt != b.size()) {
        throw EndOfStreamException();
      }
      float result = 0;
      std::memcpy(&result, b.data(), b.size());
      return Value::F32(result);
    }
    case 0x44: {
      bytes b(sizeof(double), 0);
      size_t amt = parser->ReadBytes(b);
      if (amt != b.size()) {
        throw EndOfStreamException();
      }
      double result = 0;
      std::memcpy(&result, b.data(), b.size());
      return Value::F64(result);
    }
    default:
      // TODO: Support refs, other global references, and vectors
      throw ParseException(
          absl::StrFormat("unimplemented global value: %x", opcode));
  }
  // NOLINTEND(cppcoreguidelines-avoid-magic-numbers)
}

Global parse_global(Stream* parser) {
  auto type = ParseGlobalType(parser);
  auto value = parse_const_expr(parser);
  return {.type = type, .value = value};
}

co::Future<> ModuleBuilder::ParseGlobalsSection(Stream* parser) {
  auto vector_size = leb128::decode<uint32_t>(parser);
  if (vector_size > kMaxGlobals) {
    throw ModuleTooLargeException(absl::StrFormat(
        "too many globals: %d, max: %d", vector_size, kMaxGlobals));
  }

  for (uint32_t i = 0; i < vector_size; ++i) {
    _globals.push_back(parse_global(parser));
    co_await co::MaybeYield();
  }
}

ModuleExport ModuleBuilder::ParseOneExport(Stream* parser) {
  auto name = ParseName(parser);
  auto type = parser->ReadByte();
  std::optional<ModuleExport::Description> desc;
  switch (type) {
    case 0x00: {  // func
      auto idx = ParseFuncIdx(parser);
      ValidateInRange("unknown function export", idx, _functions);
      desc = idx;
      break;
    }
    case 0x01: {  // table
      auto idx = ParseTableIdx(parser);
      ValidateInRange("unknown function export", idx, _tables);
      desc = idx;
      break;
    }
    case 0x02: {  // memory
      auto idx = ParseMemIdx(parser);
      ValidateInRange("unknown memory export", idx, _memories);
      desc = idx;
      break;
    }
    case 0x03: {  // global
      auto idx = ParseGlobalIdx(parser);
      ValidateInRange("unknown global export", idx, _globals);
      desc = idx;
      break;
    }
    default:
      throw ParseException(absl::StrFormat("unknown export type: %d", type));
  }
  return {.name = std::move(name), .description = desc.value()};
}

co::Future<> ModuleBuilder::ParseExportsSection(Stream* parser) {
  auto vector_size = leb128::decode<uint32_t>(parser);
  if (vector_size > kMaxExports) {
    throw ModuleTooLargeException(absl::StrFormat(
        "too many exports: %d, max: %d", vector_size, kMaxExports));
  }
  absl::flat_hash_set<std::string_view> names;
  for (uint32_t i = 0; i < vector_size; ++i) {
    _exports.push_back(ParseOneExport(parser));
    auto [it, inserted] = names.insert(_exports.back().name.value());
    if (!inserted) [[unlikely]] {
      throw ParseException(absl::StrFormat("duplicate exported name: %s", *it));
    }
    co_await co::MaybeYield();
  }
}

std::vector<Instruction> ParseExpression(Stream* parser,
                                         FunctionValidator* validator) {
  std::vector<Instruction> instruction_vector;
  // NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers)
  for (auto opcode = parser->ReadByte(); opcode != 0x0B;
       opcode = parser->ReadByte()) {
    std::optional<Instruction> i;
    switch (opcode) {
      case 0x0F:  // return
        i = op::Return();
        break;
      case 0x20: {  // get_local_i32
        auto idx = leb128::decode<uint32_t>(parser);
        i = op::GetLocalI32(idx);
        break;
      }
      case 0x21: {  // set_local_i32
        auto idx = leb128::decode<uint32_t>(parser);
        i = op::SetLocalI32(idx);
        break;
      }
      case 0x41: {  // const_i32
        auto v = leb128::decode<uint32_t>(parser);
        i = op::ConstI32(Value::U32(v));
        break;
      }
      case 0x6A:  // add_i32
        i = op::AddI32();
        break;
      default:
        throw ParseException(absl::StrFormat("unsupported opcode: %d", opcode));
    }
    // Validate the instruction is good.
    std::visit(*validator, i.value());
    instruction_vector.push_back(i.value());
  }
  // NOLINTEND(cppcoreguidelines-avoid-magic-numbers)
  return instruction_vector;
}

void ModuleBuilder::ParseOneCode(Stream* parser, Function* func) {
  auto expected_size = leb128::decode<uint32_t>(parser);
  auto start_position = parser->BytesConsumed();

  auto vector_size = leb128::decode<uint32_t>(parser);
  for (uint32_t i = 0; i < vector_size; ++i) {
    auto num_locals = leb128::decode<uint32_t>(parser);
    if (num_locals + func->meta.locals.size() > MAX_FUNCTION_LOCALS) {
      throw ModuleTooLargeException(absl::StrFormat(
          "too many locals: %d", num_locals + func->meta.locals.size()));
    }
    auto valtype = ParseValType(parser);
    std::fill_n(std::back_inserter(func->meta.locals), num_locals, valtype);
  }
  FunctionValidator validator(func->meta.signature, func->meta.locals);
  func->body = ParseExpression(parser, &validator);
  func->meta.max_stack_size_bytes = validator.maximum_stack_size_bytes();
  func->meta.max_stack_elements = validator.maximum_stack_elements();

  auto actual = parser->BytesConsumed() - start_position;
  if (actual != expected_size) {
    throw ParseException(
        absl::StrFormat("unexpected size of function, actual: %d expected: %d",
                        actual, expected_size));
  }
}

co::Future<> ModuleBuilder::ParseCodeSection(Stream* parser) {
  auto vector_size = leb128::decode<uint32_t>(parser);
  // We don't need to check the max size because we did that for _functions
  if (vector_size != _functions.size()) {
    throw ParseException(
        absl::StrFormat("unexpected number of code, actual: %d expected: %d",
                        vector_size, _functions.size()));
  }
  // Check the number vs the function section
  for (uint32_t i = 0; i < vector_size; ++i) {
    auto& fn = _functions[i];
    ParseOneCode(parser, &fn);
    co_await co::MaybeYield();
  }
}

co::Future<> ModuleBuilder::ParseOneSection(Stream* parser) {
  auto id = parser->ReadByte();

  if (id != 0 && id <= _latest_section_read) {
    throw ParseException(
        absl::StrFormat("invalid section order, section id %d is after id %d",
                        id, _latest_section_read));
  } else if (id != 0) {
    // Custom sections are allowed anywhere, and other sections need to
    // ensure are read in order.
    _latest_section_read = id;
  }
  auto size = leb128::decode<uint32_t>(parser);
  // NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers)
  switch (id) {
    case 0x00:  // Custom section
      // Skip over custom sections for now
      // TODO: Support debug symbols if included.
      parser->Skip(size);
      co_return;
    case 0x01:  // type section
      co_await ParseSignatureSection(parser);
      co_return;
    case 0x02:  // import section
      co_await ParseImportSection(parser);
      co_return;
    case 0x03:  // function section
      co_await ParseFunctionDeclarationSection(parser);
      co_return;
    case 0x04:  // table section
      co_await ParseTableSection(parser);
      co_return;
    case 0x05:  // memory section
      co_await ParseMemoriesSection(parser);
      co_return;
    case 0x06:  // global section
      co_await ParseGlobalsSection(parser);
      co_return;
    case 0x07:  // export section
      co_await ParseExportsSection(parser);
      co_return;
    case 0x08: {  // start section
      auto start_funcidx = ParseFuncIdx(parser);
      ValidateInRange("start function", start_funcidx, _functions);
      _start = start_funcidx;
      co_return;
    }
    case 0x09:  // element section
      // TODO: Implement me
      throw ParseException("tables unimplemented");
    case 0x0A:  // code section
      co_await ParseCodeSection(parser);
      co_return;
    case 0x0B:  // data section
    case 0x0C:  // data count section
      // TODO: Implement me
      throw ParseException("memories unimplemented");
    default:
      throw ParseException(absl::StrFormat("unknown section id: %d", id));
  }
  // NOLINTEND(cppcoreguidelines-avoid-magic-numbers)
}

co::Future<> ModuleBuilder::Parse(Stream* parser) {
  bytes magic(4, 0);
  size_t amt = parser->ReadBytes(magic);
  if (amt != magic.size()) {
    throw EndOfStreamException();
  }
  static const bytes kMagicBytes = bytes({0x00, 0x61, 0x73, 0x6D});
  if (magic != kMagicBytes) {
    throw ParseException(absl::StrFormat("magic bytes mismatch: %x %x %x %x",
                                         magic[0], magic[1], magic[2],
                                         magic[3]));
  }
  amt = parser->ReadBytes(magic);
  if (amt != magic.size()) {
    throw EndOfStreamException();
  }
  static const bytes kVersionOne = bytes({0x01, 0x00, 0x00, 0x00});
  if (magic != kVersionOne) {
    throw ParseException("unsupported wasm version");
  }
  while (parser->HasRemaining()) {
    co_await ParseOneSection(parser);
    co_await co::MaybeYield();
  }
}

}  // namespace

co::Future<ParsedModule> ParseModule(Stream* stream) {
  ModuleBuilder builder;
  co_await builder.Parse(stream);
  co_return co_await builder.Build();
}

}  // namespace wasmcc
