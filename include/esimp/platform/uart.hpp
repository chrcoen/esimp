/*
 * SPDX-License-Identifier: Apache-2.0
 * SPDX-FileCopyrightText: © 2022 Christoph Coenen <chrcoen@gmail.com>
 */
#ifndef ESIMP_PLATFORM_UART_HPP_
#define ESIMP_PLATFORM_UART_HPP_

#include "esimp/interface/platform_if.hpp"
#include "esimp/platform/systemc_thread.hpp"

namespace esimp {

class UART: public sc_core::sc_module, public UART_if {
public:
  SC_HAS_PROCESS(UART);
  UART(sc_core::sc_module_name name, int fifo_size = 4);

  sc_core::sc_out<bool> tx;
  sc_core::sc_in<bool> rx;
  sc_core::sc_out<sc_dt::sc_lv<16> > tx_dbg;
  sc_core::sc_out<sc_dt::sc_lv<16> > rx_dbg;
  sc_core::sc_out<bool> tx_sample;
  sc_core::sc_out<bool> rx_sample;


  /* UART_if */
  virtual void send(uint8_t val) override;
  virtual bool ready_to_send() override;
  virtual uint8_t recv() override;
  virtual bool set_config(const Config &cfg) override;
  virtual Config get_config() override;
  virtual Status get_status() override;
  virtual void clear_irq(Irq irq) override;
  virtual IRQ_if *get_irq() override;
  virtual void set_irq(IRQ_if *irq) override;

private:
  sc_core::sc_fifo<uint8_t> rx_data;
  sc_core::sc_fifo<uint8_t> tx_data;
  IRQ_if *irq_if;
  Config cfg;
  Status status;
  void thread_rx();
  void thread_tx();
};



} /* namespace esimp */

#endif /* ESIMP_PLATFORM_UART_HPP_ */
