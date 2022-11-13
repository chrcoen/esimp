/*
 * SPDX-License-Identifier: Apache-2.0
 * SPDX-FileCopyrightText: © 2022 Christoph Coenen <chrcoen@gmail.com>
 */

#include "esimp/platform/timer.hpp"

#include <assert.h>
#include <pthread.h>

namespace esimp {

Timer::Timer(const char *parent_name, const char *name, IRQ_if *irq)
    : SystemcThread(parent_name, name, SystemcThread::Type::Hardware),
      irq_if(irq),
      mode(TimerMode::SingleShot),
      enabled(false) {}

IRQ_if *Timer::get_irq() { return irq_if; }

int Timer::run() {
  wait(wakeup);
  for (;;) {
    while (!enabled) {
      wait(wakeup);
    }
    t_start = sc_core::sc_time_stamp();
    wait(period, sc_core::SC_NS, wakeup);
    if (enabled) {
      assert(irq_if != nullptr);
      irq_if->set_pending(true);
    }
    if (mode == TimerMode::SingleShot) {
      enabled = false;
    }
  }
  return 0;
}

void Timer::set_mode(TimerMode m) { mode = m; }

void Timer::start() {
  enabled = true;
  wakeup.notify();
}

void Timer::stop() {
  enabled = false;
  wakeup.notify();
  wait(sc_core::SC_ZERO_TIME);
}

void Timer::set_period_ns(uint64_t val) { period = val; }

uint64_t Timer::get_value_ns() {
  return (uint64_t)(sc_core::sc_time_stamp() - t_start).to_seconds() * 1e9;
}

void Timer::clear_irq() {
  irq_if->set_pending_from_sw(false);
}

void Timer::trigger_irq() {
  irq_if->set_pending_from_sw(true);
}

} /* namespace esimp */
