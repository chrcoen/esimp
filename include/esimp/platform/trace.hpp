/*
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef ESIMP_PLATFORM_TRACE_HPP_
#define ESIMP_PLATFORM_TRACE_HPP_

#include <systemc>

namespace esimp {

class Context;

namespace trace {

void init();
void set_active_context_var(Context **ctx);
void msg(const char *level, const char *text);

} /* namespace trace */

} /* namespace esimp */

#endif /* ESIMP_PLATFORM_TRACE_HPP_ */
