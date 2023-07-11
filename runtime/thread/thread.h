#pragma once

#include <functional>
#include <memory>

namespace wasmcc::runtime {
struct ThreadStack;
using StackState = std::unique_ptr<ThreadStack, void (*)(ThreadStack*)>;

struct VMThreadConfiguration {
  /** The size of the native runtime stack. */
  size_t stack_size;
  /**
   * If true enable 4k guard pages on each side of the stack,
   * and protect the memory, which means that if there is a stack
   * {over,under}flow that the process is aborted.
   */
  bool enable_guard_pages;
};

/**
 * VMThread is a thread that runs on the same hardware thread, but another
 * stack.
 *
 * This allows the runtime of a function to be suspended and resumed
 * dynamically at any point within it's execution.
 *
 * VMThread must be kept alive while it's function is executing, but otherwise
 * it's valid to destroy the thread at any point - this allows for stopping a
 * function partially through a computation.
 *
 */
class VMThread {
 public:
  enum class State {
    kStopped,
    kSuspended,
    kRunning,
  };

  // VMThread is not copyable or moveable.
  VMThread(const VMThread&) = delete;
  VMThread& operator=(const VMThread&) = delete;
  VMThread(VMThread&&) = delete;
  VMThread& operator=(VMThread&&) = delete;
  ~VMThread();

  /**
   * Create a new thread running the specified function with a stack the size of
   * `stack_size`.
   */
  static std::unique_ptr<VMThread> Create(std::function<void()>,
                                          VMThreadConfiguration);

  /**
   * Resume (or start) the thread.
   */
  void Resume();

  /** Pause the currently running VMThread. */
  static void Yield();

  /** The current state of this VMThread. */
  State state() const { return _state; }

  uintptr_t stack_bottom() const {
    // NOLINTNEXTLINE(*-reinterpret-cast)
    return reinterpret_cast<uintptr_t>(_stack_memory.get());
  }
  uintptr_t stack_top() const {
    // NOLINTNEXTLINE(*-reinterpret-cast)
    return reinterpret_cast<uintptr_t>(_stack_memory.get()) + _stack_size;
  }

 private:
  friend void VMThreadStart(VMThread*);

  using StackMemory = std::unique_ptr<void, std::function<void(void*)>>;

  VMThread(std::function<void()>, StackMemory, size_t);

  void TrampolineInToVM();
  void TrampolineOutOfVM();

  State _state = State::kStopped;
  std::function<void()> _func;

  StackMemory _stack_memory;
  size_t _stack_size;

  StackState _my_thread_state;
  StackState _main_thread_state;

#if defined(ADDRESS_SANITIZER)
  void* _asan_prev_stack = nullptr;
#endif
};

}  // namespace wasmcc::runtime
