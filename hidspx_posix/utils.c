/* utils.c
 * avrspx support routines
 * 2006-05-08 by t.k
 */
#ifndef WIN32
#include "config.h"
#endif
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#ifdef WIN32
#include <windows.h>
#else
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#ifdef MACOS
#define TTYDSCR "tty."
#endif //MACOS
#ifdef LINUX
#define TTYDSCR "ttyS"
#endif //LINUX
#endif //WIN32
#include "avrspx.h"

//----------------------------------------------------------------------------
char *progpathname;
char progpath[512];
char progname[64];
char inifilename[512];

//----------------------------------------------------------------------------
bool is_binary_str(const char *s)
{
	while (*s) {
		if (*s != '0' && *s != '1')
			return false;
		++s;
	}
	return true;
}

// default is a binary string (original compatiblity)
unsigned long strtoul_ex(const char *s, char **endptr, int base)
{
	if (base != 0 || (*s != '0' && *s != '1'))
		return strtoul(s, endptr, base);
	if (*s == '0' && *(s + 1) == 'b')	// 0b....
		return strtoul(s + 2, endptr, 2);
	return strtoul(s, endptr, is_binary_str(s) ? 2 : 0);
}

//----------------------------------------------------------------------------
int setup_commands_ex(int argc, char **argv)
{
	char *s;

	progpathname = *argv;
	strcpy(progpath, progpathname);
	s = progpath + strlen(progpath);
#ifndef WIN32	/* !WIN32 */
	while (--s >= progpath && (*s != '/'))
		;
#else
	while (--s >= progpath && (*s != ':' && *s != '\\'))
		;
#endif //!WIN32
	strcpy(progname, s + 1);
	*(s+1) = '\0';

	s = progname + strlen(progname);
	while (--s > progname && *s != '.')
		;
	if (*s == '.')
    	*s = '\0';

//    strcpy(inifilename, progpath);
    strcat(inifilename, progname);
    strcat(inifilename, ".ini");

	while (--argc) {
		s = *++argv;
		if (*s == '-' && tolower(s[1]) == 'i') {	// -i<config.ini>
			// -i option
			strcpy(inifilename, s + 2);
		}
	}
#if DEBUG
	fprintf(stderr, "prog path name [%s]\n", progpathname);
	fprintf(stderr, "prog path      [%s]\n", progpath);
	fprintf(stderr, "prog name      [%s]\n", progname);
	fprintf(stderr, "ini file name  [%s]\n", INIFILE);
#endif //DEBUG

	return argc;
}

//----------------------------------------------------------------------------
DEVPROP *search_device(char *Name)
{
	DEVPROP *dev;

	// @@@
	for (dev = (DEVPROP *)DevLst; dev->Name != NULL; dev++) {
		if (_stricmp(Name, dev->Name) == 0)
			return dev;
		if (tolower(Name[0]) == 'a' &&
			tolower(Name[1]) == 't' &&
			_stricmp(Name+2, dev->Name) == 0)
			return dev;
	}
	return dev;
//	return NULL;
}

#if 0
DEVPROP *get_device(BYTE *p)
{
	DEVPROP *dev;
	// @@@
	for (dev = (DEVPROP *)DevLst; dev->Name != 0; dev++) {
		if (memcmp(p, Device->Sign, 3) == 0)
			return dev;
	}
	return dev;
//	return NULL;
}
#endif //0

//----------------------------------------------------------------------------
#ifdef WIN32
int get_outque_count(HANDLE hComm)
{
	COMSTAT ComStat;
	DWORD dwError;

	if (ClearCommError(hComm, &dwError, &ComStat))
		return ComStat.cbOutQue;
	return -1;
}

static int check_comm(int port)
{
	char sComm[16];
	HANDLE hComm;

	sprintf(sComm, "\\\\.\\COM%u", port);
	hComm = CreateFile(sComm, GENERIC_READ|GENERIC_WRITE, 0, NULL, 
			OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	if (hComm == INVALID_HANDLE_VALUE) {
		DWORD code = GetLastError();
// 2: 指定されたファイルが見つかりません
// 5: アクセスは拒否されました
// fprintf(stderr, "Err = %d\n", code);
		if (code == 5)
			fprintf(stderr, "  COM%d busy.\n", port);
		return 0;
	}

	CloseHandle(hComm);
	fprintf(stderr, "  COM%d ready.\n", port);
	return 1;
}
void dump_port_list(void)
{
	int n;
    
	fprintf(stderr, "--- port list ---\n");
	for (n = 1; n <= 32; ++n)
		check_comm(n);
}
#else
void dump_port_list(void){
    DIR* devdir;
    struct dirent* ttyents;
    int fh;
    if((devdir = opendir("/dev"))){
        while((ttyents = readdir(devdir))){
            if(strstr(ttyents->d_name,TTYDSCR)){
                if((fh = open(ttyents->d_name,O_RDWR | O_NOCTTY|O_NONBLOCK))){
                    fprintf(stderr,"%s is ready.\n",ttyents->d_name);
                    close(fh);
                }
            }
        }
    }
    if(devdir)
        closedir(devdir);
}
#endif //WIN32

//----------------------------------------------------------------------------
