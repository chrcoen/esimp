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
static esimp::Thread_if *main_thread;
static esimp::Thread_if *thread1;
static esimp::Thread_if *thread2;
static esimp::Timer_if *timer1;

static int mystatic1;
static int mystatic2 = 99;

void Application::run_main(esimp::Thread_if *thread) {
  main_thread = thread;
  thread1 = mcu->create_thread("thread1", nullptr);
  thread2 = mcu->create_thread("thread2", nullptr);
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
    thread1->switch_to();
  }
}

void Application::run_thread(esimp::Thread_if *thread, const char *name,
                             void *pdata) {
  if (thread == thread1) {
    for (;;) {
      mcu->log("Thread 1");
      mcu->busy_wait_ns(5e8);
      thread2->switch_to();
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
      main_thread->switch_to();
    }
  } else {
    assert(0);
  }
}

void Application::run_isr(esimp::IRQ_if *irq) {
  if (irq == timer1->get_irq()) {
    timer1->clear_irq();
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
