#include "platform/simulation_if.hpp"

extern void test_main();

static MCU_if *mcu;

class Test : public Application_if {
 public:
  void run_main() override;
  void run_thread(const Thread_if *thread) override;
  void run_isr(const IRQ_if *irq) override;
};

static Test test;

void Test::run_main() { test_main(); }

void run_thread(const Thread_if *thread) {}

void run_isr(const IRQ_if *irq) {}

extern "C" Application_if *sim_register(MCU_if *obj) {
  mcu = obj;
  return &test;
}
