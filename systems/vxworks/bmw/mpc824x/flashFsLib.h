/* $Id: flashFsLib.h 1.2.346.1 Broadcom SDK $ */
#ifndef	FLASH_FS_LIB_H
#define	FLASH_FS_LIB_H

STATUS flashFsLibInit(void);
STATUS flashFsSync(void);
IMPORT STATUS sysHasDOC();

#define	FLASH_FS_NAME	((sysHasDOC()) ? "flsh:":"flash:")

#define FIOFLASHSYNC	0x10000
#define FIOFLASHINVAL	0x10001

#endif	/* !FLASH_FS_LIB_H */
