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

#include <array>
#include <memory>

#include "runtime/thread/thread.h"

namespace wasmcc::runtime {

asm(".text\n"
#ifdef __APPLE__
    ".globl _WasmccSwitch\n"
    "_WasmccSwitch:\n"
#else
    ".globl WasmccSwitch\n"
    ".type WasmccSwitch #function\n"
    ".hidden WasmccSwitch\n"
    "WasmccSwitch:\n"
#endif
    "  mov x10, sp\n"
    "  mov x11, x30\n"
    "  stp x19, x20, [x0, #(0*16)]\n"
    "  stp x21, x22, [x0, #(1*16)]\n"
    "  stp d8, d9, [x0, #(7*16)]\n"
    "  stp x23, x24, [x0, #(2*16)]\n"
    "  stp d10, d11, [x0, #(8*16)]\n"
    "  stp x25, x26, [x0, #(3*16)]\n"
    "  stp d12, d13, [x0, #(9*16)]\n"
    "  stp x27, x28, [x0, #(4*16)]\n"
    "  stp d14, d15, [x0, #(10*16)]\n"
    "  stp x29, x30, [x0, #(5*16)]\n"
    "  stp x10, x11, [x0, #(6*16)]\n"
    "  ldp x19, x20, [x1, #(0*16)]\n"
    "  ldp x21, x22, [x1, #(1*16)]\n"
    "  ldp d8, d9, [x1, #(7*16)]\n"
    "  ldp x23, x24, [x1, #(2*16)]\n"
    "  ldp d10, d11, [x1, #(8*16)]\n"
    "  ldp x25, x26, [x1, #(3*16)]\n"
    "  ldp d12, d13, [x1, #(9*16)]\n"
    "  ldp x27, x28, [x1, #(4*16)]\n"
    "  ldp d14, d15, [x1, #(10*16)]\n"
    "  ldp x29, x30, [x1, #(5*16)]\n"
    "  ldp x10, x11, [x1, #(6*16)]\n"
    "  mov sp, x10\n"
    "  br x11\n");

extern "C" void WasmccWrapMain();

__asm__(
    ".text\n"
#ifdef __APPLE__
    ".globl _WasmccWrapMain\n"
    "_WasmccWrapMain:\n"
#else
    ".globl WasmccWrapMain\n"
    ".type WasmccWrapMain #function\n"
    ".hidden WasmccWrapMain\n"
    "WasmccWrapMain:\n"
#endif
    "  mov x0, x19\n"
    "  mov x30, x21\n"
    "  br x20\n");

struct ThreadStack {
 private:
  constexpr static size_t kXCalleeSavedRegisterCount = 12;
  constexpr static size_t kVCalleeSavedRegisterCount = 8;

 public:
  /*
   * This saves the callee saved registers x19-x29
   * and additionally x30 which holds the return address.
   */
  std::array<void*, kXCalleeSavedRegisterCount> x;
  void* sp;
  void* lr;
  /*
   * This saves the lower bits of the vector registers,
   * which are callee saved.
   */
  std::array<void*, kVCalleeSavedRegisterCount> d;
};

/**
 * Assert that std::array doesn't have extra metadata in it and this struct is
 * exactly the size we expect.
 *
 * See: https://stackoverflow.com/a/17121515
 */
constexpr static size_t kExpectedStackStateSize = 22 * sizeof(void*);
static_assert(sizeof(ThreadStack) == kExpectedStackStateSize,
              "Expected std::array to not hold metadata");

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
  thread_stack->x[0] = thread;
  // NOLINTBEGIN(*-reinterpret-cast)
  thread_stack->x[1] = reinterpret_cast<void*>(VMThreadStart);
  constexpr uintptr_t kDummyReturnAddress = 0xdeaddeaddeaddead;
  // NOLINTNEXTLINE(*-no-int-to-ptr)
  thread_stack->x[2] = reinterpret_cast<void*>(kDummyReturnAddress);
  // NOLINTNEXTLINE(*-pointer-arithmetic)
  thread_stack->sp = static_cast<uint8_t*>(stack_base) + stack_size;
  thread_stack->lr = reinterpret_cast<void*>(WasmccWrapMain);
  // NOLINTEND(*-reinterpret-cast)
}

}  // namespace wasmcc::runtime
