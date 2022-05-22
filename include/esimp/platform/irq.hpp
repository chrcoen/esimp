/*
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef ESIMP_PLATFORM_IRQ_HPP_
#define ESIMP_PLATFORM_IRQ_HPP_

#include "esimp/interface/platform_if.hpp"
#include "esimp/platform/update_if.hpp"

namespace esimp {

class IRQ : public IRQ_if {
 public:
  enum class State { Idle, Suspended, Active };

  IRQ(Update_if *parent, const char *name);

  /* IRQ_if */
  void enable() override;
  void disable() override;
  void clear() override;
  void sw_trigger() override;
  void hw_trigger() override;
  void set_prio(unsigned int val) override;
  unsigned int get_prio() override;

  const char *get_name();
  bool is_pending();
  bool is_enabled();
  void set_state(State s);
  State get_state();
  void reset();

 private:
  Update_if *const parent;
  const char *name;
  unsigned int prio;
  bool pending;
  bool enabled;
  State state;
};

} /* namespace esimp */

#endif /* ESIMP_PLATFORM_IRQ_HPP_ */
