/*
 * SPDX-License-Identifier: Apache-2.0
 * SPDX-FileCopyrightText: © 2022 Christoph Coenen <chrcoen@gmail.com>
 */
#ifndef ESIMP_SYSTEMC_UART_LINE_HALF_DUPLEX_HPP_
#define ESIMP_SYSTEMC_UART_LINE_HALF_DUPLEX_HPP_

#include <systemc>

using namespace sc_core;
using namespace sc_dt;


SC_MODULE(UartLineHalfDuplex) {

    /* Port 0 */
    sc_in<bool> tx0_en;
    sc_in<bool> tx0;
    sc_out<bool> rx0;

    /* Port 1 */
    sc_in<bool> tx1_en;
    sc_in<bool> tx1;
    sc_out<bool> rx1;

    /* Ouput */
    sc_out<sc_logic> com;
    sc_out<bool> conflict;

    SC_CTOR(UartLineHalfDuplex) {
        SC_METHOD(update);
        sensitive << tx0_en << tx0 << tx1_en << tx1;
    }

    void update() {
        if (tx0_en && tx1_en) {
            rx0 = true;
            rx1 = true;
            com = sc_logic('x');
            conflict = true;
        } else if (tx0_en) {
            rx0 = tx0;
            rx1 = tx0;
            com = sc_logic(!tx0);
            conflict = false;
        } else if (tx1_en) {
            rx0 = tx1;
            rx1 = tx1;
            com = sc_logic(!tx1);
            conflict = false;
        } else {
            rx0 = true;
            rx1 = true;
            com = sc_logic('z');
            conflict = false;
        }
    }
};


#endif /* ESIMP_SYSTEMC_UART_LINE_HALF_DUPLEX_HPP_ */
