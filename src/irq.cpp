/*
 * SPDX-License-Identifier: Apache-2.0
 * SPDX-FileCopyrightText: © 2022 Christoph Coenen <chrcoen@gmail.com>
 */

#include "esimp/platform/irq.hpp"

#include <assert.h>

#include <string>

#include "esimp/platform/trace.hpp"

using std::string;

namespace esimp {

IRQ::IRQ(Update_if *p, int nr, const char *n)
    : parent(p),
      nr(nr),
      name(n),
      prio(0),
      pending(false),
      enabled(false),
      state(State::Idle),
      pdata(nullptr)
      {
      }

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

void IRQ::set_pending(bool val) {
  trace::msg("irq", (string("HW Trigger ") + name).c_str());
  pending = val;
  if (enabled && pending) {
    assert(parent != nullptr);
    parent->update_from_hw();
  }
}

void IRQ::set_pending_from_sw(bool val) {
  trace::msg("irq", (string("SW Trigger ") + name).c_str());
  pending = val;
  if (enabled && pending) {
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

int IRQ::get_nr() { return nr; }

void IRQ::set_pdata(void *d) { pdata = d; }

void* IRQ::get_pdata() { return pdata; }


} /* namespace esimp */
