#pragma once

#include <exception>

#include "core/ast.h"
#include "core/instruction.h"
#include "core/value.h"

namespace wasmcc {

// Thrown if the wasm function is ill-formed.
class ValidationException : public std::exception {};

// A representation of the possible types instructions can take consume and
// produce. Some instructions are polymorphic, hence the need for additional
// information on top of `valtype`.
//
// Spec ref:
// https://webassembly.github.io/spec/core/valid/instructions.html#polymorphism
class ValidationType {
 public:
  explicit ValidationType(ValType);

  static ValidationType any();
  static ValidationType anyref();

  bool is_i32() const;
  bool is_any() const;
  bool is_anyref() const;
  bool is_ref() const;
  size_t size_bytes() const;

  friend bool operator==(ValidationType, ValidationType) = default;
  friend std::ostream& operator<<(std::ostream&, ValidationType);

 private:
  ValidationType() = default;

  static constexpr uint8_t kAnyTypeValue = 0;
  static constexpr uint8_t kAnyRefTypeValue = 1;

  // This is a valtype or a polymorphic type above.
  uint8_t _type{0};
};

/**
 * The stack validator verifies that the stack operated on by a function is
 * well-formed.
 *
 * This ensures there will never be a case (at runtime) where the stack
 * could be popped when it's empty.
 *
 * Additionally computes the maximum memory usage for this function in terms of
 * the runtime stack. This can be used by a runtime to determine if
 *
 * Spec ref: https://webassembly.github.io/spec/core/valid/index.html
 *
 */
class FunctionValidator {
 public:
  FunctionValidator(FunctionSignature, const std::vector<ValType>& locals);

  // The maximum number of elements that are ever on the stack at
  // once.
  size_t maximum_stack_elements() const;
  // The maximum bytes that is used by the stack for this function at runtime.
  size_t maximum_stack_size_bytes() const;

  void operator()(const op::ConstI32&);
  void operator()(const op::AddI32&);
  void operator()(const op::GetLocalI32&);
  void operator()(const op::SetLocalI32&);
  void operator()(const op::Return&);
  void operator()(const op::LabelBlockStart&);
  void operator()(const op::LabelBlockAlternative&);
  void operator()(const op::LabelBlockEnd&);

  void Finalize();

 private:
  // Assert the correct type is popped
  void Pop(ValidationType);
  void Pop(ValType);
  // Push the value on the stack
  void Push(ValidationType);
  void Push(ValType);
  // Assert a local is a specific valtype
  void AssertLocal(size_t, ValType) const;
  // Assert the stack is empty
  void AssertEmpty() const;
  // Check if the stack is empty
  bool empty() const;

  std::vector<ValType> _locals;
  std::vector<ValType> _returns;

  bool _unreachable = false;

  std::vector<ValidationType> _underlying;
  size_t _current_memory_usage{0};
  size_t _max_stack_size{0};
  size_t _max_memory_usage{0};
};
}  // namespace wasmcc
