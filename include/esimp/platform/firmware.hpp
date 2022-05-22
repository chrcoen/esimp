/*
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef ESIMP_PLATFORM_FIRMWARE_HPP_
#define ESIMP_PLATFORM_FIRMWARE_HPP_

#include "esimp/interface/platform_if.hpp"

namespace esimp {

class Firmware {
 public:
  Firmware(const char *path);

  void load(MCU_if *mcu);
  void close();

  Application_if *get_application();

 private:
  const char *path;
  void *firmware;
  Application_if *app;
};

} /* namespace esimp */

#endif /* ESIMP_PLATFORM_FIRMWARE_HPP_ */
