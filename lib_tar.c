#include "lib_tar.h"
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define HEADER_SIZE 512
char c[HEADER_SIZE];

tar_header_t *header;

/**
 * Checks whether the archive is valid.
 *
 * Each non-null header of a valid archive has:
 *  - a magic value of "ustar" and a null,
 *  - a version value of "00" and no null,
 *  - a correct checksum
 *
 * @param tar_fd A file descriptor pointing to the start of a file supposed to contain a tar archive.
 *
 * @return a zero or positive value if the archive is valid, representing the number of non-null headers in the archive,
 *         -1 if the archive contains a header with an invalid magic value,
 *         -2 if the archive contains a header with an invalid version value,
 *         -3 if the archive contains a header with an invalid checksum value
 */
int check_archive(int tar_fd) {

    header = malloc(sizeof(tar_header_t));

    char chksum[8];  
    char magic[TMAGLEN];
    char version[TVERSLEN];

    int c_chksum = 0;

    read(tar_fd, &c, HEADER_SIZE);
    
    int i=0;
    int a=0;
    int b=0;
    while (i<HEADER_SIZE){
        //checksum
        if(i>=148 && i<156){
            chksum[a] = c[i];
            a++;
        }
        if(i<148 || i>155){ 
            c_chksum += c[i];
		}
        else{
            c_chksum += ' ';
        }
        //magic
        if(i>=257 && i<263){
            magic[b] = c[i];
            b++;
            a=0;
        }
        //version
        if(263<=i && i<265){
            version[a] = c[i];
            a++;
        }
        i++;
    }

    memcpy(&header->magic,magic,TMAGLEN);      
    memcpy(&header->version,version,TVERSLEN);      
    memcpy(&header->chksum,chksum,8);      


    printf("magic: %s\n",header->magic);
    printf("version: %s\n",header->version);
    printf("checksum: %ld\n",TAR_INT(chksum));

    if(strcmp(header->magic,TMAGIC)!=0){
        free(header);
        return -1;
    }
    if(strcmp(header->version,TVERSION)!=0){
        free(header);
        return -2;
    }
    if(TAR_INT(chksum)!= c_chksum){
        free(header);
        return -3;
    }

    free(header);
    return 0;
}

/**
 * Checks whether an entry exists in the archive.
 *
 * @param tar_fd A file descriptor pointing to the start of a valid tar archive file.
 * @param path A path to an entry in the archive.
 *
 * @return zero if no entry at the given path exists in the archive,
 *         any other value otherwise.
 */
int exists(int tar_fd, char *path) {
    off_t position=lseek(tar_fd, 0, SEEK_SET);
    if(position==-1){
        printf("position failed\n");
        return -1;
    }
    header = malloc(sizeof(tar_header_t));
    int n=512;
    // Lecture de tout les blocs du fichier TAR
    while((read(tar_fd, &c,HEADER_SIZE)) > 0){
        int i= 0;
        int sizing=0;
        int nbr_blocs=0;
        while(i<200){
            if(i>=0 && i<100){
                //printf("i NAME== %d\n", c[i]);
                header->name[i]=c[i];
            } else if (i>=124 && i<136){
                //printf("i SIZE== %d\n", c[i]);
                header->size[sizing]=c[i];
                sizing++;
            }
            i++;
        }

        // Calcul du nombre de bloc à passer si size n'est pas égale à 0 et que du coup, il y a des blocs de data
        if (*(header->size)!=0)
        {
            int siz = TAR_INT(header->size);
            float number_blocs= siz/512.00;
            if(number_blocs != (int) number_blocs){
                nbr_blocs=(int) number_blocs +1;
            } else {
                nbr_blocs=(int) number_blocs;
            }
            
            n=512*nbr_blocs;

            position=lseek(tar_fd, n, SEEK_CUR);
            if(position==-1){
                printf("position failed\n");
                return -1;
            }
        }
        
        //Comparaison entre notre nom de fichier et le nom qu'on cherche
        if(strcmp(header->name,path)==0){
            return *path;
        }
    }
    
    free(header);
    return 0;
}

