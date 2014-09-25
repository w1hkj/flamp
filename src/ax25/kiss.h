/*
 * Copyright 1991 Phil Karn, KA9Q
 * Copyright (C) 2014 Robert Stiles, KK5VD
 *
 * Modified for use with FLLNK/FLDIGI
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 dated June, 1991.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program;  if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave., Cambridge, MA 02139, USA.
 */
#ifndef	_KISS_H
#define	_KISS_H

#ifndef	_MBUF_H
#include "mbuf.h"
#endif

#ifndef	_IFACE_H
#include "iface.h"
#endif

/* In kiss.c: */
int kiss_free(struct iface *ifp);
int kiss_raw(struct iface *iface,struct mbuf *data);
void kiss_recv(struct iface *iface,struct mbuf *bp);
int kiss_init(struct iface *ifp);
int32 kiss_ioctl(struct iface *iface,int cmd,int set,int32 val);
void kiss_recv(struct iface *iface,struct mbuf *bp);

#endif	/* _KISS_H */
