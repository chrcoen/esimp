/*
 * SPDX-License-Identifier: Apache-2.0
 * SPDX-FileCopyrightText: © 2022 Christoph Coenen <chrcoen@gmail.com>
 */

#include "esimp/platform/gpio.hpp"

#include <assert.h>

namespace esimp {


GpioOutputPin::GpioOutputPin(sc_core::sc_module_name name) :
  sc_core::sc_module(name) {

}

void GpioOutputPin::configure(enum flags flags) {
}

void GpioOutputPin::set() {
  out = true;
}

void GpioOutputPin::clear() {
  out = false;
}

IRQ_if* GpioOutputPin::get_irq() {
  return NULL;
}

void GpioOutputPin:: set_irq(IRQ_if *irq) {
}




} /* namespace esimp */
