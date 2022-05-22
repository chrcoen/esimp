/*
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef ESIMP_PLATFORM_TIMER_HPP_
#define ESIMP_PLATFORM_TIMER_HPP_

#include "esimp/interface/platform_if.hpp"
#include "esimp/platform/systemc_thread.hpp"

namespace esimp {

class Timer : public SystemcThread, public Timer_if {
 public:
  Timer(const char *name, IRQ_if *irq);

  /* SystemcThread */
  int run() override;

  /* Timer_if */
  void set_mode(TimerMode m) override;
  void start() override;
  void stop() override;
  void set_period_ns(uint64_t val) override;
  uint64_t get_value_ns() override;
  IRQ_if *get_irq() override;

 private:
  IRQ_if *irq_if;
  TimerMode mode;
  uint64_t period;
  bool enabled;
  sc_core::sc_event wakeup;
  sc_core::sc_time t_start;
};

} /* namespace esimp */

#endif /* ESIMP_PLATFORM_TIMER_HPP_ */
