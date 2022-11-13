/*
 * SPDX-License-Identifier: Apache-2.0
 * SPDX-FileCopyrightText: © 2022 Christoph Coenen <chrcoen@gmail.com>
 */

#define SC_INCLUDE_DYNAMIC_PROCESSES
#include "esimp/platform/systemc_thread.hpp"

#include <assert.h>
#include <pthread.h>

#include <systemc>

#include "esimp/platform/trace.hpp"

namespace esimp {

SystemcThread::SystemcThread(const char *parent_name, const char* name, Type type)
    : parent_name(parent_name), name(name), type(type), process(nullptr), pthread(0) {}

SystemcThread::~SystemcThread() {}

void SystemcThread::kill() { process_handle.kill(); }

SystemcThread::Type SystemcThread::get_type() { return type; }

const char* SystemcThread::get_name() { return name.c_str(); }
const char* SystemcThread::get_full_name() { 
  full_name = parent_name + "." + name;
  return full_name.c_str();
}

void SystemcThread::set_name_intern(const char *val) {
  if (name != std::string(val)) {
    name = val;
    if (pthread > 0) {
      pthread_setname_np(pthread, get_full_name());
    }
  }
}

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
  pthread = pthread_self();
  pthread_setname_np(pthread, get_full_name());
  return run();
}

} /* namespace esimp */
