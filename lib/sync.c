/* -*-  mode:c; tab-width:8; c-basic-offset:8; indent-tabs-mode:nil;  -*- */
/*
   Copyright (C) 2016 by Ronnie Sahlberg <ronniesahlberg@gmail.com>

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
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <stdint.h>

#include <stdlib.h>

#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <stdarg.h>
#include <semaphore.h>
#include <time.h>
#include <math.h>

#if defined(HAVE_POLL_H) || defined(_WINDOWS)
#ifdef ESP_PLATFORM
#include <sys/poll.h>
#else
#include <poll.h>
#endif
#endif

#include <string.h>

#include "compat.h"

#ifndef PS2_IOP_PLATFORM
#include <time.h>
#endif

#include "smb2/smb2.h"
#include "smb2/libsmb2.h"
#include "smb2/libsmb2-raw.h"
#include "libsmb2-private.h"

struct sync_cb_data {
	int is_finished;
	int status;
	void *ptr;
};

static int wait_for_reply(struct smb2_context *smb2,
						  struct sync_cb_data *cb_data)
{
	time_t t = time(NULL);
	struct pollfd pfd;
	int res;

	//smbcontext_threadspecific_t *smbts = smbcontext_get_threadspecific(smb2);
	while (!cb_data->is_finished)
	{
		pfd.fd = smb2_get_fd(smb2);
		pfd.events = smb2_which_events(smb2);

		if (poll(&pfd, 1, 1000) < 0)
		{
			smb2_set_error(smb2, "Poll failed");
			goto end;
		}
		if (smb2->timeout)
			smb2_timeout_pdus(smb2);

		if (smb2->fd == -1 && (time(NULL) - t) > smb2->timeout)
		{
			smb2_set_error(smb2, "Timeout expired and no connection exists\n");
			goto end;
		}
		if (pfd.revents == 0)
		{
			continue;
		}

		res = smb2_service(smb2, pfd.revents);
		if (res < 0)
		{
			smb2_set_error(smb2, "smb2_service failed with : %s\n", smb2_get_error(smb2));
			goto end;
		}
	}
	return cb_data->status;
end:
	// cb_data->status = SMB2_STATUS_CANCELLED;
	// return -nterror_to_errno(cb_data->status);
	return -nterror_to_errno(SMB2_STATUS_CANCELLED);
}

static __int64_t smb2_common_action(struct smb2_context *smb2, void *_pfun, ...)
{
	struct sync_cb_data *cb_data;
	__int64_t rc = 0;

	cb_data = calloc(1, sizeof(struct sync_cb_data));
	if (cb_data == NULL)
	{
		smb2_set_error(smb2, "Failed to allocate sync_cb_data");
		return -ENOMEM;
	}

	va_list args;
	va_start(args, _pfun);
	/* Ubuntu 函数参数压栈从右往左
	 *  Android 从左往右*/
	if (_pfun == smb2_connect_share_async)
	{
		void *p0 = va_arg(args, void *), *p1 = va_arg(args, void *),
			 *p2 = va_arg(args, void *), *p3 = va_arg(args, void *);
		rc = smb2_connect_share_async(smb2, p3, p2, p1, p0, cb_data);
	}
	else if (_pfun == smb2_disconnect_share_async || _pfun == smb2_echo_async)
	{
		int (*smb2_func)(void *, void *, void *) = _pfun;
		rc = smb2_func(smb2, va_arg(args, void *), cb_data);
	}
	else if (_pfun == smb2_pread_async || _pfun == smb2_pwrite_async)
	{
		void *p0 = va_arg(args, void *);
		__uint64_t p1 = va_arg(args, __uint64_t);
		__uint32_t p2 = va_arg(args, __uint32_t);
		void *p3 = va_arg(args, void *), *p4 = va_arg(args, void *);
		int (*smb2_func)(void *, void *, void *, __uint32_t, __uint64_t, void *, void *) = _pfun;
		rc = smb2_func(smb2, p4, p3, p2, p1, p0, cb_data);
	}
	else if (_pfun == smb2_read_async || _pfun == smb2_write_async)
	{
		void *p0 = va_arg(args, void *);
		__uint32_t p1 = va_arg(args, __uint32_t);
		void *p2 = va_arg(args, void *), *p3 = va_arg(args, void *);
		int (*smb2_func)(void *, void *, void *, __uint32_t, void *, void *) = _pfun;
		rc = smb2_func(smb2, p3, p2, p1, p0, cb_data);
	}
	else if (_pfun == smb2_close_async || _pfun == smb2_unlink_async ||
			 _pfun == smb2_rmdir_async || _pfun == smb2_mkdir_async ||
			 _pfun == smb2_readlink_async || _pfun == smb2_opendir_async ||
			 _pfun == smb2_fsync_async)
	{
		void *p0 = va_arg(args, void *), *p1 = va_arg(args, void *);
		int (*smb2_func)(void *, void *, void *, void *) = _pfun;
		if (_pfun == smb2_readlink_async)
			cb_data->ptr = va_arg(args, void *);
		rc = smb2_func(smb2, p1, p0, cb_data);
	}
	else if (_pfun == smb2_fstat_async || _pfun == smb2_stat_async ||
			 _pfun == smb2_rename_async || _pfun == smb2_statvfs_async)
	{
		void *p0 = va_arg(args, void *), *p1 = va_arg(args, void *), *p2 = va_arg(args, void *);
		int (*smb2_func)(void *, void *, void *, void *, void *) = _pfun;
		rc = smb2_func(smb2, p2, p1, p0, cb_data);
	}
	else if (_pfun == smb2_truncate_async || _pfun == smb2_ftruncate_async)
	{
		void *p0 = va_arg(args, void *);
		__uint64_t p1 = va_arg(args, __uint64_t);
		void *p2 = va_arg(args, void *);
		int (*smb2_func)(void *, void *, __uint64_t, void *, void *) = _pfun;
		rc = smb2_func(smb2, p2, p1, p0, cb_data);
	}
	else if (_pfun == smb2_open_async)
	{
		void *p0 = va_arg(args, void *);
		__uint32_t p1 = va_arg(args, __uint32_t);
		void *p2 = va_arg(args, void *);
		rc = smb2_open_async(smb2, p2, p1, p0, cb_data);
	}

	va_end(args);

	if (rc < 0)
	{
		if (_pfun == smb2_opendir_async || _pfun == smb2_open_async)
		{
			errno = -rc;
			rc = 0;
		}
		goto out;
	}

	rc = wait_for_reply(smb2, cb_data);

	if (_pfun == smb2_opendir_async || _pfun == smb2_open_async)
	{
		if (rc < 0)
			errno = -rc;
		rc = (__int64_t)cb_data->ptr;
	}
	// if (cb_data->status == SMB2_STATUS_CANCELLED)
	// 	return rc;
