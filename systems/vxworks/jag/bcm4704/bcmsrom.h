/*
 * Misc useful routines to access NIC srom
 *
 * Copyright(c) 2002 Broadcom Corporation
 * $Id: bcmsrom.h 1.1 Broadcom SDK $
 */

#ifndef	_bcmsrom_h_
#define	_bcmsrom_h_

extern int srom_var_init(uint bus, void *curmap, void *osh, char **vars, int *count);

extern int srom_read(uint bus, void *curmap, void *osh, uint byteoff, uint nbytes, uint16 *buf);
extern int srom_write(uint bus, void *curmap, void *osh, uint byteoff, uint nbytes, uint16 *buf);
extern int srom_parsecis(uint8 *cis, char **vars, int *count);
	   
#endif	/* _bcmsrom_h_ */
