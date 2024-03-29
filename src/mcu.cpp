/*
 * SPDX-License-Identifier: Apache-2.0
 * SPDX-FileCopyrightText: © 2022 Christoph Coenen <chrcoen@gmail.com>
 */

#include "esimp/platform/mcu.hpp"

#include <cstring>
#include <string>

#include "esimp/platform/trace.hpp"

namespace esimp {

using sc_core::sc_time;
using sc_core::SC_US;
using std::string;

static constexpr unsigned int default_f_clk_hz = 100e6;
static constexpr uint64_t default_time_quantum_ns = 1e5;
static constexpr double ns_per_s = 1.0e9;
static constexpr double ms_per_s = 1.0e3;

MCU::Configuration::Configuration()
    : sync_with_realtime(true), f_clk_hz(default_f_clk_hz), time_quantum_ns(default_time_quantum_ns) {}

MCU::MCU(sc_core::sc_module_name name)
    : sc_module(name),
      nvic(this),
      active_firmware(nullptr),
      next_firmware(nullptr),
      exit_action(ExitAction::Shutdown),
      active_context(nullptr),
      main_thread(nullptr),
      active_rtos_thread(nullptr) {
  SC_THREAD(start);
}

void MCU::start() {
  realtime.start();
  assert(!firmware.empty());
  next_firmware = firmware[0];
  sc_core::wait(sc_core::SC_ZERO_TIME);
  while (true) {
    active_firmware = next_firmware;
    active_firmware->load(this);
    main_thread = new RTOSThread(&active_context, this->basename(), "c_main", this, active_firmware->get_application(),
                                 nullptr, true);
    trace::set_active_context_var(&active_context);
    active_context = main_thread;
    active_rtos_thread = main_thread;
    main_thread->spawn();
    sc_core::wait(reset_event);
    active_context = nullptr;
    active_rtos_thread = nullptr;
    reset();
    active_firmware->close();
    switch (exit_action) {
      case ExitAction::Shutdown: {
        sc_core::sc_stop();
        sc_core::wait(sc_core::SC_ZERO_TIME);
        break;
      }
      case ExitAction::Reset: {
        break;
      }
    }
  }
}

MCU::~MCU() { reset(); }

void MCU::reset() {
  if (main_thread != nullptr) {
    main_thread->abort(main_thread);
    sc_core::wait(sc_core::SC_ZERO_TIME);
    delete main_thread;
    main_thread = nullptr;
  }
  for (auto *t : rtos_threads) {
    t->abort(t);
    sc_core::wait(sc_core::SC_ZERO_TIME);
    delete t;
  }
  rtos_threads.clear();
  for (auto t : isrs) {
    t.second->abort(t.second);
    sc_core::wait(sc_core::SC_ZERO_TIME);
    delete t.second;
  }
  isrs.clear();
  for (auto *t : timers) {
    delete t;
  }
  timers.clear();
  active_context = nullptr;
  active_rtos_thread = nullptr;
  nvic.reset();
}

NVIC_if *MCU::get_nvic() { return &nvic; }

Thread_if *MCU::create_thread(const char *name, void *pdata) {
  RTOSThread *t = new RTOSThread(&active_context, this->basename(), name, this, active_firmware->get_application(),
                                 pdata);
  rtos_threads.push_back(t);
  t->spawn();
  sc_core::wait(sc_core::SC_ZERO_TIME);
  return t;
}

Timer_if *MCU::create_timer(const char *name) {
  IRQ_if *irq = nvic.get_irq(name);
  assert(irq != nullptr);
  Timer *t = new Timer(this->basename(), name, irq);
  timers.push_back(t);
  t->spawn();
  sc_core::wait(sc_core::SC_ZERO_TIME);
  return t;
}

UART_if *MCU::get_uart(const char *name) {
  auto it = uarts.find(name);
  if (it == uarts.end()) {
    return NULL;
  }
  return it->second;
}

GpioPin_if *MCU::get_gpio_pin(const char *port, int pin) {
  auto it = gpios.find(port);
  if (it == gpios.end()) {
    return NULL;
  }
  auto it2 = it->second.find(pin);
  if (it2 == it->second.end()) {
    return NULL;
  }
  return it2->second;
}



void MCU::add_irq(int nr, const char *name) { 
  nvic.add_irq(IRQ(this, nr, name));
}

void MCU::add_uart(int irq_nr, const char *name, UART_if *uart) {
  if (nvic.get_irq(irq_nr) == NULL) {
    add_irq(irq_nr, name);
  }
  uart->set_irq(nvic.get_irq(name));
  uarts[name] = uart;
}

void MCU::add_gpio_pin(int irq_nr, const char *port, int pin, GpioPin_if *gpio_pin) {
  if (nvic.get_irq(irq_nr) == NULL) {
    add_irq(irq_nr, port);
  }
  gpio_pin->set_irq(nvic.get_irq(port));
  gpios[port][pin] = gpio_pin;
}


void MCU::log(const char *msg) { trace::msg("log", msg); }

void MCU::set_configuration(const Configuration &c) { config = c; }

MCU::Configuration MCU::get_configuration() { return config; }

void MCU::switch_thread(Thread_if *thread_if) {
  RTOSThread *thread = static_cast<RTOSThread *>(thread_if);

  if (thread == active_rtos_thread) {
    /* RTOS thread already running */
    return;
  }
  active_rtos_thread = thread;
  update(false);
}

void MCU::update_from_hw() { update(true); }

void MCU::update_from_sw() { update(false); }

ISR *MCU::get_isr(unsigned int prio) {
  auto it = isrs.find(prio);
  ISR *isr;
  if (it == isrs.end()) {
    isr = new ISR(&active_context, this->basename(), (string("isr_") + std::to_string(prio)).c_str(), this, active_firmware->get_application());
    isrs[prio] = isr;
    isr->spawn();
  } else {
    isr = it->second;
  }
  return isr;
}

void MCU::update(bool from_hw) {
  SystemcThread *current_thread = (SystemcThread *)sc_core::sc_get_current_process_b()->custom_handle;
  if (current_thread == NULL) {
    assert(from_hw);
  } else {
    assert(from_hw == (current_thread->get_type() == SystemcThread::Type::Hardware));
  }

  IRQ *old_irq = (active_context->get_type() == Context::Type::ISR)
                     ? static_cast<ISR *>(active_context)->get_irq()
                     : nullptr;
  IRQ *irq = nvic.active_irq();
  Context *next = nullptr;
  if (irq != nullptr) {
    unsigned int prio = irq->get_prio();
    ISR *isr = get_isr(prio);
    isr->set_irq(irq);
    next = isr->as_context();
    if ((next != active_context) && (old_irq != nullptr)) {
      unsigned int old_prio = old_irq->get_prio();
      assert(prio != old_prio);
      auto state = (prio < old_prio) ? IRQ::State::Suspended : IRQ::State::Idle;
      old_irq->set_state(state);
    }
  } else {
    next = active_rtos_thread;
  }
  if (next != active_context) {
    trace::msg("mcu", (string("Switch to ") + next->get_name()).c_str());
    next->activate();
    Context *current = active_context;
    active_context = next;
    if (from_hw) {
      current->activate();
    } else {
      current->suspend();
    }
  }
}

uint64_t MCU::instructions_ns() {
  unsigned int n = active_context->get_instruction_count();
  uint64_t t_ns = (n * ns_per_s) / config.f_clk_hz;
  return t_ns;
}

void MCU::step(uint64_t dn) {
  assert(active_context != nullptr);
  active_context->increment_instruction_count(dn);
  if (instructions_ns() > config.time_quantum_ns) {
    sync();
  }
}

void MCU::sync() {
  assert(active_context != nullptr);
  uint64_t t_ns = instructions_ns();
  active_context->clear_instruction_count();
  busy_wait_ns(t_ns);
}

void MCU::busy_wait_ns(uint64_t t_ns) {
  /* Sync with realtime */
  if (config.sync_with_realtime) {
    double t_sim_ms = sc_core::sc_time_stamp().to_seconds() * ms_per_s;
    realtime.sync(t_sim_ms);
  }

  /* Wait for t_buys_ns */
  assert(active_context != nullptr);
  active_context->wait(t_ns);
}

uint64_t MCU::get_timestamp_ns() {
  assert(active_context != nullptr);
  uint64_t t1 = (uint64_t)(sc_core::sc_time_stamp().to_seconds() * ns_per_s);
  uint64_t t2 = instructions_ns();
  uint64_t res = t1 + t2;
  return res;
}

void MCU::add_firmware(Firmware *fw) { firmware.push_back(fw); }

void MCU::exit(ExitAction action, unsigned int fw_index) {
  exit_action = action;
  assert(fw_index < firmware.size());
  next_firmware = firmware[fw_index];
  reset_event.notify();
  sc_core::wait(sc_core::SC_ZERO_TIME);
}

void MCU::abort_thread(Thread_if *thread_if, Thread_if *next_if) {
  RTOSThread *thread = static_cast<RTOSThread *>(thread_if);
  RTOSThread *next = static_cast<RTOSThread *>(next_if);

  rtos_threads.erase(std::remove(rtos_threads.begin(), rtos_threads.end(), thread), rtos_threads.end());

  Context *last_context = active_context;
  active_rtos_thread = next;
  update(false);
  thread->abort(last_context);
  sc_core::wait(sc_core::SC_ZERO_TIME);
}


bool MCU::is_in_isr()
{
  return active_context->get_type() == SystemcThread::Type::ISR;
}



} /* namespace esimp */
