/*
 * SPDX-License-Identifier: Apache-2.0
 * SPDX-FileCopyrightText: © 2022 Christoph Coenen <chrcoen@gmail.com>
 */

#include "esimp/platform/isr.hpp"

#include <assert.h>

#include <string>

#include "esimp/platform/trace.hpp"

namespace esimp {

using std::string;

ISR::ISR(Context **active_ctx, const char *parent_name, const char *name, Update_if *parent, Application_if *app)
    : Context(active_ctx, parent_name, name, SystemcThread::Type::ISR),
      parent(parent),
      app_if(app),
      irq(nullptr) {}

Context *ISR::as_context() { return static_cast<Context *>(this); }

void ISR::set_irq(IRQ *i) { irq = i; }

IRQ *ISR::get_irq() { return irq; }

int ISR::run() {
  while (true) {
    assert(app_if != nullptr);
    assert(irq != nullptr);
    IRQ *current_irq = irq;
    current_irq->set_state(IRQ::State::Active);
    trace::msg("isr", (string("Start ISR for ") + irq->get_name()).c_str());
    app_if->run_isr(irq);
    current_irq->set_state(IRQ::State::Idle);
    assert(parent != nullptr);
    parent->update_from_sw();
  }
  return 0;
};

} /* namespace esimp */