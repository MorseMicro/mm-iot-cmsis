/*
 * Copyright 2025 Morse Micro
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef DEMO_CLI_H
#define DEMO_CLI_H

void cli_init(void);
bool cli_get_uart_data_received(void);
void cli_set_auto_connect_in_progress(bool in_progress);

#endif /* DEMO_CLI_H */
