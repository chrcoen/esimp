/*
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef ESIMP_PLATFORM_UPDATE_IF_HPP_
#define ESIMP_PLATFORM_UPDATE_IF_HPP_

#include "esimp/interface/platform_if.hpp"

namespace esimp {

class Update_if {
 public:
  virtual void update_from_hw() = 0;
  virtual void update_from_sw() = 0;
  virtual void switch_thread(Thread_if *thread) = 0;
};

} /* namespace esimp */

#endif /* ESIMP_PLATFORM_UPDATE_IF_HPP_ */
