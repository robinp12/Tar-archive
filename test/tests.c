#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>

#include "lib_tar.h"

/**
 * You are free to use this file to write tests for your implementation
 */

void debug_dump(const uint8_t *bytes, size_t len) {
    for (int i = 0; i < len;) {
        printf("%04x:  ", (int) i);

        for (int j = 0; j < 16 && i + j < len; j++) {
            printf("%02x ", bytes[i + j]);
        }
        printf("\t");
        for (int j = 0; j < 16 && i < len; j++, i++) {
            printf("%c ", bytes[i]);
        }
        printf("\n");
    }
}

int main(int argc, char **argv) {
    //uint8_t dest;
    //size_t len = 512;

    if (argc < 2) {
        printf("Usage: %s tar_file\n", argv[0]);
        return -1;
    }

    int fd = open(argv[1] , O_RDONLY);
    if (fd == -1) {
        perror("open(tar_file)");
        return -1;
    }

    /*int ret = check_archive(fd);
    printf("check_archive returned %d\n", ret);

    ret = is_dir(fd,(char*)'/');
    printf("is_dir returned %d\n", ret);

    ret = is_file(fd,(char*)'/');
    printf("is_file returned %d\n", ret);

    ret = is_symlink(fd,(char*)'/');
    printf("is_symlink returned %d\n", ret);

    int ret = exists(fd, "lib_tar.c");
    printf("exists returned %d\n", ret);

    ret = is_dir(fd, "testing/");
    printf("is_dir returned %d\n", ret);

    ret = is_file(fd, "Makefile");
    printf("is_file returned %d\n", ret);*/
    size_t no_entries = 50;
    char **entries = (char**) malloc(no_entries);

    int ret = list(fd,argv[1],entries,&no_entries);
    printf("list returned %d\n", ret);

//    int ret = read_file(fd, "lib_tar.c", 50, &dest, &len);
  //  printf("read_file returned %d\n", ret);

    return 0;
}