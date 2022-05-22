/*
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef ESIMP_PLATFORM_MCU_HPP_
#define ESIMP_PLATFORM_MCU_HPP_

#include <map>
#include <vector>

#include "esimp/interface/platform_if.hpp"
#include "esimp/platform/context.hpp"
#include "esimp/platform/firmware.hpp"
#include "esimp/platform/isr.hpp"
#include "esimp/platform/nvic.hpp"
#include "esimp/platform/realtime.hpp"
#include "esimp/platform/rtos_thread.hpp"
#include "esimp/platform/timer.hpp"
#include "esimp/platform/update_if.hpp"

namespace esimp {

class MCU : public sc_core::sc_module, public MCU_if, public Update_if {
 public:
  SC_HAS_PROCESS(MCU);

  struct Configuration {
    Configuration();
    bool sync_with_realtime;
    unsigned int f_clk_hz;
    uint64_t time_quantum_ns;
  };

  MCU(sc_core::sc_module_name name);
  ~MCU();

  /* MCU_if */
  void step(uint64_t dn) override;
  void busy_wait_ns(uint64_t t_ns) override;
  uint64_t get_timestamp_ns() override;
  NVIC_if *get_nvic() override;
  RTOSThread_if *create_thread(const char *name, const void *pdata) override;
  Timer_if *create_timer(const char *name) override;
  void exit(ExitAction action, unsigned int fw_index) override;
  void log(const char *msg) override;

  /* Update_if */
  void update_from_hw() override;
  void update_from_sw() override;
  void switch_thread(RTOSThread_if *thread) override;

  void set_configuration(const Configuration &c);
  Configuration get_configuration();
  void add_firmware(Firmware *fw);
  void add_irq(const char *name);

 private:
  NVIC nvic;
  Realtime realtime;
  std::vector<Firmware *> firmware;
  Firmware *active_firmware;
  Firmware *next_firmware;
  ExitAction exit_action;
  Configuration config;

  Context *active_context;
  RTOSThread *main_thread;
  RTOSThread *active_rtos_thread;
  std::vector<RTOSThread *> rtos_threads;
  std::map<unsigned int, ISR *> isrs;
  std::vector<Timer *> timers;
  std::vector<IRQ *> irqs;
  sc_core::sc_event reset_event;

  ISR *get_isr(unsigned int prio);
  void update(bool from_hw);
  void start();
  void sync();
  uint64_t instructions_ns();
  void reset();
};

} /* namespace esimp */

#endif /* ESIMP_PLATFORM_MCU_HPP_ */
