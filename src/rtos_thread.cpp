/*
 * SPDX-License-Identifier: Apache-2.0
 * SPDX-FileCopyrightText: © 2022 Christoph Coenen <chrcoen@gmail.com>
 */

#include "esimp/platform/rtos_thread.hpp"

#include <assert.h>

#include <cstring>

namespace esimp {

RTOSThread::RTOSThread(Context **active_ctx, const char *parent_name, const char *name, Update_if *parent, Application_if *app,
                       void *pdata, bool main)
    : Context(active_ctx, parent_name, name, SystemcThread::Type::RTOSThread),
      parent(parent),
      app(app),
      pdata(pdata),
      main(main) {}

Context *RTOSThread::as_context() { return static_cast<Context *>(this); }

void RTOSThread::set_name(const char *name) {
  set_name_intern(name);
}

void RTOSThread::switch_to() {
  assert(parent != nullptr);
  parent->switch_thread(static_cast<Thread_if *>(this));
}

int RTOSThread::run() {
  assert(app != nullptr);
  if (main) {
    app->run_main(static_cast<Thread_if *>(this));
  } else {
    suspend();
    app->run_thread(static_cast<Thread_if *>(this), get_name(), pdata);
    assert(0);
  }
  return 0;
}

bool RTOSThread::is_main() { return main; }

} /* namespace esimp */