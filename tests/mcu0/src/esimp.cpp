#include "esimp/interface/platform_if.hpp"

extern double square_root(int n);

class Application : public esimp::Application_if {
 public:
  void run_main() override;
  void run_thread(const esimp::RTOSThread_if *thread, const char *name,
                  const void *pdata) override;
  void run_isr(const esimp::IRQ_if *irq) override;

 private:
};

static Application app;
static esimp::MCU_if *mcu;
static esimp::RTOSThread_if *thread1;
static esimp::RTOSThread_if *thread2;

void Application::run_main() {
  thread1 = mcu->create_thread("thread1", nullptr);
  thread2 = mcu->create_thread("thread2", nullptr);
  t1->start();
  t2->start();
  for (;;) {
    mcu->busy_wait_ns(1e9);
  }
}

void Application::run_thread(const esimp::RTOSThread_if *thread,
                             const char *name, const void *pdata) {
  for (;;) {
    mcu->busy_wait_ns(5e8);
  }
}

void Application::run_isr(const esimp::IRQ_if *irq) {}

extern "C" void simulation_step(int n) { mcu->step(n); }

extern "C" esimp::Application_if *sim_register(esimp::MCU_if *obj) {
  mcu = obj;
  return &app;
}
