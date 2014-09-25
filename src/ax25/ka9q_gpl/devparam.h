/*
 * Copyright 1991 Phil Karn, KA9Q
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
#ifndef	_DEVPARAM_H
#define	_DEVPARAM_H

#ifndef	_GLOBAL_H
#include "global.h"
#endif

/* device parameter control */
#define	PARAM_DATA	0
#define	PARAM_TXDELAY	1
#define	PARAM_PERSIST	2
#define	PARAM_SLOTTIME	3
#define	PARAM_TXTAIL	4
#define	PARAM_FULLDUP	5
#define	PARAM_HW	6
#define	PARAM_MUTE	7
#define	PARAM_DTR	8
#define	PARAM_RTS	9
#define	PARAM_SPEED	10
#define	PARAM_ENDDELAY	11
#define	PARAM_GROUP	12
#define PARAM_IDLE	13
#define	PARAM_MIN	14
#define	PARAM_MAXKEY	15
#define	PARAM_WAIT	16
#define PARAM_DOWN	0x81
#define PARAM_UP	0x82
#define PARAM_BLIND	0x83	/* should be vertigo, can't tell down/up? */
#define	PARAM_RETURN	0xff

/* In devparam.c: */
int devparam(char *s);
char *parmname(int n);

#endif	/* _DEVPARAM_H */

