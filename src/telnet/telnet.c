/* telnet - simple telnet client
 *
 * This file is part of palomena's miscellaneous code collection:
 * https://github.com/palomena/misc
 * As such it may depend on other files in the misc directoy:
 * - telnet.h
 *
 * Author: https://github.com/palomena
 * Contact: palomena@protonmail.ch
 *
 * MIT License
 *
 * Copyright (c) 2019 https://github.com/palomena
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "telnet.h"
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>

#include <signal.h>

static int hit_sigpipe_handler = 0;
void sigpipe_handler(int unused)
{
    printf("----- SIGPIPE-Handler -----\n");
    hit_sigpipe_handler = 1;
}

void set_sig_action(void) {
    sigaction(SIGPIPE, &(struct sigaction){sigpipe_handler}, NULL);
    hit_sigpipe_handler = 0;
}

int telnet_connect(const char* address, const unsigned port)
{
    set_sig_action();
	int sockfd;
	struct sockaddr_in conn;
	struct hostent* host;
	struct in_addr remoteaddr;

	if(!inet_aton(address, &remoteaddr))
	{
		host = gethostbyname(address);
		if(!host) return(-1);

		remoteaddr = *(struct in_addr*)host->h_addr_list[0];
	}

	conn.sin_family = AF_INET;
	conn.sin_port = htons(port);
	conn.sin_addr = remoteaddr;

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if(sockfd == -1) return(-1);

	if(connect(sockfd, (const struct sockaddr*)&conn, sizeof(struct sockaddr_in)) != 0)
	{
		close(sockfd);
		return(-1);
	}

	return(sockfd);
}

void telnet_disconnect(int sockfd)
{
	close(sockfd);
}

ssize_t telnet_send(int sockfd, const char* message)
{
    if (hit_sigpipe_handler > 0) {
        return -1;
    }
    ssize_t r;
    r = send(sockfd, message, strlen(message), 0);
    return r;
}

ssize_t telnet_receive(int sockfd, char* buffer, unsigned size)
{
	ssize_t n;

	n = recv(sockfd, buffer, size, 0);
	buffer[n] = '\0';

	return(n);
}
