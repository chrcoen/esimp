/*
 * SPDX-License-Identifier: Apache-2.0
 * SPDX-FileCopyrightText: © 2022 Christoph Coenen <chrcoen@gmail.com>
 */

#include <cmath>
#include <cstdio>
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>

#include "esimp/interface/platform_if.hpp"

#define RUN_TEST(x)                             \
  success = true;                               \
  std::cout << "RUN " << #x << std::endl;       \
  setup();                                      \
  x();                                          \
  teardown();                                   \
  if (success) {                                \
    std::cout << "SUCCESS " << #x << std::endl; \
  } else {                                      \
    std::cout << "FAIL " << #x << std::endl;    \
    total_success = false;                      \
  }

extern void stop_while_1();
extern void while_1();


static void run_thread1();
static void run_thread2();
static void timer1_isr();
static void timer2_isr();

typedef void (*Function_t)(void);

Function_t timer1_fct;
Function_t timer2_fct;
Function_t thread1_fct;
Function_t thread2_fct;
Function_t thread_dyn_fct;
uint64_t t0;

class Application : public esimp::Application_if {
 public:
  void run_main(esimp::Thread_if *thread) override;
  void run_thread(esimp::Thread_if *thread, const char *name,
                  void *pdata) override;
  void run_isr(esimp::IRQ_if *irq) override;
};

struct TraceItem {
  uint64_t t;
  std::string fct;
};

static Application app;
static esimp::MCU_if *mcu;
static bool success;
static std::vector<TraceItem> trace;
static std::vector<TraceItem> expectation;

static esimp::Thread_if *main_thread;
static esimp::Thread_if *thread1;
static esimp::Thread_if *thread2;
static esimp::Thread_if *thread_dyn;
static int thread1_cnt;
static int thread2_cnt;
static int thread_dyn_cnt;

static esimp::Timer_if *timer1;
static esimp::Timer_if *timer2;
static int timer1_disable_count;
static int timer2_disable_count;

extern "C" esimp::Application_if *sim_register(esimp::MCU_if *obj) {
  mcu = obj;
  return &app;
}


extern "C" void simulation_step(int n) { mcu->step(n); }


#define CHECK(test)                                                  \
  do {                                                               \
    if (!(test)) {                                                   \
      std::cerr << "Check failed at " << __FILE__ << ":" << __LINE__ \
                << std::endl;                                        \
      success = false;                                               \
    }                                                                \
  } while (0)

void Application::run_thread(esimp::Thread_if *thread, const char *name,
                             void *pdata) {
  if (thread == thread1) {
    run_thread1();
  } else if (thread == thread2) {
    run_thread2();
  } else if (thread == thread_dyn) {
    thread_dyn_fct();
  } else {
    CHECK(0);
  }
}

void Application::run_isr(esimp::IRQ_if *irq) {
  if (irq == timer1->get_irq()) {
    timer1_isr();
  } else if (irq == timer2->get_irq()) {
    timer2_isr();
  } else {
    CHECK(0);
  }
}

void time_call(const char *name) {
  TraceItem item;
  item.t = mcu->get_timestamp_ns() - t0;
  item.fct = name;
  trace.push_back(item);
}

void expect(uint64_t t, const char *name) {
  TraceItem item;
  item.t = t;
  item.fct = name;
  expectation.push_back(item);
}

static void work_100us(void) { mcu->busy_wait_ns(100e3); }

static void work_300us(void) { mcu->busy_wait_ns(300e3); }

static void stop_timer1(void) { timer1->stop(); }

static void mask_isr_work_100us() {
  mcu->get_nvic()->set_primask(1);
  mcu->busy_wait_ns(100e3);
}

static void mask_isr_for_300us_work_100us() {
  mcu->get_nvic()->set_primask(1);
  mcu->busy_wait_ns(300e3);
  mcu->get_nvic()->set_primask(0);
  mcu->busy_wait_ns(100e3);
}

static void switch_to_main() {
  main_thread->switch_to();
}

static void switch_to_thread1() {
  thread1->switch_to();
}

