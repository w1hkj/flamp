#ifndef	_AHDLC_H
#define	_AHDLC_H

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

#include "global.h"
#include "mbuf.h"

/* Asynch HDLC receiver control block */
struct ahdlc {
	int escaped;		/* Escape char (0x7d) just seen */
	int hunt;		/* Flushing input until next flag */
	struct mbuf *inframe;	/* Current frame being reassembled */
	int maxsize;		/* Maximum packet size */
	uint16 fcs;		/* current CRC value */
	int32 rxframes;		/* Valid frames received */
	int32 aborts;		/* Aborts seen */
	int32 toobigs;		/* Frames larger than maxsize */
	int32 crcerrs;		/* Frames with CRC errors */
	int32 runts;		/* Frames shorter than 2 bytes */
};
#define	HDLC_ESC_ASYNC	0x7d	/* Escapes special chars (0x7d, 0x7e) */
#define	HDLC_FLAG	0x7e	/* Ends each frame */
#define	HDLC_ESC_COMPL	0x20	/* XORed with special chars in data */

void init_hdlc(struct ahdlc *,int);
struct mbuf *ahdlcrx(struct ahdlc *,char *,int);
struct mbuf *ahdlctx(struct mbuf *);

#endif	/* _AHDLC_H */

