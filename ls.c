#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <errno.h>
#include <string.h>
#include <pwd.h>
#include <grp.h>
#include <stdlib.h>
#include <time.h>

int maximum(int a, int b) {
	if (a < b) {
		return b;
	} else {
		return a;
	}
}

int startsWith(char *a, char *b) {
	if (strncmp(a, b, strlen(b)) == 0) return 1;
	return 0;
}

int isAFolder(struct stat stats) {
	mode_t perm = stats.st_mode;
	int toReturn = 0;
	toReturn = (perm & S_IFDIR) ? 1 : 0;
	return toReturn;
}

char* permissions(struct stat stats) {
	char *values = malloc(sizeof(char) * 10 + 1);
	mode_t perm = stats.st_mode;
	values[0] = (perm & S_IFDIR) ? 'd' : '-';
	values[1] = (perm & S_IRUSR) ? 'r' : '-';
	values[2] = (perm & S_IWUSR) ? 'w' : '-';
	values[3] = (perm & S_IXUSR) ? 'x' : '-';
	values[4] = (perm & S_IRGRP) ? 'r' : '-';
	values[5] = (perm & S_IWGRP) ? 'w' : '-';
	values[6] = (perm & S_IXGRP) ? 'x' : '-';
	values[7] = (perm & S_IROTH) ? 'r' : '-';
	values[8] = (perm & S_IWOTH) ? 'w' : '-';
	values[9] = (perm & S_IXOTH) ? 'x' : '-';
	values[10] = '\0';
	return values;
}

void listFolder(char* currDir) {
	DIR *pDIR;
	struct dirent *pDirEnt;

	pDIR = opendir(currDir);

	if (pDIR == NULL ) {
		fprintf(stderr, "%s %d: opendir() failed (%s)\n", __FILE__, __LINE__, strerror(errno));
		exit( -1 );
	}

	pDirEnt = readdir( pDIR );

	int permissionsLength = 0, userLength = 0, groupLength = 0, sizeLength = 0, dateLength = 0, filenameLength = 0, blocksize = 0;

	while ( pDirEnt != NULL) {
		if (startsWith(pDirEnt->d_name, ".")) {
			pDirEnt = readdir( pDIR );
			continue;
		}

		struct stat stats;
		char filePath[4096];

		strcpy(filePath, currDir);
		strcat(filePath, "/");
		strcat(filePath, pDirEnt->d_name);

		stat(filePath, &stats);

		if (isAFolder(stats) == 1) {
			listFolder(filePath);
		}

		blocksize += (int) stats.st_blocks;

		struct passwd *pw = getpwuid(stats.st_uid);
		struct group *gr = getgrgid(stats.st_gid);

		char date[30];
		struct tm *time;
		time_t t = (time_t) stats.st_mtim.tv_sec;
		time = localtime(&t);
		strftime(date, sizeof(date), "%b %d %H:%M", time);

		char sizeText[20];
		sprintf(sizeText, "%d", stats.st_size);
		permissionsLength = maximum(permissionsLength, strlen(permissions(stats)));
		userLength = maximum(userLength, strlen(pw->pw_name));
		groupLength = maximum(groupLength, strlen(gr->gr_name));
		sizeLength = maximum(sizeLength, strlen(sizeText));
		filenameLength = maximum(filenameLength, strlen(pDirEnt->d_name));

		pDirEnt = readdir( pDIR );
	}

	closedir(pDIR);

	pDIR = opendir(currDir);

	pDirEnt = readdir(pDIR);

	blocksize = blocksize / 2;

	printf("%s:\n", currDir);
	printf("total %d\n", blocksize);

	while ( pDirEnt != NULL) {
		if (startsWith(pDirEnt->d_name, ".")) {
			pDirEnt = readdir( pDIR );
			continue;
		}

		struct stat stats;
		char filePath[4096];

		strcpy(filePath, currDir);
		strcat(filePath, "/");
		strcat(filePath, pDirEnt->d_name);

		stat(filePath, &stats);

		struct passwd *pw = getpwuid(stats.st_uid);
		struct group *gr = getgrgid(stats.st_gid);

		char date[30];
		struct tm *time;
		time_t t = (time_t) stats.st_mtim.tv_sec;
		time = localtime(&t);
		strftime(date, sizeof(date), "%b %d %H:%M", time);

		printf("%*s %*s %*s %*d %*s %*s\n", permissionsLength, permissions(stats), userLength, pw->pw_name, groupLength, gr->gr_name, sizeLength, stats.st_size, dateLength, date, filenameLength, pDirEnt->d_name);

		pDirEnt = readdir( pDIR );
	}

	closedir( pDIR );
	printf("\n");
}

int main( int argc, char *argv[] ) {
	char* currDir = ".";

	listFolder(currDir);
	
	return 0;
}