static void switch_to_thread2() {
  thread2->switch_to();
}

static void loop_and_count() {
  for (;;) {
    thread_dyn_cnt++;
    mcu->busy_wait_ns(100e3);
  }
}

static void check_expectations() {
  if (expectation.empty()) {
    return;
  }
  // for (auto &item : trace) {
  //   std::cout << "TRACE " << item.t << ": " << item.fct << std::endl;
  // }
  if (trace.size() != expectation.size()) {
    success = false;
    std::cerr << "Fail: Expected " << expectation.size()
              << " trace items but got " << trace.size() << std::endl;
  }
  for (int i = 0; i < std::min(expectation.size(), trace.size()); i++) {
    CHECK(i < trace.size());
    if (i >= trace.size()) {
      continue;
    }
    TraceItem exp = expectation[i];
    TraceItem got = trace[i];
    uint64_t dt = 1e3;
    if (std::string(got.fct) != exp.fct) {
      success = false;
      std::cerr << "Fail: Expected '" << exp.fct << "' but got '" << got.fct
                << "'" << std::endl;
    } else if ((got.t > (exp.t + dt)) || (got.t < ((exp.t > dt) ? (exp.t - dt) : 0))) {
      success = false;
      std::cerr << "Fail: Expected '" << exp.fct << "' at " << exp.t
                << " but was at " << got.t << std::endl;
    }
  }
}

void run_thread1() {
  for (;;) {
    time_call("thread1");
    thread1_fct();
  }
}

void run_thread2() {
  for(;;) {
    time_call("thread2");
    thread2_fct();
  }
}

void timer1_isr() {
  time_call("timer1_begin");
  timer1->clear_irq();
  if ((timer1_disable_count > 0) && (--timer1_disable_count == 0)) {
    timer1->stop();
  }
  if (timer1_fct != nullptr) {
    timer1_fct();
  }
  time_call("timer1_end");
}

void timer2_isr() {
  time_call("timer2_begin");
  timer2->clear_irq();
  if ((timer2_disable_count > 0) && (--timer2_disable_count == 0)) {
    timer2->stop();
  }
  if (timer2_fct != nullptr) {
    timer2_fct();
  }
  time_call("timer2_end");
}

static void setup() {
  trace.clear();
  expectation.clear();
  timer1_fct = nullptr;
  timer2_fct = nullptr;
  timer1_disable_count = 0;
  timer2_disable_count = 0;
  thread_dyn_cnt = 0;
  timer1->stop();
  timer2->stop();
  timer1->get_irq()->disable();
  timer1->get_irq()->set_prio(0);
  timer2->get_irq()->disable();
  timer2->get_irq()->set_prio(0);
  timer1->clear_irq();
  timer2->clear_irq();
  mcu->get_nvic()->set_primask(0);
  mcu->get_nvic()->set_basepri(0);
  t0 = mcu->get_timestamp_ns();
}

static void teardown() {
  check_expectations();
  setup();
}

void test_suspend() {
  auto t0 = mcu->get_timestamp_ns();
  mcu->busy_wait_ns(200e3);
  auto t1 = mcu->get_timestamp_ns();
  auto dt = t1 - t0;
  CHECK(dt > 190e3);
  CHECK(dt < 210e3);
}

void test_simple_timer_interrupt() {
  expect(200e3, "timer1_begin");
  expect(200e3, "timer1_end");
  timer1->get_irq()->enable();
  timer1->set_mode(esimp::TimerMode::SingleShot);
  timer1->set_period_ns(200e3);
  timer1->start();
  mcu->busy_wait_ns(250e3);
}

void test_interrupt_twice() {
  timer1_disable_count = 2;
  expect(200e3, "timer1_begin");
  expect(200e3, "timer1_end");
  expect(400e3, "timer1_begin");
  expect(400e3, "timer1_end");
  timer1->get_irq()->enable();
  timer1->set_mode(esimp::TimerMode::Continuous);
  timer1->set_period_ns(200e3);
  timer1->start();
  mcu->busy_wait_ns(1000e3);
}

