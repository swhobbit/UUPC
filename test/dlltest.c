#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

#include "uupcdll.h"

#define UUPCInit				pUUPCInit
#define UUPCGetParm				pUUPCGetParm
#define UUPCGetNewsSpoolSize	pUUPCGetNewsSpoolSize
#define UUPCSendMail			pUUPCSendMail

HANDLE hUUPCLib = INVALID_HANDLE_VALUE;

BOOL (*pUUPCInit)(void) = NULL;
BOOL (*pUUPCGetParm)(char *parmName, char *buf, int len) = NULL;
BOOL (*pUUPCGetNewsSpoolSize)(char *system, long *count, long *bytes) = NULL;
BOOL (*pUUPCSendMail)(char *message) = NULL;
BOOL LoadUUPCDLL(void);


int main(int argc, char *argv[])
{
	char buf[BUFSIZ];
	long count = 0, size = 0;
	BOOL result;

	if (!LoadUUPCDLL()) {
		printf("Could not load DLL: exiting\n");
		exit(1);
	}

	if (!UUPCInit()) {
		printf("Problem initializing UUPC DLL\nCheck two things:\n1) Have you run REGSETUP?\n"
		"2) Is the UUPC/extended executable directory on your path?\n");
		exit(1);
	}

	result = UUPCGetParm("NewsDir", buf, BUFSIZ);

	if (!result) {
		printf("UUPCGetParm failed\n");
	} else {		
		printf("NewsDir = :%s:\n", buf);
	}


	result = UUPCGetNewsSpoolSize("bloom-beacon", &count, &size);

	if (!result) {
		printf("UUPCGetNewsSpoolSize failed\n");
	} else {
    	printf("bloom-beacon's spool:  %d files, total size %d bytes\n", count, size);
	}

	result = UUPCSendMail(
				"To: Jeff Coffler <coffler@jeck.wa.com>\n"
				"From: NetNews Service <netnews@smersh.cambridge.ma.us>\n"
				"Subject: Hello?\n"
				"\nIf you receive this, then it was sent with the UUPCSendMail\n"
				"function from within a DLL.  I'll be bloody amazed.\n"
				"\n- Dave\n");

	if (result)
		printf("UUPCSendMail should have succeeded\n");
	else
		printf("UUPCSendMail should not have failed\n");

	result = UUPCSendMail(
				"To: Jeff Coffler <coffler@jeck.wa.com>\n"
				"From: NetNews Service <netnews@smersh.cambridge.ma.us>\n"
				"Subject: Second message\n"
				"\nIf you receive this, then it was sent with the UUPCSendMail\n"
				"function from within a DLL.  I'll be bloody amazed.\n"
				"\n- Dave\n");

	if (result)
		printf("UUPCSendMail should have succeeded\n");
	else
		printf("UUPCSendMail should not have failed\n");

	result = UUPCSendMail("Is this some sort of joke?");

	if (result)
		printf("UUPCSendMail should not have succeded\n");
	else
		printf("UUPCSendMail should have failed\n");

	return 0;
}

BOOL LoadUUPCDLL(void)
{
	hUUPCLib = LoadLibrary("UUPCDLL.DLL");

	if (!hUUPCLib)
		return FALSE;

	pUUPCInit = 				(LPVOID)GetProcAddress(hUUPCLib, "UUPCInit");
	pUUPCGetParm =				(LPVOID)GetProcAddress(hUUPCLib, "UUPCGetParm");
	pUUPCGetNewsSpoolSize =		(LPVOID)GetProcAddress(hUUPCLib, "UUPCGetNewsSpoolSize");
	pUUPCSendMail = 			(LPVOID)GetProcAddress(hUUPCLib, "UUPCSendMail");

	if (pUUPCInit == NULL ||
		pUUPCGetParm == NULL ||
		pUUPCGetNewsSpoolSize == NULL ||
		pUUPCSendMail == NULL)
		return FALSE;

	return TRUE;
}