out:
	free(cb_data);
	return rc;
}

static void common_cb(struct smb2_context *, int, void *, void *, void *);

static void generic_cmdd_cb(struct smb2_context *smb2, int status,
							void *command_data, void *private_data)
{
	common_cb(smb2, status, command_data, private_data, generic_cmdd_cb);
}

/*
 * pread()
 */
static void generic_status_cb(struct smb2_context *smb2, int status,
							  void *command_data, void *private_data)
{
	common_cb(smb2, status, command_data, private_data, generic_status_cb);
}
struct readlink_cb_data
{
	char *buf;
	int len;
};

static void readlink_cb(struct smb2_context *smb2, int status, void *command_data,
						void *private_data)
{
	common_cb(smb2, status, command_data, private_data, readlink_cb);
}

static void common_cb(struct smb2_context *smb2, int status,
					  void *command_data, void *private_data, void *_pfun)
{
	struct sync_cb_data *cb_data = private_data;

	// if (cb_data->status == SMB2_STATUS_CANCELLED)
	// 	return;

	if (_pfun == generic_cmdd_cb)
		cb_data->ptr = command_data;
	else if (_pfun == generic_status_cb || _pfun == readlink_cb)
	{
		cb_data->status = status;
		if (_pfun == readlink_cb)
		{
			struct readlink_cb_data *rl_data = cb_data->ptr;
			strncpy(rl_data->buf, command_data, rl_data->len);
		}
	}
	cb_data->is_finished = 1;
}

int smb2_connect_share(struct smb2_context *smb2, const char *server,
					   const char *share, const char *user)
{
	return (int)smb2_common_action(smb2, smb2_connect_share_async,
								   generic_status_cb, user, share, server);
}

/*
 * Disconnect from share
 */
