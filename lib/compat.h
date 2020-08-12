/* -*-  mode:c; tab-width:8; c-basic-offset:8; indent-tabs-mode:nil;  -*- */
/*
   Copyright (C) 2020 by Ronnie Sahlberg <ronniesahlberg@gmail.com>

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU Lesser General Public License as published by
   the Free Software Foundation; either version 2.1 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public License
   along with this program; if not, see <http://www.gnu.org/licenses/>.
*/

#ifndef _COMPAT_H_
#define _COMPAT_H_

#ifdef PS2_EE_PLATFORM

#include <errno.h>
#include <sys/time.h>
#include <ps2ip.h>

#define getlogin_r(a,b) ENXIO
#define srandom srand
#define random rand

#define POLLIN      0x0001    /* There is data to read */
#define POLLPRI     0x0002    /* There is urgent data to read */
#define POLLOUT     0x0004    /* Writing now will not block */
#define POLLERR     0x0008    /* Error condition */
#define POLLHUP     0x0010    /* Hung up */

struct pollfd {
        int fd;
        short events;
        short revents;
};

int poll(struct pollfd *fds, unsigned int nfds, int timo);

struct iovec {
  void  *iov_base;
  size_t iov_len;
};

#define write(a,b,c) lwip_write(a,b,c)
#define read(a,b,c) lwip_read(a,b,c)

ssize_t writev(int fd, const struct iovec *iov, int iovcnt);
ssize_t readv(int fd, const struct iovec *iov, int iovcnt);

int getaddrinfo(const char *node, const char*service,
                const struct addrinfo *hints,
                struct addrinfo **res);
void freeaddrinfo(struct addrinfo *res);

#define SOL_TCP IPPROTO_TCP
#define EAI_AGAIN EAGAIN

/* just pretend they are the same so we compile */
#define sockaddr_in6 sockaddr_in

long long int be64toh(long long int x);

#endif /* PS2_EE_PLATFORM */


#ifdef PS2_IOP_PLATFORM

#include <alloc.h>
#include <errno.h>
#include <types.h>
#include <sys/time.h>
#include <sys/fcntl.h>
#include <ps2ip.h>
#include <loadcore.h>

typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef unsigned long long uint64_t;
typedef uint32_t UWORD32;

typedef char int8_t;
typedef short int16_t;
typedef short int_least16_t;
typedef int int32_t;
typedef long long int64_t;
typedef int intptr_t;

typedef size_t ssize_t;

long long int be64toh(long long int x);
char *strdup(const char *s);

int random(void);
void srandom(unsigned int seed);
time_t time(time_t *tloc);
int asprintf(char **strp, const char *fmt, ...);

#define getlogin_r(a,b) ENXIO
#define close(x) lwip_close(x)
#define snprintf(format, n, ...) sprintf(format, __VA_ARGS__)
#define getpid() 27
#define fcntl(a,b,c) lwip_fcntl(a,b,c)

#define POLLIN      0x0001    /* There is data to read */
#define POLLPRI     0x0002    /* There is urgent data to read */
#define POLLOUT     0x0004    /* Writing now will not block */
#define POLLERR     0x0008    /* Error condition */
#define POLLHUP     0x0010    /* Hung up */

struct pollfd {
        int fd;
        short events;
        short revents;
};

int poll(struct pollfd *fds, unsigned int nfds, int timo);

struct iovec {
  void  *iov_base;
  size_t iov_len;
};

#undef connect
#define connect(a,b,c) iop_connect(a,b,c)
int iop_connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen);

#define write(a,b,c) lwip_send(a,b,c,MSG_DONTWAIT)
#define read(a,b,c) lwip_recv(a,b,c,MSG_DONTWAIT)

ssize_t writev(int fd, const struct iovec *iov, int iovcnt);
ssize_t readv(int fd, const struct iovec *iov, int iovcnt);

#define SOL_TCP IPPROTO_TCP
#define EAI_AGAIN EAGAIN

#define strerror(x) "Unknown"

/* just pretend they are the same so we compile */
#define sockaddr_in6 sockaddr_in

#endif /* PS2_IOP_PLATFORM */

#endif /* _COMPAT_H_ */
