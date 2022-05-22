/*
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef ESIMP_PLATFORM_SYSTEMC_THREAD_HPP_
#define ESIMP_PLATFORM_SYSTEMC_THREAD_HPP_

#include <string>
#include <systemc>

namespace esimp {

// class SystemcThread;

// class SystemcThreadFactory: public sc_core::sc_module {
// public:
//   SC_HAS_PROCESS(SystemcThreadFactory);
//   SystemcThreadFactory(sc_core::sc_module_name name);
//   void create(SystemcThread *obj);

// private:
//   int run(SystemcThread *obj);
// };

class SystemcThread {
 public:
  enum class Type {
    Hardware,
    ISR,
    RTOSThread,
  };

  SystemcThread(const char *name, Type type);
  virtual ~SystemcThread();
  virtual int run() = 0;

  Type get_type();
  const char *get_name();

  // void set_sc_process(sc_core::sc_process_b *p);
  sc_core::sc_process_b *get_sc_process();
  void spawn();

  // static SystemcThreadFactory *factory;

 private:
  int run_thread();
  const Type type;
  const std::string name;
  sc_core::sc_process_b *process;
  sc_core::sc_process_handle process_handle;
};

} /* namespace esimp */

#endif /* ESIMP_PLATFORM_SYSTEMC_THREAD_HPP_ */
