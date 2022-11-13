/*
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef ESIMP_PLATFORM_CONTEXT_HPP_
#define ESIMP_PLATFORM_CONTEXT_HPP_

#include "esimp/platform/systemc_thread.hpp"

namespace esimp {


class Context : public SystemcThread {
 public:


  Context(Context **active_ctx, const char *parent_name, const char *name, SystemcThread::Type t);

  uint64_t get_instruction_count();
  void clear_instruction_count();
  void increment_instruction_count(uint64_t dn);

  void wait(uint64_t t_ns);
  void suspend();
  void activate();
  void abort(Context *last_context);

 protected:
  Context **active_context;
  sc_core::sc_event wakeup;
  uint64_t instruction_count;
  bool terminate;
};

} /* namespace esimp */

#endif /* ESIMP_PLATFORM_CONTEXT_HPP_ */
