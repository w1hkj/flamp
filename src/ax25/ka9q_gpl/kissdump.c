/* Tracing routines for KISS TNC 
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
#include "kiss.h"
#include "devparam.h"
#include "ax25.h"
#include "trace.h"

void
ki_dump(fp,bpp,check)
FILE *fp;
struct mbuf **bpp;
int check;
{
	int type;
	int val;

	fprintf(fp,"KISS: ");
	type = PULLCHAR(bpp);
	if(type == PARAM_DATA){
		fprintf(fp,"Data\n");
		ax25_dump(fp,bpp,check);
		return;
	}
	val = PULLCHAR(bpp);
	switch(type){
	case PARAM_TXDELAY:
		fprintf(fp,"TX Delay: %lu ms\n",val * 10L);
		break;
	case PARAM_PERSIST:
		fprintf(fp,"Persistence: %u/256\n",val + 1);
		break;
	case PARAM_SLOTTIME:
		fprintf(fp,"Slot time: %lu ms\n",val * 10L);
		break;
	case PARAM_TXTAIL:
		fprintf(fp,"TX Tail time: %lu ms\n",val * 10L);
		break;
	case PARAM_FULLDUP:
		fprintf(fp,"Duplex: %s\n",val == 0 ? "Half" : "Full");
		break;
	case PARAM_HW:
		fprintf(fp,"Hardware %u\n",val);
		break;
	case PARAM_RETURN:
		fprintf(fp,"RETURN\n");
		break;
	default:
		fprintf(fp,"code %u arg %u\n",type,val);
		break;
	}
}

int
ki_forus(iface,bp)
struct iface *iface;
struct mbuf *bp;
{
	struct mbuf *bpp;
	int i;

	if(bp->data[0] != PARAM_DATA)
		return 0;
	dup_p(&bpp,bp,1,AXALEN);
	i = ax_forus(iface,bpp);
	free_p(bpp);
	return i;
}
