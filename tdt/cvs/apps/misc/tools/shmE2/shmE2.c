#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>
#include "shmE2.h"

main(int argc, char *argv[])
{
	if(argc != 3 || (strcmp(argv[1], "set") != 0 && strcmp(argv[1], "del") != 0 && strcmp(argv[1], "get") != 0 && strcmp(argv[1], "check") != 0 && strcmp(argv[1], "create") != 0 && strcmp(argv[1], "inc") != 0 && strcmp(argv[1], "dec") != 0)) {
		printf("Usage: %s create shm\n", argv[0]);
		printf("Usage: %s set variable=value\n", argv[0]);
		printf("Usage: %s del all\n", argv[0]);
		printf("Usage: %s del variable\n", argv[0]);
		printf("Usage: %s get all\n", argv[0]);
		printf("Usage: %s get variable\n", argv[0]);
		printf("Usage: %s inc variable\n", argv[0]);
		printf("Usage: %s dec variable\n", argv[0]);
		printf("Usage: %s check variable\n", argv[0]);
		exit(0);
	}

	char *shm = NULL;
	
	if(strcmp(argv[1], "create") == 0 && strcmp(argv[2], "shm") == 0) {
		shm = createshm();
		if(shm == NULL) {
			perror("createshm");
			exit(1);
		}
		printf("shared mem created\n");
		exit(0);
	}

	shm = findshm();
	if(shm == NULL) {
		perror("findshm");
		exit(1);
	}
	
	if(strcmp(argv[1], "inc") == 0 && strcmp(argv[2], "all") != 0) {
		strcat(argv[2], "=");
		if(incshmentry(shm, argv[2]) != 1) {
			printf("error: value not set\n");
			exit(1);
		}
		exit(0);
	}
	
	if(strcmp(argv[1], "dec") == 0 && strcmp(argv[2], "all") != 0) {
		strcat(argv[2], "=");
		if(decshmentry(shm, argv[2]) != 1) {
			printf("error: value not set\n");
			exit(1);
		}
		exit(0);
	}

	if(strcmp(argv[1], "get") == 0 && strcmp(argv[2], "all") != 0) {
		char *shmbuf = NULL;
		shmbuf = malloc(256);
		strcat(argv[2], "=");
		getshmentry(shm, argv[2], shmbuf, 256);
		printf("%s\n", shmbuf);
		free(shmbuf);
		exit(0);
	}

	if(strcmp(argv[1], "get") == 0 && strcmp(argv[2], "all") == 0) {
		char *shmbuf = NULL;
		shmbuf = malloc(4096);
		getshmentryall(shm, shmbuf, 4096);
		printf("%s\n", shmbuf);
		free(shmbuf);
		exit(0);
	}

	if(strcmp(argv[1], "set") == 0) {
		if(setshmentry(shm, argv[2]) != 1) {
			printf("error: value not set\n");
			exit(1);
		}
		exit(0);
	}

	if(strcmp(argv[1], "del") == 0 && strcmp(argv[2], "all") != 0) {
		strcat(argv[2], "=");
		delshmentry(shm, argv[2]);
		exit(0);
	}

	if(strcmp(argv[1], "del") == 0 && strcmp(argv[2], "all") == 0) {
		*shm = '\0';
		exit(0);
	}

	if(strcmp(argv[1], "check") == 0) {
		strcat(argv[2], "=");
		if(checkshmentry(shm, argv[2]) == 0)
			printf("not exist\n");
		else
			printf("exist\n");
		exit(0);
	}
	
	exit(0);
}
