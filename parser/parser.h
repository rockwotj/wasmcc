#pragma once

#include "base/coro.h"
#include "base/stream.h"
#include "core/ast.h"

namespace wasmcc {

class ParseException : public std::exception {
 public:
  explicit ParseException(std::string msg) : _msg(std::move(msg)) {}

  const char* what() const noexcept final { return _msg.c_str(); }

 private:
  std::string _msg;
};

class ModuleTooLargeException : public ParseException {
 public:
  explicit ModuleTooLargeException(std::string msg)
      : ParseException(std::move(msg)) {}
};

co::Future<ParsedModule> ParseModule(Stream*);

}  // namespace wasmcc
