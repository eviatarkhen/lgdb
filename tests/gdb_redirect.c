#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <string.h>

static void usage()
{
	printf("usage: gdb_redirecrt <path_to_file>\n");
}

/**
* params:
	1. path to binary
*/
int main(int argc, char *argv[])
{
	pid_t pid;
	int outfd[2];
	int infd[2];
	char input[100];

	if (argc != 2)
	{
		usage();
		return -1;
	}

	if (pipe(infd) < 0) {
		perror("pipe: infd");
		exit(-2);
	}

	if (pipe(outfd) < 0) {
		perror("pipe: outfd");
		close(infd[0]);
		close(infd[1]);
		exit(-3);
	}

	if ((pid = fork()) < 0) {
		perror("fork");
		close(infd[0]);
		close(infd[1]);
		close(outfd[0]);
		close(outfd[1]);
		exit(-4);
	} else if (pid == 0) {     //child
		if (dup2(infd[0], STDIN_FILENO) < 0) {
			perror("dup2: infd");
			close(infd[0]);
			close(infd[1]);
			close(outfd[0]);
			close(outfd[1]);
			exit(-5);
		}

		if (dup2(outfd[1], STDOUT_FILENO) < 0) {
			perror("dup2: outfd");
			close(infd[0]);
			close(infd[1]);
			close(outfd[0]);
			close(outfd[1]);
			exit(-6);
		}

		close(infd[0]);
		close(infd[1]);
		close(outfd[0]);
		close(outfd[1]);

		if (execl("/usr/bin/gdb", "gdb", argv[1], (char *)0) < 0) {
			perror("execl");
			exit(-7);
		}
	}
	
	char c;
	close(infd[0]);
	close(outfd[1]);

	char command[10] = "b main\n";
	if (write(infd[1], command, strlen(command)) < 0) {
		perror("write");
		close(infd[1]);
		close(outfd[0]);
		exit(-8);
	}

	while (read(outfd[0], &c, 1) == 1) {
		if (write(STDOUT_FILENO, &c, 1) < 0) {
			perror("write 2");
			close(infd[1]);
			close(outfd[0]);
			exit(-9);
		}
	}

	close(infd[1]);
	close(outfd[0]);

	return 0;
}
