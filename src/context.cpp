#include "esimp/platform/context.hpp"

#include <assert.h>

#include "esimp/platform/trace.hpp"

namespace esimp {

Context::Context(const char *name, SystemcThread::Type t)
    : SystemcThread(name, t) {}

uint64_t Context::get_instruction_count() { return instruction_count; }

void Context::clear_instruction_count() { instruction_count = 0; }

void Context::increment_instruction_count(uint64_t dn) {
  instruction_count += dn;
}

void Context::wait(uint64_t t_ns, Context **active_context) {
  trace::msg("context", "wait");
  assert((*active_context) == this);
  sc_core::sc_time t_work(t_ns, sc_core::SC_NS);
  while (t_work > sc_core::sc_time(0, sc_core::SC_SEC)) {
    sc_core::sc_time t0 = sc_core::sc_time_stamp();
    sc_core::wait(t_work, wakeup);
    sc_core::sc_time t1 = sc_core::sc_time_stamp();
    t_work -= (t1 - t0);
    if ((*active_context) != this) {
      suspend();
    }
  }
  trace::msg("context", "resume");
}

void Context::suspend() {
  trace::msg("context", "suspend");
  sc_core::wait(wakeup);
  trace::msg("context", "wakeup");
}

void Context::activate() { wakeup.notify(); }

}  // namespace esimp