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

#ifndef __smb2_slist_h__
#define __smb2_slist_h__

#include <stddef.h>

/*头部追加元素*/
void SMB2_LIST_prepend(void *list, void *item);

/*尾部追加元素*/
void SMB2_LIST_append(void *list, void *item);

/*从链表中移除指定元素*/
void SMB2_LIST_remove(void *list, void *item);

// /*获取链表长度*/
// size_t SMB2_LIST_length(void *list);

#endif /* __smb2_slist_h__ */
