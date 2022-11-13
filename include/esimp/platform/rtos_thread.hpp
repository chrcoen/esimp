/*
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef ESIMP_PLATFORM_RTOS_THREAD_HPP_
#define ESIMP_PLATFORM_RTOS_THREAD_HPP_

#include "esimp/interface/platform_if.hpp"
#include "esimp/platform/context.hpp"
#include "esimp/platform/update_if.hpp"

namespace esimp {

class RTOSThread : public Context, public Thread_if {
 public:
  RTOSThread(Context **active_ctx, const char *parent_name, const char *name, Update_if *parent, Application_if *app, 
             void *pdata, bool main = false);
  Context *as_context();

  /* Thread_if */
  void set_name(const char *name) override;
  void switch_to() override;

  /* Context */
  int run() override;

  bool is_main();

 private:
  Update_if *parent;
  Application_if *app;
  void *pdata;
  bool main;
};

} /* namespace esimp */

#endif /* ESIMP_PLATFORM_RTOS_THREAD_HPP_ */
