
#include "esimp/platform/firmware.hpp"

#include <assert.h>
#include <dlfcn.h>

#include <cstdio>

namespace esimp {

Firmware::Firmware(const char* p) : path(p), firmware(nullptr), app(nullptr) {}

void Firmware::load(MCU_if* mcu) {
  assert(firmware == nullptr);
  firmware = dlopen(path, RTLD_LAZY);
  if (firmware == nullptr) {
    puts(dlerror());
    fflush(stdout);
  }
  assert(firmware != nullptr);

  Application_if* (*sim_register)(MCU_if*);
  sim_register =
      (Application_if * (*)(MCU_if*)) dlsym(firmware, "sim_register");
  assert(sim_register != nullptr);
  app = sim_register(mcu);
}

void Firmware::close() {
  assert(firmware != nullptr);
  int res = dlclose(firmware);
  firmware = nullptr;
  assert(res == 0);
}

Application_if* Firmware::get_application() { return app; }

} /* namespace esimp */
