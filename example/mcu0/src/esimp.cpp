#include <assert.h>

#include <iostream>

#include "esimp/interface/platform_if.hpp"

extern double square_root(int n);

class Application : public esimp::Application_if {
 public:
  void run_main() override;
  void run_thread(esimp::RTOSThread_if *thread, const char *name,
                  const void *pdata) override;
  void run_isr(esimp::IRQ_if *irq) override;

 private:
};

static Application app;
static esimp::MCU_if *mcu;
static esimp::RTOSThread_if *thread1;
static esimp::RTOSThread_if *thread2;
static esimp::Timer_if *timer1;

static int mystatic1;
static int mystatic2 = 99;

void Application::run_main() {
  thread1 = mcu->create_thread("thread1", nullptr);
  thread2 = mcu->create_thread("thread2", nullptr);
  thread1->start();
  thread2->start();
  timer1 = mcu->create_timer("timer1");
  timer1->set_period_ns(5e8);
  timer1->set_mode(esimp::TimerMode::Continuous);
  timer1->start();
  mcu->log("Main Start");
  char txt[128];
  sprintf(txt, "%d - %d", mystatic1, mystatic2);
  mcu->log(txt);
  mystatic1 = 44;
  mystatic2 = 55;
  sprintf(txt, "%d - %d", mystatic1, mystatic2);
  mcu->log(txt);
  for (;;) {
    mcu->log("Main");
    mcu->busy_wait_ns(1e9);
  }
}

void Application::run_thread(esimp::RTOSThread_if *thread, const char *name,
                             const void *pdata) {
  if (thread == thread1) {
    for (;;) {
      mcu->log("Thread 1");
      mcu->busy_wait_ns(5e8);
    }
  } else if (thread == thread2) {
    int count = 0;
    for (;;) {
      mcu->log("Thread 2");
      mcu->busy_wait_ns(2e8);
      count++;
      if (count >= 5) {
        mcu->exit(esimp::ExitAction::Reset, 0);
      }
    }
  } else {
    assert(0);
  }
}

void Application::run_isr(esimp::IRQ_if *irq) {
  irq->clear();
  if (irq == timer1->get_irq()) {
    mcu->log("Timer1 tick");
  } else {
    mcu->log("Unknown isr");
  }
}

extern "C" esimp::Application_if *sim_register(esimp::MCU_if *obj) {
  mcu = obj;
  esimp::Application_if *res = &app;
  return res;
}

extern "C" void simulation_step(int n) { mcu->step(n); }
