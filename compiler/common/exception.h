#pragma once

#include <exception>
#include <string>

namespace wasmcc {
/**
 * An exception for when the compiler has any errors.
 */
class CompilationException : public std::exception {
 public:
  explicit CompilationException(std::string);

  const char* what() const noexcept final;

 private:
  std::string _msg;
};
}  // namespace wasmcc