void test_nested_interrupts() {
  timer1_fct = work_100us;
  timer2_fct = work_100us;
  timer1->set_mode(esimp::TimerMode::SingleShot);
  timer2->set_mode(esimp::TimerMode::SingleShot);
  timer1->set_period_ns(200e3);
  timer2->set_period_ns(250e3);
  timer1->get_irq()->set_prio(5);
  timer2->get_irq()->set_prio(4);
  timer1->get_irq()->enable();
  timer2->get_irq()->enable();
  expect(200e3, "timer1_begin");
  expect(250e3, "timer2_begin");
  expect(350e3, "timer2_end");
  expect(400e3, "timer1_end");
  timer1->start();
  timer2->start();
  mcu->busy_wait_ns(1000e3);
}

void test_no_space_between() {
  timer1_fct = work_300us;
  timer2_fct = stop_timer1;
  timer1->set_mode(esimp::TimerMode::Continuous);
  timer2->set_mode(esimp::TimerMode::SingleShot);
  timer1->set_period_ns(100e3);
  timer2->set_period_ns(800e3);
  timer1->get_irq()->set_prio(5);
  timer2->get_irq()->set_prio(4);
  timer1->get_irq()->enable();
  timer2->get_irq()->enable();
  expect(100e3, "timer1_begin");
  expect(400e3, "timer1_end");
  expect(400e3, "timer1_begin");
  expect(700e3, "timer1_end");
  expect(700e3, "timer1_begin");
  expect(800e3, "timer2_begin");
  expect(800e3, "timer2_end");
  expect(1000e3, "timer1_end");
  expect(1000e3, "timer1_begin");
  expect(1300e3, "timer1_end");
  timer1->start();
  timer2->start();
  mcu->busy_wait_ns(1500e3);
}

void test_lower_prio_waiting() {
  timer1_fct = work_300us;
  timer2_fct = work_300us;
  timer1->set_mode(esimp::TimerMode::SingleShot);
  timer2->set_mode(esimp::TimerMode::SingleShot);
  timer1->set_period_ns(300e3);
  timer2->set_period_ns(100e3);
  timer1->get_irq()->set_prio(5);
  timer2->get_irq()->set_prio(4);
  timer1->get_irq()->enable();
  timer2->get_irq()->enable();
  expect(100e3, "timer2_begin");
  expect(400e3, "timer2_end");
  expect(400e3, "timer1_begin");
  expect(700e3, "timer1_end");
  timer1->start();
  timer2->start();
  mcu->busy_wait_ns(1000e3);
}

void test_sw_irq() {
  timer1->set_mode(esimp::TimerMode::SingleShot);
  timer1->set_period_ns(100e3);
  timer1->get_irq()->set_prio(4);
  timer1->get_irq()->enable();
  expect(100e3, "timer1_begin");
  expect(100e3, "timer1_end");
  mcu->busy_wait_ns(100e3);
  timer1->trigger_irq();
  mcu->busy_wait_ns(100e3);
}

void test_mask_interrupt() {
  timer1->set_mode(esimp::TimerMode::SingleShot);
  timer1->set_period_ns(100e3);
  timer1->get_irq()->set_prio(4);
  timer1->get_irq()->enable();
  expect(500e3, "timer1_begin");
  expect(500e3, "timer1_end");
  mcu->get_nvic()->set_primask(1);
  timer1->start();
  mcu->busy_wait_ns(500e3);
  mcu->get_nvic()->set_primask(0);
  mcu->busy_wait_ns(500e3);
}

void test_two_interrupts_pending() {
  timer2_fct = work_300us;
  timer1->set_mode(esimp::TimerMode::SingleShot);
  timer2->set_mode(esimp::TimerMode::SingleShot);
  timer1->set_period_ns(100e3);
  timer2->set_period_ns(100e3);
  timer1->get_irq()->set_prio(5);
  timer2->get_irq()->set_prio(4);
  timer1->get_irq()->enable();
  timer2->get_irq()->enable();
  expect(500e3, "timer2_begin");
  expect(800e3, "timer2_end");
  expect(800e3, "timer1_begin");
  expect(800e3, "timer1_end");
  mcu->get_nvic()->set_primask(1);
  timer1->start();
  timer2->start();
  mcu->busy_wait_ns(500e3);
  mcu->get_nvic()->set_primask(0);
  mcu->busy_wait_ns(500e3);
}

