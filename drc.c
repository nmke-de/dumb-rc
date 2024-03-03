#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/prctl.h>
#include <string.h>
#include <signal.h>
#include <dirent.h>
#include <stdlib.h>
#include "print/print.h"
#include "Itoa/itoa.h"
// #define DEBUG
#include "log.h"

extern char **environ;
int logfd;

static int children;
static void waitless(int _) {
	--children;
	dlog("drc: Child died. Children remaining: ", itoa(children, 10));
}

// Scan a directory for scripts to daemonize
int main(int argc, char **argv) {
	// Can't scan a directory null, right?
	if (argc < 2) {
		println("drc - The Dumb RunCommand. By nmke-de (c) 2024 - ???
Version: 0.1
Syntax: drc [/absolute/paths/to/directories...]

drc will scan all given directories for executable
regular files and daemonize them. Upon being killed,
all children of drc should also exit.");
		return 1;
	}
	dloginit("/tmp/drc");

	// If parent dies, die yourself.
	prctl(PR_SET_PDEATHSIG, SIGTERM);
	// Count children
	signal(SIGCLD, waitless);

	// Get dsv location
	char *dsv = getenv("DSV_LOCATION");
	if (!dsv)
		dsv = "/usr/local/bin/dsv";
	
	// Scan directories and run scripts
	pid_t child;
	struct stat sb;
	for (int i = 1; i < argc; i++) {
		// Change to directory with services.
		chdir(argv[i]);
		
		// Open directory and allocate memory for path buffer.
		dlog("drc: Scanning directory ", argv[i], " now.");
		DIR *services = opendir(argv[i]);
		if (!services)
			continue;
		char path[strlen(argv[i]) + 256];
		strncpy(path, argv[i], strlen(argv[i]));

		// Loop over directory entries
		for (struct dirent *run = readdir(services); run != NULL; run = readdir(services)) {
			dlog("Currently checking ", run->d_name);

			// Ignore dotfiles
			if (run->d_name[0] == '.')
				continue;

			// Concat path
			path[strlen(argv[i])] = '/';
			strncpy(path + strlen(argv[i]) + 1, run->d_name, strlen(run->d_name));
			path[strlen(argv[i]) + strlen(run->d_name) + 1] = 0;
			dlog("Full path: ", path);

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
				dlog("drc: Started ", run->d_name, " as child #", itoa(children, 10));
			}
		}

		// Close directory
		closedir(services);
		dlog("drc: Done Scanning directory ", argv[i], ".");
	}

	// Wait
	while (children)
		wait(NULL);
	dlogquit();

	return 0;
}
