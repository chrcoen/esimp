/*
 * SPDX-License-Identifier: Apache-2.0
 */

#include "esimp/platform/irq.hpp"

#include <assert.h>

#include <string>

#include "esimp/platform/trace.hpp"

using std::string;

namespace esimp {

IRQ::IRQ(Update_if *p, const char *n)
    : parent(p),
      name(n),
      prio(0),
      pending(false),
      enabled(false),
      state(State::Idle) {}

void IRQ::reset() {
  pending = false;
  enabled = false;
  state = State::Idle;
  prio = 0;
}

void IRQ::enable() {
  enabled = true;
  assert(parent != nullptr);
  parent->update_from_sw();
}

void IRQ::disable() {
  enabled = false;
  assert(parent != nullptr);
  parent->update_from_sw();
}

void IRQ::clear() { pending = false; }

void IRQ::hw_trigger() {
  trace::msg("irq", (string("HW Trigger ") + name).c_str());
  if (enabled) {
    pending = true;
    assert(parent != nullptr);
    parent->update_from_hw();
  }
}

void IRQ::sw_trigger() {
  trace::msg("irq", (string("SW Trigger ") + name).c_str());
  if (enabled) {
    pending = true;
    assert(parent != nullptr);
    parent->update_from_sw();
  }
}

void IRQ::set_prio(unsigned int val) {
  prio = val;
  assert(parent != nullptr);
  parent->update_from_sw();
}

unsigned int IRQ::get_prio() { return prio; }

const char *IRQ::get_name() { return name; }

bool IRQ::is_pending() { return pending; }

bool IRQ::is_enabled() { return enabled; }

void IRQ::set_state(State s) { state = s; }

IRQ::State IRQ::get_state() { return state; }

} /* namespace esimp */
