#include "esimp/platform/rtos_thread.hpp"

#include <assert.h>

#include <cstring>

namespace esimp {

RTOSThread::RTOSThread(Update_if *parent, Application_if *app, const char *name,
                       const void *pdata, bool main)
    : Context(name, SystemcThread::Type::RTOSThread),
      parent(parent),
      app(app),
      name(name),
      pdata(pdata),
      main(main) {}

Context *RTOSThread::as_context() { return static_cast<Context *>(this); }

void RTOSThread::switch_to() {
  assert(parent != nullptr);
  parent->switch_thread(static_cast<RTOSThread_if *>(this));
}

int RTOSThread::run() {
  assert(app != nullptr);
  if (main) {
    app->run_main();
  } else {
    sc_core::wait(wakeup);
    app->run_thread(static_cast<RTOSThread_if *>(this), name, pdata);
    assert(0);
  }
  return 0;
}

void RTOSThread::start() { wakeup.notify(); }

bool RTOSThread::is_main() { return main; }

} /* namespace esimp */