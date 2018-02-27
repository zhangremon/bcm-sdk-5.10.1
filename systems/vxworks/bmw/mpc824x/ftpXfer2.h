/* $Id: ftpXfer2.h 1.1.570.1 Broadcom SDK $ */
#ifndef FTPXFER2_H
#define FTPXFER2_H

STATUS ftpXfer2(char *host, char *user, char *passwd, char *acct,
		char *cmd, char *dirname, char *filename,
		int *pCtrlSock, int *pDataSock);

#endif	/* FTPXFER2_H */
