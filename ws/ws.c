/*
 * Copyright (C) 2016-2023 Davidson Francis <davidsondfgl@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>
 */

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ws.h>

#include <base64.c>
#include <handshake.c>
#include <sha1.c>
#include <utf8.c>
#include <ws.c>

#define PORT 1234

void onopen(ws_cli_conn_t client)
{
	char *cli, *port;
	cli  = ws_getaddress(client);
	port = ws_getport(client);
#ifndef DISABLE_VERBOSE
	printf("Connection opened, addr: %s, port: %s\n", cli, port);
#endif
}

void onclose(ws_cli_conn_t client)
{
	char *cli;
	cli = ws_getaddress(client);
#ifndef DISABLE_VERBOSE
	printf("Connection closed, addr: %s\n", cli);
#endif
}

void onmessage(ws_cli_conn_t client,
               const unsigned char *msg, uint64_t size, int type)
{
	char *cli;
	cli = ws_getaddress(client);
#ifndef DISABLE_VERBOSE
	printf("I receive a message: %s (size: %" PRId64 ", type: %d), from: %s\n",
           msg, size, type, cli);
#endif
    
    char Message[] = "reload";
    unsigned int MessageSize = (sizeof(Message) - 1);
	ws_sendframe_bcast(PORT, Message, MessageSize, WS_FR_OP_TXT);
}

int main(void)
{
	ws_socket(&(struct ws_server){
                  .host = "0.0.0.0",
                  .port = PORT,
                  .thread_loop   = 0,
                  .timeout_ms    = 1000,
                  .evs.onopen    = &onopen,
                  .evs.onclose   = &onclose,
                  .evs.onmessage = &onmessage
              });
    
	return (0);
}
