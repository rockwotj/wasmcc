#pragma once

#include <cstdint>
#include <variant>
#include <vector>

#include "absl/container/flat_hash_map.h"
#include "base/bytes.h"
#include "base/named_type.h"
#include "core/instruction.h"
#include "core/value.h"

namespace wasmcc {

struct FunctionSignature {
  std::vector<ValType> parameter_types;
  std::vector<ValType> result_types;

  friend bool operator==(const FunctionSignature&,
                         const FunctionSignature&) = default;
};

struct Limits {
  uint32_t min;
  uint32_t max;  // Empty maximums use numeric_limits::max
};

using Name = NamedType<std::string, struct NameTag>;
using TypeIdx = NamedType<uint32_t, struct TypeIdxTag>;
using FuncIdx = NamedType<uint32_t, struct FuncIdxTag>;
using LocalIdx = NamedType<uint32_t, struct LocalIdxTag>;
using TableIdx = NamedType<uint32_t, struct TableIdxTag>;
using MemIdx = NamedType<uint32_t, struct MemIdxTag>;
using GlobalIdx = NamedType<uint32_t, struct GlobalIdxTag>;

struct TableType {
  Limits limits;
  ValType reftype;  // funcref | externref
};

struct MemType {
  Limits limits;
};

struct GlobalType {
  ValType valtype;
  bool mut;
};

struct ModuleImport {
  using Description = std::variant<TypeIdx, TableType, MemType, GlobalType>;
  Name module_name;
  Name name;
  Description description;
};

struct Table {
  TableType type;
};

struct Mem {
  MemType type;
};

struct Global {
  GlobalType type;
  Value value;
};

struct ModuleExport {
  using Description = std::variant<FuncIdx, TableIdx, MemIdx, GlobalIdx>;
  Name name;
  Description description;
};

struct Function {
  struct Metadata {
    FunctionSignature signature;
    std::vector<ValType> locals;
    uint32_t max_stack_size_bytes;
    uint32_t max_stack_elements;
  };
  Metadata meta;
  std::vector<Instruction> body;
};

struct ParsedModule {
  std::vector<Function> functions;
  absl::flat_hash_map<Name, FuncIdx> exported_functions;
};

}  // namespace wasmcc
