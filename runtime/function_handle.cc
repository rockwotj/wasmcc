#include "runtime/function_handle.h"

#include "base/assert.h"
#include "runtime/thread/thread.h"

namespace wasmcc::runtime {

DynamicComputation::DynamicComputation(VMThread* t) : _thread(t) {}
void DynamicComputation::Execute() {
  if (_thread->state() == VMThread::State::kSuspended) {
    _thread->Resume();
  }
}
void DynamicComputation::Cancel() {
  if (_thread->state() == VMThread::State::kSuspended) {
    _thread->Stop();
  }
}
bool DynamicComputation::IsDone() const noexcept {
  return _thread->state() == VMThread::State::kStopped;
}

}  // namespace wasmcc::runtime
