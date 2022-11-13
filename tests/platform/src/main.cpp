/*
 * SPDX-License-Identifier: Apache-2.0
 * SPDX-FileCopyrightText: © 2022 Christoph Coenen <chrcoen@gmail.com>
 */

#include <systemc>

#include "esimp/interface/platform_if.hpp"
#include "esimp/platform/firmware.hpp"
#include "esimp/platform/mcu.hpp"
#include "esimp/platform/systemc_thread.hpp"
#include "esimp/platform/timer.hpp"
#include "esimp/platform/trace.hpp"

int sc_main(int argc, char* argv[]) {
  if (argc < 2) {
    printf("usage: platform <mcu0-library>\n\n");
    return 1;
  }

  esimp::trace::init();
  esimp::MCU mcu0("mcu0");
  esimp::Firmware fw(argv[1]);

  mcu0.add_firmware(&fw);
  {
    esimp::MCU::Configuration cfg;
    cfg.sync_with_realtime = false;
    cfg.f_clk_hz = 100e6;
    cfg.time_quantum_ns = 100e3;
    mcu0.set_configuration(cfg);
  }
  mcu0.add_irq(0, "timer1");
  mcu0.add_irq(1, "timer2");

  sc_core::sc_start();

  return 0;
}