int smb2_disconnect_share(struct smb2_context *smb2)
{
	return (int)smb2_common_action(smb2, smb2_disconnect_share_async, generic_status_cb);
}

struct smb2dir *smb2_opendir(struct smb2_context *smb2, const char *path)
{
	return (void *)smb2_common_action(smb2, smb2_opendir_async, generic_cmdd_cb, path);
}

struct smb2fh *smb2_open(struct smb2_context *smb2, const char *path, int flags)
{
	return (void *)smb2_common_action(smb2, smb2_open_async, generic_cmdd_cb, flags, path);
}

int smb2_close(struct smb2_context *smb2, struct smb2fh *fh)
{
	return (int)smb2_common_action(smb2, smb2_close_async, generic_status_cb, fh);
}

int smb2_fsync(struct smb2_context *smb2, struct smb2fh *fh)
{
	return (int)smb2_common_action(smb2, smb2_fsync_async, generic_status_cb, fh);
}

int smb2_pread(struct smb2_context *smb2, struct smb2fh *fh,
			   uint8_t *buf, uint32_t count, uint64_t offset)
{
	return (int)smb2_common_action(smb2, smb2_pread_async, generic_status_cb,
								   offset, count, buf, fh);
}

int smb2_pwrite(struct smb2_context *smb2, struct smb2fh *fh,
				const uint8_t *buf, uint32_t count, uint64_t offset)
{
	return (int)smb2_common_action(smb2, smb2_pwrite_async, generic_status_cb,
								   offset, count, buf, fh);
}

int smb2_read(struct smb2_context *smb2, struct smb2fh *fh,
			  uint8_t *buf, uint32_t count)
{
	return (int)smb2_common_action(smb2, smb2_read_async, generic_status_cb, count, buf, fh);
}

int smb2_write(struct smb2_context *smb2, struct smb2fh *fh,
			   const uint8_t *buf, uint32_t count)
{
	return (int)smb2_common_action(smb2, smb2_write_async, generic_status_cb, count, buf, fh);
}

int smb2_unlink(struct smb2_context *smb2, const char *path)
{
	return (int)smb2_common_action(smb2, smb2_unlink_async, generic_status_cb, path);
}

int smb2_rmdir(struct smb2_context *smb2, const char *path)
{
	return (int)smb2_common_action(smb2, smb2_rmdir_async, generic_status_cb, path);
}

int smb2_mkdir(struct smb2_context *smb2, const char *path)
{
	return (int)smb2_common_action(smb2, smb2_mkdir_async, generic_status_cb, path);
}

int smb2_fstat(struct smb2_context *smb2, struct smb2fh *fh, struct smb2_stat_64 *st)
{
	return (int)smb2_common_action(smb2, smb2_fstat_async, generic_status_cb, st, fh);
}

int smb2_stat(struct smb2_context *smb2, const char *path, struct smb2_stat_64 *st)
{
	return (int)smb2_common_action(smb2, smb2_stat_async, generic_status_cb, st, path);
}

int smb2_rename(struct smb2_context *smb2, const char *oldpath, const char *newpath)
{
	return (int)smb2_common_action(smb2, smb2_rename_async, generic_status_cb, newpath, oldpath);
}

int smb2_statvfs(struct smb2_context *smb2, const char *path, struct smb2_statvfs *st)
{
	return (int)smb2_common_action(smb2, smb2_statvfs_async, generic_status_cb, st, path);
}

int smb2_truncate(struct smb2_context *smb2, const char *path, uint64_t length)
{
	return (int)smb2_common_action(smb2, smb2_truncate_async, generic_status_cb, length, path);
}

int smb2_ftruncate(struct smb2_context *smb2, struct smb2fh *fh, uint64_t length)
{
	return (int)smb2_common_action(smb2, smb2_ftruncate_async, generic_status_cb, length, fh);
}

int smb2_readlink(struct smb2_context *smb2, const char *path, char *buf, uint32_t len)
{
	struct readlink_cb_data rl_data = {.buf = buf, .len = len};
	return (int)smb2_common_action(smb2, smb2_readlink_async, readlink_cb, path, &rl_data);
}

/*
 * Send SMB2_ECHO command to the server
 */
int smb2_echo(struct smb2_context *smb2)
{
	return (int)smb2_common_action(smb2, smb2_echo_async, generic_status_cb);
}
