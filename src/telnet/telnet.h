/* telnet - simple telnet client
 *
 * This file is part of palomena's miscellaneous code collection:
 * https://github.com/palomena/misc
 * As such it may depend on other files in the misc directoy:
 * - telnet.c
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

#ifndef TELNET_H
#define TELNET_H

#include <stdio.h>

/**
 * Connect to a telnet server
 */

int telnet_connect(const char* address, const unsigned port);

/**
 * Disconnect from telnet server
 */

void telnet_disconnect(int sockfd);

/**
 * Send a message to a telnet server
 */

ssize_t telnet_send(int sockfd, const char* message);

/**
 * Receive a message from a telnet server
 */

ssize_t telnet_receive(int sockfd, char* buffer, unsigned size);

#endif /* TELNET_H */