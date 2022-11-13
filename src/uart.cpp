/*
 * SPDX-License-Identifier: Apache-2.0
 * SPDX-FileCopyrightText: © 2022 Christoph Coenen <chrcoen@gmail.com>
 */

#include "esimp/platform/uart.hpp"

#include <assert.h>

namespace esimp {


UART::UART(sc_core::sc_module_name name, int fifo_size) :
  rx_data(fifo_size),
  tx_data(fifo_size),
  irq_if(nullptr)
{
  SC_THREAD(thread_rx);
  sensitive << rx;
  SC_THREAD(thread_tx);
  cfg.baudrate = 38400;
  cfg.data_bits = 8;
  cfg.stop_bits = 1;
  cfg.parity = Parity::None;
  cfg.rx_fifo_threshold = 1;
  cfg.tx_fifo_threshold = 1;
  status.flags.set(Irq::TxFifoEmpty);
  status.flags.set(Irq::TxFifoNotFull);
  status.flags.set(Irq::TxFifoThresholdReached);
  status.flags.set(Irq::TxFifoEmpty);
  status.flags.set(Irq::TxFifoEmpty);
}


void UART::thread_rx() {
  for (;;) {
    rx_sample = false;
    rx_dbg = "zzzzzzzzzzzzzzzz";

    /* Wait for start bit (rx == 0) */ 
    wait();
    if (rx) {
      continue;
    }
    int parity = 0;

    /* Wait half a bit time */
    auto tbit = sc_core::sc_time(1.0 / cfg.baudrate, sc_core::SC_SEC);
    wait(tbit / 2);
    rx_sample = true;

    /* Check for start bit == 0 */
    if (rx) {
      wait(20*tbit);
      continue;
    }

    /* Read data bits */
    uint8_t d = 0;
    for (int i = 0; i < cfg.data_bits; i++) {
      wait(tbit);
      rx_sample = !rx_sample;
      d |= rx ? (1 << i) : 0;
      parity += rx ? 1 : 0;
      rx_dbg = d;
    }

    /* Read and check parity bit */
    if (cfg.parity != Parity::None) {
      wait(tbit);
      parity += rx ? 1 : 0;
      if (((cfg.parity == Parity::Odd) && ((parity % 2) == 0)) ||
          ((cfg.parity == Parity::Even) && ((parity % 2) != 0))) {
        status.flags.set(Irq::ParityError);
      }
    }

    /* Wait and check for stop bit == 1 */
    wait(tbit);
    rx_sample = !rx_sample;
    if (!rx) {
      status.flags.set(Irq::FramingError);
    }

    /* Wait for idle == 1 */
    while (!rx) {
      wait();
    }

    /* Update irqs */
    if (rx_data.num_free() == 0) {
      status.flags.set(Irq::RxFifoOverflow);
    } else {
      rx_data.write(d);
      sc_core::wait(sc_core::SC_ZERO_TIME);
    }
    status.flags.set(Irq::RxFifoNotEmpty);
    if (rx_data.num_available() >= cfg.rx_fifo_threshold) {
      status.flags.set(Irq::RxFifoThresholdReached);
    }
    if (rx_data.num_free() == 0) {
      status.flags.set(Irq::RxFifoFull);
    }
    if (irq_if != nullptr) {
      irq_if->set_pending((status.flags & cfg.enabled_irqs).is_any_set());
    }
  }
}


void UART::thread_tx() {
  tx = true;
  tx_sample = false;
  tx_dbg = "zzzzzzzzzzzzzzzz";
  for (;;) {
    uint8_t d = tx_data.read();

    status.flags.set(Irq::TxFifoNotFull);
    if (tx_data.num_available() <= cfg.tx_fifo_threshold) {
      status.flags.set(Irq::TxFifoThresholdReached);
    }
    if (tx_data.num_available() == 0) {
      status.flags.set(Irq::TxFifoEmpty);
    }
    if (irq_if != nullptr) {
      irq_if->set_pending((status.flags & cfg.enabled_irqs).is_any_set());
    }

    /* Send start bit */
    tx_dbg = d;
    tx = false;
    tx_sample = true;
    auto tbit = sc_core::sc_time(1.0 / cfg.baudrate, sc_core::SC_SEC);
    wait(tbit);
    int parity = 0;

    /* Send data bits */
    for (int i = 0; i < cfg.data_bits; i++) {
      bool value = (d & (1 << i)) > 0;
      tx = value;
      parity += value ? 1 : 0;
      tx_sample = !tx_sample;
      wait(tbit);
    }

    /* Send parity bit */
    if (cfg.parity != Parity::None) {
      if (cfg.parity == Parity::Odd) {
        tx = (parity % 2) == 0;
      } else {
        tx = (parity % 2) != 0;
      }
      tx_sample = !tx_sample;
      wait(tbit);
    }

    /* Send stop bit */
    tx = true;
    tx_sample = !tx_sample;
    wait(tbit);
    tx_dbg = "zzzzzzzzzzzzzzzz";
    tx_sample = false;

    if (tx_data.num_available() == 0) {
      status.flags.set(Irq::TransferComplete);
    }
    if (irq_if != nullptr) {
      irq_if->set_pending((status.flags & cfg.enabled_irqs).is_any_set());
    }
  }
}


void UART::send(uint8_t val) {
  if (tx_data.num_free() > 0) {
    tx_data.nb_write(val);
    status.flags.clear(Irq::TxFifoEmpty);
    if (tx_data.num_free() > 0) {
      status.flags.clear(Irq::TxFifoNotFull);
    }
    if (tx_data.num_available() > cfg.tx_fifo_threshold) {
      status.flags.clear(Irq::TxFifoThresholdReached);
    }
    if (irq_if != nullptr) {
      irq_if->set_pending_from_sw((status.flags & cfg.enabled_irqs).is_any_set());
    }
  }
}


bool UART::ready_to_send() {
  return tx_data.num_free() > 0;
}


uint8_t UART::recv() {
  uint8_t res = 0;
  if (rx_data.num_available() > 0) {
    rx_data.nb_read(res);
    status.flags.clear(Irq::RxFifoFull);
    if (rx_data.num_available() == 0) {
      status.flags.clear(Irq::RxFifoNotEmpty);
    }
    if (rx_data.num_available() < cfg.rx_fifo_threshold) {
      status.flags.clear(Irq::RxFifoThresholdReached);
    }
    if (irq_if != nullptr) {
      irq_if->set_pending_from_sw((status.flags & cfg.enabled_irqs).is_any_set());
    }
  }
  return res;
}



bool UART::set_config(const Config &c) {
  if ((c.stop_bits != 1) || (c.data_bits < 1) || (c.data_bits > 16)) {
    return false;
  }
  cfg = c;
  return true;
}

UART::Config UART::get_config() {
  return cfg;
}

UART::Status UART::get_status() {
  return status;
}


void UART::clear_irq(Irq irq) {
  status.flags.clear(irq);
  if (irq_if != nullptr) {
    irq_if->set_pending_from_sw((status.flags & cfg.enabled_irqs).is_any_set());
  }
}


IRQ_if* UART::get_irq() {
  return irq_if;
}

void UART::set_irq(IRQ_if *irq) {
  irq_if = irq;
}



} /* namespace esimp */
