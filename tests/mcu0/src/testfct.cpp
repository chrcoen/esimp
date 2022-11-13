/*
 * SPDX-License-Identifier: Apache-2.0
 * SPDX-FileCopyrightText: © 2022 Christoph Coenen <chrcoen@gmail.com>
 */


/*
 * These functions will be instrumented
 */


static bool stop_while_1_flag;

void stop_while_1() {
    stop_while_1_flag = true;
}

void while_1() {
    stop_while_1_flag = false;
    while (!stop_while_1_flag) {}
}

