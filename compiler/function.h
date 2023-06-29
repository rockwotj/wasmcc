#pragma once
namespace wasmcc {
/**
 * A strongly typed wrapper around dynamically created code.
 */
class CompiledFunction {
 public:
  explicit CompiledFunction(void*);

  template <typename R, typename... A>
  R invoke(A... args) {
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    return reinterpret_cast<R (*)(A...)>(_ptr)(args...);
  }

  void* get() const;

 private:
  void* _ptr;
};

}  // namespace wasmcc