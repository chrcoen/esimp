/*
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef ESIMP_PLATFORM_CONTEXT_IF_HPP_
#define ESIMP_PLATFORM_CONTEXT_IF_HPP_

namespace esimp {

class Context;

class Context_if {
 public:
  virtual void switch_to(Context *thread) = 0;
  virtual ~Context_if() {}
};

} /* namespace esimp */

#endif /* ESIMP_PLATFORM_CONTEXT_IF_HPP_ */
