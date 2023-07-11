/**
Some of the following assembly code is taken from LuaCoco by Mike Pall.
See https://coco.luajit.org/index.html

MIT license

Copyright (C) 2004-2016 Mike Pall. All rights reserved.

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#include <memory>

#include "runtime/thread/thread.h"

namespace wasmcc::runtime {

asm(".text\n"
#ifdef __APPLE__
    ".globl _WasmccSwitch\n"
    "_WasmccSwitch:\n"
#else
    ".globl WasmccSwitch\n"
    ".type WasmccSwitch @function\n"
    ".hidden WasmccSwitch\n"
    "WasmccSwitch:\n"
#endif
    "  leaq 0x3d(%rip), %rax\n"
    "  movq %rax, (%rdi)\n"
    "  movq %rsp, 8(%rdi)\n"
    "  movq %rbp, 16(%rdi)\n"
    "  movq %rbx, 24(%rdi)\n"
    "  movq %r12, 32(%rdi)\n"
    "  movq %r13, 40(%rdi)\n"
    "  movq %r14, 48(%rdi)\n"
    "  movq %r15, 56(%rdi)\n"
    "  movq 56(%rsi), %r15\n"
    "  movq 48(%rsi), %r14\n"
    "  movq 40(%rsi), %r13\n"
    "  movq 32(%rsi), %r12\n"
    "  movq 24(%rsi), %rbx\n"
    "  movq 16(%rsi), %rbp\n"
    "  movq 8(%rsi), %rsp\n"
    "  jmpq *(%rsi)\n"
    "  ret\n");

extern "C" void WasmccWrapMain();

__asm__(
    ".text\n"
#ifdef __APPLE__
    ".globl _WasmccWrapMain\n"
    "_WasmccWrapMain:\n"
#else
    ".globl WasmccWrapMain\n"
    ".type WasmccWrapMain @function\n"
    ".hidden WasmccWrapMain\n"
    "WasmccWrapMain:\n"
#endif
    "  movq %r13, %rdi\n"
    "  jmpq *%r12\n");

struct ThreadStack {
 private:
  constexpr static size_t kXCalleeSavedRegisterCount = 12;
  constexpr static size_t kVCalleeSavedRegisterCount = 8;

 public:
  void *rip, *rsp, *rbp, *rbx, *r12, *r13, *r14, *r15;
};

static_assert(sizeof(std::unique_ptr<ThreadStack>) == sizeof(void*),
              "Expected std::unique_ptr to no hold metadata");

namespace {
void DeleteThreadStack(ThreadStack* ts) {
  // NOLINTNEXTLINE(*-owning-memory)
  delete ts;
}
}  // namespace

void VMThreadStart(VMThread*);

StackState CreateUninitializedStackState() {
  return {new ThreadStack(), &DeleteThreadStack};
}

void InitializeVMThreadStackState(ThreadStack* thread_stack, VMThread* thread,
                                  void* stack_base, size_t stack_size) {
  constexpr size_t kRedZoneSize = 128;
  // Reserve 128 bytes at the bottom of the stack for the red zone.
  // See: https://en.wikipedia.org/wiki/Red_zone_(computing)
  stack_size = stack_size - kRedZoneSize;
  // NOLINTBEGIN(*-reinterpret-cast,*-pointer-arithmetic)
  void** stack_high_ptr = reinterpret_cast<void**>(
      static_cast<uint8_t*>(stack_base) + stack_size - sizeof(void*));
  constexpr uintptr_t kDummyReturnAddress = 0xdeaddeaddeaddead;
  // NOLINTNEXTLINE(*-no-int-to-ptr)
  *stack_high_ptr = reinterpret_cast<void*>(kDummyReturnAddress);
  thread_stack->rip = reinterpret_cast<void*>(WasmccWrapMain);
  thread_stack->rsp = reinterpret_cast<void*>(stack_high_ptr);
  thread_stack->r12 = reinterpret_cast<void*>(VMThreadStart);
  thread_stack->r13 = thread;
  // NOLINTEND(*-reinterpret-cast,*-pointer-arithmetic)
}

}  // namespace wasmcc::runtime
