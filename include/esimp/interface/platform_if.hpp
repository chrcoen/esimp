/*
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef ESIMP_INTERFACE_PLATFORM_IF_HPP_
#define ESIMP_INTERFACE_PLATFORM_IF_HPP_

#include <cstddef>
#include <cstdint>

namespace esimp {

enum class ExitAction {
  Shutdown,
  Reset,
};

enum class TimerMode { SingleShot, Continuous };

class IRQ_if {
 public:
  virtual void enable() = 0;
  virtual void disable() = 0;
  virtual void clear() = 0;
  virtual void sw_trigger() = 0;
  virtual void hw_trigger() = 0;
  virtual void set_prio(unsigned int val) = 0;
  virtual unsigned int get_prio() = 0;
  virtual ~IRQ_if() {}
};

class Timer_if {
 public:
  virtual void set_mode(TimerMode m) = 0;
  virtual void start() = 0;
  virtual void stop() = 0;
  virtual void set_period_ns(uint64_t val) = 0;
  virtual uint64_t get_value_ns() = 0;
  virtual IRQ_if *get_irq() = 0;
  virtual ~Timer_if() {}
};

class NVIC_if {
 public:
  virtual void set_primask(unsigned int n) = 0;
  virtual void set_basepri(unsigned int n) = 0;
  virtual int get_primask() = 0;
  virtual int get_basepri() = 0;
  virtual IRQ_if *get_irq(const char *name) = 0;
  virtual ~NVIC_if() {}
};

class RTOSThread_if {
 public:
  virtual void start() = 0;
  virtual void switch_to() = 0;
  virtual ~RTOSThread_if() {}
};

class Application_if {
 public:
  virtual void run_main() = 0;
  virtual void run_thread(RTOSThread_if *thread, const char *name,
                          const void *pdata) = 0;
  virtual void run_isr(IRQ_if *irq) = 0;
  virtual ~Application_if() {}
};

class MCU_if {
 public:
  virtual void step(uint64_t dn) = 0;
  virtual void busy_wait_ns(uint64_t t_ns) = 0;
  virtual uint64_t get_timestamp_ns() = 0;
  virtual NVIC_if *get_nvic() = 0;
  virtual RTOSThread_if *create_thread(const char *name, const void *pdata) = 0;
  virtual Timer_if *create_timer(const char *name) = 0;
  virtual void exit(ExitAction action, unsigned int fw_index) = 0;
  virtual void log(const char *msg) = 0;

  virtual ~MCU_if() {}
};

} /* namespace esimp */

extern "C" esimp::Application_if *sim_register(esimp::MCU_if *obj);

#endif /* ESIMP_INTERFACE_PLATFORM_IF_HPP_ */
