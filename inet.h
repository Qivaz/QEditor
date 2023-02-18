/**
 * This file is copied and derivative from WebRTC.
 *
 * Copyright 2023 QEditor QH
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once
#ifdef __cplusplus
extern "C"
{
#endif
#if defined (WIN32)
#include <winsock2.h>
#include <Ws2tcpip.h>
//#pragma comment(lib, “wsock32.lib”)
#pragma comment(lib, “ws2_32.lib”)

#ifndef INET6_ADDRSTRLEN
#define INET6_ADDRSTRLEN 46
#endif
const char *inet_ntop(int af, const void *src, char* dst, socklen_t size);
int inet_pton(int af, const char* src, void *dst);
#else
#include <netinet/in.h>
#include <arpa/inet.h>
#endif
#ifdef __cplusplus
}
#endif
