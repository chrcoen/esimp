/*
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef ESIMP_PLATFORM_NVIC_HPP_
#define ESIMP_PLATFORM_NVIC_HPP_

#include <vector>

#include "esimp/interface/platform_if.hpp"
#include "esimp/platform/irq.hpp"
#include "esimp/platform/update_if.hpp"

namespace esimp {

class NVIC : public NVIC_if {
 public:
  NVIC(Update_if *parent);

  /* NVIC_if */
  void set_primask(unsigned int n) override;
  void set_basepri(unsigned int n) override;
  int get_primask() override;
  int get_basepri() override;
  IRQ_if *get_irq(const char *name) override;
  IRQ_if *get_irq(int nr) override;

  void add_irq(IRQ irq);
  IRQ *active_irq();

  void reset();

 private:
  static bool is_prior(IRQ &irq, unsigned int p);
  Update_if *parent;
  unsigned int basepri;
  unsigned int primask;
  std::vector<IRQ> irqs;
  IRQ *active;
};

} /* namespace esimp */

#endif /* ESIMP_PLATFORM_NVIC_HPP_ */
