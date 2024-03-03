// Logger
#ifndef LOG_H
#define LOG_H
#ifdef DEBUG
	#include <unistd.h>
	#include <fcntl.h>
	#include "print/print.h"
	extern int logfd;
	#define dloginit(logfile) logfd = open((logfile), O_CREAT | O_TRUNC | O_WRONLY, 0644)
	#define dlog(...) fdprintv(logfd, cargs(__VA_ARGS__, "\n"))
	#define dlogquit() close(logfd)
#else
	#define dloginit(logfile)
	#define dlog(...)
	#define dlogquit()
#endif
#endif
