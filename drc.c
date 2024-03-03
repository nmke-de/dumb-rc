#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/prctl.h>
#include <string.h>
#include <signal.h>
#include <dirent.h>
#include <stdlib.h>
#include "print/print.h"
#include "Itoa/itoa.h"

extern char **environ;

static int children;
static void waitless(int _) {
	--children;
	logln("drc: Child died. Children remaining: ", itoa(children, 10));
}

// Scan a directory for scripts to daemonize
int main(int argc, char **argv) {
	// Can't scan a directory null, right?
	if (argc < 2) {
		return 1;
	}

	// If parent dies, die yourself.
	prctl(PR_SET_PDEATHSIG, SIGTERM);
	// Count children
	signal(SIGCLD, waitless);

	// Change to directory with services.
	chdir(argv[1]);

	// Get dsv location
	char *dsv = getenv("DSV_LOCATION");
	if (!dsv)
		dsv = "/usr/local/bin/dsv";
	
	// Scan directory and run scripts
	logln("drc: Scanning directory ", argv[1], " now.");
	DIR *services = opendir(argv[1]);
	pid_t child;
	char path[strlen(argv[1]) + 256];
	struct stat sb;
	strncpy(path, argv[1], strlen(argv[1]));
	for (struct dirent *run = readdir(services); run != NULL; run = readdir(services)) {
		// Ignore dotfiles
		if (run->d_name[0] == '.')
			continue;

		// Concat path
		path[strlen(argv[1])] = '/';
		strncpy(path + strlen(argv[1]) + 1, run->d_name, strlen(run->d_name));
		
		// Check file eligibility
		if (stat(path, &sb) < 0)
			continue;
		if (!S_ISREG(sb.st_mode))
			continue;
		if (access(path, X_OK) != 0)
			continue;

		// Spawn child
		child = fork();
		if (child == 0)
			execve(dsv, (char **) cargs(dsv, path), environ);
		else if (child > 0) {
			++children;
			logln("drc: Started ", run->d_name, " as child #", itoa(children, 10));
		}
	}
	closedir(services);
	logln("drc: Done Scanning directory ", argv[1], ".");

	// Wait
	while (children)
		wait(NULL);

	return 0;
}