/**
 * Checks whether an entry exists in the archive and is a directory.
 *
 * @param tar_fd A file descriptor pointing to the start of a valid tar archive file.
 * @param path A path to an entry in the archive.
 *
 * @return zero if no entry at the given path exists in the archive or the entry is not a directory,
 *         any other value otherwise.
 */
int is_dir(int tar_fd, char *path) {

    header = malloc(sizeof(tar_header_t));

    memcpy(&header->typeflag,&c[156],1);

    if(header->typeflag != DIRTYPE){
        free(header);
        return 0;
    }
    else{
        free(header);
        return 1;
    }
    
}

/**
 * Checks whether an entry exists in the archive and is a file.
 *
 * @param tar_fd A file descriptor pointing to the start of a valid tar archive file.
 * @param path A path to an entry in the archive.
 *
 * @return zero if no entry at the given path exists in the archive or the entry is not a file,
 *         any other value otherwise.
 */
int is_file(int tar_fd, char *path) {
    
    header = malloc(sizeof(tar_header_t));

    memcpy(&header->typeflag,&c[156],1);

    if(header->typeflag != REGTYPE || header->typeflag != AREGTYPE){
        free(header);
        return 0;
    }
    else{
        free(header);
        return 1;
    }
}

/**
 * Checks whether an entry exists in the archive and is a symlink.
 *
 * @param tar_fd A file descriptor pointing to the start of a valid tar archive file.
 * @param path A path to an entry in the archive.
 * @return zero if no entry at the given path exists in the archive or the entry is not symlink,
 *         any other value otherwise.
 */
int is_symlink(int tar_fd, char *path) {
    
    header = malloc(sizeof(tar_header_t));

    memcpy(&header->typeflag,&c[156],1);

    if(header->typeflag != SYMTYPE){
        free(header);
        return 0;
    }
    else{
        free(header);
        return 1;
    }
}


/**
 * Lists the entries at a given path in the archive.
 * list() does not recurse into the directories listed at the given path.
 *
 * Example:
 *  dir/          list(..., "dir/", ...) lists "dir/a", "dir/b", "dir/c/" and "dir/e/"
 *   ├── a
 *   ├── b
 *   ├── c/
 *   │   └── d
 *   └── e/
 *
 * @param tar_fd A file descriptor pointing to the start of a valid tar archive file.
 * @param path A path to an entry in the archive. If the entry is a symlink, it must be resolved to its linked-to entry.
 * @param entries An array of char arrays, each one is long enough to contain a tar entry path.
 * @param no_entries An in-out argument.
 *                   The caller set it to the number of entries in `entries`.
 *                   The callee set it to the number of entries listed.
 *
 * @return zero if no directory at the given path exists in the archive,
 *         any other value otherwise.
 */
int list(int tar_fd, char *path, char **entries, size_t *no_entries) {
    return 0;
}

/**
 * Reads a file at a given path in the archive.
 *
 * @param tar_fd A file descriptor pointing to the start of a valid tar archive file.
 * @param path A path to an entry in the archive to read from.  If the entry is a symlink, it must be resolved to its linked-to entry.
 * @param offset An offset in the file from which to start reading from, zero indicates the start of the file.
 * @param dest A destination buffer to read the given file into.
 * @param len An in-out argument.
 *            The caller set it to the size of dest.
 *            The callee set it to the number of bytes written to dest.
 *
 * @return -1 if no entry at the given path exists in the archive or the entry is not a file,
 *         -2 if the offset is outside the file total length,
 *         zero if the file was read in its entirety into the destination buffer,
 *         a positive value if the file was partially read, representing the remaining bytes left to be read to reach
 *         the end of the file.
 *
 */
ssize_t read_file(int tar_fd, char *path, size_t offset, uint8_t *dest, size_t *len) {
    return 0;
}