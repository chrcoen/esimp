/*
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef ESIMP_PLATFORM_CONTEXT_HPP_
#define ESIMP_PLATFORM_CONTEXT_HPP_

#include "esimp/platform/systemc_thread.hpp"

namespace esimp {

// class RTOSThread;
// class ISR;

class Context : public SystemcThread {
 public:
  // enum class Type {
  //   RTOSThread,
  //   ISR
  // };

  Context(const char *name, SystemcThread::Type t);
  // RTOSThread* as_rtos_thread();
  // ISR* as_isr();
  // bool is_isr();
  // bool is_rtos_thread();

  uint64_t get_instruction_count();
  void clear_instruction_count();
  void increment_instruction_count(uint64_t dn);

  void wait(uint64_t t_ns, Context **active_context);
  void suspend();
  void activate();

 protected:
  // const Type type;
  sc_core::sc_event wakeup;
  uint64_t instruction_count;
};

} /* namespace esimp */

#endif /* ESIMP_PLATFORM_CONTEXT_HPP_ */
