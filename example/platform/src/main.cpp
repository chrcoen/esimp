#include <systemc>

#include "esimp/platform/firmware.hpp"
#include "esimp/platform/mcu.hpp"
#include "esimp/platform/systemc_thread.hpp"
#include "esimp/platform/timer.hpp"
#include "esimp/platform/trace.hpp"

int sc_main(int argc, char* argv[]) {
  if (argc < 3) {
    printf("usage: platform <bootloader> <mcu0-library>\n\n");
    return 1;
  }

  // sc_core::sc_trace_file* tf = sc_core::sc_create_vcd_trace_file("traces");
  // tf->set_time_unit(1, sc_core::SC_NS);

  esimp::trace::init();
  esimp::MCU mcu0("mcu0");
  esimp::Firmware fw_boot(argv[1]);
  esimp::Firmware fw_app(argv[2]);
  mcu0.add_firmware(&fw_boot);
  mcu0.add_firmware(&fw_app);
  {
    esimp::MCU::Configuration cfg;
    cfg.sync_with_realtime = true;
    cfg.f_clk_hz = 100e6;
    cfg.time_quantum_ns = 100e3;
    mcu0.set_configuration(cfg);
  }

  mcu0.add_irq(0, "timer1");
  mcu0.add_irq(1, "timer2");

  sc_core::sc_start();

  // sc_core::sc_close_vcd_trace_file(tf);

  return 0;
}
