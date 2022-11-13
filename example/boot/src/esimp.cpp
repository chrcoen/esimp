#include <assert.h>

#include <iostream>

#include "esimp/interface/platform_if.hpp"

extern double square_root(int n);

class Application : public esimp::Application_if {
 public:
  void run_main(esimp::Thread_if *thread) override;
  void run_thread(esimp::Thread_if *thread, const char *name,
                  void *pdata) override;
  void run_isr(esimp::IRQ_if *irq) override;

 private:
};

static Application app;
static esimp::MCU_if *mcu;

void Application::run_main(esimp::Thread_if *thread) {
  mcu->log("Bootloader start");
  mcu->busy_wait_ns(2e9);
  mcu->exit(esimp::ExitAction::Reset, 1);
}

void Application::run_thread(esimp::Thread_if *thread, const char *name,
                             void *pdata) {
  assert(0);
}

void Application::run_isr(esimp::IRQ_if *irq) { assert(0); }

extern "C" esimp::Application_if *sim_register(esimp::MCU_if *obj) {
  mcu = obj;
  esimp::Application_if *res = &app;
  return res;
}

extern "C" void simulation_step(int n) { mcu->step(n); }