void test_mask_interrupt_with_basepri() {
  timer1->set_mode(esimp::TimerMode::SingleShot);
  timer1->set_period_ns(100e3);
  timer1->get_irq()->set_prio(4);
  timer1->get_irq()->enable();
  expect(500e3, "timer1_begin");
  expect(500e3, "timer1_end");
  mcu->get_nvic()->set_basepri(1);
  timer1->start();
  mcu->busy_wait_ns(500e3);
  mcu->get_nvic()->set_basepri(0);
  mcu->busy_wait_ns(500e3);
}

void test_partially_masked() {
  timer1->set_mode(esimp::TimerMode::SingleShot);
  timer2->set_mode(esimp::TimerMode::SingleShot);
  timer1->set_period_ns(100e3);
  timer2->set_period_ns(200e3);
  timer1->get_irq()->set_prio(5);
  timer2->get_irq()->set_prio(4);
  timer1->get_irq()->enable();
  timer2->get_irq()->enable();
  expect(200e3, "timer2_begin");
  expect(200e3, "timer2_end");
  expect(500e3, "timer1_begin");
  expect(500e3, "timer1_end");
  mcu->get_nvic()->set_basepri(5);
  timer1->start();
  timer2->start();
  mcu->busy_wait_ns(500e3);
  mcu->get_nvic()->set_basepri(6);
  mcu->busy_wait_ns(500e3);
}

void test_same_prio() {
  timer1->set_mode(esimp::TimerMode::SingleShot);
  timer2->set_mode(esimp::TimerMode::SingleShot);
  timer1->set_period_ns(100e3);
  timer2->set_period_ns(200e3);
  timer1->get_irq()->set_prio(4);
  timer2->get_irq()->set_prio(4);
  timer1->get_irq()->enable();
  timer2->get_irq()->enable();
  expect(100e3, "timer1_begin");
  expect(100e3, "timer1_end");
  expect(200e3, "timer2_begin");
  expect(200e3, "timer2_end");
  timer1->start();
  timer2->start();
  mcu->busy_wait_ns(500e3);
}

void test_same_prio_pending() {
  timer1->set_mode(esimp::TimerMode::SingleShot);
  timer2->set_mode(esimp::TimerMode::SingleShot);
  timer1->set_period_ns(100e3);
  timer2->set_period_ns(200e3);
  timer1->get_irq()->set_prio(4);
  timer2->get_irq()->set_prio(4);
  timer1->get_irq()->enable();
  timer2->get_irq()->enable();
  timer1_fct = work_300us;
  timer2_fct = work_300us;
  expect(100e3, "timer1_begin");
  expect(400e3, "timer1_end");
  expect(400e3, "timer2_begin");
  expect(700e3, "timer2_end");
  timer1->start();
  timer2->start();
  mcu->busy_wait_ns(500e3);
}

void test_mask_from_isr() {
  timer1->set_mode(esimp::TimerMode::SingleShot);
  timer2->set_mode(esimp::TimerMode::SingleShot);
  timer1->set_period_ns(100e3);
  timer2->set_period_ns(200e3);
  timer1->get_irq()->set_prio(5);
  timer2->get_irq()->set_prio(4);
  timer1->get_irq()->enable();
  timer2->get_irq()->enable();
  timer1_fct = mask_isr_for_300us_work_100us;
  timer2_fct = work_300us;
  expect(100e3, "timer1_begin");
  expect(400e3, "timer2_begin");
  expect(700e3, "timer2_end");
  expect(800e3, "timer1_end");
  timer1->start();
  timer2->start();
  mcu->busy_wait_ns(500e3);
}

