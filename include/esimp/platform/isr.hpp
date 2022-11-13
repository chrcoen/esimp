/*
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef ESIMP_PLATFORM_ISR_HPP_
#define ESIMP_PLATFORM_ISR_HPP_

#include "esimp/platform/context.hpp"
#include "esimp/platform/irq.hpp"
#include "esimp/platform/update_if.hpp"

namespace esimp {

class ISR : public Context {
 public:
  ISR(Context **active_ctx, const char *parent_name, const char *name, Update_if *parent, Application_if *app_if);
  int run() override;
  void set_irq(IRQ *irq);
  IRQ *get_irq();
  Context *as_context();

 private:
  Update_if *parent;
  Application_if *app_if;
  IRQ *irq;
};

} /* namespace esimp */

#endif /* ESIMP_PLATFORM_ISR_HPP_ */
