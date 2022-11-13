/*
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef ESIMP_PLATFORM_SYSTEMC_THREAD_HPP_
#define ESIMP_PLATFORM_SYSTEMC_THREAD_HPP_

#include <string>
#include <systemc>

namespace esimp {


class SystemcThread {
 public:
  enum class Type {
    Hardware,
    ISR,
    RTOSThread,
  };

  SystemcThread(const char *parent_name, const char *name, Type type);
  virtual ~SystemcThread();
  virtual int run() = 0;

  Type get_type();
  const char *get_name();
  const char* get_full_name();
  void kill();

  // void set_sc_process(sc_core::sc_process_b *p);
  sc_core::sc_process_b *get_sc_process();
  void spawn();

  // static SystemcThreadFactory *factory;
protected:
  void set_name_intern(const char *val);

public:
  int run_thread();
  const Type type;
  sc_core::sc_process_b *process;
  sc_core::sc_process_handle process_handle;
  std::string parent_name;
  std::string name;
  std::string full_name;
  pthread_t pthread;
};

} /* namespace esimp */

#endif /* ESIMP_PLATFORM_SYSTEMC_THREAD_HPP_ */
