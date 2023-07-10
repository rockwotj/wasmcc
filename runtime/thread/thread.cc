#include "runtime/thread/thread.h"

#include <cstdlib>
#include <cstring>
#include <exception>
#include <functional>
#include <memory>
#include <stdexcept>

#include "base/align.h"

namespace wasmcc::runtime {
/** Declare these assembly functions. */
extern "C" int WasmccSwitch(ThreadStack* from, ThreadStack* to);

/** This function is architecture dependent. */
StackState CreateUninitializedStackState();
void InitializeVMThreadStackState(ThreadStack*, VMThread*, void* stack_base,
                                  size_t stack_size);

#if defined(ADDRESS_SANITIZER)
// NOLINTNEXTLINE(bugprone-reserved-identifier,cert-dcl37-c,cert-dcl51-cpp)
extern "C" void __sanitizer_start_switch_fiber(void** fake_stack_save,
                                               const void* bottom, size_t size);
// NOLINTNEXTLINE(bugprone-reserved-identifier,cert-dcl37-c,cert-dcl51-cpp)
extern "C" void __sanitizer_finish_switch_fiber(void* fake_stack_save,
                                                const void** bottom_old,
                                                size_t* size_old);
#endif

void VMThread::StackMemoryDeleter::operator()(void* stack_mem) const {
  // NOLINTNEXTLINE(*-no-malloc,*-owning-memory)
  std::free(stack_mem);
}

namespace {
// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
thread_local VMThread* current_vm_thread = nullptr;
}  // namespace

void VMThreadStart(VMThread* thread) {
  thread->_func();
  thread->_state = VMThread::State::kStopped;
  thread->TrampolineOutOfVM();
}

std::unique_ptr<VMThread> VMThread::Create(std::function<void()> func,
                                           size_t stack_size) {
  constexpr size_t kStackAlignment = 16;
  size_t aligned_stack_size = AlignUp(stack_size, kStackAlignment);
  StackMemory stack_mem =
      StackMemory(std::aligned_alloc(kStackAlignment, aligned_stack_size));
  std::memset(stack_mem.get(), 0, aligned_stack_size);
  return std::unique_ptr<VMThread>(
      new VMThread(std::move(func), std::move(stack_mem), aligned_stack_size));
}

VMThread::VMThread(std::function<void()> func, StackMemory stack_mem,
                   size_t stack_size)
    : _func(std::move(func)),
      _stack_memory(std::move(stack_mem)),
      _stack_size(stack_size),
      _my_thread_state(CreateUninitializedStackState()),
      _main_thread_state(CreateUninitializedStackState()) {}

VMThread::~VMThread() {
  if (current_vm_thread == this || _state == State::kRunning) {
    // TODO: Have better assert story
    (void)"VMThreads cannot be destroyed while running.";
    std::terminate();
  }
}
void VMThread::Resume() {
  if (current_vm_thread != nullptr) {
    throw std::runtime_error(
        "VMThread does not support calling into another VMThread");
  }
  if (_state == State::kStopped) {
    // If we're stopped, then initialize the main function before we start
    InitializeVMThreadStackState(_my_thread_state.get(), this,
                                 _stack_memory.get(), _stack_size);
  }
  _state = State::kRunning;
  TrampolineInToVM();
}
void VMThread::Yield() {
  if (current_vm_thread == nullptr) {
    throw std::runtime_error("attempting to yield when there is no VMThread");
  }
  // TODO: Check for stack overflow using a magic byte in the stack.
  current_vm_thread->_state = State::kSuspended;
  current_vm_thread->TrampolineOutOfVM();
}

void VMThread::TrampolineInToVM() {
  current_vm_thread = this;
#if defined(ADDRESS_SANITIZER)
  __sanitizer_start_switch_fiber(&_asan_prev_stack, _stack_memory.get(),
                                 _stack_size);
#endif
  WasmccSwitch(_main_thread_state.get(), _my_thread_state.get());
}

void VMThread::TrampolineOutOfVM() {
  current_vm_thread = nullptr;

#if defined(ADDRESS_SANITIZER)
  void* bottom_old = nullptr;
  size_t size_old = 0;
  __sanitizer_finish_switch_fiber(
      _asan_prev_stack, const_cast<const void**>(&bottom_old), &size_old);
  _asan_prev_stack = nullptr;
#endif

  WasmccSwitch(_my_thread_state.get(), _main_thread_state.get());
}

}  // namespace wasmcc::runtime
