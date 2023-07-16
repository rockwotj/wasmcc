#include "runtime/function_handle.h"

#include "runtime/thread/thread.h"

namespace wasmcc::runtime {

DynamicComputation::DynamicComputation(VMThread* t) : _thread(t) {}
void DynamicComputation::Execute() { _thread->Resume(); }
void DynamicComputation::Cancel() { _thread->Stop(); }
bool DynamicComputation::IsDone() const noexcept {
  return _thread->state() == VMThread::State::kStopped;
}

}  // namespace wasmcc::runtime
