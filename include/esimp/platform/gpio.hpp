/*
 * SPDX-License-Identifier: Apache-2.0
 * SPDX-FileCopyrightText: © 2022 Christoph Coenen <chrcoen@gmail.com>
 */
#ifndef ESIMP_PLATFORM_GPIO_HPP_
#define ESIMP_PLATFORM_GPIO_HPP_

#include "esimp/interface/platform_if.hpp"
#include "esimp/platform/systemc_thread.hpp"

namespace esimp {

class GpioOutputPin: public sc_core::sc_module, public GpioPin_if {
public:
  GpioOutputPin(sc_core::sc_module_name name);

  sc_core::sc_out<bool> out;

  /* GpioPin_if */
  virtual void configure(enum flags flags) override;
  virtual void set() override;
  virtual void clear() override;
  virtual IRQ_if *get_irq() override;
  virtual void set_irq(IRQ_if *irq) override;

};



} /* namespace esimp */

#endif /* ESIMP_PLATFORM_GPIO_HPP_ */
