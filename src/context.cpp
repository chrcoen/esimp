/*
 * SPDX-License-Identifier: Apache-2.0
 * SPDX-FileCopyrightText: © 2022 Christoph Coenen <chrcoen@gmail.com>
 */

#include "esimp/platform/context.hpp"

#include <assert.h>

#include "esimp/platform/trace.hpp"

namespace esimp {

Context::Context(Context **active_ctx, const char *parent_name, const char *name, SystemcThread::Type t):
  SystemcThread(parent_name, name, t),
  active_context(active_ctx),
  instruction_count(0),
  terminate(false)
  {}

uint64_t Context::get_instruction_count() { return instruction_count; }

void Context::clear_instruction_count() { instruction_count = 0; }

void Context::increment_instruction_count(uint64_t dn) {
  instruction_count += dn;
}

void Context::wait(uint64_t t_ns) {
  trace::msg("context", "wait");
  assert((*active_context) == this);
  sc_core::sc_time t_work(t_ns, sc_core::SC_NS);
  while (t_work > sc_core::sc_time(0, sc_core::SC_SEC)) {
    sc_core::sc_time t0 = sc_core::sc_time_stamp();
    sc_core::wait(t_work, wakeup);
    if (terminate) {
      this->kill();
    }
    sc_core::sc_time t1 = sc_core::sc_time_stamp();
    t_work -= (t1 - t0);
    if ((*active_context) != this) {
      suspend();
    }
  }
  trace::msg("context", "resume");
}

void Context::abort(Context *last_context) {
  if (last_context == this) {
    this->kill();
  } else {
    terminate = true;
    wakeup.notify();
  }
}

void Context::suspend() {
  trace::msg("context", "suspend");
  do {
    sc_core::wait(wakeup);
    if (terminate) {
      this->kill();
    }
  } while ((*active_context) != this);
  trace::msg("context", "wakeup");
}

void Context::activate() { wakeup.notify(); }

}  // namespace esimp