void test_finish_masked_interrupt() {
  timer1->set_mode(esimp::TimerMode::Continuous);
  timer2->set_mode(esimp::TimerMode::SingleShot);
  timer1->set_period_ns(100e3);
  timer2->set_period_ns(200e3);
  timer1->get_irq()->set_prio(5);
  timer2->get_irq()->set_prio(4);
  timer1->get_irq()->enable();
  timer2->get_irq()->enable();
  timer1_fct = work_300us;
  timer2_fct = mask_isr_work_100us;
  expect(100e3, "timer1_begin");
  expect(200e3, "timer2_begin");
  expect(300e3, "timer2_end");
  expect(500e3, "timer1_end");
  timer1->start();
  timer2->start();
  mcu->busy_wait_ns(1000e3);
}

void test_run_thread1() {
  expect(0, "thread1");
  thread1_fct = switch_to_main;
  thread1->switch_to();
}

void test_run_thread2() {
  expect(0, "thread2");
  thread2_fct = switch_to_main;
  thread2->switch_to();
}

void test_run_thread1_thread2() {
  expect(0, "thread1");
  expect(0, "thread2");
  thread1_fct = switch_to_thread2;
  thread2_fct = switch_to_main;
  thread1->switch_to();
}

void test_switch_from_isr() {
  timer1->set_mode(esimp::TimerMode::SingleShot);
  timer1->set_period_ns(100e3);
  timer1->get_irq()->set_prio(5);
  timer1->get_irq()->enable();
  timer1_fct = switch_to_thread1;
  thread1_fct = switch_to_main;
  expect(100e3, "timer1_begin");
  expect(100e3, "timer1_end");
  expect(100e3, "thread1");
  timer1->start();
  mcu->busy_wait_ns(200e3);
}

void test_while_1() {
  timer1->set_mode(esimp::TimerMode::SingleShot);
  timer1->set_period_ns(1e6);
  timer1->get_irq()->set_prio(5);
  timer1->get_irq()->enable();
  timer1_fct = stop_while_1;
  expect(1e6, "timer1_begin");
  expect(1e6, "timer1_end");
  timer1->start();
  while_1();
}


static void test_thread_abort_other_thread_dyn() {
  time_call("thread_dyn_begin");
  main_thread->switch_to();
  time_call("thread_dyn_end");
}

void test_thread_abort_other_thread() {
  expect(100e3, "thread_dyn_begin");
  expect(100e3, "main_begin");
  expect(200e3, "main_end");
  thread_dyn_fct = test_thread_abort_other_thread_dyn;
  thread_dyn = mcu->create_thread("test", nullptr);
  mcu->busy_wait_ns(100e3);
  thread_dyn->switch_to();
  time_call("main_begin");
  mcu->abort_thread(thread_dyn, main_thread);
  mcu->busy_wait_ns(100e3);
  time_call("main_end");
}

static void test_thread_abort_same_thread_dyn() {
  time_call("thread_dyn_begin");
  mcu->busy_wait_ns(100e3);
  mcu->abort_thread(thread_dyn, main_thread);
  mcu->busy_wait_ns(100e3);
  time_call("thread_dyn_end");
}

void test_thread_abort_same_thread() {
  expect(100e3, "thread_dyn_begin");
  expect(200e3, "main_begin");
  expect(300e3, "main_end");
  mcu->busy_wait_ns(100e3);
  thread_dyn_fct = test_thread_abort_same_thread_dyn;
  thread_dyn = mcu->create_thread("test", nullptr);
  thread_dyn->switch_to();
  time_call("main_begin");
  mcu->busy_wait_ns(100e3);
  time_call("main_end");
}

static void test_thread_abort_other_from_isr_thread_dyn() {
  time_call("thread_dyn_begin");
  main_thread->switch_to();
  time_call("thread_dyn_end");
}

static void test_thread_abort_other_thread_from_isr_timer1_fct() {
  mcu->abort_thread(thread_dyn, main_thread);
}

