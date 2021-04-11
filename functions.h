#define _XOPEN_SOURCE 700
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <ftw.h>
#include <time.h>

#define MAX_SIZE (1024)
#define SEC_IN_HOUR (3600)
#define SEC_IN_DAY (24*SEC_IN_HOUR)

char* ext;
time_t now;

void o_assert(int cond, const char *msg){
    if (!cond) {
        perror(msg);
        fprintf(stderr, "%s:%d\n", __FILE__, __LINE__);
        exit(EXIT_FAILURE);
    }
}

int catfile(int argc, char** argv){
	o_assert(argc == 2, "usage: cat path_to_file");
	
	int fd = open(argv[1], O_RDONLY);
	o_assert(fd != -1, "open failed");
	
	char buffer[MAX_SIZE];
	
	int bytesRead = 0;
	while ((bytesRead = read(fd, buffer, MAX_SIZE)) > 0){
		o_assert(write(STDOUT_FILENO, buffer, bytesRead) != -1, "write failed");
	}
	
	o_assert(bytesRead != -1, "read");
	
	close(fd);
    return 1;
}

int cpfile(char *argv[]) {
    int input_fd = open(argv[1], O_RDONLY);
	o_assert(input_fd != -1, "open source file failed");
	int output_fd = open(argv[2], O_WRONLY|O_TRUNC|O_CREAT, 0644);
	o_assert(output_fd != -1, "open destination file failed");
	
	char* buffer = (char*)malloc(MAX_SIZE * sizeof(char));
	o_assert(buffer != NULL, "memory allocation failed");
	
	int bytes_read = 0;
	while ((bytes_read = read(input_fd, buffer, MAX_SIZE)) > 0) {
		
		o_assert(write(output_fd, buffer, bytes_read) != -1, "write");
	}
	
	o_assert(bytes_read != -1, "read");
	
	free(buffer);
	close(input_fd);
	close(output_fd);
	
    return 1;
}

int mkdir_f(char* argv[]) {
	o_assert(mkdir(argv[1], 0766) != -1, "usage: mkdir name");
	
    return 1;
}

int rmdir_f(char* argv[]) {
    o_assert(rmdir(argv[1]) >= 0, "removing directory failed");

    return 1;
}

int chmod_f(char* argv[]) {
    mode_t access_rights = strtol(argv[2], NULL, 8);
	
	int fd = open(argv[1], O_RDONLY | O_CREAT, access_rights);
	o_assert(fd != -1, "usage: chmod path access_rights");
	close(fd);
	
	o_assert(chmod(argv[1], access_rights) != -1, "chmod failed");
	
    return 1;
}

int process_file_ext(const char* fpath, const struct stat* sb, int typeflag, struct FTW* ftwbuf){
	if (typeflag == FTW_F) {
		
		char* extension = strrchr(fpath + ftwbuf->base, '.');
		
		if (extension != NULL) {
			if (strcmp(extension + 1, ext) == 0)
				printf("%s\n", fpath + ftwbuf->base);
		}
	}
	
	return 0;
}

int filter_by_ext(char* argv[]) {
	struct stat fInfo;
	o_assert(stat(argv[1], &fInfo) != -1, "usage: filter_by_ext path_to_dir extension, stat failed");
	o_assert(S_ISDIR(fInfo.st_mode), "path does not point to directory");
	
	ext = argv[2];
	
	o_assert(nftw(argv[1], process_file_ext, 50, 0) != -1, "nftw failed");
	
    return 1;
}


int process_file_time(const char* fpath, const struct stat* sb, int typeflag, struct FTW* ftwbuf){
	if (typeflag == FTW_F) {
		time_t timeDiff = (now - sb->st_mtime)/(SEC_IN_DAY);
		if (timeDiff < 7) {
			printf("%s\n", fpath + ftwbuf->base);
		}
	}
	
	return 0;
}

int filter_by_time(char* argv[]) {
	struct stat fInfo;
	o_assert(stat(argv[1], &fInfo) != -1, "usage: filter_by_time path_to_dir, stat failed");
	o_assert(S_ISDIR(fInfo.st_mode), "path does not point to directory");
	
	time(&now);
	
	o_assert(nftw(argv[1], process_file_time, 50, 0) != -1, "nftw failed");
	
    return 1;
}