#include "esimp/platform/nvic.hpp"

#include <assert.h>

#include <cstring>
#include <limits>

namespace esimp {

NVIC::NVIC(Update_if *p) : parent(p), basepri(0), primask(0), active(nullptr) {}

void NVIC::set_primask(unsigned int n) {
  primask = n;
  assert(parent != nullptr);
  parent->update_from_sw();
}

void NVIC::set_basepri(unsigned int n) {
  basepri = n;
  assert(parent != nullptr);
  parent->update_from_sw();
}

int NVIC::get_primask() { return primask; }

int NVIC::get_basepri() { return basepri; }

void NVIC::add_irq(IRQ irq) { irqs.push_back(irq); }

IRQ_if *NVIC::get_irq(const char *name) {
  for (auto &irq : irqs) {
    if (strcmp(irq.get_name(), name) == 0) {
      return &irq;
    }
  }
  return NULL;
}

bool NVIC::is_prior(IRQ &irq, unsigned int p) {
  if (irq.get_state() == IRQ::State::Idle) {
    return irq.is_enabled() && irq.is_pending() && (irq.get_prio() < p);
  }
  return irq.get_prio() <= p;
}

IRQ *NVIC::active_irq() {
  /* New irqs are only runnable if prio < min_prio */
  unsigned int min_prio;
  if (primask > 0) {
    min_prio = 0;
  } else if (basepri > 0) {
    min_prio = basepri;
  } else {
    min_prio = std::numeric_limits<unsigned int>::max();
  }

  IRQ *next = nullptr;
  for (auto &irq : irqs) {
    if (next == nullptr) {
      if (irq.get_state() == IRQ::State::Idle) {
        if (irq.is_enabled() && irq.is_pending() &&
            (irq.get_prio() < min_prio)) {
          next = &irq;
          min_prio = irq.get_prio();
        }
      } else {
        next = &irq;
      }
    } else {
      if (irq.get_state() == IRQ::State::Idle) {
        if (irq.is_enabled() && irq.is_pending() &&
            (irq.get_prio() < min_prio)) {
          next = &irq;
          min_prio = irq.get_prio();
        }
      } else if (irq.get_state() == IRQ::State::Suspended) {
        if (irq.get_prio() <= next->get_prio()) {
          next = &irq;
        }
      } else if (irq.get_state() == IRQ::State::Active) {
        if (irq.get_prio() <= next->get_prio()) {
          next = &irq;
        }
      } else {
        assert(0);
      }
    }
  }
  return next;
}

void NVIC::reset() {
  for (auto &irq : irqs) {
    irq.reset();
  }
}

//         switch (irq) {
//             case IRQ::State::Idle: {
//                 if (next == nullptr) {
//                     if (primask == 0) {
//                         if (irq.is_enabled() && irq.is_pending()) {
//                             if (basepri == 0) {
//                                 next = &irq;
//                             } else {
//                                 if (irq.get_prio() < basepri) {
//                                     next = &irq;
//                                 }
//                             }
//                         }
//                     } else {

//                     }
//                 }
//                 break;
//             }
//             case IRQ::State::Suspended: {
//                 break;
//             }
//             case IRQ::State::Active: {
//                 break;
//             }
//         if (irq.get_state() == IRQ::State::Idle) {
//             if (irq.is_enabled() && irq.is_pending && (irq.get_prio())
//         } else {
//             if (next == nullptr) {
//                 next = &irq;
//             }
//         }
// }

// IRQ* NVIC::active_irq()
// {
//     IRQ *res = nullptr;
//     if (primask > 0) {
//         /* Interrupts are all disabled */
//         return res;
//     }

//     /* Find next possible irq */
//     unsigned int prio = std::numeric_limits<unsigned int>::max();
//     if (basepri > 0) {
//         prio = basepri;
//     }
//     for (auto &irq: irqs) {
//         if (is_prior(irq, prio)) {
//             res = &irq;
//             prio = irq.get_prio();
//         }
//     }
//     return res;
// }

} /* namespace esimp */
