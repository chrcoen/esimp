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

  IRQ(Update_if *parent, int nr, const char *name);

  /* IRQ_if */
  void enable() override;
  void disable() override;
  bool is_enabled() override;
  void set_pending(bool val) override;
  void set_pending_from_sw(bool val) override;
  void set_prio(unsigned int val) override;
  unsigned int get_prio() override;
  void set_pdata(void *d) override;
  void* get_pdata() override;


  const char *get_name();
  bool is_pending();
  void set_state(State s);
  State get_state();
  void reset();
  int get_nr();

 private:
  Update_if *const parent;
  int nr;
  const char *name;
  unsigned int prio;
  bool pending;
  bool enabled;
  State state;
  void *pdata;
};

} /* namespace esimp */

#endif /* ESIMP_PLATFORM_IRQ_HPP_ */
