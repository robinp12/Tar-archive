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

    off_t position=lseek(tar_fd, 0, SEEK_SET);
    if(position==-1){
        printf("position failed\n");
        return -1;
    }
    header = malloc(sizeof(tar_header_t));

    int n=512;
    int nb=0;

    while((read(tar_fd, header,HEADER_SIZE)) > 0){
        int c_chksum = 0;
        int nbr_blocs=0;

        int i=0;
        while (i<HEADER_SIZE){
            char* c = (char*) header+i;
            //checksum
            if(i<148 || i>155){ 
                c_chksum += *c;
            }
            else{
                c_chksum += ' ';
            }
            i++;
        }
        if(c_chksum==256){
            break;
        }

        if(strcmp(header->magic,TMAGIC)!=0){
            free(header);
            return -1;
        }
        if(TAR_INT(header->version) != TAR_INT(TVERSION)){
            free(header);
            return -2;
        }
        if(TAR_INT(header->chksum)!= c_chksum){
            free(header);
            return -3;
        }
        nb++;
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
    }

    free(header);
    return nb;
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
    while((read(tar_fd, header,HEADER_SIZE)) > 0){
        int nbr_blocs=0;

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
            free(header);
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
    off_t position=lseek(tar_fd, 0, SEEK_SET);
    if(position==-1){
        printf("position failed\n");
        return -1;
    }
    header = malloc(sizeof(tar_header_t));
    int n=512;
    // Lecture de tout les blocs du fichier TAR
    while((read(tar_fd, header,HEADER_SIZE)) > 0){
        int nbr_blocs=0;

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
        if(strcmp(header->name,path)==0 && header->typeflag==DIRTYPE){
            free(header);
            return *path;
        }
    }
    
    free(header);
    return 0;
    
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
    off_t position=lseek(tar_fd, 0, SEEK_SET);
    if(position==-1){
        printf("position failed\n");
        return -1;
    }
    header = malloc(sizeof(tar_header_t));
    int n=512;
    // Lecture de tout les blocs du fichier TAR
    while((read(tar_fd, header,HEADER_SIZE)) > 0){
        int nbr_blocs=0;

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
        if(strcmp(header->name,path)==0 && (header->typeflag==REGTYPE || header->typeflag==AREGTYPE)){
            free(header);
            return 1;
        }
    }
    
    free(header);
    return 0;
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
    
     off_t position=lseek(tar_fd, 0, SEEK_SET);
    if(position==-1){
        printf("position failed\n");
        return -1;
    }
    header = malloc(sizeof(tar_header_t));
    int n=512; 
    // Lecture de tout les blocs du fichier TAR
    while((read(tar_fd, header,HEADER_SIZE)) > 0){
        int nbr_blocs=0;

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
        if(strcmp(header->name,path)==0 && header->typeflag==SYMTYPE){
            free(header);
            return 1;
        }
    }
    
    free(header);
    return 0;
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

    int index = 0;
    char *slash = "/\0";


    // Si c'est pas un dossier on return 0
    if(is_dir(tar_fd,path)==0) {
        *no_entries = index;
        return 0;
    }
    int sym = is_symlink(tar_fd,path);

    header = malloc(sizeof(tar_header_t));
    
    // Si c'est un lien symbolic, remplacer le chemin par celui du linkname
    if(sym!=0){
        lseek(tar_fd, sym*HEADER_SIZE, SEEK_SET);
        read(tar_fd,header,HEADER_SIZE);
        path = header->linkname;
        strcat(path,slash);
        printf("oui %s\n",path);
    }

    lseek(tar_fd, 0, SEEK_SET);

    int n=HEADER_SIZE;
    int size;

    //Iteration à travers tous les fichiers de l'archive
    while(read(tar_fd,header, HEADER_SIZE)>0){

        // Taille de chacun des headers
        size=TAR_INT(header->size);
        int nbr_blocs=0;

        // Prendre seulement les fichiers avec un nom plus grand que 0 dans l'archive
        //(Ne prendre que les fichiers avec un nom)
        if(strlen(header->name)>0){
            // Ne prend pas en compte la premiere entrée (qui est le dossier lui meme [car il termine par "/"])
            if(header->name[strlen(header->name)-1] != '/'){
                // Retourner la liste des fichiers dans l'argument entries (pour y acceder en dehors ex: dans tests.c)
                strcpy(entries[index],header->name);
                // Iterer l'index en fonction du nombre de fichier dans le dossier
                index++;
            }
        }

        // Calcul du nombre de bloc à passer si size n'est pas égale à 0 et que du coup, il y a des blocs de data
        if (*(header->size)!=0){
            float number_blocs= size/512.00;
            if(number_blocs != (int) number_blocs){
                nbr_blocs=(int) number_blocs +1;
            } else {
                nbr_blocs=(int) number_blocs;
            }
            
            n=512*nbr_blocs;

            lseek(tar_fd, n, SEEK_CUR);
        }
    }
    // Mettre le nombre de fichier dans l'argument pour y acceder en dehors
    *no_entries=index;
    lseek(tar_fd,0,SEEK_SET);

    free(header);
    // Retourne le nombre de fichier listé
    return *no_entries;
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

    if(is_file(tar_fd, path)==0){
        return -1;
    }
    
    lseek(tar_fd, 0, SEEK_SET);
   
    header = malloc(sizeof(tar_header_t));

    int size;
    int n=512;

    // Lecture de tout les blocs du fichier TAR
    while((read(tar_fd, header,HEADER_SIZE)) > 0){
        int nbr_blocs=0;

        if(strcmp(header->name,path)==0 ) {
            printf("Path: %s\nName: %s\nSize of the file: %d\n\n", path, header->name, (int) TAR_INT(header->size));
            break;
        }
        // Calcul du nombre de bloc à passer si size n'est pas égale à 0 et que du coup, il y a des blocs de data
        if (*(header->size)!=0)
        {
            size = TAR_INT(header->size);
            float number_blocs= size/512.00;
            if(number_blocs != (int) number_blocs){
                nbr_blocs=(int) number_blocs +1;
            } else {
                nbr_blocs=(int) number_blocs;
            }
            
            n=512*nbr_blocs;

            lseek(tar_fd, n, SEEK_CUR);
        }
    }

    size = TAR_INT(header->size);
    if(size<=offset) return -2;
    lseek(tar_fd,offset,SEEK_CUR);

    int last = size - offset;
    int ret = last-*len;

    if(*len>last) {
        *len = last;
        ret = 0;
    }
    printf("bytes restant: %d\n offset: %d\n length: %ld\n", ret, last,*len);
   
    read(tar_fd,dest,*len);

    free(header);
    return ret;
}