void test_thread_abort_other_thread_from_isr() {
  expect(100e3, "thread_dyn_begin");
  expect(100e3, "main_begin");
  expect(300e3, "timer1_begin");
  expect(300e3, "timer1_end");
  expect(600e3, "main_end");
  mcu->busy_wait_ns(100e3);
  timer1->set_mode(esimp::TimerMode::SingleShot);
  timer1->set_period_ns(200e3);
  timer1->get_irq()->set_prio(5);
  timer1->get_irq()->enable();
  timer1_fct = test_thread_abort_other_thread_from_isr_timer1_fct;
  thread_dyn_fct = test_thread_abort_other_from_isr_thread_dyn;
  thread_dyn = mcu->create_thread("test", nullptr);
  timer1->start();
  thread_dyn->switch_to();
  time_call("main_begin");
  mcu->busy_wait_ns(500e3);
  time_call("main_end");
}

static void test_thread_abort_same_from_isr_thread_dyn() {
  time_call("thread_dyn_begin");
  mcu->busy_wait_ns(1000e3);
  time_call("thread_dyn_end");
}

static void test_thread_abort_same_thread_from_isr_timer1_fct() {
  mcu->abort_thread(thread_dyn, main_thread);
}

void test_thread_abort_same_thread_from_isr() {
  expect(100e3, "thread_dyn_begin");
  expect(300e3, "timer1_begin");
  expect(300e3, "timer1_end");
  expect(300e3, "main_begin");
  expect(400e3, "main_end");
  mcu->busy_wait_ns(100e3);
  timer1->set_mode(esimp::TimerMode::SingleShot);
  timer1->set_period_ns(200e3);
  timer1->get_irq()->set_prio(5);
  timer1->get_irq()->enable();
  timer1_fct = test_thread_abort_same_thread_from_isr_timer1_fct;
  thread_dyn_fct = test_thread_abort_same_from_isr_thread_dyn;
  thread_dyn = mcu->create_thread("test", nullptr);
  timer1->start();
  thread_dyn->switch_to();
  time_call("main_begin");
  mcu->busy_wait_ns(100e3);
  time_call("main_end");
}

void Application::run_main(esimp::Thread_if *thread) {
  main_thread = thread;
  thread1 = mcu->create_thread("thread1", nullptr);
  thread2 = mcu->create_thread("thread2", nullptr);
  timer1 = mcu->create_timer("timer1");
  timer2 = mcu->create_timer("timer2");

  success = true;
  bool total_success = true;

  RUN_TEST(test_suspend);
  RUN_TEST(test_simple_timer_interrupt);
  RUN_TEST(test_interrupt_twice);
  RUN_TEST(test_nested_interrupts);
  RUN_TEST(test_no_space_between);
  RUN_TEST(test_lower_prio_waiting);
  RUN_TEST(test_sw_irq);
  RUN_TEST(test_mask_interrupt);
  RUN_TEST(test_two_interrupts_pending);
  RUN_TEST(test_mask_interrupt_with_basepri);
  RUN_TEST(test_partially_masked);
  RUN_TEST(test_same_prio);
  RUN_TEST(test_same_prio_pending);
  RUN_TEST(test_mask_from_isr);
  RUN_TEST(test_finish_masked_interrupt);
  RUN_TEST(test_run_thread1);
  RUN_TEST(test_run_thread2);
  RUN_TEST(test_run_thread1_thread2);
  RUN_TEST(test_switch_from_isr);
  RUN_TEST(test_while_1);
  RUN_TEST(test_thread_abort_other_thread);
  RUN_TEST(test_thread_abort_same_thread);
  RUN_TEST(test_thread_abort_other_thread_from_isr);
  RUN_TEST(test_thread_abort_same_thread_from_isr);


  std::cout << "=====================================" << std::endl;
  if (total_success) {
    std::cout << "Success" << std::endl;
  } else {
    std::cerr << "Tests failed" << std::endl;
  }
  std::cout << "=====================================" << std::endl;
  mcu->exit(esimp::ExitAction::Shutdown, 0);
}
