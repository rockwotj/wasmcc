#include "testing/wat.h"

#include <stdexcept>

#include "wabt/binary-writer.h"
#include "wabt/error-formatter.h"
#include "wabt/stream.h"
#include "wabt/validator.h"
#include "wabt/wast-parser.h"

namespace wasmcc {

namespace {
constexpr static wabt::Features kFeatures;
}

bytes Wat2Wasm(std::string_view wat) {
  wabt::Errors errors;
  std::unique_ptr<wabt::WastLexer> lexer = wabt::WastLexer::CreateBufferLexer(
      "testdata.wat", wat.data(), wat.size(), &errors);
  std::unique_ptr<wabt::Module> mod;
  wabt::WastParseOptions parse_wast_options(kFeatures);
  auto result = ParseWatModule(lexer.get(), &mod, &errors, &parse_wast_options);
  if (Failed(result)) {
    auto line_finder = lexer->MakeLineFinder();
    std::string err = wabt::FormatErrorsToString(
        errors, wabt::Location::Type::Text, line_finder.get());
    throw std::runtime_error(err);
  }
  wabt::MemoryStream stream;
  wabt::WriteBinaryOptions write_wasm_options(kFeatures,
                                              /*canonicalize_lebs=*/false,
                                              /*relocatable=*/false,
                                              /*write_debug_names=*/false);
  result = wabt::WriteBinaryModule(&stream, mod.get(), write_wasm_options);
  if (Failed(result)) {
    throw std::runtime_error("failed to write binary output");
  }
  return std::move(stream.ReleaseOutputBuffer()->data);
}

}  // namespace wasmcc
