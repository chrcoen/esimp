/*
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef ESIMP_PLATFORM_HW_IRQ_IF_HPP_
#define ESIMP_PLATFORM_HW_IRQ_IF_HPP_

namespace esimp {

class HW_IRQ_if {
 public:
  virtual void hw_irq(const char *name) = 0;
};

} /* namespace esimp */

#endif /* ESIMP_PLATFORM_HW_IRQ_IF_HPP_ */
