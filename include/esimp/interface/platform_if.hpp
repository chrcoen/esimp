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
  virtual bool is_enabled() = 0;
  virtual void set_pending(bool val) = 0;
  virtual void set_pending_from_sw(bool val) = 0;
  virtual void set_prio(unsigned int val) = 0;
  virtual unsigned int get_prio() = 0;
  virtual void set_pdata(void *d) = 0;
  virtual void* get_pdata() = 0;
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
  virtual void clear_irq() = 0;
  virtual void trigger_irq() = 0;
  virtual ~Timer_if() {}
};

class UART_if {
 public:
  enum class Parity {
    None,
    Odd,
    Even,
  };
  enum class Irq : uint32_t {
    TxFifoNotFull = (1U << 0),
    TxFifoEmpty = (1U << 1),
    TxFifoThresholdReached = (1U << 2),
    TransferComplete = (1U << 3),
    RxFifoNotEmpty = (1U << 4),
    RxFifoFull = (1U << 5),
    RxFifoThresholdReached = (1U << 6),
    RxFifoOverflow = (1U << 7),
    ParityError = (1U << 8),
    FramingError = (1U << 9),
  };
  class IrqFlags {
  public:
    IrqFlags(uint32_t flags = 0) : flags(flags) {}
    void set(Irq irq) { 
      flags |= static_cast<uint32_t>(irq);
    }
    void clear(Irq irq) {
      flags &= ~static_cast<uint32_t>(irq);
    }
    bool is_set(Irq irq) {
      return (flags & static_cast<uint32_t>(irq)) > 0;
    }
    IrqFlags operator|(IrqFlags other) {
      return IrqFlags(flags | static_cast<uint32_t>(other.flags));
    }
    IrqFlags operator&(IrqFlags other) {
      return IrqFlags(flags & static_cast<uint32_t>(other.flags));
    }
    bool is_any_set() {
      return flags > 0;
    }
    private:
      uint32_t flags;
  };
  struct Config {
    int baudrate;
    Parity parity;
    int stop_bits;
    int data_bits;
    int tx_fifo_threshold;
    int rx_fifo_threshold;
    IrqFlags enabled_irqs;
  };
  struct Status {
    IrqFlags flags;
  };
  virtual void send(uint8_t val) = 0;
  virtual bool ready_to_send() = 0;
  virtual uint8_t recv() = 0;
  virtual bool set_config(const Config &cfg) = 0;
  virtual Config get_config() = 0;
  virtual Status get_status() = 0;
  virtual void clear_irq(Irq irq) = 0;
  virtual IRQ_if *get_irq() = 0;
  virtual void set_irq(IRQ_if *irq) = 0;
  virtual ~UART_if() {}
};


class GpioPin_if {
 public:
  enum flags {
    GPIO_INPUT = (1u << 8),
    GPIO_OUTPUT = (1u << 9),
    GPIO_INT_ENABLE = (1u << 14),
  };
  virtual void configure(enum flags flags) = 0;
  virtual void set() = 0;
  virtual void clear() = 0;
  virtual IRQ_if *get_irq() = 0;
  virtual void set_irq(IRQ_if *irq) = 0;
  virtual ~GpioPin_if() {}
};

class NVIC_if {
 public:
  virtual void set_primask(unsigned int n) = 0;
  virtual void set_basepri(unsigned int n) = 0;
  virtual int get_primask() = 0;
  virtual int get_basepri() = 0;
  virtual IRQ_if *get_irq(const char *name) = 0;
  virtual IRQ_if *get_irq(int irq) = 0;
  virtual ~NVIC_if() {}
};

class Thread_if {
 public:
  virtual void switch_to() = 0;
  virtual void set_name(const char *name) = 0;
  virtual ~Thread_if() {}
};

class Application_if {
 public:
  virtual void run_main(Thread_if *thread) = 0;
  virtual void run_thread(Thread_if *thread, const char *name,
                          void *pdata) = 0;
  virtual void run_isr(IRQ_if *irq) = 0;
  virtual ~Application_if() {}
};

class MCU_if {
 public:
  virtual void step(uint64_t dn) = 0;
  virtual void busy_wait_ns(uint64_t t_ns) = 0;
  virtual uint64_t get_timestamp_ns() = 0;
  virtual NVIC_if *get_nvic() = 0;
  virtual Thread_if *create_thread(const char *name, void *pdata) = 0;
  virtual Timer_if *create_timer(const char *name) = 0;
  virtual UART_if *get_uart(const char *name) = 0;
  virtual GpioPin_if *get_gpio_pin(const char *port, int pin) = 0;
  virtual void exit(ExitAction action, unsigned int fw_index) = 0;
  virtual void abort_thread(Thread_if *thread, Thread_if *next) = 0;
  virtual void log(const char *msg) = 0;
  virtual bool is_in_isr() = 0;

  virtual ~MCU_if() {}
};

} /* namespace esimp */

extern "C" esimp::Application_if *sim_register(esimp::MCU_if *obj);

#endif /* ESIMP_INTERFACE_PLATFORM_IF_HPP_ */
