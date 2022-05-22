#include "esimp/platform/trace.hpp"

#include <iomanip>
#include <iostream>
#include <map>
#include <string>

#include "esimp/platform/isr.hpp"
#include "esimp/platform/systemc_thread.hpp"

using std::cout;
using std::endl;
using std::fixed;
using std::left;
using std::map;
using std::setprecision;
using std::setw;
using std::string;

namespace esimp {
namespace trace {

static map<sc_core::sc_process_b *, SystemcThread *> threads;
static string current = "-";
static Context **context_var;

void msg(const char *level, const char *text) {
  std::string s(level);
  if (s == "sysc")    { return; }
  if (s == "context") { return; }
  if (s == "mcu")     { return; }
  if (s == "irq")     { return; }

  const char *ctx = (context_var != nullptr) ? (*context_var)->get_name() : "-";
  cout << fixed << setw(11) << setprecision(6)
       << sc_core::sc_time_stamp().to_seconds() << setw(10) << level
       << std::left << setw(20) << ctx << " " << std::left << setw(20)
       << current.c_str() << " " << text << endl;
}

static string thread_name(void *t) {
  if (t == nullptr) {
    return "unknown";
  }
  SystemcThread *sc = static_cast<SystemcThread *>(t);
  string res = sc->get_name();
  if (sc->get_type() == SystemcThread::Type::ISR) {
    IRQ *irq = static_cast<ISR *>(sc)->get_irq();
    if (irq != nullptr) {
      res += std::string(".") + irq->get_name();
    }
  }
  return res;
}

void systemc_suspend(sc_core::sc_process_b *t) {
  string name = thread_name(t->custom_handle);
  msg("sysc", (string("<-- ") + name).c_str());
  current = "-";
}

void systemc_resume(sc_core::sc_process_b *t) {
  string name = thread_name(t->custom_handle);
  current = name;
  msg("sysc", (string("--> ") + name).c_str());
}

void init() {
  sc_core::sc_process_b::trace_suspend = systemc_suspend;
  sc_core::sc_process_b::trace_resume = systemc_resume;
}

void set_active_context_var(Context **ctx) { context_var = ctx; }

} /* namespace trace */

} /* namespace esimp */
