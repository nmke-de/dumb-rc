#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/prctl.h>
#include <string.h>
#include <signal.h>
#include <dirent.h>
#include "print/print.h"

extern char **environ;
#define sys(args) execve(*(args), (args), environ)

/*
// Kill child process and stop supervising.
pid_t child;
char *childname;
int running = 1;
void fine(int signum) {
	logln("Killing ", childname);
	kill(child, SIGTERM);
	running = 0;
}*/

static int children;
static void waitless(int _) {
	logln("drc: Child died.");
	--children;
}

// Scan a directory for scripts to daemonize
int main(int argc, char **argv) {
	// Can't scan a directory null, right?
	if (argc < 2) {
		return 1;
	}

	// If parent dies, die yourself.
	prctl(PR_SET_PDEATHSIG, SIGTERM);

	// Change to directory with services.
	chdir(argv[1]);

	/*
	// Check whether specified executable even exists (and whether it is a regular executable file)
	struct stat sb;
	if (stat(argv[1], &sb) < 0) {
		logln("drc: Directory ", argv[1], " not Found.");
		return 2;
	}
	if (!S_ISDIR(sb.st_mode)) {
		logln("drc: File ", argv[1], " is not a regular file.");
		return 3;
	}
	if (access(argv[1], X_OK) != 0) {
		logln("drc: Directory ", argv[1], " is not executable.");
		return 4;
	}
	*/

	// Scan directory and run scripts
	logln("drc: Scanning directory ", argv[1], " now.");
	DIR *services = opendir(argv[1]);
	pid_t child;
	char path[strlen(argv[1]) + 256];
	strncpy(path, argv[1], strlen(argv[1]));
	for (struct dirent *run = readdir(services); run != NULL; run = readdir(services)) {
		path[strlen(argv[1])] = '/';
		strncpy(path + strlen(argv[1]) + 1, run->d_name, strlen(run->d_name));
		child = fork();
		if (child == 0)
			execve("/usr/local/bin/dsv", (char **) cargs("/usr/local/bin/dsv", path), environ);
		else if (child > 0) {
			++children;
			logln("drc: Started ", run->d_name);
		}
	}
	closedir(services);
	logln("drc: Done Scanning directory ", argv[1], ".");

	// Wait
	signal(SIGCLD, waitless);
	while (children)
		wait(NULL);

	return 0;
}
