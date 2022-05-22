/*
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef ESIMP_PLATFORM_REALTIME_HPP_
#define ESIMP_PLATFORM_REALTIME_HPP_

#include <chrono>
#include <thread>

namespace esimp {

class Realtime {
 public:
  Realtime() : t_start_ms(0) {}

  void start() { t_start_ms = get_current_realtime_ms(); }

  void sync(double t_sim_ms) {
    double t_real_ms = get_current_realtime_ms() - t_start_ms;
    double dt_ms = t_sim_ms - t_real_ms;
    if (dt_ms > 10.0) {
      std::this_thread::sleep_for(std::chrono::milliseconds((uint64_t)(dt_ms)));
    }
  }

 private:
  static double get_current_realtime_ms() {
    return std::chrono::duration_cast<std::chrono::milliseconds>(
               std::chrono::high_resolution_clock::now().time_since_epoch())
        .count();
  }
  double t_start_ms;
};

} /* namespace esimp */

#endif /* ESIMP_PLATFORM_REALTIME_HPP_ */
