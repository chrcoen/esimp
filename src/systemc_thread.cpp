#define SC_INCLUDE_DYNAMIC_PROCESSES
#include "esimp/platform/systemc_thread.hpp"

#include <assert.h>
#include <pthread.h>

#include <systemc>

#include "esimp/platform/trace.hpp"

namespace esimp {

SystemcThread::SystemcThread(const char* name, Type type)
    : name(name), type(type), process(nullptr) {}

SystemcThread::~SystemcThread() { process_handle.kill(); }

SystemcThread::Type SystemcThread::get_type() { return type; }

const char* SystemcThread::get_name() { return name.c_str(); }

sc_core::sc_process_b* SystemcThread::get_sc_process() { return process; }

void SystemcThread::spawn() {
  SystemcThread* c = this;
  int r = 0;
  process_handle =
      sc_core::sc_spawn(&r, sc_core::sc_bind(&SystemcThread::run_thread, c));
};

int SystemcThread::run_thread() {
  process = sc_core::sc_get_current_process_b();
  process->custom_handle = this;
  pthread_setname_np(pthread_self(), get_name());
  return run();
}

} /* namespace esimp */
