#include "runtime/thread/thread.h"

#include <sys/mman.h>
#include <unistd.h>

#include <cstdint>
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

namespace {
// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
thread_local VMThread* current_vm_thread = nullptr;

constexpr size_t kStackAlignment = 16;

void MProtect(void* addr, size_t size, bool readonly) {
  bool err =
      ::mprotect(addr, size, readonly ? PROT_READ : PROT_READ | PROT_WRITE);
  if (err) {
    std::string msg = "unable to protect stack guard page: ";
    std::string_view errno_str = std::strerror(errno);
    msg.append(errno_str);
    throw std::runtime_error(msg);
  }
}

}  // namespace

void VMThreadStart(VMThread* thread) {
  thread->_func();
  thread->_state = VMThread::State::kStopped;
  thread->TrampolineOutOfVM();
}

std::unique_ptr<VMThread> VMThread::Create(std::function<void()> func,
                                           VMThreadConfiguration config) {
  size_t aligned_stack_size = 0;
  StackMemory stack_mem = {nullptr, std::free};
  if (config.enable_guard_pages) {
    size_t page_size = getpagesize();
    aligned_stack_size = AlignUp(config.stack_size, page_size);
    size_t full_stack_size = aligned_stack_size + (page_size * 2);
    stack_mem =
        StackMemory(std::aligned_alloc(page_size, full_stack_size), std::free);
    MProtect(stack_mem.get(), page_size, /*readonly=*/true);
    // NOLINTBEGIN(*-pointer-arithmetic)
    MProtect(
        static_cast<uint8_t*>(stack_mem.get()) + page_size + aligned_stack_size,
        page_size, /*readonly=*/true);
    stack_mem =
        StackMemory(static_cast<uint8_t*>(stack_mem.release()) + page_size,
                    [aligned_stack_size, page_size](void* ptr) {
                      auto* p = static_cast<uint8_t*>(ptr);
                      MProtect(p - page_size, page_size, /*readonly=*/false);
                      MProtect(p + aligned_stack_size - page_size, page_size,
                               /*readonly=*/false);
                      // NOLINTNEXTLINE(*-owning-memory,*-no-malloc)
                      std::free(p - page_size);
                    });
    // NOLINTEND(*-pointer-arithmetic)
  } else {
    aligned_stack_size = AlignUp(config.stack_size, kStackAlignment);
    stack_mem = StackMemory(
        std::aligned_alloc(kStackAlignment, aligned_stack_size), std::free);
    std::memset(stack_mem.get(), 0, aligned_stack_size);
  }
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
